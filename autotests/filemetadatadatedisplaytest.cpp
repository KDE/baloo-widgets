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
#include <QTemporaryFile>
#include <QTest>

void initLocale()
{
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
}
Q_CONSTRUCTOR_FUNCTION(initLocale)

QTEST_MAIN(FileMetadataDateDisplayTest)

static void setFileTime(const QString &filePath, const QDateTime &fileTime)
{
    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadOnly), filePath.toUtf8().constData());
    QVERIFY2(file.setFileTime(fileTime, QFileDevice::FileModificationTime), filePath.toUtf8().constData());
}

static void copyToTemporaryAndSetFileTime(const QString &filePath, const QDateTime &fileTime, QString *tempFilePath)
{
    QTemporaryFile tempF;
    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadOnly), filePath.toUtf8().constData());
    QVERIFY2(tempF.open(), filePath.toUtf8().constData());
    const QByteArray data = file.readAll();
    QCOMPARE(tempF.write(data), data.size());
    tempF.setAutoRemove(false);
    *tempFilePath = tempF.fileName();
    tempF.close();
    setFileTime(*tempFilePath, fileTime);
}

void FileMetadataDateDisplayTest::initTestCase()
{
    qRegisterMetaType<KFileItemList>("KFileItemList");
    QStandardPaths::setTestModeEnabled(true);

    copyToTemporaryAndSetFileTime(QFINDTESTDATA("samplefiles/testtagged.m4a"), QDateTime::currentDateTime().addDays(-1), &m_m4aFilePath);
    copyToTemporaryAndSetFileTime(QFINDTESTDATA("samplefiles/testtagged.mp3"), QDateTime::currentDateTime().addYears(-10), &m_mp3FilePath);
}

void FileMetadataDateDisplayTest::cleanupTestCase()
{
    QFile::remove(m_m4aFilePath);
    QFile::remove(m_mp3FilePath);
}

static QRegularExpression yesterdayShortRegex()
{
    // Checking for "yesterday" and a time indication
    return QRegularExpression(QStringLiteral("Yesterday at (?:[1-2][0-9]|[1-9]):[0-5][0-9] [AP]M"));
}

static QRegularExpression longAgoShortRegex()
{
    // Checking for a 1- or 2-digit day and an hour
    return QRegularExpression(QStringLiteral("(?:[1-3][0-9]|[1-9]) at (?:[1-2][0-9]|[1-9]):[0-5][0-9] [AP]M"));
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

    QTest::addRow("Short date, long ago") << Baloo::DateFormats::ShortFormat << QUrl::fromLocalFile(m_mp3FilePath) << longAgoShortRegex();

    QTest::addRow("Short date, yesterday") << Baloo::DateFormats::ShortFormat << QUrl::fromLocalFile(m_m4aFilePath) << yesterdayShortRegex();

    QTest::addRow("Long date, long ago") << Baloo::DateFormats::LongFormat << QUrl::fromLocalFile(m_mp3FilePath)
                                         << QRegularExpression(QStringLiteral(
                                                "[A-Z][a-z]+, [A-Z][a-z]+ (?:[1-3][0-9]|[1-9]), 20[0-9]{2} at (?:1[0-2]|[1-9]):[0-5][0-9] [AP]M"));

    QTest::addRow("Long date, yesterday") << Baloo::DateFormats::LongFormat << QUrl::fromLocalFile(m_m4aFilePath)
                                          << QRegularExpression(QStringLiteral("Yesterday at (?:1[0-2]|[1-9]):[0-5][0-9] [AP]M"));
}

void FileMetadataDateDisplayTest::shouldDisplayLongAndShortDates()
{
    QFETCH(Baloo::DateFormats, format);
    QFETCH(QUrl, file);
    QFETCH(QRegularExpression, regex);

    const auto widget = new Baloo::FileMetaDataWidget();
    widget->setDateFormat(format);
    QSignalSpy spy(widget, &Baloo::FileMetaDataWidget::metaDataRequestFinished);
    KFileItemList lst;
    const KFileItem item{file, QString(), KFileItem::Unknown};
    lst.append(item);
    widget->setItems(lst);
    QVERIFY(spy.wait());

    auto dateWidget = widget->findChild<QLabel *>(QStringLiteral("kfileitem#modified"), Qt::FindDirectChildrenOnly);
    QVERIFY2(dateWidget, "Date widget not found");
    QVERIFY2(regex.match(dateWidget->text()).hasMatch(), qPrintable(QStringLiteral("\"%1\" did not match %2").arg(dateWidget->text(), regex.pattern())));
}
