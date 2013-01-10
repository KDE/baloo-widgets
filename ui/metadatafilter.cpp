/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2012  Vishesh Handa <me@vhanda.in>

    Adapated from KFileMetadataWidget
    Copyright (C) 2008 by Sebastian Trueg <trueg@kde.org>
    Copyright (C) 2009-2010 by Peter Penz <peter.penz@gmx.at>

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


#include "metadatafilter.h"

#include <KConfig>
#include <KConfigGroup>
#include <KDebug>

#include <Nepomuk2/Types/Property>
#include <Nepomuk2/Variant>

#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/NAO>
#include <Nepomuk2/Vocabulary/NCO>
#include <Nepomuk2/Vocabulary/NMM>

using namespace Soprano::Vocabulary;
using namespace Nepomuk2::Vocabulary;

namespace Nepomuk2 {

MetadataFilter::MetadataFilter(QObject* parent): QObject(parent)
{
    initMetaInformationSettings();
}

MetadataFilter::~MetadataFilter()
{

}

void MetadataFilter::initMetaInformationSettings()
{
    const int currentVersion = 5; // increase version, if the blacklist of disabled
    // properties should be updated

    KConfig config("kmetainformationrc", KConfig::NoGlobals);
    if (config.group("Misc").readEntry("version", 0) < currentVersion) {
        // The resource file is read the first time. Assure
        // that some meta information is disabled per default.

        // clear old info
        config.deleteGroup("Show");
        KConfigGroup settings = config.group("Show");

        static const char* const disabledProperties[] = {
            "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#comment",
            "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentSize",
            "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#depends",
            "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#isPartOf",
            "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#lastModified",
            "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#created",
            "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentCreated",
            "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#mimeType",
            "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#plainTextContent",
            "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#url",
            "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#hasPart",
            "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#averageBitrate",
            "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#channels",
            "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#fileName",
            "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#fileSize",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#apertureValue",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureBiasValue",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureTime",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#flash",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#focalLength",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#focalLengthIn35mmFilm",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#isoSpeedRatings",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#make",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#meteringMode",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#model",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#orientation",
            "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#whiteBalance",
            "http://www.semanticdesktop.org/ontologies/2007/08/15/nao#modified",
            "http://www.semanticdesktop.org/ontologies/2007/08/15/nao#lastModified",
            "http://www.semanticdesktop.org/ontologies/2007/08/15/nao#created",
            "http://www.semanticdesktop.org/ontologies/2007/08/15/nao#annotation",
            "http://www.semanticdesktop.org/ontologies/2007/08/15/nao#hasSubResource",
            "http://www.w3.org/1999/02/22-rdf-syntax-ns#type",
            "kfileitem#owner",
            "kfileitem#permissions",
            "kfileitem#modified",
            0 // mandatory last entry
        };

        for (int i = 0; disabledProperties[i] != 0; ++i) {
            settings.writeEntry(disabledProperties[i], false);
        }

        // mark the group as initialized
        config.group("Misc").writeEntry("version", currentVersion);
    }
}

QHash<QUrl, Variant> MetadataFilter::filter(const QHash<QUrl, Nepomuk2::Variant>& data)
{
    if( data.isEmpty() )
        return data;

    QList<QUrl> types = data.value( RDF::type() ).toUrlList();

    //
    // Special filtering for certain types
    //

    if( types.contains( NAO::Tag() ) ) {
        QHash<QUrl, Variant> finalData;

        if( data.contains(NAO::identifier()) )
            finalData.insert( NAO::identifier(), data.value(NAO::identifier()) );
        if( data.contains(NAO::prefLabel()) )
            finalData.insert( NAO::prefLabel(), data.value(NAO::prefLabel()) );

        return finalData;
    }

    QHash<QUrl, Variant> finalData( data );

    // Remove editable stuff for contacts and albums
    if( types.contains( NCO::Contact() ) || types.contains( NMM::MusicAlbum() ) ) {
        finalData.remove( NAO::hasTag() );
        finalData.remove( NAO::numericRating() );
        finalData.remove( NAO::description() );
    }

    //
    // Remove all the meta-properties
    //
    finalData.remove( RDF::type() );

    finalData.remove( NAO::lastModified() );
    finalData.remove( NAO::created() );
    finalData.remove( NAO::userVisible() );

    //
    // Remove all items, that are marked as hidden in kmetainformationrc
    KConfig config("kmetainformationrc", KConfig::NoGlobals);
    KConfigGroup settings = config.group("Show");
    QHash<QUrl, Variant>::iterator it = finalData.begin();
    while (it != finalData.end()) {
        const QString uriString = it.key().toString();
        if (!settings.readEntry(uriString, true) || !Types::Property(it.key()).userVisible()) {
            it = finalData.erase(it);
        } else {
            ++it;
        }
    }

    return finalData;
}

}
