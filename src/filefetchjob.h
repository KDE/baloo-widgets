/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_FILEFETCHJOB_H
#define BALOO_FILEFETCHJOB_H

#include "ondemandextractor.h"
#include <KJob>
#include <QVariantMap>

namespace Baloo
{
class FileFetchJob : public KJob
{
    Q_OBJECT
public:
    enum class UseRealtimeIndexing : char {
        Disabled = 0,
        Only,
        Fallback,
    };
    explicit FileFetchJob(const QStringList &urls, bool canEditAll, UseRealtimeIndexing useRealtime, QObject *parent = nullptr);

    QList<QVariantMap> data() const;
    bool canEditAll() const;

    void start() override;

protected:
    bool doKill() override;

private Q_SLOTS:
    void doStart();
    void slotExtractorFinished();

private:
    QStringList m_urls;
    qsizetype m_index = 0;
    QList<QVariantMap> m_data;
    QVariantMap m_currentData;
    bool m_canEditAll;
    UseRealtimeIndexing m_useRealtime;
    Private::OnDemandExtractor m_extractor;
};
}

#endif // BALOO_FILEFETCHJOB_H
