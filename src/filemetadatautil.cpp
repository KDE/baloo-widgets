/*
    SPDX-FileCopyrightText: 2019 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "filemetadatautil_p.h"
#include <KFileMetaData/PropertyInfo>
#include <KFileMetaData/UserMetaData>
#include <QSet>

#include <algorithm>

namespace
{
QVariant intersect(const QVariant &v1, const QVariant &v2)
{
    if (!v1.isValid() || !v2.isValid()) {
        return {};
    }

    // List and String
    if (v1.type() == QVariant::StringList && v2.type() == QVariant::String) {
        QStringList list = v1.toStringList();
        QString str = v2.toString();

        if (!list.contains(str)) {
            list << str;
        }

        return QVariant(list);
    }

    // String and List
    if (v1.type() == QVariant::String && v2.type() == QVariant::StringList) {
        QStringList list = v2.toStringList();
        QString str = v1.toString();

        if (!list.contains(str)) {
            list << str;
        }

        return QVariant(list);
    }

    // List and List
    if (v1.type() == QVariant::StringList && v2.type() == QVariant::StringList) {
        QSet<QString> s1(v1.toStringList().cbegin(), v1.toStringList().cend());
        QSet<QString> s2(v2.toStringList().cbegin(), v2.toStringList().cend());

        return QVariant(s1.intersect(s2).values());
    }

    if (v1 == v2) {
        return v1;
    }

    return {};
}
} // anonymous namespace

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

QVariantMap toNamedVariantMap(const KFileMetaData::PropertyMultiMap &propMap)
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

void totalProperties(QVariantMap& target, const QString &prop, const QList<QVariantMap> &resources, QSet<QString> &allProperties)
{
    if (allProperties.contains(prop)) {
        int total = 0;
        for (const QVariantMap &map : resources) {
            QVariantMap::const_iterator it = map.constFind(prop);
            if (it == map.constEnd()) {
                total = 0;
                break;
            } else {
                total += it.value().toInt();
            }
        }

        if (total) {
            target.insert(prop, QVariant(total));
        }
        allProperties.remove(prop);
    }
}

void mergeCommonData(QVariantMap& target, const QList<QVariantMap> &files)
{
    //
    // Only report the stuff that is common to all the files
    //
    QSet<QString> allProperties;
    QList<QVariantMap> propertyList;
    for (const QVariantMap &fileData : files) {
        propertyList << fileData;
        auto uniqueValues = fileData.keys();
        uniqueValues.erase(std::unique(uniqueValues.begin(), uniqueValues.end()), uniqueValues.end());
        allProperties += QSet<QString>(uniqueValues.begin(), uniqueValues.end());
    }

    // Special handling for certain properties
    totalProperties(target, QStringLiteral("duration"), propertyList, allProperties);
    totalProperties(target, QStringLiteral("characterCount"), propertyList, allProperties);
    totalProperties(target, QStringLiteral("wordCount"), propertyList, allProperties);
    totalProperties(target, QStringLiteral("lineCount"), propertyList, allProperties);

    for (const QString &propUri : std::as_const(allProperties)) {
        for (const QVariantMap &map : std::as_const(propertyList)) {
            QVariantMap::const_iterator it = map.find(propUri);
            if (it == map.constEnd()) {
                target.remove(propUri);
                break;
            }

            QVariantMap::iterator dit = target.find(it.key());
            if (dit == target.end()) {
                target.insert(propUri, it.value());
            } else {
                QVariant finalValue = intersect(it.value(), dit.value());
                if (finalValue.isValid()) {
                    target[propUri] = finalValue;
                } else {
                    target.remove(propUri);
                    break;
                }
            }
        }
    }
}

} // namespace KFMPrivate
} // namespace Baloo
