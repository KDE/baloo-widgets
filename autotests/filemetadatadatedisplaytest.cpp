/*
    SPDX-FileCopyrightText: 2018 Michael Heidelbach <ottwolt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "filemetadatadatedisplaytest.h"

// This is an implementation detail of how dates are returned
#include <KFormat>

#include <QDateTime>
#include <QLabel>
#include <QMetaType>
#include <QObject>
#include <QRegularExpression>
#include <QScopedPointer>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

void initLocale()
{
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
}
Q_CONSTRUCTOR_FUNCTION(initLocale)

QTEST_MAIN(FileMetadataDateDisplayTest)

bool FileMetadataDateDisplayTest::setFileTime(const QString &filePath, const QDateTime &fileTime)
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

    QVERIFY(setFileTime(QFINDTESTDATA("samplefiles/testtagged.m4a"), QDateTime::currentDateTime().addDays(-1)));

    QVERIFY(setFileTime(QFINDTESTDATA("samplefiles/testtagged.mp3"), QDateTime::currentDateTime().addYears(-10)));
}

static QRegularExpression yesterdayShortRegex()
{
    return QRegularExpression(QStringLiteral("Yesterday, (?:[1-2][0-9]|[1-9]):[0-5][0-9] [AP]M"));
}

static QRegularExpression longAgoShortRegex()
{
    return QRegularExpression(QStringLiteral("(?:[1-3][0-9]|[1-9]) (?:[1-2][0-9]|[1-9]):[0-5][0-9] [AP]M"));
}

void FileMetadataDateDisplayTest::validateDateFormats()
{
    auto yesterday = QDateTime::currentDateTime().addDays(-1);
    auto long_ago = QDateTime::currentDateTime().addYears(-10);

    // This tests only the "short form" regular expressions also found in shouldDisplayLongAndShortDates_data()
    auto yesterday_re = yesterdayShortRegex();
    auto long_ago_re = longAgoShortRegex();

    KFormat form;

    auto yesterday_s = form.formatRelativeDateTime(yesterday, QLocale::ShortFormat);
    auto long_ago_s = form.formatRelativeDateTime(long_ago, QLocale::ShortFormat);

    QVERIFY2(yesterday_re.match(yesterday_s).hasMatch(), qPrintable(QStringLiteral("\"%1\" did not match %2").arg(yesterday_s, yesterday_re.pattern())));
    QVERIFY2(long_ago_re.match(long_ago_s).hasMatch(), qPrintable(QStringLiteral("\"%1\" did not match %2").arg(long_ago_s, long_ago_re.pattern())));
}

void FileMetadataDateDisplayTest::shouldDisplayLongAndShortDates_data()
{
    QTest::addColumn<Baloo::DateFormats>("format");
    QTest::addColumn<QUrl>("file");
    QTest::addColumn<QRegularExpression>("regex");

    QTest::addRow("Short date, long ago") << Baloo::DateFormats::ShortFormat << QUrl::fromLocalFile(QFINDTESTDATA("samplefiles/testtagged.mp3"))
                                          << longAgoShortRegex();

    QTest::addRow("Short date, yesterday") << Baloo::DateFormats::ShortFormat << QUrl::fromLocalFile(QFINDTESTDATA("samplefiles/testtagged.m4a"))
                                           << yesterdayShortRegex();

    QTest::addRow("Long date, long ago")
        << Baloo::DateFormats::LongFormat << QUrl::fromLocalFile(QFINDTESTDATA("samplefiles/testtagged.mp3"))
        << QRegularExpression(
               QStringLiteral("[A-Z][a-z]+, [A-Z][a-z]+ (?:[1-3][0-9]|[1-9]), 20[0-9]{2} (?:1[0-2]|[1-9]):[0-5][0-9]:[0-5][0-9] [AP]M [A-Z]{3,4}"));

    QTest::addRow("Long date, yesterday") << Baloo::DateFormats::LongFormat << QUrl::fromLocalFile(QFINDTESTDATA("samplefiles/testtagged.m4a"))
                                          << QRegularExpression(QStringLiteral("Yesterday, (?:1[0-2]|[1-9]):[0-5][0-9]:[0-5][0-9] [AP]M [A-Z]{3,4}"));
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

    QLabel *dateWidget = widget->findChild<QLabel *>(QStringLiteral("kfileitem#modified"), Qt::FindDirectChildrenOnly);
    QVERIFY2(dateWidget, "Date widget not found");
    QVERIFY2(regex.match(dateWidget->text()).hasMatch(), qPrintable(QStringLiteral("\"%1\" did not match %2").arg(dateWidget->text(), regex.pattern())));
}
