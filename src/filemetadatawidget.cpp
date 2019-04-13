/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2012-2013  Vishesh Handa <me@vhanda.in>

    Adapted from KFileMetadataWidget
    Copyright (C) 2008 by Sebastian Trueg <trueg@kde.org>
    Copyright (C) 2009-2010 by Peter Penz <peter.penz@gmx.at>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "filemetadatawidget.h"
#include "metadatafilter.h"
#include "widgetfactory.h"
#include "filemetadataprovider.h"

#include <KFileItem>
#include <KFileMetaData/UserMetaData>

#include <QGridLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QList>
#include <QSet>
#include <QString>
#include <QTimer>
#include <QFileInfo>
#include <QDebug>

using namespace Baloo;

class Baloo::FileMetaDataWidget::Private
{
public:
    struct Row
    {
        QLabel* label;
        QWidget* value;
    };

    Private(FileMetaDataWidget* parent);
    ~Private();

    /**
     * Parses the configuration file "kmetainformationrc" and
     * updates the visibility of all rows that got their data
     * from KFileItem.
     */
    void updateFileItemRowsVisibility();

    void deleteRows();

    void slotLoadingFinished();
    void slotLinkActivated(const QString& link);
    void slotDataChangeStarted();
    void slotDataChangeFinished();

    QStringList sortedKeys(const QVariantMap& data) const;
    QLabel* createLabel(const QString &key,  const QString& itemLabel, FileMetaDataWidget* parent);

    QList<Row> m_rows;
    FileMetaDataProvider* m_provider;
    QGridLayout* m_gridLayout;

    MetadataFilter* m_filter;
    WidgetFactory* m_widgetFactory;

private:
    FileMetaDataWidget* const q;
};

FileMetaDataWidget::Private::Private(FileMetaDataWidget* parent)
    : m_rows()
    , m_provider(nullptr)
    , m_gridLayout(nullptr)
    , q(parent)
{
    m_filter = new MetadataFilter(q);

    m_widgetFactory = new WidgetFactory(q);
    connect(m_widgetFactory, &WidgetFactory::urlActivated, q, &FileMetaDataWidget::urlActivated);

    // TODO: If KFileMetaDataProvider might get a public class in future KDE releases,
    // the following code should be moved into KFileMetaDataWidget::setModel():
    m_provider = new FileMetaDataProvider(q);
    connect(m_provider, SIGNAL(loadingFinished()), q, SLOT(slotLoadingFinished()));
}

FileMetaDataWidget::Private::~Private()
{
}

void FileMetaDataWidget::Private::deleteRows()
{
    foreach (const Row& row, m_rows) {
        delete row.label;
        row.value->deleteLater();
    }

    m_rows.clear();
}

QLabel* FileMetaDataWidget::Private::createLabel(const QString &key, const QString& itemLabel, FileMetaDataWidget* parent)
{
    QLabel* label = new QLabel(itemLabel + QLatin1Char(':'), parent);
    label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    label->setForegroundRole(parent->foregroundRole());
    label->setFont(parent->font());
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignTop | Qt::AlignRight);
    label->setObjectName(QStringLiteral("L_%1").arg(key));
    return label;
}

void FileMetaDataWidget::Private::slotLoadingFinished()
{
    deleteRows();

    if (m_gridLayout == nullptr) {
        m_gridLayout = new QGridLayout(q);
        m_gridLayout->setContentsMargins(0, 0, 0, 0);
        m_gridLayout->setSpacing(q->fontMetrics().height() / 4);
    }

    QVariantMap data = m_filter->filter( m_provider->data() );
    m_widgetFactory->setNoLinks( m_provider->realTimeIndexing() );

    int rowIndex = 0;
    // Iterate through all remaining items.
    // Embed the label and the value as new row in the widget
    const QStringList keys = sortedKeys(data);
    for (const auto key: keys) {
        Row row;
        const int spacerWidth = QFontMetrics(q->font()).size(Qt::TextSingleLine, " ").width();
        m_gridLayout->addItem(new QSpacerItem(spacerWidth, 1), rowIndex, 1);

        row.label = createLabel(key, m_provider->label(key), q);
        m_gridLayout->addWidget(row.label, rowIndex, 0, Qt::AlignRight);

        row.value = m_widgetFactory->createWidget(key, data[key], q);
        m_gridLayout->addWidget(row.value, rowIndex, 2, Qt::AlignLeft);

        // Remember the label and value-widget as row
        m_rows.append(row);
        ++rowIndex;
    }

    q->updateGeometry();
    emit q->metaDataRequestFinished(m_provider->items());
}

void FileMetaDataWidget::Private::slotLinkActivated(const QString& link)
{
    const QUrl url = QUrl::fromUserInput(link);
    if (url.isValid()) {
        emit q->urlActivated(url);
    }
}

void FileMetaDataWidget::Private::slotDataChangeStarted()
{
    q->setEnabled(false);
}

void FileMetaDataWidget::Private::slotDataChangeFinished()
{
    q->setEnabled(true);
}

QStringList FileMetaDataWidget::Private::sortedKeys(const QVariantMap& data) const
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

        map.insertMulti(key, propName);
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

FileMetaDataWidget::FileMetaDataWidget(QWidget* parent)
    : QWidget(parent)
    , d(new Private(this))
{
}

FileMetaDataWidget::~FileMetaDataWidget()
{
    delete d;
}

void FileMetaDataWidget::setItems(const KFileItemList& items)
{
    KFileItemList localItemsList;
    QStringList list;

    bool xAttrSuppored = true;

    foreach(const KFileItem& item, items) {
        QUrl url = item.targetUrl();
        if (url.isLocalFile()) {
            localItemsList << item;
            QString path = url.toLocalFile();
            list << path;

            KFileMetaData::UserMetaData md(path);
            xAttrSuppored &= md.isSupported();
        }
    }
    setReadOnly(!xAttrSuppored);

    d->m_provider->setItems(localItemsList);
    d->m_widgetFactory->setItems(list);

    setReadOnly(!xAttrSuppored);
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
    foreach (const Private::Row& row, d->m_rows) {
        const QWidget* valueWidget = row.value;
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
    int height = d->m_gridLayout->margin() * 2 + d->m_gridLayout->spacing() * (d->m_rows.count() - 1);
    foreach (const Private::Row& row, d->m_rows) {
        const QWidget* valueWidget = row.value;
        const int rowHeight = qMax(row.label->heightForWidth(leftWidthMax),
                                   valueWidget->heightForWidth(rightWidthMax));
        height += rowHeight;
    }

    const int width = d->m_gridLayout->margin() * 2 + leftWidthMax +
    d->m_gridLayout->spacing() + rightWidthMax;

    return QSize(width, height);
}

#include "moc_filemetadatawidget.cpp"
