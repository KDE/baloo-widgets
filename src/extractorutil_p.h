/*
    SPDX-FileCopyrightText: 2019 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KFileMetaData/Properties>
#include <QDataStream>

/*
 * Deserialize prop from QDataStream
 */
static inline QDataStream &operator>>(QDataStream &stream, KFileMetaData::Property::Property &prop)
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
