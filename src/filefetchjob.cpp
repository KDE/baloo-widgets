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

#include <QTimer>
#include <Baloo/File>

#include <KFileMetaData/UserMetaData>

using namespace Baloo;

FileFetchJob::FileFetchJob(const QStringList& urls, bool canEditAll, QObject* parent)
    : KJob(parent)
    , m_urls(urls)
    , m_canEditAll(canEditAll)
{
}

void FileFetchJob::start()
{
    QTimer::singleShot(0, this, SLOT(doStart()));
}

void FileFetchJob::doStart()
{
    for (const QString& filePath : m_urls) {
        Baloo::File file(filePath);
        file.load();

        QVariantMap prop = Baloo::Private::toNamedVariantMap(file.properties());
        KFileMetaData::UserMetaData umd(filePath);

        if (umd.isSupported()) {
            // FIXME - check writable

            QVariantMap attributes = Baloo::Private::convertUserMetaData(umd);
            prop.unite(attributes);
        } else {
            m_canEditAll = false;
        }

        m_data << prop;
    }

    emitResult();
}

QList<QVariantMap>  Baloo::FileFetchJob::data() const
{
    return m_data;
}

bool FileFetchJob::canEditAll() const
{
    return m_canEditAll;
}
