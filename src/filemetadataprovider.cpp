/*
    SPDX-FileCopyrightText: 2010 Peter Penz <peter.penz@gmx.at>
    SPDX-FileCopyrightText: 2012 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2021 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "filemetadataprovider.h"
#include "filemetadatautil_p.h"
#include "filefetchjob.h"

#include <KFileMetaData/PropertyInfo>
#include <KFormat>
#include <KLocalizedString>
#include <KProtocolInfo>
#include <KShell>

#include <QPair>

// Required includes for subDirectoriesCount():
#ifdef Q_OS_WIN
#include <QDir>
#else
#include <QFile>
#include <dirent.h>
#endif

using namespace Baloo;

namespace
{
/**
 * The standard QMap::unite will contain the key multiple times if both \p v1 and \p v2
 * contain the same key.
 *
 * This will only take the key from \p v2 into account
 */
QVariantMap unite(const QVariantMap &v1, const QVariantMap &v2)
{
    QVariantMap v(v1);
    QMapIterator<QString, QVariant> it(v2);
    while (it.hasNext()) {
        it.next();

        v[it.key()] = it.value();
    }

    return v;
}

/**
* @return The number of files and hidden files for the directory path.
*/
QPair<int, int> subDirectoriesCount(const QString &path)
{
#ifdef Q_OS_WIN
    QDir dir(path);
    int count = dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System).count();
    int hiddenCount = dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden).count();
    return QPair<int, int>(count, hiddenCount);
#else
    // Taken from kdelibs/kio/kio/kdirmodel.cpp
    // SPDX-FileCopyrightText: 2006 David Faure <faure@kde.org>

    int count = -1;
    int hiddenCount = -1;
    DIR *dir = ::opendir(QFile::encodeName(path).constData());
    if (dir) {
        count = 0;
        hiddenCount = 0;
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
                // hidden files
                hiddenCount++;
            } else {
                ++count;
            }
        }
        ::closedir(dir);
    }
    return QPair<int, int>(count, hiddenCount);
#endif
}
} // anonymous namespace

void FileMetaDataProvider::slotFileFetchFinished(KJob *job)
{
    auto fetchJob = static_cast<FileFetchJob *>(job);
    QList<QVariantMap> files = fetchJob->data();

    Q_ASSERT(!files.isEmpty());

    if (files.size() > 1) {
        Baloo::Private::mergeCommonData(m_data, files);
    } else {
        m_data = unite(m_data, files.first());

        const auto width = m_data.value(QStringLiteral("width"));
        const auto height = m_data.value(QStringLiteral("height"));
        if ((width.type() == QVariant::Double || width.type() == QVariant::Int) && (height.type() == QVariant::Double || height.type() == QVariant::Int)) {
            m_data.insert(QStringLiteral("dimensions"), i18nc("width × height", "%1 × %2", width.toInt(), height.toInt()));
        }

        bool okLatitude;
        const auto gpsLatitude = m_data.value(QStringLiteral("photoGpsLatitude")).toFloat(&okLatitude);
        bool okLongitude;
        const auto gpsLongitude = m_data.value(QStringLiteral("photoGpsLongitude")).toFloat(&okLongitude);

        if (okLatitude && okLongitude) {
            m_data.insert(QStringLiteral("gpsLocation"), QVariant::fromValue(QPair<float, float>(gpsLatitude, gpsLongitude)));
        }
    }
    m_readOnly = !fetchJob->canEditAll();

    insertEditableData();
    Q_EMIT loadingFinished();
}

