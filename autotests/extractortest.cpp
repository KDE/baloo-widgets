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
#include "config.h"

#include <QTest>
#include <QProcess>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QTextStream>
#include <QStandardPaths>
#include <QDebug>
#include <QDir>

void ExtractorTest::test()
{
    QString fileUrl(TESTS_SAMPLE_FILES_PATH + QDir::separator() + QStringLiteral("test.mp3"));

    QString exe = QStandardPaths::findExecutable(QLatin1String("baloo_filemetadata_temp_extractor"));

    QStringList args;
    args << fileUrl;

    QProcess process;
    process.start(exe, args);
    QVERIFY(process.waitForFinished(10000));
    QCOMPARE(process.exitStatus(), QProcess::NormalExit);

    qDebug() << process.readAllStandardError();
    QByteArray bytearray = QByteArray::fromBase64(process.readAllStandardOutput());
    QVariantMap data;
    QDataStream in(&bytearray, QIODevice::ReadOnly);
    in >> data;

    QCOMPARE(data.size(), 2);
    QCOMPARE(data.value(QLatin1String("channels")).toInt(), 2);
    QCOMPARE(data.value(QLatin1String("sampleRate")).toInt(), 44100);
}

QTEST_MAIN(ExtractorTest)