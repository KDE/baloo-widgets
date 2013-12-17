/*****************************************************************************
 * Copyright (C) 2010 by Peter Penz <peter.penz@gmx.at>                      *
 *                                                                           *
 * This library is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Library General Public               *
 * License as published by the Free Software Foundation; either              *
 * version 2 of the License, or (at your option) any later version.          *
 *                                                                           *
 * This library is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Library General Public License for more details.                          *
 *                                                                           *
 * You should have received a copy of the GNU Library General Public License *
 * along with this library; see the file COPYING.LIB.  If not, write to      *
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 * Boston, MA 02110-1301, USA.                                               *
 *****************************************************************************/

#include "knfotranslator_p.h"
#include <klocale.h>
#include <kstandarddirs.h>

#include <kurl.h>

struct TranslationItem {
    const char* const key;
    const char* const context;
    const char* const value;
};

// TODO: a lot of NFOs are missing yet
static const TranslationItem g_translations[] = {
    { "comment", I18N_NOOP2_NOSTRIP("@label", "Comment") },
    { "contentCreated", I18N_NOOP2_NOSTRIP("@label creation date", "Created") },
    { "contentSize", I18N_NOOP2_NOSTRIP("@label file content size", "Size") },
    { "depends", I18N_NOOP2_NOSTRIP("@label file depends from", "Depends") },
    { "description", I18N_NOOP2_NOSTRIP("@label", "Description") },
    { "generator", I18N_NOOP2_NOSTRIP("@label Software used to generate content", "Generator") },
    { "hasPart", I18N_NOOP2_NOSTRIP("@label see http://www.semanticdesktop.org/ontologies/2007/01/19/nie#hasPart", "Has Part") },
    { "hasLogicalPart", I18N_NOOP2_NOSTRIP("@label see http://www.semanticdesktop.org/ontologies/2007/01/19/nie#hasLogicalPart", "Has Logical Part") },
    { "isPartOf", I18N_NOOP2_NOSTRIP("@label parent directory", "Part of") },
    { "keyword", I18N_NOOP2_NOSTRIP("@label", "Keyword") },
    { "lastModified", I18N_NOOP2_NOSTRIP("@label modified date of file", "Modified") },
    { "mimeType", I18N_NOOP2_NOSTRIP("@label", "MIME Type") },
    { "plainTextContent", I18N_NOOP2_NOSTRIP("@label", "Content") },
    { "relatedTo", I18N_NOOP2_NOSTRIP("@label", "Related To") },
    { "subject", I18N_NOOP2_NOSTRIP("@label", "Subject") },
    { "title", I18N_NOOP2_NOSTRIP("@label music title", "Title") },
    { "url", I18N_NOOP2_NOSTRIP("@label file URL", "Location") },
    { "creator", I18N_NOOP2_NOSTRIP("@label", "Creator") },
    { "averageBitrate", I18N_NOOP2_NOSTRIP("@label", "Average Bitrate") },
    { "channels", I18N_NOOP2_NOSTRIP("@label", "Channels") },
    { "characterCount", I18N_NOOP2_NOSTRIP("@label number of characters", "Characters") },
    { "codec",  I18N_NOOP2_NOSTRIP("@label", "Codec") },
    { "colorDepth", I18N_NOOP2_NOSTRIP("@label", "Color Depth") },
    { "duration", I18N_NOOP2_NOSTRIP("@label", "Duration") },
    { "fileName", I18N_NOOP2_NOSTRIP("@label", "Filename") },
    { "hasHash", I18N_NOOP2_NOSTRIP("@label", "Hash") },
    { "height", I18N_NOOP2_NOSTRIP("@label", "Height") },
    { "interlaceMode", I18N_NOOP2_NOSTRIP("@label", "Interlace Mode") },
    { "lineCount", I18N_NOOP2_NOSTRIP("@label number of lines", "Lines") },
    { "programmingLanguage", I18N_NOOP2_NOSTRIP("@label", "Programming Language") },
    { "sampleRate", I18N_NOOP2_NOSTRIP("@label", "Sample Rate") },
    { "width", I18N_NOOP2_NOSTRIP("@label", "Width") },
    { "wordCount", I18N_NOOP2_NOSTRIP("@label number of words", "Words") },
    { "apertureValue", I18N_NOOP2_NOSTRIP("@label EXIF aperture value", "Aperture") },
    { "exposureBiasValue", I18N_NOOP2_NOSTRIP("@label EXIF", "Exposure Bias Value") },
    { "exposureTime", I18N_NOOP2_NOSTRIP("@label EXIF", "Exposure Time") },
    { "flash", I18N_NOOP2_NOSTRIP("@label EXIF", "Flash") },
    { "focalLength", I18N_NOOP2_NOSTRIP("@label EXIF", "Focal Length") },
    { "focalLengthIn35mmFilm", I18N_NOOP2_NOSTRIP("@label EXIF", "Focal Length 35 mm") },
    { "isoSpeedRatings", I18N_NOOP2_NOSTRIP("@label EXIF", "ISO Speed Ratings") },
    { "make", I18N_NOOP2_NOSTRIP("@label EXIF", "Make") },
    { "meteringMode", I18N_NOOP2_NOSTRIP("@label EXIF", "Metering Mode") },
    { "model", I18N_NOOP2_NOSTRIP("@label EXIF", "Model") },
    { "orientation", I18N_NOOP2_NOSTRIP("@label EXIF", "Orientation") },
    { "whiteBalance", I18N_NOOP2_NOSTRIP("@label EXIF", "White Balance") },
    { "director",  I18N_NOOP2_NOSTRIP("@label video director", "Director") },
    { "genre",  I18N_NOOP2_NOSTRIP("@label music genre", "Genre") },
    { "musicAlbum", I18N_NOOP2_NOSTRIP("@label music album", "Album") },
    { "performer", I18N_NOOP2_NOSTRIP("@label", "Performer") },
    { "releaseDate", I18N_NOOP2_NOSTRIP("@label", "Release Date") },
    { "trackNumber", I18N_NOOP2_NOSTRIP("@label music track number", "Track") },
    { "created", I18N_NOOP2_NOSTRIP("@label resource created time", "Resource Created")},
    { "hasSubResource", I18N_NOOP2_NOSTRIP("@label", "Sub Resource")},
    { "lastModified", I18N_NOOP2_NOSTRIP("@label resource last modified", "Resource Modified")},
    { "numericRating", I18N_NOOP2_NOSTRIP("@label", "Numeric Rating")},
    { "copiedFrom", I18N_NOOP2_NOSTRIP("@label", "Copied From")},
    { "firstUsage", I18N_NOOP2_NOSTRIP("@label", "First Usage")},
    { "lastUsage", I18N_NOOP2_NOSTRIP("@label", "Last Usage")},
    { "usageCount", I18N_NOOP2_NOSTRIP("@label", "Usage Count")},
    { "unixFileGroup", I18N_NOOP2_NOSTRIP("@label", "Unix File Group")},
    { "unixFileMode", I18N_NOOP2_NOSTRIP("@label", "Unix File Mode")},
    { "unixFileOwner", I18N_NOOP2_NOSTRIP("@label", "Unix File Owner")},
    { "type", I18N_NOOP2_NOSTRIP("@label file type", "Type") },
    { "translation.fuzzy", I18N_NOOP2_NOSTRIP("@label Number of fuzzy translations", "Fuzzy Translations") },
    { "translation.last_translator", I18N_NOOP2_NOSTRIP("@label Name of last translator", "Last Translator") },
    { "translation.obsolete", I18N_NOOP2_NOSTRIP("@label Number of obsolete translations", "Obsolete Translations") },
    { "translation.source_date", I18N_NOOP2_NOSTRIP("@label", "Translation Source Date") },
    { "translation.total", I18N_NOOP2_NOSTRIP("@label Number of total translations", "Total Translations") },
    { "translation.translated", I18N_NOOP2_NOSTRIP("@label Number of translated strings", "Translated") },
    { "translation.translation_date", I18N_NOOP2_NOSTRIP("@label", "Translation Date") },
    { "translation.untranslated", I18N_NOOP2_NOSTRIP("@label Number of untranslated strings" , "Untranslated") },
    { 0, 0, 0 } // mandatory last entry
};

class KNfoTranslatorSingleton
{
public:
    KNfoTranslator instance;
};
K_GLOBAL_STATIC(KNfoTranslatorSingleton, s_nfoTranslator)

KNfoTranslator& KNfoTranslator::instance()
{
    return s_nfoTranslator->instance;
}

QString KNfoTranslator::translation(const QString& propName) const
{
    if (m_hash.contains(propName)) {
        return m_hash.value(propName);
    }

    return propName;
}

KNfoTranslator::KNfoTranslator() :
    m_hash()
{
    const TranslationItem* item = &g_translations[0];
    while (item->key != 0) {
        m_hash.insert(item->key, i18nc(item->context, item->value));
        ++item;
    }
}

KNfoTranslator::~KNfoTranslator()
{
}