void FileMetaDataProvider::insertSingleFileBasicData()
{
    // TODO: Handle case if remote URLs are used properly. isDir() does
    // not work, the modification date needs also to be adjusted...
    Q_ASSERT(m_fileItems.count() == 1);
    {
        const KFileItem &item = m_fileItems.first();

        KFormat format;
        if (item.isDir()) {
            bool isSizeUnknown = !item.isLocalFile() || item.isSlow();
            if (!isSizeUnknown) {
                const QPair<int, int> counts = subDirectoriesCount(item.url().path());
                const int count = counts.first;
                isSizeUnknown = count == -1;
                if (!isSizeUnknown) {
                    QString itemCountString = i18ncp("@item:intable", "%1 item", "%1 items", count);
                    m_data.insert(QStringLiteral("kfileitem#size"), itemCountString);

                    const int hiddenCount = counts.second;
                    if (hiddenCount > 0) {
                        // add hidden items count
                        QString hiddenCountString = i18ncp("@item:intable", "%1 item", "%1 items", hiddenCount);
                        m_data.insert(QStringLiteral("kfileitem#hiddenItems"), hiddenCountString);
                    }
                }
            } else if (item.entry().contains(KIO::UDSEntry::UDS_SIZE)) {
                m_data.insert(QStringLiteral("kfileitem#size"), format.formatByteSize(item.size()));
            }
            if (item.entry().contains(KIO::UDSEntry::UDS_RECURSIVE_SIZE)) {
                m_data.insert(QStringLiteral("kfileitem#totalSize"), format.formatByteSize(item.recursiveSize()));
            }
        } else {
            if (item.entry().contains(KIO::UDSEntry::UDS_SIZE)) {
                m_data.insert(QStringLiteral("kfileitem#size"), format.formatByteSize(item.size()));
            }
        }

        m_data.insert(QStringLiteral("kfileitem#type"), item.mimeComment());
        if (item.isLink()) {
            m_data.insert(QStringLiteral("kfileitem#linkDest"), item.linkDest());
        }
        if (item.entry().contains(KIO::UDSEntry::UDS_TARGET_URL)) {
            m_data.insert(QStringLiteral("kfileitem#targetUrl"), KShell::tildeCollapse(item.targetUrl().toDisplayString(QUrl::PreferLocalFile)));
        }
        QDateTime modificationTime = item.time(KFileItem::ModificationTime);
        if (modificationTime.isValid()) {
            m_data.insert(QStringLiteral("kfileitem#modified"), modificationTime);
        }
        QDateTime creationTime = item.time(KFileItem::CreationTime);
        if (creationTime.isValid()) {
            m_data.insert(QStringLiteral("kfileitem#created"), creationTime);
        }
        QDateTime accessTime = item.time(KFileItem::AccessTime);
        if (accessTime.isValid()) {
            m_data.insert(QStringLiteral("kfileitem#accessed"), accessTime);
        }

        m_data.insert(QStringLiteral("kfileitem#owner"), item.user());
        m_data.insert(QStringLiteral("kfileitem#group"), item.group());
        m_data.insert(QStringLiteral("kfileitem#permissions"), item.permissionsString());

        const auto extraFields = KProtocolInfo::extraFields(item.url());
        for (int i = 0; i < extraFields.count(); ++i) {
            const auto &field = extraFields.at(i);
            if (field.type == KProtocolInfo::ExtraField::Invalid) {
                continue;
            }

            const QString text = item.entry().stringValue(KIO::UDSEntry::UDS_EXTRA + i);
            if (text.isEmpty()) {
                continue;
            }

            const QString key = QStringLiteral("kfileitem#extra_%1_%2").arg(item.url().scheme(), QString::number(i + 1));

            if (field.type == KProtocolInfo::ExtraField::DateTime) {
                const QDateTime date = QDateTime::fromString(text, Qt::ISODate);
                if (!date.isValid()) {
                    continue;
                }

                m_data.insert(key, date);
            } else {
                m_data.insert(key, text);
            }
        }
    }
}

