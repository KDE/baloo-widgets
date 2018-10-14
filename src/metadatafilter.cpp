/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2012  Vishesh Handa <me@vhanda.in>

    Adapted from KFileMetadataWidget
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

using namespace Baloo;

MetadataFilter::MetadataFilter(QObject* parent): QObject(parent)
{
    initMetaInformationSettings();
}

MetadataFilter::~MetadataFilter()
{

}

void MetadataFilter::initMetaInformationSettings()
{
    const int currentVersion = 10; // increase version, if the blacklist of disabled
    // properties should be updated

    KConfig config("baloofileinformationrc", KConfig::NoGlobals);
    if (config.group("Misc").readEntry("version", 0) < currentVersion) {
        // The resource file is read the first time. Assure
        // that some meta information is disabled per default.

        // clear old info
        config.deleteGroup("Show");
        KConfigGroup settings = config.group("Show");

        static const char* const disabledProperties[] = {
            "comment",
            "contentSize",
            "depends",
            "isPartOf",
            "lastModified",
            "created",
            "contentCreated",
            "mimeType",
            "plainTextContent",
            "url",
            "hasPart",
            "averageBitrate",
            "channels",
            "fileName",
            "fileSize",
            "Exif.Photo.ApertureValue",
            "Exif.Photo.ExposureBiasValue",
            "Exif.Photo.ExposureTime",
            "Exif.Photo.Flash",
            "Exif.Photo.FocalLength",
            "Exif.Photo.FocalLengthIn35mmFilm",
            "Exif.Photo.IsoSpeedRatings",
            "Exif.Photo.MeteringMode",
            "Exif.Photo.Orientation",
            "Exif.Photo.WhiteBalance",
            "Exif.Image.Make",
            "Exif.Image.Model",
            "Exif.Image.DateTime",
            "Exif.Image.Orientation",
            "kfileitem#owner",
            "kfileitem#group",
            "kfileitem#permissions",
            "replayGainAlbumPeak",
            "replayGainAlbumGain",
            "replayGainTrackPeak",
            "replayGainTrackGain",
            "embeddedRating",
            nullptr // mandatory last entry
        };

        for (int i = 0; disabledProperties[i] != nullptr; ++i) {
            settings.writeEntry(disabledProperties[i], false);
        }

        // mark the group as initialized
        config.group("Misc").writeEntry("version", currentVersion);
    }
}

QVariantMap MetadataFilter::filter(const QVariantMap& data)
{
    if( data.isEmpty() )
        return data;

    QVariantMap finalData(data);

    //
    // Remove all items, that are marked as hidden in kmetainformationrc
    KConfig config("baloofileinformationrc", KConfig::NoGlobals);
    KConfigGroup settings = config.group("Show");
    QVariantMap::iterator it = finalData.begin();
    while (it != finalData.end()) {
        const QString uriString = it.key();
        if (!settings.readEntry(uriString, true)) {
            it = finalData.erase(it);
        } else {
            ++it;
        }
    }

    return finalData;
}
