/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "extractorutil_p.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDataStream>
#include <QDebug>
#include <QFile>
#include <QMimeDatabase>

#include <KFileMetaData/Extractor>
#include <KFileMetaData/ExtractorCollection>
#include <KFileMetaData/MimeUtils>
#include <KFileMetaData/SimpleExtractionResult>

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

    const QString url = parser.positionalArguments().constFirst();
    const QString mimetype = KFileMetaData::MimeUtils::strictMimeType(url, QMimeDatabase()).name();

    KFileMetaData::SimpleExtractionResult result(url, mimetype, KFileMetaData::ExtractionResult::ExtractMetaData);
    KFileMetaData::ExtractorCollection collection;

    const QList<KFileMetaData::Extractor *> exList = collection.fetchExtractors(mimetype);

    for (KFileMetaData::Extractor *ex : exList) {
        ex->extract(&result);
    }

    QFile out;
    out.open(stdout, QIODevice::WriteOnly);
    QDataStream stream(&out);

    stream << result.properties();

    return 0;
}
