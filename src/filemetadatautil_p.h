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
QVariantMap toNamedVariantMap(const KFileMetaData::PropertyMap &propMap);

} // namespace Private
} // namespace Baloo
