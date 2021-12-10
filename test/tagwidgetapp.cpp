/*
    SPDX-FileCopyrightText: 2010 Sebastian Trueg <trueg@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "tagwidgettest.h"

#include <QApplication>
#include <QCoreApplication>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("TagWidgetApp"));
    TagWidgetTest tw;
    tw.show();
    return app.exec();
}
