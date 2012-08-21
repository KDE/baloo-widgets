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

#include "proxyfacet.h"
#include "andterm.h"
#include "query.h"

#include "kguiitem.h"
#include "kdebug.h"

class Nepomuk2::Utils::ProxyFacet::Private
{
public:
    Private()
        : m_sourceFacet(0),
          m_facetConditionMet(true) {
    }

    void updateConditionStatus();

    Facet* m_sourceFacet;

    Query::Term m_facetCondition;
    bool m_facetConditionMet;

    ProxyFacet* q;
};


namespace {
    /**
     * Checks if a query contains a certain term in a non-optional manner. Basically this means
     * that either the query's term is the term in question or the query term is an AndTerm which
     * contains the requested term. All other situations result in an optional usage of \p term
     * or are too complex to handle here.
     */
    bool containsTerm( const Nepomuk2::Query::Query& query, const Nepomuk2::Query::Term& term ) {
        Nepomuk2::Query::Term queryTerm = query.term().optimized();
        if( queryTerm == term ) {
            return true;
        }
        else if( queryTerm.isAndTerm() ) {
            Q_FOREACH( const Nepomuk2::Query::Term& subTerm, queryTerm.toAndTerm().subTerms() ) {
                if( subTerm == term ) {
                    return true;
                }
            }
        }

        // fallback
        return false;
    }
}

void Nepomuk2::Utils::ProxyFacet::Private::updateConditionStatus()
{
    bool newFacetConditionMet = true;
    if( m_facetCondition.isValid() ) {
        newFacetConditionMet = containsTerm( q->clientQuery(), m_facetCondition );
        kDebug() << m_facetConditionMet << newFacetConditionMet;
    }

    if( newFacetConditionMet != m_facetConditionMet ) {
        m_facetConditionMet = newFacetConditionMet;
        q->setLayoutChanged();
        q->setQueryTermChanged();
    }

    if( !m_facetConditionMet ) {
        q->clearSelection();
    }
}


Nepomuk2::Utils::ProxyFacet::ProxyFacet( QObject* parent )
    : Facet(parent),
      d(new Private())
{
    d->q = this;
}


Nepomuk2::Utils::ProxyFacet::~ProxyFacet()
{
    delete d;
}


void Nepomuk2::Utils::ProxyFacet::setSourceFacet( Facet* source )
{
    if( d->m_sourceFacet ) {
        d->m_sourceFacet->disconnect(this);
    }

    d->m_sourceFacet = source;
    if( d->m_sourceFacet ) {
        connect(d->m_sourceFacet, SIGNAL(queryTermChanged(Nepomuk2::Utils::Facet*,Nepomuk2::Query::Term)),
                this, SIGNAL(queryTermChanged(Nepomuk2::Utils::Facet*,Nepomuk2::Query::Term)));
        connect(d->m_sourceFacet, SIGNAL(selectionChanged(Nepomuk2::Utils::Facet*)),
                this, SIGNAL(selectionChanged(Nepomuk2::Utils::Facet*)));
        connect(d->m_sourceFacet, SIGNAL(layoutChanged(Nepomuk2::Utils::Facet*)),
                this, SIGNAL(layoutChanged(Nepomuk2::Utils::Facet*)));
    }

    setLayoutChanged();
    setQueryTermChanged();
    setSelectionChanged();
}


Nepomuk2::Utils::Facet* Nepomuk2::Utils::ProxyFacet::sourceFacet() const
{
    return d->m_sourceFacet;
}


Nepomuk2::Utils::Facet::SelectionMode Nepomuk2::Utils::ProxyFacet::selectionMode() const
{
    return d->m_sourceFacet ? d->m_sourceFacet->selectionMode() : MatchOne;
}


Nepomuk2::Query::Term Nepomuk2::Utils::ProxyFacet::queryTerm() const
{
    return facetConditionMet() && d->m_sourceFacet ? d->m_sourceFacet->queryTerm() : Query::Term();
}


int Nepomuk2::Utils::ProxyFacet::count() const
{
    return d->m_sourceFacet && facetConditionMet() ? d->m_sourceFacet->count() : 0;
}


bool Nepomuk2::Utils::ProxyFacet::isSelected( int index ) const
{
    return d->m_sourceFacet ? d->m_sourceFacet->isSelected(index) : false;
}


KGuiItem Nepomuk2::Utils::ProxyFacet::guiItem( int index ) const
{
    return d->m_sourceFacet ? d->m_sourceFacet->guiItem(index) : KGuiItem();
}


void Nepomuk2::Utils::ProxyFacet::setSelected( int index, bool selected )
{
    if( d->m_sourceFacet && facetConditionMet() ) {
        d->m_sourceFacet->setSelected( index, selected );
    }
}


void Nepomuk2::Utils::ProxyFacet::clearSelection()
{
    if( d->m_sourceFacet ) {
        d->m_sourceFacet->clearSelection();
    }
}


bool Nepomuk2::Utils::ProxyFacet::selectFromTerm( const Nepomuk2::Query::Term& term )
{
    if( d->m_sourceFacet && facetConditionMet() ) {
        return d->m_sourceFacet->selectFromTerm( term );
    }
    else {
        return false;
    }
}


void Nepomuk2::Utils::ProxyFacet::handleClientQueryChange()
{
    d->updateConditionStatus();
    if( d->m_sourceFacet ) {
        d->m_sourceFacet->setClientQuery( clientQuery() );
    }
}


void Nepomuk2::Utils::ProxyFacet::setFacetCondition( const Nepomuk2::Query::Term& term )
{
    d->m_facetCondition = term;
    d->updateConditionStatus();
}


bool Nepomuk2::Utils::ProxyFacet::facetConditionMet() const
{
    return d->m_facetConditionMet;
}

#include "proxyfacet.moc"
