/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2007-2010 Sebastian Trueg <trueg@kde.org>

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

#include "resourcemodel.h"

#include <QtCore/QUrl>
#include <QtCore/QList>
#include <QtCore/QMimeData>

#include <KUrl>
#include <KDebug>
#include <KCategorizedSortFilterProxyModel>
#include <KIcon>
#include <KLocale>

#include <Nepomuk2/Resource>
#include <Nepomuk2/Query/Result>
#include <Nepomuk2/Types/Class>
#include <Nepomuk2/Variant>

#include <Soprano/Vocabulary/RDFS>
#include <Soprano/Vocabulary/NAO>


Q_DECLARE_METATYPE(Nepomuk2::Types::Class)


class Nepomuk2::Utils::ResourceModel::Private
{
public:
    QList<Nepomuk2::Resource> resources;
};


Nepomuk2::Utils::ResourceModel::ResourceModel( QObject* parent )
    : QAbstractItemModel( parent ),
      d( new Private() )
{
}


Nepomuk2::Utils::ResourceModel::~ResourceModel()
{
    delete d;
}


Nepomuk2::Resource Nepomuk2::Utils::ResourceModel::resourceForIndex(const QModelIndex& index) const
{
    if ( index.isValid() && index.row() < d->resources.count() ) {
        return d->resources[index.row()];
    }
    else {
        return Resource();
    }
}


QModelIndex Nepomuk2::Utils::ResourceModel::indexForResource(const Nepomuk2::Resource& res)
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


int Nepomuk2::Utils::ResourceModel::rowCount( const QModelIndex& parent ) const
{
    if ( parent.isValid() ) {
        return 0;
    }
    else {
        return d->resources.count();
    }
}


QModelIndex Nepomuk2::Utils::ResourceModel::index( int row, int column, const QModelIndex& parent ) const
{
    if ( !parent.isValid() && row < d->resources.count() ) {
        return createIndex( row, column, 0 );
    }
    else {
        return QModelIndex();
    }
}


bool Nepomuk2::Utils::ResourceModel::removeRows(int row, int count, const QModelIndex& parent)
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


QModelIndex Nepomuk2::Utils::ResourceModel::parent( const QModelIndex& child ) const
{
    Q_UNUSED(child);
    return QModelIndex();
}


int Nepomuk2::Utils::ResourceModel::columnCount( const QModelIndex& parent ) const
{
    Q_UNUSED(parent);
    return ResourceModelColumnCount;
}


QVariant Nepomuk2::Utils::ResourceModel::data( const QModelIndex& index, int role ) const
{
    Nepomuk2::Resource res = resourceForIndex( index );
    if( !res.isValid() ) {
        return QVariant();
    }

    //
    // Part 1: column specific data
    //
    switch( index.column() ) {
    case ResourceColumn:
        switch( role ) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            return res.genericLabel();

        case Qt::DecorationRole: {
            QString iconName = res.genericIcon();
            if( !iconName.isEmpty() ) {
                return KIcon( iconName );
            }
            else {
                QIcon icon = Types::Class(res.type()).icon();
                if( !icon.isNull() )
                    return icon;
                else
                    return QVariant();
            }
        }

        case Qt::ToolTipRole:
            return KUrl( res.uri() ).prettyUrl();

        }

    case ResourceTypeColumn:
        switch( role ) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            return Types::Class( res.type() ).label();

        case Qt::DecorationRole: {
            QIcon icon = Types::Class(res.type()).icon();
            if( !icon.isNull() )
                return icon;
            else
                return QVariant();
        }

        case Qt::ToolTipRole:
            return KUrl(res.type()).prettyUrl();
        }
    }


    //
    // Part 2: column-independant data
    //
    switch( role ) {
    case KCategorizedSortFilterProxyModel::CategorySortRole:
        // FIXME: sort files before other stuff and so on

    case KCategorizedSortFilterProxyModel::CategoryDisplayRole: {
        Q_ASSERT( !res.type().isEmpty() );
        Nepomuk2::Types::Class c( res.type() );
        QString cat = c.label();
        if ( cat.isEmpty() ) {
            cat = c.name();
        }
        if ( c.uri() == Soprano::Vocabulary::RDFS::Resource() || cat.isEmpty() ) {
            cat = i18nc( "@title KCategorizedSortFilterProxyModel grouping for all Nepomukj resources that are of type rdfs:Resource", "Miscellaneous" );
        }

        return cat;
    }

    case ResourceRole:
        return QVariant::fromValue( res );

    case ResourceTypeRole:
        return QVariant::fromValue( Nepomuk2::Types::Class(res.type()) );

    case ResourceCreationDateRole:
        return res.property( Soprano::Vocabulary::NAO::created() ).toDateTime();
    }

    // fallback
    return QVariant();
}


