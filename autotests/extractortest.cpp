/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "extractortest.h"
#include <extractorutil_p.h>

#include <QDataStream>
#include <QDebug>
#include <QProcess>
#include <QStandardPaths>
#include <QTest>

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

    KFileMetaData::PropertyMultiMap data;
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
