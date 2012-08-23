/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2010 Sebastian Trueg <trueg@kde.org>

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

#include "dynamicresourcefacet.h"
#include "dynamicresourcefacet_p.h"
#include "searchwidget.h"

#include <Nepomuk2/Query/AndTerm>
#include <Nepomuk2/Query/OrTerm>
#include <Nepomuk2/Query/ResourceTerm>
#include <Nepomuk2/Query/ResourceTypeTerm>
#include <Nepomuk2/Query/ComparisonTerm>
#include <Nepomuk2/Query/Result>
#include <Nepomuk2/Query/Query>
#include <Nepomuk2/Query/QueryServiceClient>

#include <Nepomuk2/Types/Property>
#include <Nepomuk2/Types/Class>
#include <Nepomuk2/Resource>
#include <Nepomuk2/ResourceManager>

#include <KGuiItem>
#include <KDebug>

#include <QtCore/QSet>


void Nepomuk2::Utils::DynamicResourceFacet::Private::rebuild( bool clearSelection )
{
    kDebug() << clearSelection;
    m_haveMore = false;
    m_resources.clear();
    if( clearSelection )
        m_selectedResources.clear();
    q->setLayoutChanged();

    Query::Query query = q->resourceQuery( m_currentQuery );
    query.setLimit( m_maxRows+1 );
    startQuery( query );
}


void Nepomuk2::Utils::DynamicResourceFacet::Private::startQuery( const Query::Query& query )
{
    kDebug() << query;
    m_queryClient.query( query );
}


void Nepomuk2::Utils::DynamicResourceFacet::Private::addResource( const Nepomuk2::Resource& res )
{
    if ( !m_resources.contains( res ) ) {
        m_resources.append( res );
        q->setLayoutChanged();
    }
}


void Nepomuk2::Utils::DynamicResourceFacet::Private::_k_newEntries( const QList<Nepomuk2::Query::Result>& entries )
{
    kDebug();
    bool selectionChanged = false;
    Q_FOREACH( const Query::Result& result, entries ) {
        if( m_resources.count() == m_maxRows ) {
            // add the more... button
            m_haveMore = true;
        }
        else if( !m_resources.contains(result.resource()) ){
            m_resources.append( result.resource() );
            if( m_selectionMode == Facet::MatchOne &&
                m_selectedResources.isEmpty() ) {
                m_selectedResources << m_resources.first();
                selectionChanged = true;
            }
            else if( m_selectedResources.contains(result.resource()) ) {
                // in case we remember a selection from before
                selectionChanged = true;
            }
        }
    }
    q->setLayoutChanged();
}


void Nepomuk2::Utils::DynamicResourceFacet::Private::_k_populateFinished()
{
    kDebug() << m_resources.count();
    m_queryClient.close();

    // clean up selection in case rebuild was called without clearing it
    // FIXME: shouldn't we rather add all the selected ones that are not in the list yet?
    QSet<Resource>::iterator it = m_selectedResources.begin();
    while( it != m_selectedResources.end() ) {
        if( m_resources.contains( *it ) )
            ++it;
        else
            it = m_selectedResources.erase(it);
    }
}


Nepomuk2::Utils::DynamicResourceFacet::DynamicResourceFacet( QObject* parent )
    : Facet(parent),
      d(new Private())
{
    d->q = this;
    connect( &d->m_queryClient, SIGNAL(newEntries(QList<Nepomuk2::Query::Result>)),
             this, SLOT(_k_newEntries(QList<Nepomuk2::Query::Result>)) );
    connect( &d->m_queryClient, SIGNAL(finishedListing()),
             this, SLOT(_k_populateFinished()) );
}


Nepomuk2::Utils::DynamicResourceFacet::~DynamicResourceFacet()
{
    delete d;
}


void Nepomuk2::Utils::DynamicResourceFacet::setSelectionMode( SelectionMode mode )
{
    d->m_selectionMode = mode;
    clearSelection();
}


Nepomuk2::Utils::Facet::SelectionMode Nepomuk2::Utils::DynamicResourceFacet::selectionMode() const
{
    return d->m_selectionMode;
}


