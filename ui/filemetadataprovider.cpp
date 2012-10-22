/*****************************************************************************
 * Copyright (C) 2010 by Peter Penz <peter.penz@gmx.at>                      *
 * Copyright (C) 2012 by Vishesh Handa <me@vhanda.in>                        *
 *                                                                           *
 * This library is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Library General Public               *
 * License as published by the Free Software Foundation; either              *
 * version 2 of the License, or (at your option) any later version.          *
 *                                                                           *
 * This library is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Library General Public License for more details.                          *
 *                                                                           *
 * You should have received a copy of the GNU Library General Public License *
 * along with this library; see the file COPYING.LIB.  If not, write to      *
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 * Boston, MA 02110-1301, USA.                                               *
 *****************************************************************************/

#include "filemetadataprovider_p.h"
#include "tagwidget.h"
#include "resourceloader.h"
#include "kcommentwidget_p.h"
#include "knfotranslator_p.h"

#include <kfileitem.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kratingwidget.h>
#include <KDebug>

#include <Nepomuk2/Tag>
#include <Nepomuk2/Resource>
#include <Nepomuk2/ResourceManager>
#include <nepomuk2/utils.h>
#include <Nepomuk2/DataManagement>
#include <Nepomuk2/Types/Property>

#include <Soprano/Vocabulary/NAO>

#include <QEvent>
#include <QLabel>

// Required includes for subDirectoriesCount():
#ifdef Q_WS_WIN
    #include <QDir>
#else
    #include <dirent.h>
    #include <QFile>
#endif

using namespace Soprano::Vocabulary;

namespace {
    static QString plainText(const QString& richText)
    {
        QString plainText;
        plainText.reserve(richText.length());

        bool skip = false;
        for (int i = 0; i < richText.length(); ++i) {
            const QChar c = richText.at(i);
            if (c == QLatin1Char('<')) {
                skip = true;
            } else if (c == QLatin1Char('>')) {
                skip = false;
            } else if (!skip) {
                plainText.append(c);
            }
        }

        return plainText;
    }
}

// The default size hint of QLabel tries to return a square size.
// This does not work well in combination with layouts that use
// heightForWidth(): In this case it is possible that the content
// of a label might get clipped. By specifying a size hint
// with a maximum width that is necessary to contain the whole text,
// using heightForWidth() assures having a non-clipped text.
class ValueWidget : public QLabel
{
public:
    explicit ValueWidget(QWidget* parent = 0);
    virtual QSize sizeHint() const;
};

ValueWidget::ValueWidget(QWidget* parent) :
    QLabel(parent)
{
}

QSize ValueWidget::sizeHint() const
{
    QFontMetrics metrics(font());
    // TODO: QLabel internally provides already a method sizeForWidth(),
    // that would be sufficient. However this method is not accessible, so
    // as workaround the tags from a richtext are removed manually here to
    // have a proper size hint.
    return metrics.size(Qt::TextSingleLine, plainText(text()));
}


namespace Nepomuk2 {

class FileMetaDataProvider::Private
{

public:
    Private(FileMetaDataProvider* parent);
    ~Private();

    void slotLoadingFinished(ResourceLoader* loader);

    void slotRatingChanged(unsigned int rating);
    void slotTagsChanged(const QList<Nepomuk2::Tag>& tags);
    void slotCommentChanged(const QString& comment);

    void slotMetaDataUpdateDone();
    void slotTagClicked(const Nepomuk2::Tag& tag);
    void slotLinkActivated(const QString& link);

    /**
     * Disables the metadata widget and starts the job that
     * changes the meta data asynchronously. After the job
     * has been finished, the metadata widget gets enabled again.
     */
    void startChangeDataJob(KJob* job);

    QList<Resource> resourceList() const;
    QList<QUrl> resourceUriList() const;

    QWidget* createRatingWidget(int rating, QWidget* parent);
    QWidget* createTagWidget(const QList<Tag>& tags, QWidget* parent);
    QWidget* createCommentWidget(const QString& comment, QWidget* parent);
    QWidget* createValueWidget(const QString& value, QWidget* parent);

    /*
     * @return The number of subdirectories for the directory \a path.
     */
    static int subDirectoriesCount(const QString &path);

    bool m_readOnly;
    bool m_nepomukActivated;
    QList<KFileItem> m_fileItems;

    QHash<KUrl, Variant> m_data;

