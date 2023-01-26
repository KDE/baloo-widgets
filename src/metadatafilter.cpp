/*
    SPDX-FileCopyrightText: 2012 Vishesh Handa <me@vhanda.in>

    Adapted from KFileMetadataWidget
    SPDX-FileCopyrightText: 2008 Sebastian Trueg <trueg@kde.org>
    SPDX-FileCopyrightText: 2009-2010 Peter Penz <peter.penz@gmx.at>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "metadatafilter.h"

#include <KConfig>
#include <KConfigGroup>

using namespace Baloo;

MetadataFilter::MetadataFilter()
{
    initMetaInformationSettings();
}

MetadataFilter::~MetadataFilter() = default;

void MetadataFilter::initMetaInformationSettings()
{
    const int currentVersion = 13; // increase version, if the blacklist of disabled
    // properties should be updated

    KConfig config(QStringLiteral("baloofileinformationrc"), KConfig::NoGlobals);
    if (config.group("Misc").readEntry("version", 0) < currentVersion) {
        // The resource file is read the first time. Assure
        // that some meta information is disabled per default.

        // clear old info
        config.deleteGroup("Show");
        KConfigGroup settings = config.group("Show");

        static const char *const disabledProperties[] = {
            "width",
            "height", // replaced by dimensions
            "comment",
            "contentSize",
            "depends",
            "lastModified",
            "created",
            "contentCreated",
            "mimeType",
            "url",
            "channels",
            "fileName",
            "fileSize",
            "kfileitem#owner",
            "kfileitem#group",
            "kfileitem#permissions",
            "replayGainAlbumPeak",
            "replayGainAlbumGain",
            "replayGainTrackPeak",
            "replayGainTrackGain",
            "embeddedRating",
            "lyrics",
            "photoWhiteBalance",
            "photoMeteringMode",
            "photoSharpness",
            "photoSaturation",
            "photoPixelXDimension",
            "photoPixelYDimension",
            "photoGpsLongitude",
            "photoGpsLatitude",
        };

        for (const auto property : disabledProperties) {
            settings.writeEntry(property, false);
        }

        // mark the group as initialized
        config.group("Misc").writeEntry("version", currentVersion);
    }
}

QVariantMap MetadataFilter::filter(const QVariantMap &data)
{
    if (data.isEmpty())
        return data;

    QVariantMap finalData(data);

    //
    // Remove all items, that are marked as hidden in kmetainformationrc
    KConfig config(QStringLiteral("baloofileinformationrc"), KConfig::NoGlobals);
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
