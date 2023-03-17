/*
    SPDX-FileCopyrightText: 2019 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KFileMetaData/Properties>
#include <KFileMetaData/UserMetaData>
#include <QString>
#include <QStringList>
#include <QVariantMap>

namespace Baloo
{
namespace Private
{
/**
 * Converts the UserMetaData attributes into a variant map
 *
 * /sa: KFileMetaData::UserMetaData
 */
QVariantMap convertUserMetaData(const QMap<KFileMetaData::UserMetaData::Attribute, QVariant>& metaData);

/**
 * Fetches the data for the file assiciated with /p metadata
 *
 * /p wantedAttributes Set of attributes which should be fetched
 *
 * This may block
 * /sa: KFileMetaData::UserMetaData
 */
QMap<KFileMetaData::UserMetaData::Attribute, QVariant>
fetchUserMetaData(const KFileMetaData::UserMetaData &metaData, KFileMetaData::UserMetaData::Attributes wantedAttributes);

/**
 * Fetches and converts the data for the file assiciated with /p metadata
 *
 * This is an amalgation of fetchUserMetaData and convertUserMetaData,
 * and thus may block.
 */
QVariantMap convertUserMetaData(const KFileMetaData::UserMetaData &metaData);

/**
 * Converts the property map into a variant map using the
 * property name as key. In case a key has multiple values,
 * all its values are collected in a QVariantList that is
 * stored as a single entry.
 */
QVariantMap toNamedVariantMap(const KFileMetaData::PropertyMultiMap &propMap);

/**
 * Merges common data for \p files into \p target
 *
 * Properties where a total makes sense are summed up (e.g. duration or
 * wordcount). All other properties are only kept if the values match,
 * e.g. when all files have the same "width". Properties which are
 * only present for some files are removed.
 */
void mergeCommonData(QVariantMap& target, const QList<QVariantMap> &files);

/*
 * Sort tags ie. {"c", "200", "B", "3", "a"} to {"3", "200", "a", "B", "c"}
 * It respects locale and will sort numbers correctly.
 */
QStringList sortTags(const QStringList &tags);

} // namespace Private
} // namespace Baloo
