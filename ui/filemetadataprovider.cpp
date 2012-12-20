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
#include "indexeddataretriever.h"

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



namespace Nepomuk2 {

class FileMetaDataProvider::Private
{

public:
    Private(FileMetaDataProvider* parent);
    ~Private();

    void slotLoadingFinished(ResourceLoader* loader);
    void slotLoadingFinished(KJob* job);

    void insertBasicData();

    /*
     * @return The number of subdirectories for the directory \a path.
     */
    static int subDirectoriesCount(const QString &path);

    bool m_readOnly;
    bool m_nepomukActivated;
    QList<KFileItem> m_fileItems;

    QHash<QUrl, Variant> m_data;
private:
    FileMetaDataProvider* const q;
};

FileMetaDataProvider::Private::Private(FileMetaDataProvider* parent) :
    m_readOnly(false),
    m_nepomukActivated(false),
    m_fileItems(),
    m_data(),
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
    loader = 0;

    // FIXME: Is this really the best way?
    foreach(const Resource& res, resources) {
        QHash<QUrl, Variant> hash = res.properties();
        QHash< QUrl, Variant >::const_iterator it = hash.constBegin();
        for( ; it != hash.constEnd(); it++ ) {
            m_data.insert( it.key(), it.value() );
        }
    }

    insertBasicData();

    emit q->loadingFinished();
}

void FileMetaDataProvider::Private::slotLoadingFinished(KJob* job)
{
    IndexedDataRetriever* ret = dynamic_cast<IndexedDataRetriever*>( job );
    m_data = ret->data();

    insertBasicData();

    emit q->loadingFinished();
}

void FileMetaDataProvider::Private::insertBasicData()
{
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
    }
    else if (m_fileItems.count() > 1) {
        // Calculate the size of all items
        quint64 totalSize = 0;
        foreach (const KFileItem& item, m_fileItems) {
            if (!item.isDir() && !item.isLink()) {
                totalSize += item.size();
            }
        }
        m_data.insert(KUrl("kfileitem#totalSize"), KIO::convertSize(totalSize));
    }

    // Insert tags, ratings and comments, if Nepomuk activated
    if( m_nepomukActivated && !m_readOnly ) {
        if( !m_data.contains(NAO::hasTag()) )
            m_data.insert( NAO::hasTag(), Variant() );
        if( !m_data.contains(NAO::numericRating()) )
            m_data.insert( NAO::numericRating(), Variant() );
        if( !m_data.contains(NAO::description()) )
            m_data.insert( NAO::description(), Variant() );
    }
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
    d->m_data.clear();

    if (items.isEmpty()) {
        return;
    }

    if( items.size() == 1 ) {
        const KFileItem item = items.first();
        const QUrl url = item.targetUrl();
        const QUrl uri = item.nepomukUri();

        Resource res;
        if( uri.isValid() )
            res = Resource(uri);
        else
            res = Resource(url);

        if( !res.exists() ) {
            IndexedDataRetriever *ret = new IndexedDataRetriever( url.toLocalFile(), this );
            connect( ret, SIGNAL(finished(KJob*)), this, SLOT(slotLoadingFinished(KJob*)) );
            ret->start();
            return;
        }
    }

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

QHash<QUrl, Variant> FileMetaDataProvider::data() const
{
    return d->m_data;
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
