/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QMimeDatabase>
#include <QDebug>

#include <KFileMetaData/SimpleExtractionResult>
#include <KFileMetaData/ExtractorCollection>
#include <KFileMetaData/Extractor>
#include <KFileMetaData/PropertyInfo>

#include <iostream>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addPositionalArgument(QLatin1String("url"), QString());
    parser.process(app);

    if (parser.positionalArguments().size() != 1) {
        parser.showHelp(1);
    }

    const QString url = parser.positionalArguments().first();
    const QString mimetype = QMimeDatabase().mimeTypeForFile(url).name();

    KFileMetaData::SimpleExtractionResult result(url, mimetype, KFileMetaData::ExtractionResult::ExtractMetaData);
    KFileMetaData::ExtractorCollection collection;

    QList<KFileMetaData::Extractor *> exList = collection.fetchExtractors(mimetype);

    Q_FOREACH (KFileMetaData::Extractor *ex, exList) {
        ex->extract(&result);
    }

    QByteArray arr;
    QDataStream stream(&arr, QIODevice::WriteOnly);

    QVariantMap map;

    QMapIterator<KFileMetaData::Property::Property, QVariant> it(result.properties());
    while (it.hasNext()) {
        it.next();
        KFileMetaData::PropertyInfo pi(it.key());
        map.insert(pi.name(), it.value());
    }
    stream << map;

    qDebug() << map;
    std::cout << arr.toBase64().constData();

    return 0;
}