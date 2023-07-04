/*
    SPDX-FileCopyrightText: 2018 Michael Heidelbach <ottwolt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "filemetadataitemcounttest.h"

#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

#include <KConfig>
#include <KConfigGroup>
#include <KFileItem>

#include <memory>

QTEST_MAIN(FileMetadataItemCountTest)

void FileMetadataItemCountTest::initTestCase()
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
}

void FileMetadataItemCountTest::testItemCount()
{
    auto widget = std::make_unique<Baloo::FileMetaDataWidget>();

    // the number of items will increase in the future adding the file creation time field
    // when the system has KIO 5.58, glibc 2.28, linux 4.11 and a filesystem storing file creation times (btrfs, ext4...)
    // The expectedItems count will need to be updated
    const int expectedItems = 20;
    const int widgetsPerItem = 2;

    QSignalSpy spy(widget.get(), &Baloo::FileMetaDataWidget::metaDataRequestFinished);
    const auto fileUrl = QUrl::fromLocalFile(QFINDTESTDATA("samplefiles/testtagged.mp3"));
    widget->setItems({KFileItem{fileUrl}});

    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);

    QList<QWidget *> items = widget->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly);
    QCOMPARE(items.count(), expectedItems * widgetsPerItem);
}

#include "moc_filemetadataitemcounttest.cpp"
