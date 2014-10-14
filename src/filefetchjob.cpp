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

#include <QTimer>
#include <Baloo/File>

#include <KFileMetaData/UserMetaData>
#include <KFileMetaData/PropertyInfo>

using namespace Baloo;

FileFetchJob::FileFetchJob(const QStringList& urls, QObject* parent)
    : KJob(parent)
    , m_urls(urls)
{
}

void FileFetchJob::start()
{
    QTimer::singleShot(0, this, SLOT(doStart()));
}

static QVariantMap convertPropertyMap(const KFileMetaData::PropertyMap& propMap)
{
    QVariantMap map;
    KFileMetaData::PropertyMap::const_iterator it = propMap.constBegin();
    for (; it != propMap.constEnd(); it++) {
        KFileMetaData::PropertyInfo pi(it.key());
        map.insertMulti(pi.name(), it.value());
    }

    return map;
}

void FileFetchJob::doStart()
{
    for (const QString& filePath : m_urls) {
        Baloo::File file(filePath);
        file.load();

        QVariantMap prop = convertPropertyMap(file.properties());

        KFileMetaData::UserMetaData md(filePath);
        QStringList tags = md.tags();
        if (!tags.isEmpty()) {
            prop.insert("tags", tags);
        }

        int rating = md.rating();
        if (rating) {
            prop.insert("rating", rating);
        }

        QString comment = md.userComment();
        if (!comment.isEmpty()) {
            prop.insert("userComment", comment);
        }

        m_data << prop;
    }

    emitResult();
}

QList<QVariantMap>  Baloo::FileFetchJob::data() const
{
    return m_data;
}
