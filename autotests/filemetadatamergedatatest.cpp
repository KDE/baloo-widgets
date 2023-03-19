/*
    SPDX-FileCopyrightText: 2023 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "filemetadatamergedatatest.h"
#include "filemetadatautil_p.h"

#include <QTest>

#include <tuple>

QTEST_MAIN(FileMetadataMergeDataTest)

void FileMetadataMergeDataTest::testMergeData()
{
    QFETCH(QList<QVariantMap>, filesData);
    QFETCH(QVariantMap, expected);

    QVariantMap data;

    Baloo::Private::mergeCommonData(data, filesData);

    QCOMPARE(data.keys(), expected.keys());
    QCOMPARE(data, expected);
}

void FileMetadataMergeDataTest::testMergeData_data()
{
    QTest::addColumn<QList<QVariantMap>>("filesData");
    QTest::addColumn<QVariantMap>("expected");

    // Wrapper to allow initialization without explicit types
    auto addRow = [](const char* name, const QList<QVariantMap>& data, const QVariantMap& result) {
        QTest::addRow("%s", name) << data << result;
    };

    auto sDuration = QStringLiteral("duration");
    auto sCCount = QStringLiteral("characterCount");
    auto sWCount = QStringLiteral("wordCount");
    auto sLCount = QStringLiteral("lineCount");

    auto sWidth = QStringLiteral("width");
    auto sHeight = QStringLiteral("height");

    // Trivial
    addRow("nodata", {{}}, {});

    // Various check for totalizing props like duration, character count
    addRow("totalizing", { {{sDuration, 100}} }, {{sDuration, 100}} );

    addRow("totalizing media", { {{sDuration, 100}},   // file1
                                 {{sDuration, 200}} }, // file2
           {{sDuration, 300}} );  // sum

    addRow("totalizing mixed", { {{sDuration, 100}},   // file1
                                 {{sDuration, 200}},   // file2
                                 {} },                 // file3 without duration
           {} );


    addRow("totalizing text allcommon", { {{sCCount, 100}, {sWCount, 20}},    // file1
                                          {{sCCount, 200}, {sWCount, 30}} },  // file2
           {{sCCount, 300}, {sWCount, 50}} );
    addRow("totalizing text only chars", { {{sCCount, 100}, {sWCount, 20}, {sLCount, 5}},    // file1
                                           {{sCCount, 200},                {sLCount, 7}},    // file2
                                           {{sCCount, 300}, {sWCount, 30}}               },  // file3
           {{sCCount, 600}} );

    // Various check props requiring same values
    addRow("same size", { {{sWidth, 100}, {sHeight, 60}},    // file1
                          {{sWidth, 100}, {sHeight, 60}},    // file2
                          {{sWidth, 100}, {sHeight, 60}} },  // file3
           {{sWidth, 100}, {sHeight, 60}} );

    addRow("same width", { {{sWidth, 100}, {sHeight, 10}},    // file1
                           {{sWidth, 100}, {sHeight, 10}},    // file2
                           {{sWidth, 100}, {sHeight,  1}} },  // file3
           {{sWidth, 100}} );
}
