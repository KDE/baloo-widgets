/*
    SPDX-FileCopyrightText: 2018 Michael Heidelbach <ottwolt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef FILEMETADATAWIDGETTEST_H
#define FILEMETADATAWIDGETTEST_H

#include <filemetadatawidget.h>

class FileMetadataWidgetTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();

    void shouldSignalOnceWithoutFile();
    void shouldSignalOnceFile();
    void shouldSignalOnceFiles();
    void shouldShowProperties();
    void shouldShowCommonProperties();
    void shouldShowMultiValueProperties();

private:
    Baloo::FileMetaDataWidget *m_widget;
    bool m_mayTestRating = false;
};

#endif // FILEMETADATAWIDGETTEST_H
