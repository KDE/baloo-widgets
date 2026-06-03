/*
    SPDX-FileCopyrightText: 2019 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "ondemandextractor.h"
#include "extractorutil_p.h"
#include "widgetsdebug.h"

#include <QDataStream>
#include <QSignalBlocker>
#include <QStandardPaths>

namespace Baloo
{
namespace Private
{
OnDemandExtractor::OnDemandExtractor(QObject *parent)
    : QObject(parent)
{
    m_process.setReadChannel(QProcess::StandardOutput);
    connect(&m_process, &QProcess::finished, this, &OnDemandExtractor::slotIndexedFile);
    connect(&m_process, &QProcess::errorOccurred, this, &OnDemandExtractor::slotProcessError);
}

OnDemandExtractor::~OnDemandExtractor() = default;

void OnDemandExtractor::process(const QString &filePath)
{
    const QString exe = QStandardPaths::findExecutable(QLatin1String("baloo_filemetadata_temp_extractor"));

    m_process.start(exe, QStringList{filePath});
}

void OnDemandExtractor::slotIndexedFile(int, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::CrashExit) {
        qCWarning(WIDGETS) << "Extractor crashed when processing" << m_process.arguments();
        m_properties.clear();
        Q_EMIT fileFinished(exitStatus);
        return;
    }
    QByteArray data = m_process.readAllStandardOutput();
    QDataStream in(&data, QIODevice::ReadOnly);

    m_properties.clear();
    in >> m_properties;
    Q_EMIT fileFinished(QProcess::NormalExit);
}

void OnDemandExtractor::slotProcessError(QProcess::ProcessError error)
{
    if (error != QProcess::FailedToStart) {
        return;
    }
    qCWarning(WIDGETS) << "Failed to start extractor for" << m_process.arguments();
    m_properties.clear();
    Q_EMIT fileFinished(QProcess::CrashExit);
}

bool OnDemandExtractor::waitFinished()
{
    return m_process.waitForFinished();
}

void OnDemandExtractor::cancel()
{
    if (m_process.state() != QProcess::NotRunning) {
        const QSignalBlocker blocker(&m_process);
        m_process.kill();
        m_process.waitForFinished();
    }
}

KFileMetaData::PropertyMultiMap OnDemandExtractor::properties() const
{
    return m_properties;
}

} // namespace Private
} // namespace Baloo

#include "moc_ondemandextractor.cpp"
