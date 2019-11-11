/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2019  Stefan Brüns <stefan.bruens@rwth-aachen.de>
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

#include "ondemandextractor.h"
#include "widgetsdebug.h"
#include "extractorutil_p.h"

#include <QDataStream>
#include <QStandardPaths>

namespace Baloo {
namespace Private {

OnDemandExtractor::OnDemandExtractor(QObject* parent) : QObject(parent)
{
}

OnDemandExtractor::~OnDemandExtractor()
{
}

void OnDemandExtractor::process(const QString& filePath)
{
    const QString exe = QStandardPaths::findExecutable(QLatin1String("baloo_filemetadata_temp_extractor"));

    m_process.setReadChannel(QProcess::StandardOutput);

    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &OnDemandExtractor::slotIndexedFile);
    m_process.start(exe, QStringList{ filePath });
}

void OnDemandExtractor::slotIndexedFile(int, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::CrashExit) {
        qCWarning(WIDGETS) << "Extractor crashed when processing" << m_process.arguments();
        emit fileFinished(exitStatus);
        return;
    }
    QByteArray data = m_process.readAllStandardOutput();
    QDataStream in(&data, QIODevice::ReadOnly);

    m_properties.clear();
    in >> m_properties;
    emit fileFinished(QProcess::NormalExit);
}

bool OnDemandExtractor::waitFinished()
{
    return m_process.waitForFinished();
}

KFileMetaData::PropertyMap OnDemandExtractor::properties() const
{
    return m_properties;
}

} // namespace Private
} // namespace Baloo
