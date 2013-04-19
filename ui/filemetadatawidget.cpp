/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2012  Vishesh Handa <me@vhanda.in>

    Adapated from KFileMetadataWidget
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

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kfileitem.h>
#include <klocale.h>

#include <QGridLayout>
#include <QLabel>
#include <QList>
#include <QSet>
#include <QString>
#include <QTimer>

#include <Nepomuk2/Types/Property>
#include <Nepomuk2/Tag>

#include <QSpacerItem>

#include "filemetadataprovider_p.h"
#include <KDebug>

namespace Nepomuk2 {

class FileMetaDataWidget::Private
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

    QList<QUrl> sortedKeys(const QHash<QUrl, Nepomuk2::Variant>& data) const;

    /**
     * @return True, if at least one of the file items \a m_fileItems has
     *         a valid Nepomuk URI.
     */
    bool hasNepomukUris() const;

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
    , m_provider(0)
    , m_gridLayout(0)
    , q(parent)
{
    m_filter = new MetadataFilter(q);

    m_widgetFactory = new WidgetFactory(q);
    connect(m_widgetFactory, SIGNAL(urlActivated(KUrl)), q, SIGNAL(urlActivated(KUrl)));

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
        delete row.value;
    }

    m_rows.clear();
}

void FileMetaDataWidget::Private::slotLoadingFinished()
{
    deleteRows();

    if (!hasNepomukUris()) {
        q->updateGeometry();
        emit q->metaDataRequestFinished(m_provider->items());
        return;
    }

    if (m_gridLayout == 0) {
        m_gridLayout = new QGridLayout(q);
        m_gridLayout->setMargin(0);
        m_gridLayout->setSpacing(q->fontMetrics().height() / 4);
    }

    // Filter the data
    QHash<QUrl, Variant> data = m_filter->filter( m_provider->data() );
    m_widgetFactory->setNoLinks( m_provider->realTimeIndexing() );

    // Iterate through all remaining items embed the label
    // and the value as new row in the widget
    int rowIndex = 0;
    //FIXME: Sorting of keys?
    const QList<QUrl> keys = sortedKeys(data);
    foreach (const QUrl& key, keys) {
        const Variant value = data[key];
        QString itemLabel = m_provider->label(key);
        itemLabel.append(QLatin1Char(':'));

        // Create label
        QLabel* label = new QLabel(itemLabel, q);
        label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        label->setForegroundRole(q->foregroundRole());
        label->setFont(q->font());
        label->setWordWrap(true);
        label->setAlignment(Qt::AlignTop | Qt::AlignRight);

        // Create value-widget
        QWidget* valueWidget = m_widgetFactory->createWidget(key, value, q);

        // Add the label and value-widget to grid layout
        m_gridLayout->addWidget(label, rowIndex, 0, Qt::AlignRight);
        const int spacerWidth = QFontMetrics(q->font()).size(Qt::TextSingleLine, " ").width();
        m_gridLayout->addItem(new QSpacerItem(spacerWidth, 1), rowIndex, 1);
        m_gridLayout->addWidget(valueWidget, rowIndex, 2, Qt::AlignLeft);

        // Remember the label and value-widget as row
        Row row;
        row.label = label;
        row.value = valueWidget;
        m_rows.append(row);

        ++rowIndex;
    }

    q->updateGeometry();
    emit q->metaDataRequestFinished(m_provider->items());
}

void FileMetaDataWidget::Private::slotLinkActivated(const QString& link)
{
    const KUrl url(link);
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

QList<QUrl> FileMetaDataWidget::Private::sortedKeys(const QHash<QUrl, Variant>& data) const
{
    // Create a map, where the translated label prefixed with the
    // sort priority acts as key. The data of each entry is the URI
    // of the data. By this the all URIs are sorted by the sort priority
    // and sub sorted by the translated labels.
    QMap<QString, QUrl> map;
    QHash<QUrl, Variant>::const_iterator hashIt = data.constBegin();
    while (hashIt != data.constEnd()) {
        const QUrl uri = hashIt.key();

        QString key = m_provider->group(uri);
        key += m_provider->label(uri);

        map.insert(key, uri);
        ++hashIt;
    }

    // Apply the URIs from the map to the list that will get returned.
    // The list will then be alphabetically ordered by the translated labels of the URIs.
    QList<QUrl> list;
    QMap<QString, QUrl>::const_iterator mapIt = map.constBegin();
    while (mapIt != map.constEnd()) {
        list.append(mapIt.value());
        ++mapIt;
    }

    return list;
}

bool FileMetaDataWidget::Private::hasNepomukUris() const
{
    foreach (const KFileItem& fileItem, m_provider->items()) {
        if (fileItem.nepomukUri().isValid()) {
            return true;
        }
    }
    return false;
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
    d->m_provider->setItems(items);

    QList<QUrl> uriList;
    foreach(const KFileItem& item, items) {
        const QUrl uri = item.nepomukUri();
        if( uri.isValid() )
            uriList << uri;
    }
    d->m_widgetFactory->setUris( uriList );
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

QSize FileMetaDataWidget::sizeHint() const
{
    if (d->m_gridLayout == 0) {
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

}

#include "filemetadatawidget.moc"