QVariant Nepomuk2::Utils::ResourceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role == Qt::DisplayRole ) {
        switch( section ) {
        case ResourceColumn:
            return i18nc("@title:column The Nepomuk resource label and icon", "Resource");
        case ResourceTypeColumn:
            return i18nc("@title:column The Nepomuk resource's RDF type", "Resource Type");
        }
    }

    return QAbstractItemModel::headerData(section, orientation, role);
}


Qt::ItemFlags Nepomuk2::Utils::ResourceModel::flags( const QModelIndex& index ) const
{
    if ( index.isValid() ) {
        return QAbstractItemModel::flags( index ) | Qt::ItemIsDragEnabled;
    }
    else {
        return QAbstractItemModel::flags( index );
    }
}


QMimeData* Nepomuk2::Utils::ResourceModel::mimeData( const QModelIndexList& indexes ) const
{
    KUrl::List uris;
    foreach ( const QModelIndex& index, indexes ) {
        if (index.isValid()) {
            uris << index.data( ResourceRole ).value<Resource>().uri();
        }
    }

    QMimeData* mimeData = new QMimeData();
    uris.populateMimeData( mimeData );

    QByteArray data;
    QDataStream s( &data, QIODevice::WriteOnly );
    s << uris;
    mimeData->setData( mimeTypes().first(), data );

    return mimeData;
}


QStringList Nepomuk2::Utils::ResourceModel::mimeTypes() const
{
    return( QStringList()
            << QLatin1String( "application/x-nepomuk-resource-uri" )
           << KUrl::List::mimeDataTypes() );
}


bool Nepomuk2::Utils::ResourceModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
    return QAbstractItemModel::setData(index, value, role);
}


void Nepomuk2::Utils::ResourceModel::setResources( const QList<Nepomuk2::Resource>& resources )
{
    d->resources = resources;
    reset();
}


void Nepomuk2::Utils::ResourceModel::addResources( const QList<Nepomuk2::Resource>& resources )
{
    if(!resources.isEmpty()) {
        beginInsertRows( QModelIndex(), d->resources.count(), d->resources.count() + resources.count() - 1 );
        d->resources << resources;
        endInsertRows();
    }
}


void Nepomuk2::Utils::ResourceModel::addResource( const Nepomuk2::Resource& resource )
{
    addResources( QList<Resource>() << resource );
}


void Nepomuk2::Utils::ResourceModel::setResults( const QList<Nepomuk2::Query::Result>& results)
{
    clear();
    addResults( results );
}


void Nepomuk2::Utils::ResourceModel::addResults( const QList<Nepomuk2::Query::Result>& results )
{
    Q_FOREACH( const Query::Result& result, results ) {
        addResource( result.resource() );
    }
}


void Nepomuk2::Utils::ResourceModel::addResult( const Nepomuk2::Query::Result result )
{
    addResource( result.resource() );
}


void Nepomuk2::Utils::ResourceModel::clear()
{
    d->resources.clear();
    reset();
}

#include "resourcemodel.moc"