Nepomuk2::Query::Term Nepomuk2::Utils::DynamicResourceFacet::queryTerm() const
{
    if( d->m_resources.isEmpty() ||
        d->m_selectedResources.isEmpty() ) {
        return Query::Term();
    }
    else {
        switch( d->m_selectionMode ) {
        case MatchAll: {
            Query::AndTerm andTerm;
            Q_FOREACH( const Resource& res, d->m_selectedResources ) {
                andTerm.addSubTerm( termForResource( res ) );
            }
            return andTerm.optimized();
        }
        case MatchAny: {
            Query::OrTerm orTerm;
            Q_FOREACH( const Resource& res, d->m_selectedResources ) {
                orTerm.addSubTerm( termForResource(res) );
            }
            return orTerm.optimized();
        }
        case MatchOne:
            return termForResource( *d->m_selectedResources.begin() );
        }
    }

    // make gcc shut up
    return Query::Term();
}


int Nepomuk2::Utils::DynamicResourceFacet::count() const
{
    int c = d->m_resources.count();
    if ( d->m_haveMore )
        ++c;
    return c;
}


bool Nepomuk2::Utils::DynamicResourceFacet::isSelected( int index ) const
{
    if( index < d->m_resources.count() )
        return d->m_selectedResources.contains( d->m_resources[index] );
    else
        return false;
}


void Nepomuk2::Utils::DynamicResourceFacet::setRelation( const Types::Property& prop )
{
    d->m_property = prop;
    d->rebuild();
}


void Nepomuk2::Utils::DynamicResourceFacet::setResourceType( const Types::Class& type )
{
    d->m_resourceType = type;
    d->rebuild();
}


void Nepomuk2::Utils::DynamicResourceFacet::setMaxRows( int max )
{
    d->m_maxRows = max;
    d->rebuild();
}


Nepomuk2::Types::Property Nepomuk2::Utils::DynamicResourceFacet::relation() const
{
    return d->m_property;
}


Nepomuk2::Types::Class Nepomuk2::Utils::DynamicResourceFacet::resourceType() const
{
    return d->resourceType();
}


QList<Nepomuk2::Resource> Nepomuk2::Utils::DynamicResourceFacet::selectedResources() const
{
    return d->m_selectedResources.toList();
}


int Nepomuk2::Utils::DynamicResourceFacet::maxRows() const
{
    return d->m_maxRows;
}


KGuiItem Nepomuk2::Utils::DynamicResourceFacet::guiItem( int index ) const
{
    KGuiItem item;

    if ( index < d->m_resources.count() ) {
        item.setText( d->m_resources[index].genericLabel() );
        return item;
    }
    else if ( d->m_haveMore && index == count()-1 ) {
        item.setText( i18nc( "@option:check An item in a list of resources that allows to query for more resources to put in the list", "More..." ) );
    }

    return item;
}


void Nepomuk2::Utils::DynamicResourceFacet::setSelected( const Resource& res, bool selected )
{
    kDebug() << res.uri() << selected;
    if( res.hasType( d->resourceType() ) ) {
        if( selected ) {
            d->addResource(res);
        }
        if ( d->m_resources.contains( res ) ) {
            setSelected( d->m_resources.indexOf( res ), selected );
        }
    }
}


void Nepomuk2::Utils::DynamicResourceFacet::setSelected( int index, bool selected )
{
    kDebug() << index << selected;
    if ( d->m_haveMore && index == count()-1 && selected ) {
        const QList<Resource> rl = getMoreResources();
        Q_FOREACH( const Resource& res, rl ) {
            // FIXME: try to honor d->m_maxRows
            if ( !d->m_resources.contains( res ) ) {
                d->m_resources.append( res );
                setLayoutChanged();
            }

            // select the new item
            setSelected( d->m_resources.indexOf(res) );
        }

        // unselect the more item
        setSelected( d->m_resources.count(), false );
    }
    else if( index < d->m_resources.count() ) {
        Resource res = d->m_resources[index];
        if ( selectionMode() == MatchOne ) {
            if ( d->m_selectedResources.contains( res ) && !selected ) {
                clearSelection();
            }
            else if ( selected ) {
                d->m_selectedResources.clear();
                d->m_selectedResources.insert( res );
            }
        }
        else if ( selected ) {
            d->m_selectedResources.insert( res );
        }
        else {
            d->m_selectedResources.remove( res );
        }
    }
    setSelectionChanged();
    setQueryTermChanged();
}


