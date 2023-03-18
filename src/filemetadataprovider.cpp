/*
    SPDX-FileCopyrightText: 2010 Peter Penz <peter.penz@gmx.at>
    SPDX-FileCopyrightText: 2012 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2021 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "filemetadataprovider.h"
#include "filemetadatautil_p.h"
#include "filefetchjob.h"

#include <Baloo/IndexerConfig>
#include <KFileMetaData/PropertyInfo>
#include <KFormat>
#include <KLocalizedString>
#include <KProtocolInfo>
#include <KShell>
#include <ThreadWeaver/ThreadWeaver>

#include <QPair>
#include <QSize>

// Required includes for subDirectoriesCount():
#ifdef Q_OS_WIN
#include <QDir>
#else
#include <QFile>
#include <dirent.h>
#endif

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>

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

/**
 * Fill \p data with properties can be derived from others
 */
void extractDerivedProperties(QVariantMap &data)
{
    const auto width = data.value(QStringLiteral("width"));
    const auto height = data.value(QStringLiteral("height"));
    if (!width.isNull() && !height.isNull()) {
        data.insert(QStringLiteral("dimensions"), QSize(width.toInt(), height.toInt()));
    }

    bool okLatitude;
    const auto gpsLatitude = data.value(QStringLiteral("photoGpsLatitude")).toFloat(&okLatitude);
    bool okLongitude;
    const auto gpsLongitude = data.value(QStringLiteral("photoGpsLongitude")).toFloat(&okLongitude);

    if (okLatitude && okLongitude) {
        data.insert(QStringLiteral("gpsLocation"), QVariant::fromValue(QPair<float, float>(gpsLatitude, gpsLongitude)));
    }
}
} // anonymous namespace

class Q_DECL_HIDDEN Baloo::FileMetaDataProviderPrivate : public QObject
{
public:
    enum Status { PROCESSING, CANCELLING, CANCELED, ERRORED, FINISHED, ABORT };

    FileMetaDataProviderPrivate(FileMetaDataProvider *parent, std::shared_ptr<Baloo::IndexerConfig> indexerConfig, bool async)
        : QObject(parent)
        , m_parent(parent)
        , m_readOnly(false)
        , m_fetchJob(nullptr)
        , m_config(indexerConfig)
        , m_isAsync(async)
        , m_status(FINISHED)
        , m_uses(0)
        , m_shouldAbort(false)
    {
        if (!m_config) {
            m_config = std::make_shared<Baloo::IndexerConfig>();
        }
    }

    ~FileMetaDataProviderPrivate()
    {
        m_shouldAbort = true;
        cancelSync();

        // Ensure that all threads have stopped all processing
        // and no one is using us
        std::lock_guard processingLock(m_processingMutex);
        std::lock_guard dataLock(m_dataMutex);
        std::lock_guard statusLock(m_statusMutex);

        Q_ASSERT(m_status == CANCELED || m_status == ERRORED || m_status == FINISHED);

        std::unique_lock lock(m_usesMutex);
        if (m_uses > 0) {
            // Wait all queued tasks have finished
            m_usesChange.wait(lock, [this] {
                return m_uses <= 0;
            });
        }
    }

    void insertEditableData(QVariantMap &data);

    QVariantMap processFileItems(std::shared_ptr<Baloo::IndexerConfig> indexerConfig, const KFileItemList &items);

    /**
     * Updates `data` with loaded metadata from given FileFetchJob
     * returns true if it's editable, otherwise false
     */
    bool processFetchResult(FileFetchJob *fetchJob, QVariantMap &data);

    void setFileItem();
    void setFileItems();

    /**
     * Insert basic data of a single file
     */
    void insertSingleFileBasicData(const KFileItemList &items, QVariantMap &data);

    /**
     * Insert basic data of a list of files
     */
    void insertFilesListBasicData(const KFileItemList &items, QVariantMap &data);

    void finish(const QVariantMap &data);
    void doRefresh(const std::chrono::milliseconds &skipMs = std::chrono::milliseconds(0));
    void cancelSync();
    void refreshSync(std::shared_ptr<Baloo::IndexerConfig> indexerConfig, const std::chrono::milliseconds &skipMs);
    bool updateStatus(Status newStatus);

