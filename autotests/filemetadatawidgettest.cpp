/*
    SPDX-FileCopyrightText: 2018 Michael Heidelbach <ottwolt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "filemetadatawidgettest.h"

#include <QDebug>
#include <QLabel>
#include <QProcess>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

#include <KConfig>
#include <KConfigGroup>
#include <KFileItem>
#include <KRatingWidget>

void initLocale()
{
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
}
Q_CONSTRUCTOR_FUNCTION(initLocale)

QTEST_MAIN(FileMetadataWidgetTest)

void FileMetadataWidgetTest::initTestCase()
{
    qRegisterMetaType<KFileItemList>("KFileItemList");

    QStandardPaths::setTestModeEnabled(true);

    KConfig balooConfig(QStringLiteral("baloofilerc"), KConfig::NoGlobals);
    KConfigGroup balooSettings = balooConfig.group("General");
    // If we use .writePathEntry here, the test will fail.
    balooSettings.writeEntry(QStringLiteral("folders"), QString());

    // Ensure show configuration
    KConfig config(QStringLiteral("baloofileinformationrc"), KConfig::NoGlobals);
    KConfigGroup settings = config.group("Show");
    const auto keys = settings.keyList();
    for (const auto &key : keys) {
        settings.writeEntry(key, true);
    }

    const QString exe = QStandardPaths::findExecutable(QStringLiteral("setfattr"));

    if (exe.isEmpty()) {
        return;
    }

    const QStringList args = {QStringLiteral("--name=user.baloo.rating"),
                              QStringLiteral("--value=5"),
                              QFINDTESTDATA("samplefiles/testtagged.mp3"),
                              QFINDTESTDATA("samplefiles/testtagged.m4a")};

    QProcess process;
    process.start(exe, args);
    if (!process.waitForFinished(10000)) {
        qDebug() << "setfattr timed out";
        return;
    }

    if (process.exitStatus() == QProcess::NormalExit) {
        m_mayTestRating = true;
    } else {
        qDebug() << "setfattr err:" << process.readAllStandardError();
    }
}

void FileMetadataWidgetTest::init()
{
    m_widget = new Baloo::FileMetaDataWidget;
}

void FileMetadataWidgetTest::cleanup()
{
    delete m_widget;
}

void FileMetadataWidgetTest::shouldSignalOnceWithoutFile()
{
    QSignalSpy spy(m_widget, &Baloo::FileMetaDataWidget::metaDataRequestFinished);
    m_widget->setItems(KFileItemList());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_widget->items().count(), 0);
}

void FileMetadataWidgetTest::shouldSignalOnceFile()
{
    QSignalSpy spy(m_widget, &Baloo::FileMetaDataWidget::metaDataRequestFinished);
    KFileItemList lst;
    const KFileItem item{QUrl::fromLocalFile(QFINDTESTDATA("samplefiles/testtagged.m4a")), QString(), KFileItem::Unknown};
    lst.append(item);
    m_widget->setItems(lst);
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_widget->items().count(), 1);
}

void FileMetadataWidgetTest::shouldSignalOnceFiles()
{
    QSignalSpy spy(m_widget, &Baloo::FileMetaDataWidget::metaDataRequestFinished);
    KFileItemList lst;
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(QFINDTESTDATA("samplefiles/test.mp3")) << QUrl::fromLocalFile(QFINDTESTDATA("samplefiles/testtagged.mp3"))
         << QUrl::fromLocalFile(QFINDTESTDATA("samplefiles/testtagged.m4a"));
    for (const auto &url : std::as_const(urls)) {
        const KFileItem item{url, QString(), KFileItem::Unknown};
        lst.append(item);
    }

    m_widget->setItems(lst);
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_widget->items().count(), 3);
}

void FileMetadataWidgetTest::shouldShowProperties()
{
    QSignalSpy spy(m_widget, &Baloo::FileMetaDataWidget::metaDataRequestFinished);
    KFileItemList lst;
    const KFileItem item{QUrl::fromLocalFile(QFINDTESTDATA("samplefiles/testtagged.mp3")), QString(), KFileItem::Unknown};
    lst.append(item);
    m_widget->setItems(lst);

    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);

    // simple property
    auto valueWidget = m_widget->findChild<QLabel *>(QStringLiteral("kfileitem#type"));
    QVERIFY2(valueWidget, "Type data missing");
    QCOMPARE(valueWidget->text(), QLatin1String("MP3 audio"));

    if (m_mayTestRating) {
        // editable property
        auto ratingWidget = m_widget->findChild<KRatingWidget *>(QStringLiteral("rating"));
        QVERIFY2(ratingWidget, "Rating data missing");
        QCOMPARE(ratingWidget->rating(), 5u);
    } else {
        qDebug() << "Skipped 'Rating' test";
    }
    // async property
    valueWidget = m_widget->findChild<QLabel *>(QStringLiteral("albumArtist"));
    QVERIFY2(valueWidget, "albumArtist data was not found");
    QCOMPARE(valueWidget->text(), QLatin1String("Bill Laswell"));
}

void FileMetadataWidgetTest::shouldShowCommonProperties()
{
    QSignalSpy spy(m_widget, &Baloo::FileMetaDataWidget::metaDataRequestFinished);
    KFileItemList lst;
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(QFINDTESTDATA("samplefiles/testtagged.mp3")) << QUrl::fromLocalFile(QFINDTESTDATA("samplefiles/testtagged.m4a"));
    for (const auto &url : std::as_const(urls)) {
        const KFileItem item{url, QString(), KFileItem::Unknown};
        lst.append(item);
    }
    m_widget->setItems(lst);
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);

    // simple property
    auto valueWidget = m_widget->findChild<QLabel *>(QStringLiteral("kfileitem#type"));
    QVERIFY(!valueWidget);

    valueWidget = m_widget->findChild<QLabel *>(QStringLiteral("kfileitem#totalSize"));
    // circumvent i18n formatting
    QCOMPARE(valueWidget->text().left(3), QLatin1String("153"));

    // editable property
    if (m_mayTestRating) {
        auto ratingWidget = m_widget->findChild<KRatingWidget *>(QStringLiteral("rating"));
        QVERIFY2(ratingWidget, "Rating data missing");
        QCOMPARE(ratingWidget->rating(), 5u);
    } else {
        qDebug() << "Skipped 'Rating' test";
    }
    // async property
    // FIXME: Make this pass
    // QCOMPARE( map->value("Album Artist:"), QLatin1String("Bill Laswell"));
}

void FileMetadataWidgetTest::shouldShowMultiValueProperties()
{
    QSignalSpy spy(m_widget, &Baloo::FileMetaDataWidget::metaDataRequestFinished);
    KFileItemList lst;
    const KFileItem item{QUrl::fromLocalFile(QFINDTESTDATA("samplefiles/test_multivalue.ogg")), QString(), KFileItem::Unknown};
    lst.append(item);
    m_widget->setItems(lst);
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
    auto artistWidget = m_widget->findChild<QLabel *>(QStringLiteral("artist"));
    QVERIFY2(artistWidget, "artist not found");
    QCOMPARE(artistWidget->text(), QStringLiteral("Artist1 and Artist2"));
    auto genreWidget = m_widget->findChild<QLabel *>(QStringLiteral("genre"));
    QVERIFY2(genreWidget, "genre not found");
    QCOMPARE(genreWidget->text(), QStringLiteral("Genre1, Genre2, and Genre3"));
}