void Nepomuk2::Utils::DynamicResourceFacet::clearSelection()
{
    kDebug();
    d->m_selectedResources.clear();
    if( selectionMode() == MatchOne && !d->m_resources.isEmpty() )
        d->m_selectedResources.insert(d->m_resources.first());
    setSelectionChanged();
    setQueryTermChanged();
}


bool Nepomuk2::Utils::DynamicResourceFacet::selectFromTerm( const Nepomuk2::Query::Term& term )
{
    kDebug() << term;
    Resource res = resourceForTerm(term);
    if( res.isValid() ) {
        setSelected( res );
        return true;
    }
    else if( ( term.isAndTerm() && selectionMode() == MatchAll ) ||
             ( term.isOrTerm() && selectionMode() == MatchAny ) ) {
        QList<Resource> resources;

        // first check if all of the terms are usable
        const QList<Query::Term> subTerms = term.isAndTerm() ? term.toAndTerm().subTerms() : term.toOrTerm().subTerms();
        Q_FOREACH( const Query::Term& subTerm, subTerms ) {
            Resource res = resourceForTerm(subTerm);
            if( res.isValid() )
                resources << res;
            else
                return false;
        }

        // all terms are in facet usable
        Q_FOREACH( const Resource& res, resources ) {
            setSelected( res );
        }
        return true;
    }
    else {
        return false;
    }
}


void Nepomuk2::Utils::DynamicResourceFacet::handleClientQueryChange()
{
    kDebug();
    d->rebuild( false );
}


Nepomuk2::Query::Query Nepomuk2::Utils::DynamicResourceFacet::resourceQuery( const Query::Query& clientQuery ) const
{
    // we only select resources that make sense with the current query, ie. those that would actually
    // change the current result set
    Query::ComparisonTerm clientQueryRestrictionTerm( d->m_property, clientQuery.term() );
    clientQueryRestrictionTerm.setInverted(true);

    // we sort the resources by usage
    Nepomuk2::Query::ComparisonTerm term( d->m_property, Nepomuk2::Query::Term() );
    term.setSortWeight( 1, Qt::DescendingOrder );
    term.setAggregateFunction( Nepomuk2::Query::ComparisonTerm::Count );
    term.setInverted(true);

    return Query::Query( Query::ResourceTypeTerm( resourceType() ) && clientQueryRestrictionTerm && term );
}


Nepomuk2::Query::Term Nepomuk2::Utils::DynamicResourceFacet::termForResource( const Resource& res ) const
{
    return d->m_property == Query::ResourceTerm( res );
}


Nepomuk2::Resource Nepomuk2::Utils::DynamicResourceFacet::resourceForTerm( const Nepomuk2::Query::Term& term ) const
{
    if( term.isComparisonTerm() &&
        term.toComparisonTerm().property() == d->m_property &&
        term.toComparisonTerm().subTerm().isResourceTerm() &&
        term.toComparisonTerm().subTerm().toResourceTerm().resource().hasType( d->resourceType() ) ) {
        return term.toComparisonTerm().subTerm().toResourceTerm().resource();
    }
    else {
        return Resource();
    }
}


QList<Nepomuk2::Resource> Nepomuk2::Utils::DynamicResourceFacet::getMoreResources() const
{
    return SearchWidget::searchResources( 0, resourceQuery( d->m_currentQuery ), SearchWidget::NoConfigFlags );
}


Nepomuk2::Resource Nepomuk2::Utils::DynamicResourceFacet::resourceAt(int i) const
{
    if( i < d->m_resources.count() )
        return d->m_resources[i];
    else
        return Resource();
}

#include "dynamicresourcefacet.moc"
