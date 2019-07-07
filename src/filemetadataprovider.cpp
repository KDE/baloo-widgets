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
} // anonymous namespace

void FileMetaDataProvider::totalPropertyAndInsert(const QString& prop,
                                                  const QList<QVariantMap>& resources,
                                                  QSet<QString>& allProperties)
{
    if (allProperties.contains(prop)) {
        int total = 0;
        for (const QVariantMap& map : resources) {
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
    m_readOnly = !fetchJob->canEditAll();

    insertEditableData();
    emit loadingFinished();
}

void FileMetaDataProvider::slotLoadingFinished(KJob* job)
{
    IndexedDataRetriever* ret = dynamic_cast<IndexedDataRetriever*>(job);
    m_data = unite(m_data, ret->data());
    m_readOnly = !ret->canEdit();

    emit loadingFinished();
}

void FileMetaDataProvider::insertSingleFileBasicData()
{
    // TODO: Handle case if remote URLs are used properly. isDir() does
    // not work, the modification date needs also to be adjusted...
    Q_ASSERT(m_fileItems.count() <= 1);
    if (m_fileItems.count() == 1) {
      const KFileItem& item = m_fileItems.first();

      if (item.isDir()) {
          const int count = subDirectoriesCount(item.url().path());
          if (count == -1) {
              m_data.insert(QStringLiteral("kfileitem#size"), i18nc("unknown file size", "Unknown"));
          } else {
              const QString itemCountString = i18ncp("@item:intable", "%1 item", "%1 items", count);
              m_data.insert(QStringLiteral("kfileitem#size"), itemCountString);
          }
      } else {
          KFormat format;
          m_data.insert(QStringLiteral("kfileitem#size"), format.formatByteSize(item.size()));
      }

      m_data.insert(QStringLiteral("kfileitem#type"), item.mimeComment());
      m_data.insert(QStringLiteral("kfileitem#modified"), item.time(KFileItem::ModificationTime));
      QDateTime creationTime = item.time(KFileItem::CreationTime);
      if (creationTime.isValid()) {
          m_data.insert(QStringLiteral("kfileitem#created"), creationTime);
      }
      m_data.insert(QStringLiteral("kfileitem#accessed"), item.time(KFileItem::AccessTime));
      m_data.insert(QStringLiteral("kfileitem#owner"), item.user());
      m_data.insert(QStringLiteral("kfileitem#group"), item.group());
      m_data.insert(QStringLiteral("kfileitem#permissions"), item.permissionsString());
    }
}

void FileMetaDataProvider::insertFilesListBasicData()
{
    // If all directories
    Q_ASSERT(m_fileItems.count() > 1);
    bool allDirectories = true;
    for (const KFileItem& item : qAsConst(m_fileItems)) {
        allDirectories &= item.isDir();
        if (!allDirectories) {
            break;
        }
    }

    if (allDirectories) {
        int count = 0;
        for (const KFileItem& item : qAsConst(m_fileItems)) {
            count += subDirectoriesCount(item.url().path());
        }
        const QString itemCountString = i18ncp("@item:intable", "%1 item", "%1 items", count);
        m_data.insert(QStringLiteral("kfileitem#totalSize"), itemCountString);

    } else {
        // Calculate the size of all items
        quint64 totalSize = 0;
        for (const KFileItem& item : qAsConst(m_fileItems)) {
            if (!item.isDir() && !item.isLink()) {
                totalSize += item.size();
            }
        }
        KFormat format;
        m_data.insert(QStringLiteral("kfileitem#totalSize"), format.formatByteSize(totalSize));
    }
}

void FileMetaDataProvider::insertEditableData()
{
    if (!m_readOnly) {
        if (!m_data.contains(QStringLiteral("tags"))) {
            m_data.insert(QStringLiteral("tags"), QVariant());
        }
        if (!m_data.contains(QStringLiteral("rating"))) {
            m_data.insert(QStringLiteral("rating"), 0);
        }
        if (!m_data.contains(QStringLiteral("userComment"))) {
            m_data.insert(QStringLiteral("userComment"), QVariant());
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
    for (const QVariantMap& fileData : files) {
        propertyList << fileData;
        allProperties.unite(fileData.uniqueKeys().toSet());
    }

    // Special handling for certain properties
    totalPropertyAndInsert(QStringLiteral("duration"), propertyList, allProperties);
    totalPropertyAndInsert(QStringLiteral("characterCount"), propertyList, allProperties);
    totalPropertyAndInsert(QStringLiteral("wordCount"), propertyList, allProperties);
    totalPropertyAndInsert(QStringLiteral("lineCount"), propertyList, allProperties);

    for (const QString& propUri : qAsConst(allProperties)) {
        for (const QVariantMap& map : qAsConst(propertyList)) {
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
        // FIXME - are extended attributes supported for remote files?
        m_readOnly = true;
        insertSingleFileBasicData();
        emit loadingFinished();
        return;
    }

    // Not indexed or only basic file indexing (no content)
    const QString filePath = url.toLocalFile();
    if (!m_config.fileIndexingEnabled() || !m_config.shouldBeIndexed(filePath)
            || m_config.onlyBasicIndexing()) {
        m_realTimeIndexing = true;

        insertSingleFileBasicData();
        insertEditableData();

        IndexedDataRetriever *ret = new IndexedDataRetriever(filePath, this);
        connect(ret, &IndexedDataRetriever::finished, this, &FileMetaDataProvider::slotLoadingFinished);
        ret->start();

    // Fully indexed by Baloo
    } else {
        FileFetchJob* job = new FileFetchJob(QStringList{filePath}, true, this);
        connect(job, &FileFetchJob::finished, this, &FileMetaDataProvider::slotFileFetchFinished);
        job->start();
    }
}

void FileMetaDataProvider::setFileItems()
{
    // Multiple Files -
    //   * Not Indexed
    //   * Indexed

    QStringList urls;
    urls.reserve(m_fileItems.size());
    // Only extract data from indexed files,
    // it would be too expensive otherwise.
    for (const KFileItem& item : qAsConst(m_fileItems)) {
        const QUrl url = item.targetUrl();
        if (url.isLocalFile()) {
            urls << url.toLocalFile();
        }
    }

    insertFilesListBasicData();
    if (!urls.isEmpty()) {
        // Editing only if all URLs are local
        bool canEdit = (urls.size() == m_fileItems.size());

        FileFetchJob* job = new FileFetchJob(urls, canEdit, this);
        connect(job, &FileFetchJob::finished, this, &FileMetaDataProvider::slotFileFetchFinished);
        job->start();

    } else {
        // FIXME - are extended attributes supported for remote files?
        m_readOnly = true;
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
    static QHash<QString, QString> hash = {
        { QStringLiteral("kfileitem#comment"), i18nc("@label", "Comment") },
        { QStringLiteral("kfileitem#created"), i18nc("@label", "Created") },
        { QStringLiteral("kfileitem#accessed"), i18nc("@label", "Accessed") },
        { QStringLiteral("kfileitem#modified"), i18nc("@label", "Modified") },
        { QStringLiteral("kfileitem#owner"), i18nc("@label", "Owner") },
        { QStringLiteral("kfileitem#group"), i18nc("@label", "Group") },
        { QStringLiteral("kfileitem#permissions"), i18nc("@label", "Permissions") },
        { QStringLiteral("kfileitem#rating"), i18nc("@label", "Rating") },
        { QStringLiteral("kfileitem#size"), i18nc("@label", "Size") },
        { QStringLiteral("kfileitem#tags"), i18nc("@label", "Tags") },
        { QStringLiteral("kfileitem#totalSize"), i18nc("@label", "Total Size") },
        { QStringLiteral("kfileitem#type"), i18nc("@label", "Type") },
        { QStringLiteral("tags"), i18nc("@label", "Tags") },
        { QStringLiteral("rating"), i18nc("@label", "Rating") },
        { QStringLiteral("userComment"), i18nc("@label", "Comment") },
        { QStringLiteral("originUrl"), i18nc("@label", "Downloaded From") },
    };

    QString value = hash.value(metaDataLabel);
    if (value.isEmpty()) {
        value = KFileMetaData::PropertyInfo::fromName(metaDataLabel).displayName();
    }

    return value;
}

QString FileMetaDataProvider::group(const QString& label) const
{
    static QHash<QString, QString> uriGrouper = {

        // KFileItem Data
        { QStringLiteral("kfileitem#type"), QStringLiteral("0FileItemA") },
        { QStringLiteral("kfileitem#size"), QStringLiteral("0FileItemB") },
        { QStringLiteral("kfileitem#totalSize"), QStringLiteral("0FileItemB") },
        { QStringLiteral("kfileitem#modified"), QStringLiteral("0FileItemC") },
        { QStringLiteral("kfileitem#accessed"), QStringLiteral("0FileItemD") },
        { QStringLiteral("kfileitem#created"), QStringLiteral("0FileItemE") },
        { QStringLiteral("kfileitem#owner"), QStringLiteral("0FileItemF") },
        { QStringLiteral("kfileitem#group"), QStringLiteral("0FileItemG") },
        { QStringLiteral("kfileitem#permissions"), QStringLiteral("0FileItemH") },

        // Editable Data
        { QStringLiteral("tags"), QStringLiteral("1EditableDataA") },
        { QStringLiteral("rating"), QStringLiteral("1EditableDataB") },
        { QStringLiteral("userComment"), QStringLiteral("1EditableDataC") },

        // Image Data
        { QStringLiteral("width"), QStringLiteral("2ImageA") },
        { QStringLiteral("height"), QStringLiteral("2ImageB") },
        { QStringLiteral("photoFNumber"), QStringLiteral("2ImageC") },
        { QStringLiteral("photoExposureTime"), QStringLiteral("2ImageD") },
        { QStringLiteral("photoExposureBiasValue"), QStringLiteral("2ImageE") },
        { QStringLiteral("photoISOSpeedRatings"), QStringLiteral("2ImageF") },
        { QStringLiteral("photoFocalLength"), QStringLiteral("2ImageG") },
        { QStringLiteral("photoFocalLengthIn35mmFilm"), QStringLiteral("2ImageH") },
        { QStringLiteral("photoFlash"), QStringLiteral("2ImageI") },
        { QStringLiteral("imageOrientation"), QStringLiteral("2ImageJ") },
        { QStringLiteral("photoGpsLatitude"), QStringLiteral("2ImageK") },
        { QStringLiteral("photoGpsLongitude"), QStringLiteral("2ImageL") },
        { QStringLiteral("photoGpsAltitude"), QStringLiteral("2ImageM") },
        { QStringLiteral("manufacturer"), QStringLiteral("2ImageN") },
        { QStringLiteral("model"), QStringLiteral("2ImageO") },

        // Media Data
        { QStringLiteral("title"), QStringLiteral("3MediaA") },
        { QStringLiteral("artist"), QStringLiteral("3MediaB") },
        { QStringLiteral("album"), QStringLiteral("3MediaC") },
        { QStringLiteral("albumArtist"), QStringLiteral("3MediaD") },
        { QStringLiteral("genre"), QStringLiteral("3MediaE") },
        { QStringLiteral("trackNumber"), QStringLiteral("3MediaF") },
        { QStringLiteral("discNumber"), QStringLiteral("3MediaG") },
        { QStringLiteral("releaseYear"), QStringLiteral("3MediaH") },
        { QStringLiteral("duration"), QStringLiteral("3MediaI") },
        { QStringLiteral("sampleRate"), QStringLiteral("3MediaJ") },
        { QStringLiteral("bitRate"), QStringLiteral("3MediaK") },

        // Miscellaneous Data
        { QStringLiteral("originUrl"), QStringLiteral("4MiscA") },
    };

    const QString val = uriGrouper.value(label);
    if (val.isEmpty()) {
        return QStringLiteral("lastGroup");
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
    DIR* dir = ::opendir(QFile::encodeName(path).constData());
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
