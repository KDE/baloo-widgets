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

#include "config.h"
#include "filemetadatawidgettest.h"

#include <QObject>
#include <KFileItem>
#include <QTest>
#include <QSignalSpy>
#include <QMetaType>

QTEST_MAIN(FileMetadataWidgetTest)

void FileMetadataWidgetTest::initTestCase()
{
    qRegisterMetaType<KFileItemList>("KFileItemList");
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
        << QUrl::fromLocalFile(QStringLiteral("%1/testtagged.m4a").arg(TESTS_SAMPLE_FILES_PATH))
    );
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_widget->items().count(), 1);
    
}

void FileMetadataWidgetTest::shouldSignalOnceFiles()
{
    QSignalSpy spy(m_widget, &Baloo::FileMetaDataWidget::metaDataRequestFinished);
    m_widget->setItems(KFileItemList() 
        << QUrl::fromLocalFile(QStringLiteral("%1/test.mp3").arg(TESTS_SAMPLE_FILES_PATH))
        << QUrl::fromLocalFile(QStringLiteral("%1/testtagged.mp3").arg(TESTS_SAMPLE_FILES_PATH))
        << QUrl::fromLocalFile(QStringLiteral("%1/testtagged.m4a").arg(TESTS_SAMPLE_FILES_PATH))
    );
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_widget->items().count(), 3);
    
}
