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

} // namespace Private
} // namespace Baloo