    QWeakPointer<KRatingWidget> m_ratingWidget;
    QWeakPointer<TagWidget> m_tagWidget;
    QWeakPointer<KCommentWidget> m_commentWidget;

private:
    FileMetaDataProvider* const q;
};

FileMetaDataProvider::Private::Private(FileMetaDataProvider* parent) :
    m_readOnly(false),
    m_nepomukActivated(false),
    m_fileItems(),
    m_data(),
    m_ratingWidget(),
    m_tagWidget(),
    m_commentWidget(),
    q(parent)
{
    m_nepomukActivated = ResourceManager::instance()->initialized();
}

FileMetaDataProvider::Private::~Private()
{
}

void FileMetaDataProvider::Private::slotLoadingFinished(ResourceLoader* loader)
{
    QList<Resource> resources = loader->resources();
    loader->deleteLater();

    // FIXME: Is this really the best way?
    foreach(const Resource& res, resources) {
        QHash<QUrl, Variant> hash = res.properties();
        QHash< QUrl, Variant >::const_iterator it = hash.constBegin();
        for( ; it != hash.constEnd(); it++ ) {
            Variant value = Utils::formatPropertyValue( it.key(), it.value(),
                                                        QList<Resource>() << res,
                                                        Utils::WithKioLinks );
            m_data.insert( it.key(), value );
        }

        // Required - These 3 should always be shown by the FileMetadataWidget
        m_data.insert( KUrl("kfileitem#rating"), res.rating() );
        m_data.insert( KUrl("kfileitem#comment"), res.description() );
        m_data.insert( KUrl("kfileitem#tags"), res.property( NAO::hasTag() ) );
    }

    if (m_fileItems.count() == 1) {
        // TODO: Handle case if remote URLs are used properly. isDir() does
        // not work, the modification date needs also to be adjusted...
        const KFileItem& item = m_fileItems.first();

        if (item.isDir()) {
            const int count = subDirectoriesCount(item.url().pathOrUrl());
            if (count == -1) {
                m_data.insert(KUrl("kfileitem#size"), QString("Unknown"));
            } else {
                const QString itemCountString = i18ncp("@item:intable", "%1 item", "%1 items", count);
                m_data.insert(KUrl("kfileitem#size"), itemCountString);
            }
        } else {
            m_data.insert(KUrl("kfileitem#size"), KIO::convertSize(item.size()));
        }
        m_data.insert(KUrl("kfileitem#type"), item.mimeComment());
        m_data.insert(KUrl("kfileitem#modified"), KGlobal::locale()->formatDateTime(item.time(KFileItem::ModificationTime), KLocale::FancyLongDate));
        m_data.insert(KUrl("kfileitem#owner"), item.user());
        m_data.insert(KUrl("kfileitem#permissions"), item.permissionsString());
    } else if (m_fileItems.count() > 1) {
        // Calculate the size of all items
        quint64 totalSize = 0;
        foreach (const KFileItem& item, m_fileItems) {
            if (!item.isDir() && !item.isLink()) {
                totalSize += item.size();
            }
        }
        m_data.insert(KUrl("kfileitem#totalSize"), KIO::convertSize(totalSize));
    }

    emit q->loadingFinished();
}

void FileMetaDataProvider::Private::slotRatingChanged(unsigned int rating)
{
    KJob* job = Nepomuk2::setProperty( resourceUriList(), NAO::numericRating(), QVariantList() << rating );
    startChangeDataJob(job);
}

void FileMetaDataProvider::Private::slotTagsChanged(const QList<Nepomuk2::Tag>& tags)
{
    if (!m_tagWidget.isNull()) {
        m_tagWidget.data()->setSelectedTags(tags);

        QVariantList tagUris;
        foreach( const Tag& tag, tags )
            tagUris << tag.uri();

        KJob* job = Nepomuk2::setProperty( resourceUriList(), NAO::hasTag(), tagUris );
        startChangeDataJob(job);
    }
}

void FileMetaDataProvider::Private::slotCommentChanged(const QString& comment)
{
    KJob* job = Nepomuk2::setProperty( resourceUriList(), NAO::description(), QVariantList() << comment );
    startChangeDataJob(job);
}

void FileMetaDataProvider::Private::slotTagClicked(const Nepomuk2::Tag& tag)
{
    emit q->urlActivated(tag.uri());
}

void FileMetaDataProvider::Private::slotLinkActivated(const QString& link)
{
    emit q->urlActivated(KUrl(link));
}

void FileMetaDataProvider::Private::startChangeDataJob(KJob* job)
{
    connect(job, SIGNAL(result(KJob*)),
            q, SIGNAL(dataChangeFinished()));
    emit q->dataChangeStarted();
    job->start();
}

QList<Resource> FileMetaDataProvider::Private::resourceList() const
{
    QList<Resource> list;
    foreach (const KFileItem& item, m_fileItems) {
        const KUrl url = item.nepomukUri();
        if(url.isValid())
            list.append(Resource(url));
    }
    return list;
}

QList< QUrl > FileMetaDataProvider::Private::resourceUriList() const
{
    QList<QUrl> list;
    foreach(const KFileItem& item, m_fileItems) {
        const KUrl uri = item.nepomukUri();
        if( uri.isValid() )
            list << uri;
    }

    return list;
}


QWidget* FileMetaDataProvider::Private::createRatingWidget(int rating, QWidget* parent)
{
    KRatingWidget* ratingWidget = new KRatingWidget(parent);
    const Qt::Alignment align = (ratingWidget->layoutDirection() == Qt::LeftToRight) ?
                                Qt::AlignLeft : Qt::AlignRight;
    ratingWidget->setAlignment(align);
    ratingWidget->setRating(rating);
    const QFontMetrics metrics(parent->font());
    ratingWidget->setPixmapSize(metrics.height());

    connect(ratingWidget, SIGNAL(ratingChanged(uint)),
            q, SLOT(slotRatingChanged(uint)));

    m_ratingWidget = ratingWidget;

    return ratingWidget;
}

QWidget* FileMetaDataProvider::Private::createTagWidget(const QList<Tag>& tags, QWidget* parent)
{
    TagWidget* tagWidget = new TagWidget(parent);
    tagWidget->setModeFlags(m_readOnly
                            ? TagWidget::MiniMode | TagWidget::ReadOnly
                            : TagWidget::MiniMode);
    tagWidget->setSelectedTags(tags);

    connect(tagWidget, SIGNAL(selectionChanged(QList<Nepomuk2::Tag>)),
            q, SLOT(slotTagsChanged(QList<Nepomuk2::Tag>)));
    connect(tagWidget, SIGNAL(tagClicked(Nepomuk2::Tag)),
            q, SLOT(slotTagClicked(Nepomuk2::Tag)));

    m_tagWidget = tagWidget;

    return tagWidget;
}

QWidget* FileMetaDataProvider::Private::createCommentWidget(const QString& comment, QWidget* parent)
{
    KCommentWidget* commentWidget = new KCommentWidget(parent);
    commentWidget->setText(comment);
    commentWidget->setReadOnly(m_readOnly);

    connect(commentWidget, SIGNAL(commentChanged(QString)),
            q, SLOT(slotCommentChanged(QString)));

    m_commentWidget = commentWidget;

    return commentWidget;
}

QWidget* FileMetaDataProvider::Private::createValueWidget(const QString& value, QWidget* parent)
{
    ValueWidget* valueWidget = new ValueWidget(parent);
    valueWidget->setWordWrap(true);
    valueWidget->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    valueWidget->setText(m_readOnly ? plainText(value) : value);
    connect(valueWidget, SIGNAL(linkActivated(QString)), q, SLOT(slotLinkActivated(QString)));
    return valueWidget;
}

FileMetaDataProvider::FileMetaDataProvider(QObject* parent) :
    QObject(parent),
    d(new Private(this))
{
}

FileMetaDataProvider::~FileMetaDataProvider()
{
    delete d;
}

void FileMetaDataProvider::setItems(const KFileItemList& items)
{
    d->m_fileItems = items;

    if (items.isEmpty()) {
        return;
    }
    //Q_PRIVATE_SLOT(d, void slotDataChangeStarted())
    //Q_PRIVATE_SLOT(d, void slotDataChangeFinished())

    QList<QUrl> urls;
    foreach (const KFileItem& item, items) {
        const QUrl url = item.nepomukUri();
        if (url.isValid()) {
            urls.append(url);
        }
    }

    ResourceLoader* loader = new ResourceLoader( urls, this );
    connect( loader, SIGNAL(finished(ResourceLoader*)),
             this, SLOT(slotLoadingFinished(ResourceLoader*)) );
    loader->start();
}

QString FileMetaDataProvider::label(const KUrl& metaDataUri) const
{
    struct TranslationItem {
        const char* const key;
        const char* const context;
        const char* const value;
    };

    static const TranslationItem translations[] = {
        { "kfileitem#comment", I18N_NOOP2_NOSTRIP("@label", "Comment") },
        { "kfileitem#modified", I18N_NOOP2_NOSTRIP("@label", "Modified") },
        { "kfileitem#owner", I18N_NOOP2_NOSTRIP("@label", "Owner") },
        { "kfileitem#permissions", I18N_NOOP2_NOSTRIP("@label", "Permissions") },
        { "kfileitem#rating", I18N_NOOP2_NOSTRIP("@label", "Rating") },
        { "kfileitem#size", I18N_NOOP2_NOSTRIP("@label", "Size") },
        { "kfileitem#tags", I18N_NOOP2_NOSTRIP("@label", "Tags") },
        { "kfileitem#totalSize", I18N_NOOP2_NOSTRIP("@label", "Total Size") },
        { "kfileitem#type", I18N_NOOP2_NOSTRIP("@label", "Type") },
        { 0, 0, 0} // Mandatory last entry
    };

    static QHash<QString, QString> hash;
    if (hash.isEmpty()) {
        const TranslationItem* item = &translations[0];
        while (item->key != 0) {
            hash.insert(item->key, i18nc(item->context, item->value));
            ++item;
        }
    }

    QString value = hash.value(metaDataUri.url());
    if (value.isEmpty()) {
        value = KNfoTranslator::instance().translation(metaDataUri);
    }

    return value;
}

QString FileMetaDataProvider::group(const KUrl& metaDataUri) const
{
    QString group; // return value

    const QString uri = metaDataUri.url();
    if (uri == QLatin1String("kfileitem#type")) {
        group = QLatin1String("0FileItemA");
    } else if (uri == QLatin1String("kfileitem#size")) {
        group = QLatin1String("0FileItemB");
    } else if (uri == QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#width")) {
        group = QLatin1String("0SizeA");
    } else if (uri == QLatin1String("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#height")) {
        group = QLatin1String("0SizeB");
    }

    return group;
}

KFileItemList FileMetaDataProvider::items() const
{
    return d->m_fileItems;
}

void FileMetaDataProvider::setReadOnly(bool readOnly)
{
    d->m_readOnly = readOnly;
}

bool FileMetaDataProvider::isReadOnly() const
{
    return d->m_readOnly;
}

QHash<KUrl, Variant> FileMetaDataProvider::data() const
{
    return d->m_data;
}

QWidget* FileMetaDataProvider::createValueWidget(const KUrl& metaDataUri,
                                                  const Variant& value,
                                                  QWidget* parent) const
{
    Q_ASSERT(parent != 0);
    QWidget* widget = 0;

    if (d->m_nepomukActivated) {
        const QString uri = metaDataUri.url();
        if (uri == QLatin1String("kfileitem#rating")) {
            widget = d->createRatingWidget(value.toInt(), parent);
        } else if (uri == QLatin1String("kfileitem#tags")) {
            const QStringList tagNames = value.toStringList();
            QList<Tag> tags;
            foreach (const QString& tagName, tagNames) {
                tags.append(Tag(tagName));
            }

            widget = d->createTagWidget(tags, parent);
        } else if (uri == QLatin1String("kfileitem#comment")) {
            widget = d->createCommentWidget(value.toString(), parent);
        }
    }

    if (widget == 0) {
        widget = d->createValueWidget(value.toString(), parent);
    }

    widget->setForegroundRole(parent->foregroundRole());
    widget->setFont(parent->font());

    return widget;
}

int FileMetaDataProvider::Private::subDirectoriesCount(const QString& path)
{
#ifdef Q_WS_WIN
    QDir dir(path);
    return dir.entryList(QDir::AllEntries|QDir::NoDotAndDotDot|QDir::System).count();
#else
    // Taken from kdelibs/kio/kio/kdirmodel.cpp
    // Copyright (C) 2006 David Faure <faure@kde.org>

    int count = -1;
    DIR* dir = ::opendir(QFile::encodeName(path));
    if (dir) {
        count = 0;
        struct dirent *dirEntry = 0;
        while ((dirEntry = ::readdir(dir))) { // krazy:exclude=syscalls
            if (dirEntry->d_name[0] == '.') {
                if (dirEntry->d_name[1] == '\0') {
                    // Skip "."
                    continue;
                }
                if (dirEntry->d_name[1] == '.' && dirEntry->d_name[2] == '\0') {
                    // Skip ".."
                    continue;
                }
            }
            ++count;
        }
        ::closedir(dir);
    }
    return count;
#endif
}

}

#include "filemetadataprovider_p.moc"