/*
 * This file is part of the KDE Baloo Project
 * Copyright 2018  Michael Heidelbach <ottwolt@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "filemetadatadatedisplaytest.h"


#include <QObject>
#include <QTest>
#include <QLabel>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QRegularExpression>
#include <QMetaType>
#include <QDateTime>
#include <QScopedPointer>

void initLocale()
{
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
}
Q_CONSTRUCTOR_FUNCTION(initLocale)

QTEST_MAIN(FileMetadataDateDisplayTest)

bool FileMetadataDateDisplayTest::setFileTime(const QString& filePath, const QDateTime& fileTime)
{
    bool ret;
    QScopedPointer<QFile> file{new QFile(filePath)};
    file->open(QIODevice::ReadOnly);
    ret = file->setFileTime(fileTime, QFileDevice::FileModificationTime);
    file->close();
    return ret;
}

void FileMetadataDateDisplayTest::initTestCase()
{
    qRegisterMetaType<KFileItemList>("KFileItemList");
    QStandardPaths::setTestModeEnabled(true);

    QVERIFY(
        setFileTime(QFINDTESTDATA("samplefiles/testtagged.m4a"),
        QDateTime::currentDateTime().addDays(-1))
    );

    QVERIFY(
        setFileTime(QFINDTESTDATA("samplefiles/testtagged.mp3"),
        QDateTime::currentDateTime().addYears(-10))
    );

}

void FileMetadataDateDisplayTest::shouldDisplayLongAndShortDates_data()
{
    QTest::addColumn<Baloo::DateFormats>("format");
    QTest::addColumn<QUrl>("file");
    QTest::addColumn<QRegularExpression>("regex");

    QTest::addRow("Short date, long ago")
        << Baloo::DateFormats::ShortFormat
        << QUrl::fromLocalFile(QFINDTESTDATA("samplefiles/testtagged.mp3"))
        << QRegularExpression(QStringLiteral("(?:[1-3][0-9]|[1-9]) (?:[1-2][0-9]|[1-9]):[0-5][0-9] [AP]M"))
    ;

    QTest::addRow("Short date, yesterday")
        << Baloo::DateFormats::ShortFormat
        << QUrl::fromLocalFile(QFINDTESTDATA("samplefiles/testtagged.m4a"))
        << QRegularExpression(QStringLiteral("Yesterday, (?:[1-2][0-9]|[1-9]):[0-5][0-9] [AP]M"))
    ;

    QTest::addRow("Long date, long ago")
        << Baloo::DateFormats::LongFormat
        << QUrl::fromLocalFile(QFINDTESTDATA("samplefiles/testtagged.mp3"))
        << QRegularExpression(QStringLiteral("[A-Z][a-z]+, [A-Z][a-z]+ (?:[1-3][0-9]|[1-9]), 20[0-9]{2} (?:1[0-2]|[1-9]):[0-5][0-9]:[0-5][0-9] [AP]M [A-Z]{3,4}"))
    ;

    QTest::addRow("Long date, yesterday")
        << Baloo::DateFormats::LongFormat
        << QUrl::fromLocalFile(QFINDTESTDATA("samplefiles/testtagged.m4a"))
        << QRegularExpression(QStringLiteral("Yesterday, (?:1[0-2]|[1-9]):[0-5][0-9]:[0-5][0-9] [AP]M [A-Z]{3,4}"))
    ;

}

void FileMetadataDateDisplayTest::shouldDisplayLongAndShortDates()
{
    QFETCH(Baloo::DateFormats, format);
    QFETCH(QUrl, file);
    QFETCH(QRegularExpression, regex);

    const auto widget = new Baloo::FileMetaDataWidget();
    widget->setDateFormat(format);
    QSignalSpy spy(widget, &Baloo::FileMetaDataWidget::metaDataRequestFinished);
    widget->setItems(KFileItemList() << file);
    QVERIFY(spy.wait());

    QLabel* dateWidget = widget->findChild<QLabel*>(QStringLiteral("kfileitem#modified"), Qt::FindDirectChildrenOnly);
    QVERIFY2(dateWidget, "Date widget not found");
    QVERIFY2(regex.match(dateWidget->text()).hasMatch(),
        qPrintable(QStringLiteral("\"%1\" did not match %2").arg(dateWidget->text(), regex.pattern()))
    );
}
