/*
    SPDX-FileCopyrightText: 2019 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "filemetadatautil_p.h"
#include <KFileMetaData/PropertyInfo>
#include <KFileMetaData/UserMetaData>
#include <QSet>

#include <QCollator>
#include <QString>
#include <QStringList>

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

// Precondition:
// if commonProperties contains <prop>, all <files> must also provide <prop>
void totalProperties(QVariantMap& target, const QString &prop, const QList<QVariantMap> &files, QList<QString> &commonProperties)
{
    auto propIndex = commonProperties.indexOf(prop);

    if (propIndex >= 0) {
        int total = 0;
        for (const QVariantMap &file : files) {
            QVariantMap::const_iterator it = file.constFind(prop);
            Q_ASSERT(it != file.constEnd());

            total += it.value().toInt();
        }

        target.insert(prop, QVariant(total));

        commonProperties.removeAt(propIndex);
    }
}
} // anonymous namespace

namespace Baloo
{
namespace Private
{
QMap<KFileMetaData::UserMetaData::Attribute, QVariant>
fetchUserMetaData(const KFileMetaData::UserMetaData &metaData, KFileMetaData::UserMetaData::Attributes wantedAttributes)
{
    using Attribute = KFileMetaData::UserMetaData::Attribute;

    auto attributes = metaData.queryAttributes(wantedAttributes);

    QMap<Attribute, QVariant> properties;

    if (attributes & Attribute::Tags) {
        QStringList tags = metaData.tags();
        if (!tags.isEmpty()) {
            properties.insert(Attribute::Tags, tags);
        }
    }

    if (attributes & Attribute::Rating) {
        int rating = metaData.rating();
        if (rating) {
            properties.insert(Attribute::Rating, rating);
        }
    }

    if (attributes & Attribute::Comment) {
        QString comment = metaData.userComment();
        if (!comment.isEmpty()) {
            properties.insert(Attribute::Comment, comment);
        }
    }

    if (attributes & Attribute::OriginUrl) {
        const QString originUrl = metaData.originUrl().toDisplayString();
        if (!originUrl.isEmpty()) {
            properties.insert(Attribute::OriginUrl, originUrl);
        }
    }

    return properties;
}

QVariantMap convertUserMetaData(const QMap<KFileMetaData::UserMetaData::Attribute, QVariant>& metaData)
{
    using Attribute = KFileMetaData::UserMetaData::Attribute;

    QVariantMap properties;
    for (auto it = metaData.begin(); it != metaData.end(); ++it) {
        if (it.key() == Attribute::Tags) {
            properties.insert(QStringLiteral("tags"), it.value());

        } else if (it.key() == Attribute::Rating) {
            properties.insert(QStringLiteral("rating"), it.value());

        } else if (it.key() == Attribute::Comment) {
            properties.insert(QStringLiteral("userComment"), it.value());

        } else if (it.key() == Attribute::OriginUrl) {
            properties.insert(QStringLiteral("originUrl"), it.value());
        }
    }

    return properties;
}

QVariantMap convertUserMetaData(const KFileMetaData::UserMetaData &metaData)
{
    using Attribute = KFileMetaData::UserMetaData::Attribute;

    auto attributeData = fetchUserMetaData(metaData, Attribute::Tags | Attribute::Rating | Attribute::Comment | Attribute::OriginUrl);

    return convertUserMetaData(attributeData);
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

void mergeCommonData(QVariantMap& target, const QList<QVariantMap> &files)
{
    if (files.empty()) {
        target.clear();
        return;
    }

    if (files.size() == 1) {
        target = files[0];
        return;
    }

    //
    // Only report the stuff that is common to all the files
    //
    QList<QString> commonProperties = files[0].keys();
    auto end = commonProperties.end();
    for (const QVariantMap &fileData : files) {
        end = std::remove_if(commonProperties.begin(), end,
                [&fileData](const QString& prop) { return !fileData.contains(prop); }
        );
    }
    commonProperties.erase(end, commonProperties.end());

    // Special handling for certain properties
    totalProperties(target, QStringLiteral("duration"), files, commonProperties);
    totalProperties(target, QStringLiteral("characterCount"), files, commonProperties);
    totalProperties(target, QStringLiteral("wordCount"), files, commonProperties);
    totalProperties(target, QStringLiteral("lineCount"), files, commonProperties);

    for (const QString &propUri : std::as_const(commonProperties)) {
        QVariant value = files[0][propUri];
        for (const QVariantMap &file : files) {
            QVariantMap::const_iterator it = file.find(propUri);
            Q_ASSERT(it != file.constEnd());

            value = intersect(it.value(), value);
            if (!value.isValid()) {
                break;
            }
        }

        if (value.isValid())
            target[propUri] = value;
    }
}

QStringList sortTags(const QStringList &tags)
{
    QCollator coll;
    coll.setNumericMode(true);
    QStringList sortedTags = tags;
    std::sort(sortedTags.begin(), sortedTags.end(), [&](const QString &s1, const QString &s2) {
        return coll.compare(s1, s2) < 0;
    });
    return sortedTags;
}

} // namespace KFMPrivate
} // namespace Baloo
