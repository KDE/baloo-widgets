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

namespace Baloo
{
namespace Private
{

QVariantMap convertUserMetaData(const KFileMetaData::UserMetaData& metaData)
{
    QVariantMap properties;

    const QStringList tags = metaData.tags();
    if (!tags.isEmpty()) {
        properties.insert(QStringLiteral("tags"), tags);
    }

    int rating = metaData.rating();
    if (rating) {
        properties.insert(QStringLiteral("rating"), rating);
    }

    const QString comment = metaData.userComment();
    if (!comment.isEmpty()) {
        properties.insert(QStringLiteral("userComment"), comment);
    }

    const QString originUrl = metaData.originUrl().toDisplayString();
    if (!originUrl.isEmpty()) {
        properties.insert(QStringLiteral("originUrl"), originUrl);
    }

    return properties;
}

} // namespace KFMPrivate
} // namespace Baloo
