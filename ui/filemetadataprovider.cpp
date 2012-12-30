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
#include <Soprano/Vocabulary/RDF>
#include <Nepomuk2/Vocabulary/NFO>
#include <Nepomuk2/Vocabulary/NMM>
#include <Nepomuk2/Vocabulary/NIE>

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
using namespace Nepomuk2::Vocabulary;


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

namespace {
    Nepomuk2::Variant intersect( const Nepomuk2::Variant& v1, const Nepomuk2::Variant& v2 ) {
        if( !v1.isValid() || !v2.isValid() )
            return Nepomuk2::Variant();

        // Single value
        if( !v1.isList() && !v2.isList() ) {
            if( v1 == v2 )
                return v1;
            else
                return Variant();
        }
        // List and single
        if( v1.isResourceList() && v2.isResource() ) {
            QList<Resource> v1List = v1.toResourceList();
            Resource v2Res = v2.toResource();
            if( v1List.contains(v2Res) ) {
                return Variant( v2Res );
            }
        }
        if( v2.isResourceList() && v1.isResource() ) {
            QList<Resource> v2List = v2.toResourceList();
            Resource v1Res = v1.toResource();
            if( v2List.contains(v1Res) ) {
                return Variant( v1Res );
            }
        }
        else if( v1.isResourceList() && v2.isResourceList() ) {
            QSet<Resource> v1Set = v1.toResourceList().toSet();
            QSet<Resource> v2Set = v2.toResourceList().toSet();

            QSet<Resource> inter = v1Set.intersect( v2Set );
            return Variant( inter.toList() );
        }
        // TODO: Target more list types?

        return Variant();
    }
}

void FileMetaDataProvider::Private::slotLoadingFinished(ResourceLoader* loader)
{
    QList<Resource> resources = loader->resources();
    loader->deleteLater();
    loader = 0;

    if( resources.size() == 1 ) {
        m_data = resources.first().properties();
    }
    else {
        //
        // Only report the stuff that is common to all the resources
        //

        QSet<QUrl> allProperties;
        foreach(const Resource& res, resources) {
            allProperties.unite( res.properties().uniqueKeys().toSet() );
        }

        // Remove properties which cannot be the same
        allProperties.remove( NIE::url() );
        allProperties.remove( RDF::type() );
        allProperties.remove( NAO::lastModified() );
        allProperties.remove( NIE::lastModified() );

        // Special handling for nfo:duration
        // Take the sum
        if( allProperties.contains( NFO::duration() ) ) {
            int total = 0;
            foreach(const Resource& res, resources) {
                QHash<QUrl, Variant> hash = res.properties();
                QHash< QUrl, Variant >::iterator it = hash.find( NFO::duration() );
                if( it == hash.end() ) {
                    total = 0;
                    break;
                }
                else {
                    total += it.value().toInt();
                }
            }

            if( total )
                m_data.insert( NFO::duration(), Variant(total) );
            allProperties.remove( NFO::duration() );
        }

        foreach( const QUrl& propUri, allProperties ) {
            foreach(const Resource& res, resources) {
                QHash<QUrl, Variant> hash = res.properties();
                QHash< QUrl, Variant >::iterator it = hash.find( propUri );
                if( it == hash.end() ) {
                    m_data.remove( propUri );
                    goto nextProperty;
                }
                else {
                    QHash< QUrl, Variant >::iterator dit = m_data.find( it.key() );
                    if( dit == m_data.end() ) {
                        m_data.insert( propUri, it.value() );
                    }
                    else {
                        Variant finalValue = intersect( it.value(), dit.value() );
                        if( finalValue.isValid() )
                            m_data[propUri] = finalValue;
                        else {
                            m_data.remove( propUri );
                            goto nextProperty;
                        }
                    }
                }
            }
            nextProperty:
            ;
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
        // Tags, ratings and comments are stored by their normal property as well
        { "http://www.semanticdesktop.org/ontologies/2007/08/15/nao#hasTag", I18N_NOOP2_NOSTRIP("@label", "Tags") },
        { "http://www.semanticdesktop.org/ontologies/2007/08/15/nao#numericRating", I18N_NOOP2_NOSTRIP("@label", "Rating") },
        { "http://www.semanticdesktop.org/ontologies/2007/08/15/nao#description", I18N_NOOP2_NOSTRIP("@label", "Comment") },
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
    static QHash<QUrl, QString> uriGrouper;
    if( uriGrouper.isEmpty() ) {
        // KFileItem Data
        uriGrouper.insert( QUrl("kfileitem#type"), QLatin1String("0FileItemA") );
        uriGrouper.insert( QUrl("kfileitem#size"), QLatin1String("0FileItemB") );
        uriGrouper.insert( QUrl("kfileitem#totalSize"), QLatin1String("0FileItemB") );
        uriGrouper.insert( QUrl("kfileitem#modified"), QLatin1String("0FileItemC") );
        uriGrouper.insert( QUrl("kfileitem#owner"), QLatin1String("0FileItemD") );
        uriGrouper.insert( QUrl("kfileitem#permissions"), QLatin1String("0FileItemE") );

        // Editable Data
        uriGrouper.insert( NAO::hasTag(), QLatin1String("1EditableDataA") );
        uriGrouper.insert( NAO::numericRating(), QLatin1String("1EditableDataB") );
        uriGrouper.insert( NAO::description(), QLatin1String("1EditableDataC") );

        // Image Data
        uriGrouper.insert( NFO::width(), QLatin1String("2SizA") );
        uriGrouper.insert( NFO::height(), QLatin1String("2SizeB") );

        // Music Data
        uriGrouper.insert( NIE::title(), QLatin1String("3MusicA") );
        uriGrouper.insert( NMM::performer(), QLatin1String("3MusicB") );
        uriGrouper.insert( NMM::musicAlbum(), QLatin1String("3MusicC") );
        uriGrouper.insert( NMM::genre(), QLatin1String("3MusicD") );
        uriGrouper.insert( NMM::trackNumber(), QLatin1String("3MusicE") );

        // Audio Data
        uriGrouper.insert( NFO::duration(), QLatin1String("4AudioA") );
        uriGrouper.insert( NFO::sampleRate(), QLatin1String("4AudioB") );
        uriGrouper.insert( NFO::sampleCount(), QLatin1String("4AudioC") );
    }

    return uriGrouper.value( metaDataUri );
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