void FileMetaDataProvider::insertFilesListBasicData()
{
    // If all directories
    Q_ASSERT(m_fileItems.count() > 1);
    bool allDirectories = true;
    for (const KFileItem &item : std::as_const(m_fileItems)) {
        allDirectories &= item.isDir();
        if (!allDirectories) {
            break;
        }
    }

    if (allDirectories) {
        int count = 0;
        int hiddenCount = 0;
        for (const KFileItem &item : std::as_const(m_fileItems)) {
            if (!item.isLocalFile() || item.isSlow()) {
                return;
            }
            const QPair<int, int> counts = subDirectoriesCount(item.url().path());
            const int subcount = counts.first;
            if (subcount == -1) {
                return;
            }
            count += subcount;
            hiddenCount += counts.second;
        }
        QString itemCountString = i18ncp("@item:intable", "%1 item", "%1 items", count);
        if (hiddenCount > 0) {
            // add hidden items count
            QString hiddenCountString = i18ncp("@item:intable", "%1 item", "%1 items", hiddenCount);
            m_data.insert(QStringLiteral("kfileitem#hiddenItems"), hiddenCountString);
        }
        m_data.insert(QStringLiteral("kfileitem#totalSize"), itemCountString);

    } else {
        // Calculate the size of all items
        quint64 totalSize = 0;
        for (const KFileItem &item : std::as_const(m_fileItems)) {
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

FileMetaDataProvider::FileMetaDataProvider(QObject *parent)
    : QObject(parent)
    , m_readOnly(false)
{
}

FileMetaDataProvider::~FileMetaDataProvider() = default;

void FileMetaDataProvider::setFileItem()
{
    // There are 3 code paths -
    // Remote file
    // Single local file -
    //   * Not Indexed
    //   * Indexed
    //
    insertSingleFileBasicData();
    const QUrl url = m_fileItems.first().targetUrl();
    if (!url.isLocalFile() || m_fileItems.first().isSlow()) {
        // FIXME - are extended attributes supported for remote files?
        m_readOnly = true;
        Q_EMIT loadingFinished();
        return;
    }

    const QString filePath = url.toLocalFile();
    FileFetchJob *job;

    // Not indexed or only basic file indexing (no content)
    if (!m_config.fileIndexingEnabled() || !m_config.shouldBeIndexed(filePath) || m_config.onlyBasicIndexing()) {
        job = new FileFetchJob(QStringList{filePath}, true, FileFetchJob::UseRealtimeIndexing::Only, this);

        // Fully indexed by Baloo
    } else {
        job = new FileFetchJob(QStringList{filePath}, true, FileFetchJob::UseRealtimeIndexing::Fallback, this);
    }
    connect(job, &FileFetchJob::finished, this, &FileMetaDataProvider::slotFileFetchFinished);
    job->start();
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
    for (const KFileItem &item : std::as_const(m_fileItems)) {
        const QUrl url = item.targetUrl();
        if (url.isLocalFile() && !item.isSlow()) {
            urls << url.toLocalFile();
        }
    }

    insertFilesListBasicData();
    if (!urls.isEmpty()) {
        // Editing only if all URLs are local
        bool canEdit = (urls.size() == m_fileItems.size());

        auto job = new FileFetchJob(urls, canEdit, FileFetchJob::UseRealtimeIndexing::Disabled, this);
        connect(job, &FileFetchJob::finished, this, &FileMetaDataProvider::slotFileFetchFinished);
        job->start();

    } else {
        // FIXME - are extended attributes supported for remote files?
        m_readOnly = true;
        Q_EMIT loadingFinished();
    }
}

void FileMetaDataProvider::setItems(const KFileItemList &items)
{
    m_fileItems = items;
    m_data.clear();

    if (items.isEmpty()) {
        Q_EMIT loadingFinished();
    } else if (items.size() == 1) {
        setFileItem();
    } else {
        setFileItems();
    }
}

QString FileMetaDataProvider::label(const QString &metaDataLabel) const
{
    static QHash<QString, QString> hash = {
        {QStringLiteral("kfileitem#comment"), i18nc("@label", "Comment")},
        {QStringLiteral("kfileitem#created"), i18nc("@label", "Created")},
        {QStringLiteral("kfileitem#accessed"), i18nc("@label", "Accessed")},
        {QStringLiteral("kfileitem#modified"), i18nc("@label", "Modified")},
        {QStringLiteral("kfileitem#owner"), i18nc("@label", "Owner")},
        {QStringLiteral("kfileitem#group"), i18nc("@label", "Group")},
        {QStringLiteral("kfileitem#permissions"), i18nc("@label", "Permissions")},
        {QStringLiteral("kfileitem#rating"), i18nc("@label", "Rating")},
        {QStringLiteral("kfileitem#size"), i18nc("@label", "Size")},
        {QStringLiteral("kfileitem#tags"), i18nc("@label", "Tags")},
        {QStringLiteral("kfileitem#totalSize"), i18nc("@label", "Total Size")},
        {QStringLiteral("kfileitem#hiddenItems"), i18nc("@label", "Hidden items")},
        {QStringLiteral("kfileitem#type"), i18nc("@label", "Type")},
        {QStringLiteral("kfileitem#linkDest"), i18nc("@label", "Link to")},
        {QStringLiteral("kfileitem#targetUrl"), i18nc("@label", "Points to")},
        {QStringLiteral("tags"), i18nc("@label", "Tags")},
        {QStringLiteral("rating"), i18nc("@label", "Rating")},
        {QStringLiteral("userComment"), i18nc("@label", "Comment")},
        {QStringLiteral("originUrl"), i18nc("@label", "Downloaded From")},
        {QStringLiteral("dimensions"), i18nc("@label", "Dimensions")},
        {QStringLiteral("gpsLocation"), i18nc("@label", "GPS Location")},
    };

    QString value = hash.value(metaDataLabel);
    if (value.isEmpty()) {
        static const auto extraPrefix = QStringLiteral("kfileitem#extra_");
        if (metaDataLabel.startsWith(extraPrefix)) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            const auto parts = metaDataLabel.splitRef(QLatin1Char('_'));
#else
            const auto parts = metaDataLabel.split(QLatin1Char('_'));
#endif
            Q_ASSERT(parts.count() == 3);
            const auto protocol = parts.at(1);
            const int extraNumber = parts.at(2).toInt() - 1;

            // Have to construct a dummy URL for KProtocolInfo::extraFields...
            QUrl url;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            url.setScheme(protocol.toString());
#else
            url.setScheme(protocol);
#endif
            const auto extraFields = KProtocolInfo::extraFields(url);
            auto field = extraFields.value(extraNumber);
            if (field.type != KProtocolInfo::ExtraField::Invalid) {
                value = field.name;
            }
        }
    }

    if (value.isEmpty()) {
        value = KFileMetaData::PropertyInfo::fromName(metaDataLabel).displayName();
    }

    return value;
}

QString FileMetaDataProvider::group(const QString &label) const
{
    static QHash<QString, QString> uriGrouper = {

        // KFileItem Data
        {QStringLiteral("kfileitem#type"), QStringLiteral("0FileItemA")},
        {QStringLiteral("kfileitem#linkDest"), QStringLiteral("0FileItemB")},
        {QStringLiteral("kfileitem#size"), QStringLiteral("0FileItemC")},
        {QStringLiteral("kfileitem#totalSize"), QStringLiteral("0FileItemC")},
        {QStringLiteral("kfileitem#hiddenItems"), QStringLiteral("0FileItemD")},
        {QStringLiteral("kfileitem#modified"), QStringLiteral("0FileItemE")},
        {QStringLiteral("kfileitem#accessed"), QStringLiteral("0FileItemF")},
        {QStringLiteral("kfileitem#created"), QStringLiteral("0FileItemG")},
        {QStringLiteral("kfileitem#owner"), QStringLiteral("0FileItemH")},
        {QStringLiteral("kfileitem#group"), QStringLiteral("0FileItemI")},
        {QStringLiteral("kfileitem#permissions"), QStringLiteral("0FileItemJ")},

        // Editable Data
        {QStringLiteral("tags"), QStringLiteral("1EditableDataA")},
        {QStringLiteral("rating"), QStringLiteral("1EditableDataB")},
        {QStringLiteral("userComment"), QStringLiteral("1EditableDataC")},

        // Image Data
        {QStringLiteral("width"), QStringLiteral("2ImageA")},
        {QStringLiteral("height"), QStringLiteral("2ImageB")},
        {QStringLiteral("dimensions"), QStringLiteral("2ImageCA")},
        {QStringLiteral("photoFNumber"), QStringLiteral("2ImageC")},
        {QStringLiteral("photoExposureTime"), QStringLiteral("2ImageD")},
        {QStringLiteral("photoExposureBiasValue"), QStringLiteral("2ImageE")},
        {QStringLiteral("photoISOSpeedRatings"), QStringLiteral("2ImageF")},
        {QStringLiteral("photoFocalLength"), QStringLiteral("2ImageG")},
        {QStringLiteral("photoFocalLengthIn35mmFilm"), QStringLiteral("2ImageH")},
        {QStringLiteral("photoFlash"), QStringLiteral("2ImageI")},
        {QStringLiteral("imageOrientation"), QStringLiteral("2ImageJ")},
        {QStringLiteral("photoGpsLocation"), QStringLiteral("2ImageK")},
        {QStringLiteral("photoGpsLatitude"), QStringLiteral("2ImageL")},
        {QStringLiteral("photoGpsLongitude"), QStringLiteral("2ImageM")},
        {QStringLiteral("photoGpsAltitude"), QStringLiteral("2ImageN")},
        {QStringLiteral("manufacturer"), QStringLiteral("2ImageO")},
        {QStringLiteral("model"), QStringLiteral("2ImageP")},

        // Media Data
        {QStringLiteral("title"), QStringLiteral("3MediaA")},
        {QStringLiteral("artist"), QStringLiteral("3MediaB")},
        {QStringLiteral("album"), QStringLiteral("3MediaC")},
        {QStringLiteral("albumArtist"), QStringLiteral("3MediaD")},
        {QStringLiteral("genre"), QStringLiteral("3MediaE")},
        {QStringLiteral("trackNumber"), QStringLiteral("3MediaF")},
        {QStringLiteral("discNumber"), QStringLiteral("3MediaG")},
        {QStringLiteral("releaseYear"), QStringLiteral("3MediaH")},
        {QStringLiteral("duration"), QStringLiteral("3MediaI")},
        {QStringLiteral("sampleRate"), QStringLiteral("3MediaJ")},
        {QStringLiteral("bitRate"), QStringLiteral("3MediaK")},

        // Miscellaneous Data
        {QStringLiteral("originUrl"), QStringLiteral("4MiscA")},
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
