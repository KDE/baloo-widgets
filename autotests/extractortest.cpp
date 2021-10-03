/*
 * This file is part of the KDE Baloo Project
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

#include "extractortest.h"
#include <extractorutil_p.h>

#include <KFileMetaData/Properties>

#include <QDataStream>
#include <QDebug>
#include <QDir>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTest>
#include <QTextStream>

void ExtractorTest::test()
{
    using namespace KFileMetaData::Property;
    QString fileUrl = QFINDTESTDATA("samplefiles/test.mp3");

    QString exe = QStandardPaths::findExecutable(QLatin1String("baloo_filemetadata_temp_extractor"));

    QStringList args;
    args << fileUrl;

    QProcess process;
    process.start(exe, args);
    QVERIFY(process.waitForFinished(10000));
    QCOMPARE(process.exitStatus(), QProcess::NormalExit);

    qDebug() << process.readAllStandardError();
    QByteArray bytearray = process.readAllStandardOutput();
    QDataStream in(&bytearray, QIODevice::ReadOnly);

    KFileMetaData::PropertyMap data;
    in >> data;

    QCOMPARE(data.value(Property::Channels).toInt(), 2);
    QCOMPARE(data.value(Property::SampleRate).toInt(), 44100);
    if (data.size() == 3) {
        QCOMPARE(data.value(Property::BitRate).toInt(), 255000);
    } else {
        QCOMPARE(data.size(), 2);
    }
}

QTEST_MAIN(ExtractorTest)
