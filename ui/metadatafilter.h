/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2012  Vishesh Handa <me@vhanda.in>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef _NEPOMUK2_METADATAFILTER_H
#define _NEPOMUK2_METADATAFILTER_H

#include <QtCore/QUrl>
#include <QtCore/QHash>

namespace Nepomuk2 {

    class Variant;

    class MetadataFilter : public QObject
    {
    public:
        explicit MetadataFilter(QObject* parent = 0);
        virtual ~MetadataFilter();

        /**
         * Takes all the data by the provider and filters the data.
         * This acts as a filter and a data aggregator
         */
        QHash<QUrl, Variant> filter(const QList< QHash<QUrl, Variant> >& data );
    private:
        /**
         * Initializes the configuration file "kmetainformationrc"
         * with proper default settings for the first start in
         * an uninitialized environment.
         */
        void initMetaInformationSettings();
    };
}

#endif // _NEPOMUK2_METADATAFILTER_H
