/*
    SPDX-FileCopyrightText: 2023 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef FILEMETADATAMERGEDATATEST_H
#define FILEMETADATAMERGEDATATEST_H

#include <QObject>

class FileMetadataMergeDataTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    //void initTestCase();
    //void init();
    //void cleanup();

    void testMergeData();
    void testMergeData_data();

private:
};

#endif // FILEMETADATAMERGEDATATEST_H
