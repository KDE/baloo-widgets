/*
    Copyright (C) 2012-2015  Vishesh Handa <vhanda@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "filemetadatawidget.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QFileInfo>
#include <kfileitem.h>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("FileMetadataWidgetApp"));

    QCommandLineParser parser;
    parser.addPositionalArgument(QStringLiteral("filename"), QStringLiteral("files"));
    parser.process(app);

    Baloo::FileMetaDataWidget* widget = new Baloo::FileMetaDataWidget();

    KFileItemList list;
    for (const QString& path : parser.positionalArguments()) {
        QFileInfo fi(path);
        list << KFileItem(QUrl::fromLocalFile(fi.absoluteFilePath()), QString(), mode_t());
    }

    widget->show();
    widget->setItems(list);

    return app.exec();
}
