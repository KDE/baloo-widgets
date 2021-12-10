/*
    SPDX-FileCopyrightText: 2019 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "ondemandextractor.h"
#include "extractorutil_p.h"
#include "widgetsdebug.h"

#include <QDataStream>
#include <QStandardPaths>

namespace Baloo
{
namespace Private
{
OnDemandExtractor::OnDemandExtractor(QObject *parent)
    : QObject(parent)
{
}

OnDemandExtractor::~OnDemandExtractor()
{
}

void OnDemandExtractor::process(const QString &filePath)
{
    const QString exe = QStandardPaths::findExecutable(QLatin1String("baloo_filemetadata_temp_extractor"));

    m_process.setReadChannel(QProcess::StandardOutput);

    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &OnDemandExtractor::slotIndexedFile);
    m_process.start(exe, QStringList{filePath});
}

void OnDemandExtractor::slotIndexedFile(int, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::CrashExit) {
        qCWarning(WIDGETS) << "Extractor crashed when processing" << m_process.arguments();
        Q_EMIT fileFinished(exitStatus);
        return;
    }
    QByteArray data = m_process.readAllStandardOutput();
    QDataStream in(&data, QIODevice::ReadOnly);

    m_properties.clear();
    in >> m_properties;
    Q_EMIT fileFinished(QProcess::NormalExit);
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
