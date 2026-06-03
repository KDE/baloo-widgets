/*
    SPDX-FileCopyrightText: 2019 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef ONDEMANDEXTRACTOR_H
#define ONDEMANDEXTRACTOR_H

#include <KFileMetaData/Properties>
#include <QProcess>

namespace Baloo
{
namespace Private
{
class OnDemandExtractor : public QObject
{
    Q_OBJECT

public:
    explicit OnDemandExtractor(QObject *parent = nullptr);
    ~OnDemandExtractor() override;

    void process(const QString &filePath);
    void cancel();

    bool waitFinished();
    KFileMetaData::PropertyMultiMap properties() const;

Q_SIGNALS:
    void fileFinished(QProcess::ExitStatus exitStatus);

private Q_SLOTS:
    void slotIndexedFile(int exitCode, QProcess::ExitStatus exitStatus);
    void slotProcessError(QProcess::ProcessError error);

private:
    QProcess m_process;
    KFileMetaData::PropertyMultiMap m_properties;
};

} // namespace Private
} // namespace Baloo

#endif // ONDEMANDEXTRACTOR_H
