/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2008-2010 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "simpleresourcemodel.h"

#include <QtCore/QUrl>
#include <QtCore/QList>

#include <Nepomuk2/Resource>
#include <Nepomuk2/Query/Result>

#include <KDebug>
#include <KUrl>


class Nepomuk2::Utils::SimpleResourceModel::Private
{
public:
    QList<Nepomuk2::Resource> resources;
};


Nepomuk2::Utils::SimpleResourceModel::SimpleResourceModel( QObject* parent )
    : ResourceModel( parent ),
      d( new Private() )
{
}


Nepomuk2::Utils::SimpleResourceModel::~SimpleResourceModel()
{
    delete d;
}


QModelIndex Nepomuk2::Utils::SimpleResourceModel::indexForResource( const Resource& res ) const
{
    Q_ASSERT( res.isValid() );
    // FIXME: performance
    int i = 0;
    QList<Nepomuk2::Resource>::const_iterator end = d->resources.constEnd();
    for ( QList<Nepomuk2::Resource>::const_iterator it = d->resources.constBegin(); it != end; ++it ) {
        if ( *it == res ) {
            return index( i, 0 );
        }
        ++i;
    }

    return QModelIndex();
}


Nepomuk2::Resource Nepomuk2::Utils::SimpleResourceModel::resourceForIndex( const QModelIndex& index ) const
{
    if ( index.isValid() && index.row() < d->resources.count() ) {
        return d->resources[index.row()];
    }
    else {
        return Resource();
    }
}


int Nepomuk2::Utils::SimpleResourceModel::rowCount( const QModelIndex& parent ) const
{
    if ( parent.isValid() ) {
        return 0;
    }
    else {
        return d->resources.count();
    }
}


QModelIndex Nepomuk2::Utils::SimpleResourceModel::index( int row, int column, const QModelIndex& parent ) const
{
    if ( !parent.isValid() && row < d->resources.count() ) {
        return createIndex( row, column, 0 );
    }
    else {
        return QModelIndex();
    }
}


bool Nepomuk2::Utils::SimpleResourceModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if( count < 1 || row < 0 || (row + count) > d->resources.size() || parent.isValid() )
        return false;

    beginRemoveRows( parent, row, row + count -1 );

    QList<Resource>::iterator begin, end;
    begin = end = d->resources.begin();
    begin += row;
    end += row + count;
    d->resources.erase( begin, end );

    endRemoveRows();
    return true;
}


void Nepomuk2::Utils::SimpleResourceModel::setResources( const QList<Nepomuk2::Resource>& resources )
{
    d->resources = resources;
    reset();
}


void Nepomuk2::Utils::SimpleResourceModel::addResources( const QList<Nepomuk2::Resource>& resources )
{
    if(!resources.isEmpty()) {
        beginInsertRows( QModelIndex(), d->resources.count(), d->resources.count() + resources.count() - 1 );
        d->resources << resources;
        endInsertRows();
    }
}


void Nepomuk2::Utils::SimpleResourceModel::addResource( const Nepomuk2::Resource& resource )
{
    addResources( QList<Resource>() << resource );
}


void Nepomuk2::Utils::SimpleResourceModel::setResults( const QList<Nepomuk2::Query::Result>& results)
{
    clear();
    addResults( results );
}

void Nepomuk2::Utils::SimpleResourceModel::addResults( const QList<Nepomuk2::Query::Result>& results )
{
    Q_FOREACH( const Query::Result& result, results ) {
        addResource( result.resource() );
    }
}

void Nepomuk2::Utils::SimpleResourceModel::addResult( const Nepomuk2::Query::Result result )
{
    addResource( result.resource() );
}


void Nepomuk2::Utils::SimpleResourceModel::clear()
{
    d->resources.clear();
    reset();
}

#include "simpleresourcemodel.moc"
