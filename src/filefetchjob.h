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

#ifndef BALOO_FILEFETCHJOB_H
#define BALOO_FILEFETCHJOB_H

#include <KJob>
#include <QVariantMap>
#include "ondemandextractor.h"

namespace Baloo {

class FileFetchJob : public KJob
{
    Q_OBJECT
public:
    enum class UseRealtimeIndexing : char {
        Disabled = 0,
        Only,
        Fallback
    };
    explicit FileFetchJob(const QStringList& urls, bool canEditAll,
        UseRealtimeIndexing useRealtime, QObject* parent = nullptr);

    QList<QVariantMap> data() const;
    bool canEditAll() const;

    void start() override;

private Q_SLOTS:
    void doStart();

private:
    QStringList m_urls;
    QList<QVariantMap> m_data;
    bool m_canEditAll;
    UseRealtimeIndexing m_useRealtime;
    Private::OnDemandExtractor m_extractor;
};
}

#endif // BALOO_FILEFETCHJOB_H
