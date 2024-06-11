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
    KConfigGroup balooSettings = balooConfig.group(QStringLiteral("General"));
    // If we use .writePathEntry here, the test will fail.
    balooSettings.writeEntry(QStringLiteral("folders"), QString());

    // Ensure show configuration
    KConfig config(QStringLiteral("baloofileinformationrc"), KConfig::NoGlobals);
    KConfigGroup settings = config.group(QStringLiteral("Show"));
    const auto keys = settings.keyList();
    for (const auto &key : keys) {
        settings.writeEntry(key, true);
    }
}

void FileMetadataItemCountTest::testItemCount()
{
    auto widget = std::make_unique<Baloo::FileMetaDataWidget>();

    // TODO this test needs improving to not have a "random" number but actually check things
    int expectedItems = 20;
    const int widgetsPerItem = 2;

    QSignalSpy spy(widget.get(), &Baloo::FileMetaDataWidget::metaDataRequestFinished);
    const auto fileUrl = QUrl::fromLocalFile(QFINDTESTDATA("samplefiles/testtagged.mp3"));
    const KFileItem fileItem = KFileItem{fileUrl};
    if (fileItem.time(KFileItem::CreationTime).isValid()) {
        ++expectedItems;
    }
    widget->setItems({fileItem});

    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);

    QList<QWidget *> items = widget->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly);
    QCOMPARE(items.count(), expectedItems * widgetsPerItem);
}

#include "moc_filemetadataitemcounttest.cpp"
