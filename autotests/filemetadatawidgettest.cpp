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

#include "filemetadatawidgettest.h"

#include <QObject>
#include <QTest>
#include <QMap>
#include <QLabel>
#include <QSignalSpy>
#include <QMetaType>
#include <QProcess>
#include <QStandardPaths>
#include <QDebug>

#include <KFileItem>
#include <KRatingWidget>
#include <KConfig>
#include <KConfigGroup>

QTEST_MAIN(FileMetadataWidgetTest)

void FileMetadataWidgetTest::initTestCase()
{
    qRegisterMetaType<KFileItemList>("KFileItemList");
    
    qputenv("LC_ALL", "en_US.UTF-8");
    
    QStandardPaths::setTestModeEnabled(true);

    KConfig balooConfig("baloofilerc", KConfig::NoGlobals);
    KConfigGroup balooSettings = balooConfig.group("General");
    // If we use .writePathEntry here, the test will fail.
    balooSettings.writeEntry(QStringLiteral("folders"), QString());
    
    // Ensure show configuration
    KConfig config("baloofileinformationrc", KConfig::NoGlobals);
    KConfigGroup settings = config.group("Show");
    const auto keys = settings.keyList();
    for (const auto &key: keys) {
        settings.writeEntry(key, true);
    }

    const QString exe = QStandardPaths::findExecutable(QStringLiteral("setfattr"));

    if (exe.isEmpty()) {
        return;
    }
    
    const QStringList args = {QStringLiteral("--name=user.baloo.rating"),
            QStringLiteral("--value=5") ,
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
    m_widget->setItems(KFileItemList() << QUrl());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_widget->items().count(), 0);
}

void FileMetadataWidgetTest::shouldSignalOnceWithEmptyFile()
{
    QSignalSpy spy(m_widget, &Baloo::FileMetaDataWidget::metaDataRequestFinished);
    m_widget->setItems(KFileItemList());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_widget->items().count(), 0);
}

void FileMetadataWidgetTest::shouldSignalOnceFile()
{
    QSignalSpy spy(m_widget, &Baloo::FileMetaDataWidget::metaDataRequestFinished);
    m_widget->setItems(KFileItemList() 
        << QUrl::fromLocalFile(QFINDTESTDATA("samplefiles/testtagged.m4a"))
    );
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_widget->items().count(), 1);
    
}

void FileMetadataWidgetTest::shouldSignalOnceFiles()
{
    QSignalSpy spy(m_widget, &Baloo::FileMetaDataWidget::metaDataRequestFinished);
    m_widget->setItems(KFileItemList() 
        << QUrl::fromLocalFile(QFINDTESTDATA("samplefiles/test.mp3"))
        << QUrl::fromLocalFile(QFINDTESTDATA("samplefiles/testtagged.mp3"))
        << QUrl::fromLocalFile(QFINDTESTDATA("samplefiles/testtagged.m4a"))
    );
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_widget->items().count(), 3);
}

void FileMetadataWidgetTest::shouldShowProperties()
{
    QSignalSpy spy(m_widget, &Baloo::FileMetaDataWidget::metaDataRequestFinished);
    m_widget->setItems(KFileItemList() 
        << QUrl::fromLocalFile(QFINDTESTDATA("samplefiles/testtagged.mp3"))
    );
    
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
    
    // simple property
    QLabel* valueWidget = m_widget->findChild<QLabel*>("kfileitem#type");
    QVERIFY2(valueWidget, "Type data missing");
    QCOMPARE(valueWidget->text(), QLatin1String("MP3 audio"));
    
    if (m_mayTestRating) {
        // editable property
        KRatingWidget* ratingWidget = m_widget->findChild<KRatingWidget*>("rating");
        QVERIFY2(ratingWidget, "Rating data missing");
        QCOMPARE(ratingWidget->rating(), 5u);
    } else {
        qDebug() << "Skipped 'Rating' test";
    }
    // async property
    valueWidget = m_widget->findChild<QLabel*>("albumArtist");
    QVERIFY2(valueWidget, "albumArtist data was not found");
    QCOMPARE(valueWidget->text(), QLatin1String("Bill Laswell"));
    
}

void FileMetadataWidgetTest::shouldShowCommonProperties()
{
    QSignalSpy spy(m_widget, &Baloo::FileMetaDataWidget::metaDataRequestFinished);
    m_widget->setItems(KFileItemList() 
        << QUrl::fromLocalFile(QFINDTESTDATA("samplefiles/testtagged.mp3"))
        << QUrl::fromLocalFile(QFINDTESTDATA("samplefiles/testtagged.m4a"))
    );
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
    
    // simple property
    QLabel* valueWidget = m_widget->findChild<QLabel*>("kfileitem#type");
    QVERIFY(!valueWidget);
    
    valueWidget = m_widget->findChild<QLabel*>("kfileitem#totalSize");
    // circumvent i18n formatting
    QCOMPARE(valueWidget->text().left(3), QLatin1String("153"));
    
    // editable property
    if (m_mayTestRating) {
        KRatingWidget* ratingWidget = m_widget->findChild<KRatingWidget*>("rating");
        QVERIFY2(ratingWidget, "Rating data missing");
        QCOMPARE(ratingWidget->rating(), 5u);
    } else {
        qDebug() << "Skipped 'Rating' test";
    }
    // async property
    // FIXME: Make this pass
    // QCOMPARE( map->value("Album Artist:"), QLatin1String("Bill Laswell"));
}

