/*
    SPDX-FileCopyrightText: 2012-2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "filemetadatawidget.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QFileInfo>
#include <kfileitem.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("FileMetadataWidgetApp"));

    QCommandLineParser parser;
    parser.addPositionalArgument(QStringLiteral("filename"), QStringLiteral("files"));
    parser.process(app);

    Baloo::FileMetaDataWidget *widget = new Baloo::FileMetaDataWidget();

    KFileItemList list;
    const auto args = parser.positionalArguments();
    for (const QString &path : args) {
        QFileInfo fi(path);
        list << KFileItem(QUrl::fromLocalFile(fi.absoluteFilePath()), QString(), mode_t());
    }

    widget->show();
    widget->setItems(list);

    return app.exec();
}
