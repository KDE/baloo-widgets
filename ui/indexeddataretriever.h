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
#include <KProcess>

#include <Nepomuk2/Variant>

namespace Nepomuk2 {

class IndexedDataRetriever : public KJob
{
    Q_OBJECT
public:
    IndexedDataRetriever(const QString& fileUrl, QObject* parent = 0);
    virtual ~IndexedDataRetriever();

    virtual void start();

    QHash<QUrl, Variant> data();

private slots:
    void slotIndexedFile(int error);

private:
    QString m_url;
    QProcess* m_process;
    QHash<QUrl, Variant> m_data;
};

}

#endif // INDEXEDDATARETRIEVER_H
