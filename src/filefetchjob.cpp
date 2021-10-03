/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
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
}

void FileFetchJob::start()
{
    QTimer::singleShot(0, this, SLOT(doStart()));
}

void FileFetchJob::doStart()
{
    for (const QString &filePath : m_urls) {
        bool extractorRunning = false;
        KFileMetaData::PropertyMap fileProperties;

        // UseRealtimeIndexing::Fallback: try DB first, then filesystem
        // UseRealtimeIndexing::Disabled: DB contents only
        // UseRealtimeIndexing::Only:     DB disabled, use filesystem
        if (m_useRealtime != UseRealtimeIndexing::Only) {
            Baloo::File file(filePath);
            file.load();
            fileProperties = file.properties();
            qCDebug(WIDGETS) << filePath << "DB properties:" << fileProperties;
        }
        if (fileProperties.empty() && m_useRealtime != UseRealtimeIndexing::Disabled) {
            extractorRunning = true;
            m_extractor.process(filePath);
        }

        QVariantMap prop;
        KFileMetaData::UserMetaData umd(filePath);

        if (umd.isSupported()) {
            prop = Baloo::Private::convertUserMetaData(umd);
        } else {
            m_canEditAll = false;
        }
        if (m_canEditAll) {
            m_canEditAll = QFileInfo(filePath).isWritable();
        }

        if (extractorRunning) {
            m_extractor.waitFinished();
            fileProperties = m_extractor.properties();
            qCDebug(WIDGETS) << filePath << "  properties:" << fileProperties;
        }
        prop.unite(Baloo::Private::toNamedVariantMap(fileProperties));

        m_data << prop;
    }

    emitResult();
}

QList<QVariantMap> Baloo::FileFetchJob::data() const
{
    return m_data;
}

bool FileFetchJob::canEditAll() const
{
    return m_canEditAll;
}
