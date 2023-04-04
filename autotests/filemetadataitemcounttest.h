/*
    SPDX-FileCopyrightText: 2018 Michael Heidelbach <ottwolt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef FILEMETADATAITEMCOUNTTEST_H
#define FILEMETADATAITEMCOUNTTEST_H

#include <filemetadatawidget.h>

class FileMetadataItemCountTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    void testItemCount();
};

#endif // FILEMETADATAITEMCOUNTTEST_H
