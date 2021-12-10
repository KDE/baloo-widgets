/*
    SPDX-FileCopyrightText: 2019 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "filemetadatautil_p.h"
#include <KFileMetaData/PropertyInfo>
#include <KFileMetaData/UserMetaData>

#include <algorithm>

namespace Baloo
{
namespace Private
{
QVariantMap convertUserMetaData(const KFileMetaData::UserMetaData &metaData)
{
    using Attribute = KFileMetaData::UserMetaData::Attribute;
    QVariantMap properties;

    QFlags<Attribute> attributes = metaData.queryAttributes(Attribute::Tags | Attribute::Rating | Attribute::Comment | Attribute::OriginUrl);

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

QVariantMap toNamedVariantMap(const KFileMetaData::PropertyMap &propMap)
{
    QVariantMap map;
    if (propMap.isEmpty()) {
        return map;
    }

    using entry = std::pair<const KFileMetaData::Property::Property &, const QVariant &>;

    auto begin = propMap.constKeyValueBegin();

    while (begin != propMap.constKeyValueEnd()) {
        auto key = (*begin).first;
        KFileMetaData::PropertyInfo property(key);
        auto rangeEnd = std::find_if(begin, propMap.constKeyValueEnd(), [key](const entry &e) {
            return e.first != key;
        });

        auto distance = std::distance(begin, rangeEnd);
        if (distance > 1) {
            QVariantList list;
            list.reserve(static_cast<int>(distance));
            std::for_each(begin, rangeEnd, [&list](const entry &s) {
                list.append(s.second);
            });
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
