/*
    SPDX-FileCopyrightText: 2018 Michael Heidelbach <ottwolt@gmail.com>
    SPDX-FileCopyrightText: 2022 Adriaan de Groot <groot@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef FILEMETADATADATEDISPLAYTEST_H
#define FILEMETADATADATEDISPLAYTEST_H

#include <filemetadatawidget.h>

class QString;
class QDateTime;

class FileMetadataDateDisplayTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    /// Validate that KFormat produces the expected date strings
    void validateDateFormats();

    void shouldDisplayLongAndShortDates();
    void shouldDisplayLongAndShortDates_data();

private:
    bool setFileTime(const QString &file, const QDateTime &filetime);
};

#endif // FILEMETADATADATEDISPLAYTEST_H
