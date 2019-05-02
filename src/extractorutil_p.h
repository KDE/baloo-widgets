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

#include <QDataStream>
#include <KFileMetaData/Properties>

/*
 * Deserialize prop from QDataStream
 */
static inline QDataStream &operator>>(QDataStream &stream, KFileMetaData::Property::Property& prop)
{
    quint16 intProp;
    stream >> intProp;
    prop = static_cast<KFileMetaData::Property::Property>(intProp);
    return stream;
}

/*
 * Serialize prop for QDataStream
 */
static inline QDataStream &operator<<(QDataStream &stream, const KFileMetaData::Property::Property prop)
{
    stream << static_cast<quint16>(prop);
    return stream;
}
