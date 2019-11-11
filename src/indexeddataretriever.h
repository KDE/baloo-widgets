/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2012  Vishesh Handa <me@vhanda.in>
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

#ifndef INDEXEDDATARETRIEVER_H
#define INDEXEDDATARETRIEVER_H

#include <KJob>
#include "ondemandextractor.h"
#include <QVariant>

namespace Baloo {

class IndexedDataRetriever : public KJob
{
    Q_OBJECT
public:
    explicit IndexedDataRetriever(const QString& fileUrl, QObject* parent = nullptr);
    ~IndexedDataRetriever() override;

    void start() override;

    QVariantMap data() const;
    bool canEdit() const;

private Q_SLOTS:
    void slotIndexedFile(QProcess::ExitStatus exitStatus);

private:
    QString m_url;
    Private::OnDemandExtractor m_extractor;
    QVariantMap m_data;
    bool m_canEdit = false;
};

}

#endif // INDEXEDDATARETRIEVER_H
