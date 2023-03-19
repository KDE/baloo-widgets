/*
    SPDX-FileCopyrightText: 2019 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KFileMetaData/Properties>
#include <KFileMetaData/UserMetaData>
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

/**
 * Checks for the existence of \p uri in \p allProperties, and accordingly
 * inserts the total integer value of that property in \p target. On completion
 * it removes \p uri from \p allProperties
 */
void totalProperties(QVariantMap& target, const QString &prop, const QList<QVariantMap> &resources, QSet<QString> &allProperties);

} // namespace Private
} // namespace Baloo
