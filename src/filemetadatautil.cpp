/*****************************************************************************
 * Copyright (C) 2019 by Stefan Brüns <stefan.bruens@rwth-aachen.de>         *
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

#include "filemetadatautil_p.h"
#include <KFileMetaData/UserMetaData>
#include <KFileMetaData/PropertyInfo>

#include <algorithm>

namespace Baloo
{
namespace Private
{

QVariantMap convertUserMetaData(const KFileMetaData::UserMetaData& metaData)
{
    using Attribute = KFileMetaData::UserMetaData::Attribute;
    QVariantMap properties;

    QFlags<Attribute> attributes = metaData.queryAttributes(Attribute::Tags | Attribute::Rating |
                                                            Attribute::Comment | Attribute::OriginUrl);

    if (attributes & Attribute::Tags) {
        QStringList tags = metaData.tags();
        if (!tags.isEmpty()) {
            properties.insert(QStringLiteral("tags"), tags);
        }
    }

    if (attributes & Attribute::Rating) {
        int rating = metaData.rating();
        if (rating) {
            properties.insert(QStringLiteral("rating"), rating);
        }
    }

    if (attributes & Attribute::Comment) {
        QString comment = metaData.userComment();
        if (!comment.isEmpty()) {
            properties.insert(QStringLiteral("userComment"), comment);
        }
    }

    if (attributes & Attribute::OriginUrl) {
        const QString originUrl = metaData.originUrl().toDisplayString();
        if (!originUrl.isEmpty()) {
            properties.insert(QStringLiteral("originUrl"), originUrl);
        }
    }

    return properties;
}

QVariantMap toNamedVariantMap(const KFileMetaData::PropertyMap& propMap)
{
    QVariantMap map;
    if (propMap.isEmpty()) {
        return map;
    }

    using entry = std::pair<const KFileMetaData::Property::Property&, const QVariant&>;

    auto begin = propMap.constKeyValueBegin();

    while (begin != propMap.constKeyValueEnd()) {
        auto key = (*begin).first;
        KFileMetaData::PropertyInfo property(key);
        auto rangeEnd = std::find_if(begin, propMap.constKeyValueEnd(),
            [key](const entry& e) { return e.first != key; });

        auto distance = std::distance(begin, rangeEnd);
        if (distance > 1) {
            QVariantList list;
            list.reserve(static_cast<int>(distance));
            std::for_each(begin, rangeEnd, [&list](const entry& s) { list.append(s.second); });
            map.insert(property.name(), list);
        } else {
            map.insert(property.name(), (*begin).second);
        }
        begin = rangeEnd;
    }

    return map;
}

} // namespace KFMPrivate
} // namespace Baloo
