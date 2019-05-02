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

#include "indexeddataretriever.h"
#include "filemetadatautil_p.h"
#include "extractorutil_p.h"

#include <QDataStream>
#include <QProcess>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDebug>

using namespace Baloo;

IndexedDataRetriever::IndexedDataRetriever(const QString& fileUrl, QObject* parent): KJob(parent)
{
    m_url = QFileInfo(fileUrl).canonicalFilePath();
}

IndexedDataRetriever::~IndexedDataRetriever()
{
}

void IndexedDataRetriever::start()
{
    const QString exe = QStandardPaths::findExecutable(QLatin1String("baloo_filemetadata_temp_extractor"));

    m_process = new QProcess(this);
    m_process->setReadChannel(QProcess::StandardOutput);

    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &IndexedDataRetriever::slotIndexedFile);
    m_process->start(exe, QStringList() << m_url);
}

void IndexedDataRetriever::slotIndexedFile(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::CrashExit) {
        qWarning() << "Extractor crashed when processing" << m_url;
    }
    QByteArray data = m_process->readAllStandardOutput();
    QDataStream in(&data, QIODevice::ReadOnly);

    KFileMetaData::PropertyMap properties;
    in >> properties;

    m_data = Baloo::Private::toNamedVariantMap(properties);

    KFileMetaData::UserMetaData umd(m_url);
    QVariantMap attributes = Baloo::Private::convertUserMetaData(umd);
    m_data.unite(attributes);

    emitResult();
}

QVariantMap IndexedDataRetriever::data() const
{
    return m_data;
}
