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

#include "filemetadataprovider.h"
#include "indexeddataretriever.h"
#include "filefetchjob.h"

#include <KFileMetaData/PropertyInfo>
#include <KLocalizedString>
#include <KFormat>

#include <QTimer>
#include <QDebug>

// Required includes for subDirectoriesCount():
#ifdef Q_OS_WIN
    #include <QDir>
#else
    #include <dirent.h>
    #include <QFile>
#endif

using namespace Baloo;

namespace {
    QVariant intersect(const QVariant& v1, const QVariant& v2) {
        if (!v1.isValid() || !v2.isValid()) {
            return QVariant();
        }

        // List and String
        if (v1.type() == QVariant::StringList && v2.type() == QVariant::String) {
            QStringList list = v1.toStringList();
            QString str = v2.toString();

            if (!list.contains(str)) {
                list << str;
            }

            return QVariant(list);
        }

        // String and List
        if (v1.type() == QVariant::String && v2.type() == QVariant::StringList) {
            QStringList list = v2.toStringList();
            QString str = v1.toString();

            if (!list.contains(str)) {
                list << str;
            }

            return QVariant(list);
        }

        // List and List
        if (v1.type() == QVariant::StringList && v2.type() == QVariant::StringList) {
            QSet<QString> s1 = v1.toStringList().toSet();
            QSet<QString> s2 = v2.toStringList().toSet();

            return QVariant(s1.intersect(s2).toList());
        }

        if (v1 == v2) {
            return v1;
        }

        return QVariant();
    }

    /**
     * The standard QMap::unite will contain the key multiple times if both \p v1 and \p v2
     * contain the same key.
     *
     * This will only take the key from \p v2 into account
     */
    QVariantMap unite(const QVariantMap& v1, const QVariantMap& v2)
    {
        QVariantMap v(v1);
        QMapIterator<QString, QVariant> it(v2);
        while (it.hasNext()) {
            it.next();

            v[it.key()] = it.value();
        }

        return v;
    }
    
}

void FileMetaDataProvider::totalPropertyAndInsert(const QString& prop,
                                                  const QList<QVariantMap>& resources,
                                                  QSet<QString>& allProperties)
{
    if (allProperties.contains(prop)) {
        int total = 0;
        foreach (const QVariantMap& map, resources) {
            QVariantMap::const_iterator it = map.constFind(prop);
            if (it == map.constEnd()) {
                total = 0;
                break;
            } else {
                total += it.value().toInt();
            }
        }

        if (total) {
            m_data.insert (prop, QVariant(total));
        }
        allProperties.remove(prop);
    }
}

void FileMetaDataProvider::slotFileFetchFinished(KJob* job)
{
    FileFetchJob* fetchJob = static_cast<FileFetchJob*>(job);
    QList<QVariantMap> files = fetchJob->data();
    
    Q_ASSERT(!files.isEmpty());
    
    if (files.size() > 1) {
        insertCommonData(files);
    } else {
        m_data = files.first();
        insertSingleFileBasicData();
    }

    insertEditableData();
    emit loadingFinished();
}

void FileMetaDataProvider::slotLoadingFinished(KJob* job)
{
    IndexedDataRetriever* ret = dynamic_cast<IndexedDataRetriever*>(job);
    m_data = unite(m_data, ret->data());

    emit loadingFinished();
}

void FileMetaDataProvider::insertSingleFileBasicData()
{
    // TODO: Handle case if remote URLs are used properly. isDir() does
    // not work, the modification date needs also to be adjusted...
    Q_ASSERT(m_fileItems.count() <= 1);
    if (m_fileItems.count() == 1) {
      KFormat format;
      const KFileItem& item = m_fileItems.first();

      if (item.isDir()) {
          const int count = subDirectoriesCount(item.url().path());
          if (count == -1) {
              m_data.insert("kfileitem#size", i18nc("unknown file size", "Unknown"));
          } else {
              const QString itemCountString = i18ncp("@item:intable", "%1 item", "%1 items", count);
              m_data.insert("kfileitem#size", itemCountString);
          }
      } else {
          m_data.insert("kfileitem#size", format.formatByteSize(item.size()));
      }

      m_data.insert("kfileitem#type", item.mimeComment());
      m_data.insert("kfileitem#modified", item.time(KFileItem::ModificationTime));
      m_data.insert("kfileitem#owner", item.user());
      m_data.insert("kfileitem#group", item.group());
      m_data.insert("kfileitem#permissions", item.permissionsString());
    }

}     

