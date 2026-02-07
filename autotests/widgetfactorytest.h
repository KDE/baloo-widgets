/*
    SPDX-FileCopyrightText: 2026 Méven Car <meven@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef WIDGETFACTORYTEST_H
#define WIDGETFACTORYTEST_H

#include <QObject>

class WidgetFactoryTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testFormatDateTime_data();
    void testFormatDateTime();

private:
};

#endif // WIDGETFACTORYTEST_H
