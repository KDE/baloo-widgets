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

    m_sampleFiles = {
        { "untagged", QFINDTESTDATA("samplefiles/test.mp3")},
        { "taggedmp3", QFINDTESTDATA("samplefiles/testtagged.mp3")},
        { "taggedm4a", QFINDTESTDATA("samplefiles/testtagged.m4a")},
        { "multivalue", QFINDTESTDATA("samplefiles/test_multivalue.ogg")},
    };
    for (const auto &file : std::as_const(m_sampleFiles)) {
        QVERIFY(!file.isEmpty());
    }

    const QString exe = QStandardPaths::findExecutable(QStringLiteral("setfattr"));
    if (exe.isEmpty()) {
        return;
    }

    m_mayTestRating = true;

    for (const auto &file : { "taggedmp3", "taggedm4a" }) {
        const QStringList args = {QStringLiteral("--name=user.baloo.rating"),
                                  QStringLiteral("--value=5"),
                                  m_sampleFiles[file]};

        QProcess process;
        process.start(exe, args);
        if (!process.waitForFinished(10000)) {
            qDebug() << "setfattr timed out";
            m_mayTestRating = false;
            return;
        }

        if ((process.exitStatus() != QProcess::NormalExit) || (process.exitCode() != 0)) {
            qDebug() << "setfattr err:" << process.readAllStandardError();
            m_mayTestRating = false;
            return;
        }
    }
}

void FileMetadataWidgetTest::shouldSignalOnceWithoutFile()
{
    auto widget = std::make_unique<Baloo::FileMetaDataWidget>();

    QSignalSpy spy(widget.get(), &Baloo::FileMetaDataWidget::metaDataRequestFinished);
    widget->setItems(KFileItemList());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(widget->items().count(), 0);
}

void FileMetadataWidgetTest::shouldSignalOnceFile()
{
    auto widget = std::make_unique<Baloo::FileMetaDataWidget>();

    QSignalSpy spy(widget.get(), &Baloo::FileMetaDataWidget::metaDataRequestFinished);
    widget->setItems({KFileItem{QUrl::fromLocalFile(m_sampleFiles["taggedm4a"])}});
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(widget->items().count(), 1);
}

void FileMetadataWidgetTest::shouldSignalOnceFiles()
{
    auto widget = std::make_unique<Baloo::FileMetaDataWidget>();

    QSignalSpy spy(widget.get(), &Baloo::FileMetaDataWidget::metaDataRequestFinished);
    widget->setItems({
        KFileItem{QUrl::fromLocalFile(m_sampleFiles["untagged"])},
        KFileItem{QUrl::fromLocalFile(m_sampleFiles["taggedmp3"])},
        KFileItem{QUrl::fromLocalFile(m_sampleFiles["taggedm4a"])},
    });
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(widget->items().count(), 3);
}

void FileMetadataWidgetTest::shouldShowProperties()
{
    auto widget = std::make_unique<Baloo::FileMetaDataWidget>();

    QSignalSpy spy(widget.get(), &Baloo::FileMetaDataWidget::metaDataRequestFinished);
    widget->setItems({KFileItem{QUrl::fromLocalFile(m_sampleFiles["taggedmp3"])}});

    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);

    // simple property
    auto valueWidget = widget->findChild<QLabel *>(QStringLiteral("kfileitem#type"));
    QVERIFY2(valueWidget, "Type data missing");
    QCOMPARE(valueWidget->text(), QLatin1String("MP3 audio"));

    if (m_mayTestRating) {
        // editable property
        auto ratingWidget = widget->findChild<KRatingWidget *>(QStringLiteral("rating"));
        QVERIFY2(ratingWidget, "Rating data missing");
        QCOMPARE(ratingWidget->rating(), 5u);
    } else {
        qDebug() << "Skipped 'Rating' test";
    }
    // async property
    valueWidget = widget->findChild<QLabel *>(QStringLiteral("albumArtist"));
    QVERIFY2(valueWidget, "albumArtist data was not found");
    QCOMPARE(valueWidget->text(), QLatin1String("Bill Laswell"));
}

void FileMetadataWidgetTest::shouldShowCommonProperties()
{
    auto widget = std::make_unique<Baloo::FileMetaDataWidget>();

    QSignalSpy spy(widget.get(), &Baloo::FileMetaDataWidget::metaDataRequestFinished);
    widget->setItems({
        KFileItem{QUrl::fromLocalFile(m_sampleFiles["taggedmp3"])},
        KFileItem{QUrl::fromLocalFile(m_sampleFiles["taggedm4a"])},
    });
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);

    // simple property
    auto valueWidget = widget->findChild<QLabel *>(QStringLiteral("kfileitem#type"));
    QVERIFY(!valueWidget);

    valueWidget = widget->findChild<QLabel *>(QStringLiteral("kfileitem#totalSize"));
    // circumvent i18n formatting
    QCOMPARE(valueWidget->text().left(3), QLatin1String("153"));

    // editable property
    if (m_mayTestRating) {
        auto ratingWidget = widget->findChild<KRatingWidget *>(QStringLiteral("rating"));
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
    auto widget = std::make_unique<Baloo::FileMetaDataWidget>();

    QSignalSpy spy(widget.get(), &Baloo::FileMetaDataWidget::metaDataRequestFinished);
    widget->setItems({KFileItem{QUrl::fromLocalFile(m_sampleFiles["multivalue"])}});
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
    auto artistWidget = widget->findChild<QLabel *>(QStringLiteral("artist"));
    QVERIFY2(artistWidget, "artist not found");
    QCOMPARE(artistWidget->text(), QStringLiteral("Artist1 and Artist2"));
    auto genreWidget = widget->findChild<QLabel *>(QStringLiteral("genre"));
    QVERIFY2(genreWidget, "genre not found");
    QCOMPARE(genreWidget->text(), QStringLiteral("Genre1, Genre2, and Genre3"));
}

#include "moc_filemetadatawidgettest.cpp"