    FileMetaDataProvider *m_parent;

    std::atomic_bool m_readOnly;

    QList<KFileItem> m_fileItems;

    QVariantMap m_data;
    std::shared_ptr<Baloo::IndexerConfig> m_config;

    FileFetchJob *m_fetchJob;

    bool m_isAsync;
    mutable std::mutex m_dataMutex;
    std::timed_mutex m_processingMutex;

    Status m_status;
    std::mutex m_statusMutex;
    std::atomic_int m_uses;
    std::mutex m_usesMutex;
    std::condition_variable m_usesChange;
    std::atomic_bool m_shouldAbort;
};

bool FileMetaDataProviderPrivate::processFetchResult(FileFetchJob *fetchJob, QVariantMap &data)
{
    QList<QVariantMap> files = fetchJob->data();

    Q_ASSERT(!files.isEmpty());

    if (files.size() > 1) {
        Baloo::Private::mergeCommonData(data, files);
    } else {
        data = unite(data, files.first());
    }
    extractDerivedProperties(data);

    if (fetchJob->canEditAll()) {
        insertEditableData(data);
        return true;
    }

    return false;
}

void FileMetaDataProviderPrivate::insertSingleFileBasicData(const KFileItemList &items, QVariantMap &data)
{
    // TODO: Handle case if remote URLs are used properly. isDir() does
    // not work, the modification date needs also to be adjusted...
    Q_ASSERT(items.count() <= 1);
    if (items.count() == 1) {
        const KFileItem &item = items.first();

        KFormat format;
        if (item.isDir()) {
            if (item.isLocalFile() && !item.isSlow()) {
                const QPair<int, int> counts = subDirectoriesCount(item.url().path());
                const int count = counts.first;
                if (count != -1) {
                    QString itemCountString = i18ncp("@item:intable", "%1 item", "%1 items", count);
                    data.insert(QStringLiteral("kfileitem#size"), itemCountString);

                    const int hiddenCount = counts.second;
                    if (hiddenCount > 0) {
                        // add hidden items count
                        QString hiddenCountString = i18ncp("@item:intable", "%1 item", "%1 items", hiddenCount);
                        data.insert(QStringLiteral("kfileitem#hiddenItems"), hiddenCountString);
                    }
                }
            } else if (item.entry().contains(KIO::UDSEntry::UDS_SIZE)) {
                data.insert(QStringLiteral("kfileitem#size"), format.formatByteSize(item.size()));
            }
            if (item.entry().contains(KIO::UDSEntry::UDS_RECURSIVE_SIZE)) {
                data.insert(QStringLiteral("kfileitem#totalSize"), format.formatByteSize(item.recursiveSize()));
            }
        } else {
            if (item.entry().contains(KIO::UDSEntry::UDS_SIZE)) {
                data.insert(QStringLiteral("kfileitem#size"), format.formatByteSize(item.size()));
            }
        }

        data.insert(QStringLiteral("kfileitem#type"), item.mimeComment());
        if (item.isLink()) {
            data.insert(QStringLiteral("kfileitem#linkDest"), item.linkDest());
        }
        if (item.entry().contains(KIO::UDSEntry::UDS_TARGET_URL)) {
            data.insert(QStringLiteral("kfileitem#targetUrl"), KShell::tildeCollapse(item.targetUrl().toDisplayString(QUrl::PreferLocalFile)));
        }
        QDateTime modificationTime = item.time(KFileItem::ModificationTime);
        if (modificationTime.isValid()) {
            data.insert(QStringLiteral("kfileitem#modified"), modificationTime);
        }
        QDateTime creationTime = item.time(KFileItem::CreationTime);
        if (creationTime.isValid()) {
            data.insert(QStringLiteral("kfileitem#created"), creationTime);
        }
        QDateTime accessTime = item.time(KFileItem::AccessTime);
        if (accessTime.isValid()) {
            data.insert(QStringLiteral("kfileitem#accessed"), accessTime);
        }

        data.insert(QStringLiteral("kfileitem#owner"), item.user());
        data.insert(QStringLiteral("kfileitem#group"), item.group());
        data.insert(QStringLiteral("kfileitem#permissions"), item.permissionsString());

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

                data.insert(key, date);
            } else {
                data.insert(key, text);
            }
        }
    }
}

