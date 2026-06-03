/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "filefetchjob.h"
#include "filemetadatautil_p.h"
#include "widgetsdebug.h"

#include <Baloo/File>
#include <QFileInfo>
#include <QTimer>

#include <KFileMetaData/UserMetaData>

using namespace Baloo;

FileFetchJob::FileFetchJob(const QStringList &urls, bool canEditAll, FileFetchJob::UseRealtimeIndexing useRealtime, QObject *parent)
    : KJob(parent)
    , m_urls(urls)
    , m_canEditAll(canEditAll)
    , m_useRealtime(useRealtime)
{
    connect(&m_extractor, &Private::OnDemandExtractor::fileFinished, this, &FileFetchJob::slotExtractorFinished);
}

void FileFetchJob::start()
{
    QTimer::singleShot(0, this, &FileFetchJob::doStart);
}

void FileFetchJob::doStart()
{
    if (isFinished()) {
        return;
    }

    while (m_index < m_urls.size()) {
        const QString &filePath = m_urls.at(m_index);
        KFileMetaData::PropertyMultiMap fileProperties;

        // UseRealtimeIndexing::Fallback: try DB first, then filesystem
        // UseRealtimeIndexing::Disabled: DB contents only
        // UseRealtimeIndexing::Only:     DB disabled, use filesystem
        if (m_useRealtime != UseRealtimeIndexing::Only) {
            Baloo::File file(filePath);
            file.load();
            fileProperties = file.properties();
            qCDebug(WIDGETS) << filePath << "DB properties:" << fileProperties;
        }

        QVariantMap prop;
        KFileMetaData::UserMetaData umd(filePath);

        if (umd.isSupported()) {
            using Attribute = KFileMetaData::UserMetaData::Attribute;
            auto umdData = Baloo::Private::fetchUserMetaData(umd, Attribute::Tags | Attribute::Rating | Attribute::Comment | Attribute::OriginUrl);
            prop = Baloo::Private::convertUserMetaData(umdData);
        } else {
            m_canEditAll = false;
        }
        if (m_canEditAll) {
            m_canEditAll = QFileInfo(filePath).isWritable();
        }

        if (fileProperties.empty() && m_useRealtime != UseRealtimeIndexing::Disabled) {
            m_currentData = prop;
            m_extractor.process(filePath);
            return;
        }
        prop.insert(Baloo::Private::toNamedVariantMap(fileProperties));

        m_data << prop;
        ++m_index;
    }

    emitResult();
}

void FileFetchJob::slotExtractorFinished()
{
    const KFileMetaData::PropertyMultiMap fileProperties = m_extractor.properties();
    qCDebug(WIDGETS) << m_urls.at(m_index) << "  properties:" << fileProperties;

    m_currentData.insert(Baloo::Private::toNamedVariantMap(fileProperties));
    m_data << m_currentData;
    m_currentData.clear();
    ++m_index;

    doStart();
}

bool FileFetchJob::doKill()
{
    m_extractor.cancel();
    return true;
}

QList<QVariantMap> Baloo::FileFetchJob::data() const
{
    return m_data;
}

bool FileFetchJob::canEditAll() const
{
    return m_canEditAll;
}

#include "moc_filefetchjob.cpp"
