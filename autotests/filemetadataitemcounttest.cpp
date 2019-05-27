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

#include "filemetadataitemcounttest.h"

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


QTEST_MAIN(FileMetadataItemCountTest)

void FileMetadataItemCountTest::initTestCase()
{
    qRegisterMetaType<KFileItemList>("KFileItemList");
    
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
}

void FileMetadataItemCountTest::init()
{
    m_widget = new Baloo::FileMetaDataWidget;
}

void FileMetadataItemCountTest::cleanup()
{
    delete m_widget;
}

void FileMetadataItemCountTest::testItemCount()
{
    // the number of items will increase in the future adding the file creation time field
    // when the system has KIO 5.58, glibc 2.28, linux 4.11 and a filesystem storing file creation times (btrfs, ext4...)
    // The expectedItems count will need to be updated
    const int expectedItems = 20;
    const int widgetsPerItem = 2;
    
    QSignalSpy spy(m_widget, &Baloo::FileMetaDataWidget::metaDataRequestFinished);
    m_widget->setItems(KFileItemList() 
        << QUrl::fromLocalFile(QFINDTESTDATA("samplefiles/testtagged.mp3"))
    );
    
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
    
    QList<QWidget*> items = m_widget->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
    QCOMPARE(items.count(), expectedItems * widgetsPerItem);

}