void FileMetaDataProviderPrivate::insertFilesListBasicData(const KFileItemList &items, QVariantMap &data)
{
    // If all directories
    Q_ASSERT(items.count() > 1);
    bool allDirectories = true;
    for (const KFileItem &item : std::as_const(items)) {
        allDirectories &= item.isDir();
        if (!allDirectories) {
            break;
        }
    }

    if (allDirectories) {
        int count = 0;
        int hiddenCount = 0;
        for (const KFileItem &item : std::as_const(items)) {
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
            data.insert(QStringLiteral("kfileitem#hiddenItems"), hiddenCountString);
        }
        data.insert(QStringLiteral("kfileitem#totalSize"), itemCountString);

    } else {
        // Calculate the size of all items
        quint64 totalSize = 0;
        for (const KFileItem &item : std::as_const(items)) {
            if (!item.isDir() && !item.isLink()) {
                totalSize += item.size();
            }
        }
        KFormat format;
        data.insert(QStringLiteral("kfileitem#totalSize"), format.formatByteSize(totalSize));
    }
}

void FileMetaDataProviderPrivate::insertEditableData(QVariantMap &data)
{
    if (!data.contains(QStringLiteral("tags"))) {
        data.insert(QStringLiteral("tags"), QVariant());
    }
    if (!data.contains(QStringLiteral("rating"))) {
        data.insert(QStringLiteral("rating"), 0);
    }
    if (!data.contains(QStringLiteral("userComment"))) {
        data.insert(QStringLiteral("userComment"), QVariant());
    }
}

FileMetaDataProvider::FileMetaDataProvider(QObject *parent, std::shared_ptr<Baloo::IndexerConfig> indexerConfig, bool async)
    : QObject(parent)
    , d(new FileMetaDataProviderPrivate(this, indexerConfig, async))
{
}

FileMetaDataProvider::~FileMetaDataProvider() = default;

QVariantMap FileMetaDataProviderPrivate::processFileItems(std::shared_ptr<Baloo::IndexerConfig> indexerConfig, const KFileItemList &items)
{
    // There are several code paths -
    // Remote file
    // Single local file -
    //   * Not Indexed
    //   * Indexed
    //
    // Multiple Files -
    //   * Not Indexed
    //   * Indexed

    auto data = QVariantMap();
    std::lock_guard processingLock(m_processingMutex);
    if (!updateStatus(PROCESSING)) {
        return data;
    }

    if (items.isEmpty()) {
        updateStatus(FINISHED);
        return data;
    }

    bool singleFileMode = m_fileItems.size() <= 1;

    QStringList urls;
    urls.reserve(items.size());

    // Only extract data from indexed files,
    // it would be too expensive otherwise.
    for (const KFileItem &item : std::as_const(items)) {
        const QUrl url = item.targetUrl();
        if (url.isLocalFile() && !item.isSlow()) {
            urls << url.toLocalFile();
        }
    }

    if (singleFileMode) {
        insertSingleFileBasicData(items, data);
    } else {
        insertFilesListBasicData(items, data);
    }

    auto finalStatus = FINISHED;
    if (!urls.isEmpty()) {
        // Editing only if all URLs are local
        bool canEdit = (urls.size() == items.size());

        // Don't use indexing when we have multiple files
        auto indexingMode = FileFetchJob::UseRealtimeIndexing::Disabled;

        if (singleFileMode) {
            // Fully indexed by Baloo
            indexingMode = FileFetchJob::UseRealtimeIndexing::Fallback;

            if (!indexerConfig->fileIndexingEnabled() || !indexerConfig->shouldBeIndexed(urls.first()) || indexerConfig->onlyBasicIndexing()) {
                // Not indexed or only basic file indexing (no content)
                indexingMode = FileFetchJob::UseRealtimeIndexing::Only;
            }
        }

        if (!updateStatus(PROCESSING)) {
            return data;
        }

        // We want to control when job will be deleted ourselves
        auto job = std::make_unique<FileFetchJob>(urls, canEdit, indexingMode);
        job->setAutoDelete(false);

        std::unique_lock dataLock(m_dataMutex);
        // Can be cancelled
        m_fetchJob = job.get();
        dataLock.unlock();

        if (job->exec()) {
            if (!updateStatus(PROCESSING)) {
                return data;
            }
            m_readOnly = !processFetchResult(job.get(), data);
        } else if (job->error() == KJob::KilledJobError) {
            finalStatus = CANCELED;
        } else {
            finalStatus = ERRORED;
        }
        dataLock.lock();
        m_fetchJob = nullptr;
        dataLock.unlock();
    } else {
        // FIXME - are extended attributes supported for remote files?
        m_readOnly = true;
    };

    updateStatus(finalStatus);

    return data;
}

