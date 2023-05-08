/*
    SPDX-FileCopyrightText: 2018 Michael Heidelbach <ottwolt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "filemetadatadatedisplaytest.h"

// This is an implementation detail of how dates are returned
#include <KFormat>

#include <QDateTime>
#include <QLabel>
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

static QString filenameYesterday()
{
    return QStringLiteral("file_yesterday.txt");
}

static QString filenameLongAgo()
{
    return QStringLiteral("file_longago.txt");
}

static void setFileTime(const QString &filePath, const QDateTime &fileTime)
{
    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadOnly), qUtf8Printable(filePath));
    QVERIFY2(file.setFileTime(fileTime, QFileDevice::FileModificationTime), qUtf8Printable(filePath));
}

static void createFile(const QString &filePath)
{
    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadWrite), qUtf8Printable(filePath));
    QVERIFY2(file.write("dummy"), qUtf8Printable(filePath));
}

void FileMetadataDateDisplayTest::initTestCase()
{
    qRegisterMetaType<KFileItemList>("KFileItemList");
    QStandardPaths::setTestModeEnabled(true);

    QVERIFY(m_testDir.isValid());

    auto now = QDateTime::currentDateTime();
    createFile(m_testDir.filePath(filenameYesterday()));
    setFileTime(m_testDir.filePath(filenameYesterday()), now.addDays(-1));

    createFile(m_testDir.filePath(filenameLongAgo()));
    setFileTime(m_testDir.filePath(filenameLongAgo()), now.addYears(-10));
}

void FileMetadataDateDisplayTest::cleanupTestCase()
{
}

static QRegularExpression yesterdayShortRegex()
{
    // the last space is a Narrow No-Break Space (NNBSP) caracter
    return QRegularExpression(QStringLiteral("Yesterday at ([1-2][0-9]|[1-9]):[0-5][0-9][  ][AP]M"));
}

static QRegularExpression longAgoShortRegex()
{
    // the last space group includes is a Narrow No-Break Space (NNBSP) caracter
    return QRegularExpression(QStringLiteral("([1-3][0-9]|[1-9]) at ([1-2][0-9]|[1-9]):[0-5][0-9][  ][AP]M"));
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

    QVERIFY2(yesterday_re.match(yesterday_s).hasMatch(), qPrintable(QStringLiteral("\"%1\" did not match \"%2\"").arg(yesterday_s, yesterday_re.pattern())));
    QVERIFY2(long_ago_re.match(long_ago_s).hasMatch(), qPrintable(QStringLiteral("\"%1\" did not match \"%2\"").arg(long_ago_s, long_ago_re.pattern())));
}

void FileMetadataDateDisplayTest::shouldDisplayLongAndShortDates_data()
{
    QTest::addColumn<Baloo::DateFormats>("format");
    QTest::addColumn<QUrl>("file");
    QTest::addColumn<QRegularExpression>("regex");

    auto urlLongAgo = QUrl::fromLocalFile(m_testDir.filePath(filenameLongAgo()));
    auto urlYesterday = QUrl::fromLocalFile(m_testDir.filePath(filenameYesterday()));

    QTest::addRow("Short date, long ago") << Baloo::DateFormats::ShortFormat << urlLongAgo << longAgoShortRegex();

    QTest::addRow("Short date, yesterday") << Baloo::DateFormats::ShortFormat << urlYesterday << yesterdayShortRegex();

    // the last space group includes is a Narrow No-Break Space (NNBSP) caracter
    QTest::addRow("Long date, long ago") << Baloo::DateFormats::LongFormat << urlLongAgo
                                         << QRegularExpression(QStringLiteral(
                                                "[A-Z][a-z]+, [A-Z][a-z]+ ([1-3][0-9]|[1-9]), 20[0-9]{2} at (1[0-2]|[1-9]):[0-5][0-9][  ][AP]M"));

    // the last space group includes is a Narrow No-Break Space (NNBSP) caracter
    QTest::addRow("Long date, yesterday") << Baloo::DateFormats::LongFormat << urlYesterday
                                          << QRegularExpression(QStringLiteral("Yesterday at (1[0-2]|[1-9]):[0-5][0-9][  ][AP]M"));
}

void FileMetadataDateDisplayTest::shouldDisplayLongAndShortDates()
{
    QFETCH(Baloo::DateFormats, format);
    QFETCH(QUrl, file);
    QFETCH(QRegularExpression, regex);

    const auto widget = new Baloo::FileMetaDataWidget();
    widget->setDateFormat(format);
    QSignalSpy spy(widget, &Baloo::FileMetaDataWidget::metaDataRequestFinished);
    widget->setItems({KFileItem{file}});
    QVERIFY(spy.wait());

    auto dateWidget = widget->findChild<QLabel *>(QStringLiteral("kfileitem#modified"), Qt::FindDirectChildrenOnly);
    QVERIFY2(dateWidget, "Date widget not found");
    QVERIFY2(regex.match(dateWidget->text()).hasMatch(), qPrintable(QStringLiteral("\"%1\" did not match \"%2\"").arg(dateWidget->text(), regex.pattern())));
}
