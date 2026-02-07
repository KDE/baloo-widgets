/*
    SPDX-FileCopyrightText: 2026 Méven Car <meven@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "widgetfactorytest.h"
#include "../src/widgetfactory_p.h"

#include <QTest>

void initLocale()
{
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
}
Q_CONSTRUCTOR_FUNCTION(initLocale)

QTEST_MAIN(WidgetFactoryTest)

void WidgetFactoryTest::testFormatDateTime_data()
{
    QTest::addColumn<QVariant>("date");
    QTest::addColumn<QString>("expected");

    auto dt = QDateTime::currentDateTimeUtc();
    QTest::addRow("today") << QVariant(dt) << QStringLiteral("Just now");

    dt = QDateTime::currentDateTimeUtc().addSecs(11 * 3600); // in 11 hours
    // Utc + 11
    auto tz = QTimeZone("UTC+11:00");
    dt.setTimeZone(tz);
    Q_ASSERT(tz.isValid());

    QTest::addRow("today, in 11 hours UTC+11") << QVariant(dt) << QStringLiteral("Just now");

    dt = QDateTime::currentDateTimeUtc().addSecs(-11 * 3600); // -11 hours
    // Utc - 11
    tz = QTimeZone("UTC-11:00");
    dt.setTimeZone(tz);
    Q_ASSERT(tz.isValid());

    QTest::addRow("today, 11 hours ago UTC-11") << QVariant(dt) << QStringLiteral("Just now");

    QTest::addRow("Date default tz") << QVariant(QDate::currentDate()) << QStringLiteral("Today");

    dt = QDateTime();
    QTest::addRow("invalid Datetime") << QVariant(dt) << QStringLiteral("");

    dt = QDateTime::currentDateTimeUtc();
    tz = QTimeZone("invalid");
    dt.setTimeZone(tz);
    Q_ASSERT(!tz.isValid());

    QTest::addRow("invalid timezone") << QVariant(dt) << QStringLiteral("");
}

void WidgetFactoryTest::testFormatDateTime()
{
    QFETCH(QVariant, date);
    QFETCH(QString, expected);

    // local timezone set to UTC
    QTimeZone tz = QTimeZone(QTimeZone::UTC);

    QCOMPARE(formatDateTime(date, QLocale::LongFormat, tz), expected);
}

#include "moc_widgetfactorytest.cpp"