void FileMetaDataProvider::cancel()
{
    if (d->m_isAsync) {
        d->m_uses++;
        ThreadWeaver::enqueue([this]() {
            if (!d->m_shouldAbort) {
                d->cancelSync();
            }
            {
                std::lock_guard lock(d->m_usesMutex);
                d->m_uses--;
            }
            d->m_usesChange.notify_all();
        });
    } else {
        d->cancelSync();
    }
}

void FileMetaDataProviderPrivate::cancelSync()
{
    std::unique_lock statusLock(m_statusMutex);
    if (m_status == PROCESSING) {
        m_status = CANCELLING;
        statusLock.unlock();
        {
             const std::lock_guard dataLock(m_dataMutex);
             if (m_fetchJob) {
                 m_fetchJob->kill();
                 m_fetchJob = nullptr;
             }
        }
    }
}

void FileMetaDataProvider::setItems(const KFileItemList &items)
{
    {
        std::lock_guard lock(d->m_dataMutex);
        // FIXME: Make KFileItemList threadsafe
        d->m_fileItems = KFileItemList();
        for (KFileItem item : items) {
            d->m_fileItems.push_back(item);
        }
    }
    d->doRefresh();
}

void FileMetaDataProvider::refresh()
{
    // Lets skip refresh if we're already currently doing refresh
    // becauses then it doesn't make sense to refresh again right after
    // as it's unlikly that something will have changed
    d->doRefresh(std::chrono::milliseconds(1));
}

void FileMetaDataProviderPrivate::doRefresh(const std::chrono::milliseconds &skipMs)
{
    if (m_isAsync) {
        m_uses++;
        auto indexerConfig = m_config;
        ThreadWeaver::enqueue([this, indexerConfig, skipMs]() {
            if (!m_shouldAbort) {
                refreshSync(indexerConfig, skipMs);
            }
            {
                std::lock_guard lock(m_usesMutex);
                m_uses--;
            }
            m_usesChange.notify_all();
        });
    } else {
        refreshSync(m_config, skipMs);
    }
}

void FileMetaDataProviderPrivate::refreshSync(std::shared_ptr<Baloo::IndexerConfig> indexerConfig, const std::chrono::milliseconds &skipMs)
{
    if (skipMs > skipMs.zero()) {
        // If we need to wait more than skipMs (milliseconds) then we will skip refreshing
        if (!m_processingMutex.try_lock_for(skipMs)) {
            return;
        }
        m_processingMutex.unlock();
    }
    cancelSync();

    std::unique_lock dataLock(m_dataMutex);
    const QList<KFileItem> items = m_fileItems;
    dataLock.unlock();

    auto result = processFileItems(indexerConfig, items);
    finish(result);
}

bool FileMetaDataProviderPrivate::updateStatus(FileMetaDataProviderPrivate::Status newStatus)
{
    std::lock_guard lock(m_statusMutex);
    if (m_status == CANCELLING) {
        m_status = CANCELED;
        return false;
    } else {
        m_status = newStatus;
    }
    return true;
}

void FileMetaDataProviderPrivate::finish(const QVariantMap &data)
{
    {
        std::lock_guard lock(m_statusMutex);
        if (m_status != FINISHED) {
            return;
        }
    }
    {
        std::lock_guard lock(m_dataMutex);
        m_data = data;
    }
    Q_EMIT m_parent->loadingFinished();
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
    std::lock_guard lock(d->m_dataMutex);
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

QVariantMap FileMetaDataProvider::data() const
{
    std::lock_guard lock(d->m_dataMutex);
    return d->m_data;
}

#include "moc_filemetadataprovider.cpp"