void FileMetaDataProvider::insertFilesListBasicData()
{
  
    KFormat format;
    // If all directories
    Q_ASSERT(m_fileItems.count() > 1);
    bool allDirectories = true;
    for (const KFileItem& item : m_fileItems) {
        allDirectories &= item.isDir();
        if (!allDirectories) {
            break;
        }
    }

    if (allDirectories) {
        int count = 0;
        for (const KFileItem& item : m_fileItems) {
            count += subDirectoriesCount(item.url().path());
        }

        if (count) {
            const QString itemCountString = i18ncp("@item:intable", "%1 item", "%1 items", count);
            m_data.insert("kfileitem#totalSize", itemCountString);
        }
    } else {
        // Calculate the size of all items
        quint64 totalSize = 0;
        for (const KFileItem& item : m_fileItems) {
            if (!item.isDir() && !item.isLink()) {
                totalSize += item.size();
            }
        }
        m_data.insert("kfileitem#totalSize", format.formatByteSize(totalSize));
    }
}

void FileMetaDataProvider::insertBasicData()
{
    if (m_fileItems.count() > 1) {
      insertFilesListBasicData();
    } else {
      insertSingleFileBasicData();
    }
}

void FileMetaDataProvider::insertEditableData()
{
    if (!m_readOnly) {
        if (!m_data.contains("tags")) {
            m_data.insert("tags", QVariant());
        }
        if (!m_data.contains("rating")) {
            m_data.insert("rating", 0);
        }
        if (!m_data.contains("userComment")) {
            m_data.insert("userComment", QVariant());
        }
    }
}

void FileMetaDataProvider::insertCommonData(const QList<QVariantMap>& files)
{
    //
    // Only report the stuff that is common to all the files
    //
    QSet<QString> allProperties;
    QList<QVariantMap> propertyList;
    foreach (const QVariantMap& fileData, files) {
        propertyList << fileData;
        allProperties.unite(fileData.uniqueKeys().toSet());
    }

    // Special handling for certain properties
    totalPropertyAndInsert("duration", propertyList, allProperties);
    totalPropertyAndInsert("characterCount", propertyList, allProperties);
    totalPropertyAndInsert("wordCount", propertyList, allProperties);
    totalPropertyAndInsert("lineCount", propertyList, allProperties);

    foreach (const QString& propUri, allProperties) {
        foreach (const QVariantMap& map, propertyList) {
            QVariantMap::const_iterator it = map.find(propUri);
            if (it == map.constEnd()) {
                m_data.remove(propUri);
                break;
            } 
                
            QVariantMap::iterator dit = m_data.find(it.key());
            if (dit == m_data.end()) {
                m_data.insert(propUri, it.value());
            } else {
                QVariant finalValue = intersect(it.value(), dit.value());
                if (finalValue.isValid()) {
                    m_data[propUri] = finalValue;
                } else {
                    m_data.remove(propUri);
                    break;
                }
            }
        }
    }
}

FileMetaDataProvider::FileMetaDataProvider(QObject* parent)
    : QObject(parent)
    , m_readOnly(false)
    , m_realTimeIndexing(false)
{
}

FileMetaDataProvider::~FileMetaDataProvider()
{
}

void FileMetaDataProvider::setFileItem()
{
    // There are 3 code paths -
    // Remote file
    // Single local file -
    //   * Not Indexed
    //   * Indexed
    //
    const QUrl url = m_fileItems.first().targetUrl();
    if (!url.isLocalFile()) {
        insertBasicData();
        emit loadingFinished();
        return;
    }

    // Not Indexed
    const QString filePath = url.toLocalFile();
    if (!m_config.fileIndexingEnabled() || !m_config.shouldBeIndexed(filePath)) {
        m_realTimeIndexing = true;

        insertBasicData();
        insertEditableData();

        IndexedDataRetriever *ret = new IndexedDataRetriever(filePath, this);
        connect(ret, SIGNAL(finished(KJob*)), this, SLOT(slotLoadingFinished(KJob*)));
        ret->start();
        
    } else {
        FileFetchJob* job = new FileFetchJob(QStringList() << filePath, this);
        connect(job, SIGNAL(finished(KJob*)), this, SLOT(slotFileFetchFinished(KJob*)));
        job->start();
    }
}

void FileMetaDataProvider::setFileItems()
{
    // Multiple Files -
    //   * Not Indexed
    //   * Indexed

    QStringList urls;
    // Only extract data from indexed files, 
    // it would be too expensive otherwise.
    Q_FOREACH (const KFileItem& item, m_fileItems) {
        const QUrl url = item.targetUrl();
        if (url.isLocalFile()) {
            urls << url.toLocalFile();
        }
    }

    if (!urls.isEmpty()) {
        insertBasicData();
        
        FileFetchJob* job = new FileFetchJob(urls, this);
        connect(job, SIGNAL(finished(KJob*)), this, SLOT(slotFileFetchFinished(KJob*)));
        job->start();
    } else {
        insertBasicData();
        emit loadingFinished();
    }
}

void FileMetaDataProvider::setItems(const KFileItemList& items)
{
    m_fileItems = items;
    m_data.clear();
    m_realTimeIndexing = false;

    if (items.isEmpty()) {
        emit loadingFinished();
    } else if (items.size() == 1)  {
        setFileItem();
    } else {
        setFileItems();
    }
}

