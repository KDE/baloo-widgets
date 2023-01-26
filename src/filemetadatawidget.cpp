/*
    SPDX-FileCopyrightText: 2012-2013 Vishesh Handa <me@vhanda.in>

    Adapted from KFileMetadataWidget
    SPDX-FileCopyrightText: 2008 Sebastian Trueg <trueg@kde.org>
    SPDX-FileCopyrightText: 2009-2010 Peter Penz <peter.penz@gmx.at>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "filemetadatawidget.h"
#include "filemetadataprovider.h"
#include "filemetadatawidget_p.h"
#include "metadatafilter.h"
#include "widgetfactory.h"

#include <KConfig>
#include <KConfigGroup>

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QList>
#include <QSpacerItem>
#include <QString>

using namespace Baloo;

FileMetaDataWidgetPrivate::FileMetaDataWidgetPrivate(FileMetaDataWidget *parent)
    : m_rows()
    , q(parent)
{
    m_filter = new MetadataFilter(q);

    m_widgetFactory = new WidgetFactory(q);
    QObject::connect(m_widgetFactory, &WidgetFactory::urlActivated, q, &FileMetaDataWidget::urlActivated);

    // TODO: If KFileMetaDataProvider might get a public class in future KDE releases,
    // the following code should be moved into KFileMetaDataWidget::setModel():
    m_provider = new FileMetaDataProvider(q);
    QObject::connect(m_provider, &FileMetaDataProvider::loadingFinished, q, [this]() {
        slotLoadingFinished();
    });
}

FileMetaDataWidgetPrivate::~FileMetaDataWidgetPrivate() = default;

void FileMetaDataWidgetPrivate::deleteRows()
{
    for (const Row &row : std::as_const(m_rows)) {
        delete row.label;
        row.value->deleteLater();
        if (row.checkBox) {
            row.checkBox->deleteLater();
        }
    }

    m_rows.clear();
}

QLabel *FileMetaDataWidgetPrivate::createLabel(const QString &key, const QString &itemLabel, FileMetaDataWidget *parent)
{
    auto label = new QLabel(itemLabel + QLatin1Char(':'), parent);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    label->setForegroundRole(parent->foregroundRole());
    label->setFont(parent->font());
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignTop | Qt::AlignRight);
    label->setObjectName(QStringLiteral("L_%1").arg(key));
    return label;
}

void FileMetaDataWidgetPrivate::slotLoadingFinished()
{
    deleteRows();

    if (m_gridLayout == nullptr) {
        m_gridLayout = new QGridLayout(q);
        m_gridLayout->setContentsMargins(0, 0, 0, 0);
        m_gridLayout->setSpacing(q->fontMetrics().height() / 4);
    }

    QVariantMap data = m_provider->data();
    QStringList active;
    if (m_configureVisibleProperties) {
        active = m_filter->filter(data).keys();
        auto changedIt = m_visibilityChanged.constBegin();
        while (changedIt != m_visibilityChanged.constEnd()) {
            if (changedIt.value()) {
                active.append(changedIt.key());
            } else {
                active.removeAll(changedIt.key());
            }
            changedIt++;
        }
        m_widgetFactory->setReadOnly(true);
        m_gridLayout->setColumnStretch(0, 1);
        m_gridLayout->setColumnStretch(1, 3);
        m_gridLayout->setColumnStretch(2, 0);
        m_gridLayout->setColumnStretch(3, 6);
    } else {
        data = m_filter->filter(data);
        m_widgetFactory->setReadOnly(m_provider->isReadOnly());
        m_gridLayout->setColumnStretch(0, 4);
        m_gridLayout->setColumnStretch(1, 0);
        m_gridLayout->setColumnStretch(2, 6);
        m_gridLayout->setColumnStretch(3, 0);
    }

    int rowIndex = 0;
    // Iterate through all remaining items.
    // Embed the label and the value as new row in the widget
    const QStringList keys = sortedKeys(data);
    const int spacerWidth = QFontMetrics(q->font()).size(Qt::TextSingleLine, QStringLiteral(" ")).width();

    const int labelColumn = m_configureVisibleProperties ? 1 : 0;

    for (const auto &key : keys) {
        Row row;
        if (m_configureVisibleProperties) {
            row.checkBox = new QCheckBox(q);
            if (active.contains(key)) {
                row.checkBox->setChecked(true);
            }
            m_gridLayout->addWidget(row.checkBox, rowIndex, 0, Qt::AlignTop | Qt::AlignRight);
            QObject::connect(row.checkBox, &QCheckBox::stateChanged, q, [this, key](int state) {
                this->m_visibilityChanged[key] = (state == Qt::Checked);
            });
        } else {
            row.checkBox = nullptr;
        }

        row.label = createLabel(key, m_provider->label(key), q);
        m_gridLayout->addWidget(row.label, rowIndex, labelColumn + 0, Qt::AlignRight);

        m_gridLayout->addItem(new QSpacerItem(spacerWidth, 1), rowIndex, labelColumn + 1);

        row.value = m_widgetFactory->createWidget(key, data[key], q);
        m_gridLayout->addWidget(row.value, rowIndex, labelColumn + 2, Qt::AlignLeft);

        m_gridLayout->setRowStretch(rowIndex, 0);

        // Remember the label and value-widget as row
        m_rows.append(row);
        ++rowIndex;
    }

    // Add vertical stretch - when the widget is embedded with extra vertical
    // space, it should be added at the bottom, not distributed between the
    // items.
    m_gridLayout->addItem(new QSpacerItem(0, 0), rowIndex, 0, 1, -1);
    m_gridLayout->setRowStretch(rowIndex, 1);

    q->updateGeometry();
    Q_EMIT q->metaDataRequestFinished(m_provider->items());
}

void FileMetaDataWidgetPrivate::slotLinkActivated(const QString &link)
{
    const QUrl url = QUrl::fromUserInput(link);
    if (url.isValid()) {
        Q_EMIT q->urlActivated(url);
    }
}

void FileMetaDataWidgetPrivate::slotDataChangeStarted()
{
    q->setEnabled(false);
}

void FileMetaDataWidgetPrivate::slotDataChangeFinished()
{
    q->setEnabled(true);
}

QStringList FileMetaDataWidgetPrivate::sortedKeys(const QVariantMap &data) const
{
    // Create a map, where the translated label prefixed with the
    // sort priority acts as key. The data of each entry is the URI
    // of the data. By this the all URIs are sorted by the sort priority
    // and sub sorted by the translated labels.
    QMap<QString, QString> map;
    QVariantMap::const_iterator hashIt = data.constBegin();
    while (hashIt != data.constEnd()) {
        const QString propName = hashIt.key();

        QString key = m_provider->group(propName);
        key += m_provider->label(propName);

        map.insert(key, propName);
        ++hashIt;
    }

    // Apply the URIs from the map to the list that will get returned.
    // The list will then be alphabetically ordered by the translated labels of the URIs.
    QStringList list;
    QMap<QString, QString>::const_iterator mapIt = map.constBegin();
    while (mapIt != map.constEnd()) {
        list.append(mapIt.value());
        ++mapIt;
    }

    return list;
}

void FileMetaDataWidgetPrivate::saveConfig()
{
    if (m_visibilityChanged.isEmpty()) {
        return;
    }

    KConfig config(QStringLiteral("baloofileinformationrc"), KConfig::NoGlobals);
    KConfigGroup showGroup = config.group("Show");

    auto changedIt = m_visibilityChanged.constBegin();
    while (changedIt != m_visibilityChanged.constEnd()) {
        showGroup.writeEntry(changedIt.key(), changedIt.value());
        changedIt++;
    }

    showGroup.sync();
}

FileMetaDataWidget::FileMetaDataWidget(QWidget *parent)
    : QWidget(parent)
    , d(new FileMetaDataWidgetPrivate(this))
{
}

FileMetaDataWidget::~FileMetaDataWidget() = default;

void FileMetaDataWidget::setItems(const KFileItemList &items)
{
    d->m_provider->setItems(items);
    d->m_widgetFactory->setItems(items);
}

KFileItemList FileMetaDataWidget::items() const
{
    return d->m_provider->items();
}

void FileMetaDataWidget::setReadOnly(bool readOnly)
{
    d->m_provider->setReadOnly(readOnly);
    d->m_widgetFactory->setReadOnly(readOnly);
}

bool FileMetaDataWidget::isReadOnly() const
{
    return d->m_provider->isReadOnly();
}
void FileMetaDataWidget::setDateFormat(const DateFormats format)
{
    d->m_widgetFactory->setDateFormat(format);
}

DateFormats FileMetaDataWidget::dateFormat() const
{
    return d->m_widgetFactory->dateFormat();
}

QSize FileMetaDataWidget::sizeHint() const
{
    if (d->m_gridLayout == nullptr) {
        return QWidget::sizeHint();
    }

    // Calculate the required width for the labels and values
    int leftWidthMax = 0;
    int rightWidthMax = 0;
    int rightWidthAverage = 0;
    for (const FileMetaDataWidgetPrivate::Row &row : std::as_const(d->m_rows)) {
        const QWidget *valueWidget = row.value;
        const int rightWidth = valueWidget->sizeHint().width();
        rightWidthAverage += rightWidth;
        if (rightWidth > rightWidthMax) {
            rightWidthMax = rightWidth;
        }

        const int leftWidth = row.label->sizeHint().width();
        if (leftWidth > leftWidthMax) {
            leftWidthMax = leftWidth;
        }
    }

    // Some value widgets might return a very huge width for the size hint.
    // Limit the maximum width to the double width of the overall average
    // to assure a less messed layout.
    if (d->m_rows.count() > 1) {
        rightWidthAverage /= d->m_rows.count();
        if (rightWidthMax > rightWidthAverage * 2) {
            rightWidthMax = rightWidthAverage * 2;
        }
    }

    // Based on the available width calculate the required height
    int height = getMargin() * 2 + d->m_gridLayout->spacing() * (d->m_rows.count() - 1);
    for (const FileMetaDataWidgetPrivate::Row &row : std::as_const(d->m_rows)) {
        const QWidget *valueWidget = row.value;
        const int rowHeight = qMax(row.label->heightForWidth(leftWidthMax), valueWidget->heightForWidth(rightWidthMax));
        height += rowHeight;
    }

    const int width = getMargin() * 2 + leftWidthMax + d->m_gridLayout->spacing() + rightWidthMax;

    return QSize{width, height};
}

void FileMetaDataWidget::setConfigurationMode(ConfigurationMode mode)
{
    if (mode == ConfigurationMode::ReStart) {
        d->m_configureVisibleProperties = true;
    } else if (mode == ConfigurationMode::Accept) {
        d->saveConfig();
        d->m_configureVisibleProperties = false;
    } else if (mode == ConfigurationMode::Cancel) {
        d->m_configureVisibleProperties = false;
    }
    d->m_visibilityChanged.clear();
    d->slotLoadingFinished();
}

int FileMetaDataWidget::getMargin() const
{
    int left, top, right, bottom;
    d->m_gridLayout->getContentsMargins(&left, &top, &right, &bottom);
    if (left == top && top == right && right == bottom) {
        return left;
    } else {
        return -1;
    }
}

#include "moc_filemetadatawidget.cpp"
