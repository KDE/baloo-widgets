/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "extractortest.h"
#include "ondemandextractor.h"

#include <QDataStream>
#include <QDebug>
#include <QProcess>
#include <QStandardPaths>
#include <QTest>

void ExtractorTest::test()
{
    using namespace KFileMetaData::Property;
    QString fileUrl = QFINDTESTDATA("samplefiles/test.mp3");

    Baloo::Private::OnDemandExtractor extractor;
    extractor.process(fileUrl);
    QVERIFY(extractor.waitFinished());

    auto data = extractor.properties();

    QCOMPARE(data.value(Property::Channels).toInt(), 2);
    QCOMPARE(data.value(Property::SampleRate).toInt(), 44100);
    if (data.size() == 3) {
        QCOMPARE(data.value(Property::BitRate).toInt(), 255000);
    } else {
        QCOMPARE(data.size(), 2);
    }
}

QTEST_MAIN(ExtractorTest)

#include "moc_extractortest.cpp"
