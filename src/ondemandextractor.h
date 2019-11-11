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

#ifndef ONDEMANDEXTRACTOR_H
#define ONDEMANDEXTRACTOR_H

#include <QProcess>
#include <KFileMetaData/Properties>

namespace Baloo {
namespace Private {

class OnDemandExtractor : public QObject
{
    Q_OBJECT

public:
    explicit OnDemandExtractor(QObject* parent = nullptr);
    ~OnDemandExtractor();

    void process(const QString& filePath);

    KFileMetaData::PropertyMap properties() const;

Q_SIGNALS:
    void fileFinished(QProcess::ExitStatus exitStatus);

private Q_SLOTS:
    void slotIndexedFile(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QProcess m_process;
    KFileMetaData::PropertyMap m_properties;
};

} // namespace Private
} // namespace Baloo

#endif // ONDEMANDEXTRACTOR_H