QString FileMetaDataProvider::label(const QString& metaDataLabel) const
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
        { "kfileitem#group", I18N_NOOP2_NOSTRIP("@label", "Group") },
        { "kfileitem#permissions", I18N_NOOP2_NOSTRIP("@label", "Permissions") },
        { "kfileitem#rating", I18N_NOOP2_NOSTRIP("@label", "Rating") },
        { "kfileitem#size", I18N_NOOP2_NOSTRIP("@label", "Size") },
        { "kfileitem#tags", I18N_NOOP2_NOSTRIP("@label", "Tags") },
        { "kfileitem#totalSize", I18N_NOOP2_NOSTRIP("@label", "Total Size") },
        { "kfileitem#type", I18N_NOOP2_NOSTRIP("@label", "Type") },
        { "tags", I18N_NOOP2_NOSTRIP("@label", "Tags") },
        { "rating", I18N_NOOP2_NOSTRIP("@label", "Rating") },
        { "userComment", I18N_NOOP2_NOSTRIP("@label", "Comment") },
        { "originUrl", I18N_NOOP2_NOSTRIP("@label", "Downloaded From") },
        { nullptr, nullptr, nullptr} // Mandatory last entry
    };

    static QHash<QString, QString> hash;
    if (hash.isEmpty()) {
        const TranslationItem* item = &translations[0];
        while (item->key != nullptr) {
            hash.insert(item->key, i18nc(item->context, item->value));
            ++item;
        }
    }

    QString value = hash.value(metaDataLabel);
    if (value.isEmpty()) {
        value = KFileMetaData::PropertyInfo::fromName(metaDataLabel).displayName();
    }

    return value;
}

QString FileMetaDataProvider::group(const QString& label) const
{
    static QHash<QString, QString> uriGrouper;
    if (uriGrouper.isEmpty()) {
        // KFileItem Data
        uriGrouper.insert(QLatin1String("kfileitem#type"), QLatin1String("0FileItemA"));
        uriGrouper.insert(QLatin1String("kfileitem#size"), QLatin1String("0FileItemB"));
        uriGrouper.insert(QLatin1String("kfileitem#totalSize"), QLatin1String("0FileItemB"));
        uriGrouper.insert(QLatin1String("kfileitem#modified"), QLatin1String("0FileItemC"));
        uriGrouper.insert(QLatin1String("kfileitem#owner"), QLatin1String("0FileItemD"));
        uriGrouper.insert(QLatin1String("kfileitem#group"), QLatin1String("0FileItemE"));
        uriGrouper.insert(QLatin1String("kfileitem#permissions"), QLatin1String("0FileItemF"));

        // Editable Data
        uriGrouper.insert(QLatin1String("tags"), QLatin1String("1EditableDataA"));
        uriGrouper.insert(QLatin1String("rating"), QLatin1String("1EditableDataB"));
        uriGrouper.insert(QLatin1String("userComment"), QLatin1String("1EditableDataC"));

        // Image Data
        uriGrouper.insert(QLatin1String("width"), QLatin1String("2SizA"));
        uriGrouper.insert(QLatin1String("height"), QLatin1String("2SizeB"));

        // Music Data
        uriGrouper.insert("title", QLatin1String("3MusicA"));
        uriGrouper.insert("artist", QLatin1String("3MusicB"));
        uriGrouper.insert("album", QLatin1String("3MusicC"));
        uriGrouper.insert("genre", QLatin1String("3MusicD"));
        uriGrouper.insert("trackNumber", QLatin1String("3MusicE"));

        // Audio Data
        uriGrouper.insert("duration", QLatin1String("4AudioA"));
        uriGrouper.insert("sampleRate", QLatin1String("4AudioB"));
        uriGrouper.insert("sampleCount", QLatin1String("4AudioC"));

        // Miscellaneous Data
        uriGrouper.insert("originUrl", QLatin1String("5MiscA"));
    }

    const QString val = uriGrouper.value(label);
    if (val.isEmpty()) {
        return "lastGroup";
    }
    return val;
}

KFileItemList FileMetaDataProvider::items() const
{
    return m_fileItems;
}

void FileMetaDataProvider::setReadOnly(bool readOnly)
{
    m_readOnly = readOnly;
}

bool FileMetaDataProvider::isReadOnly() const
{
    return m_readOnly;
}

QVariantMap FileMetaDataProvider::data() const
{
    return m_data;
}

int FileMetaDataProvider::subDirectoriesCount(const QString& path)
{
#ifdef Q_OS_WIN
    QDir dir(path);
    return dir.entryList(QDir::AllEntries|QDir::NoDotAndDotDot|QDir::System).count();
#else
    // Taken from kdelibs/kio/kio/kdirmodel.cpp
    // Copyright (C) 2006 David Faure <faure@kde.org>

    int count = -1;
    DIR* dir = ::opendir(QFile::encodeName(path));
    if (dir) {
        count = 0;
        struct dirent *dirEntry = nullptr;
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

bool FileMetaDataProvider::realTimeIndexing()
{
    return m_realTimeIndexing;
}
