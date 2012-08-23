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

#include "datefacet.h"
#include "daterange.h"
#include "daterangeselectionwidget.h"

#include <Nepomuk2/Query/ComparisonTerm>
#include <Nepomuk2/Query/OrTerm>
#include <Nepomuk2/Query/AndTerm>
#include <Nepomuk2/Query/LiteralTerm>
#include <Nepomuk2/Query/StandardQuery>
#include <Nepomuk2/Query/Query>

#include <Nepomuk2/Vocabulary/NIE>
#include <Nepomuk2/Vocabulary/NUAO>

#include <QtCore/QDate>
#include <QtCore/QPair>
#include <QtGui/QMenu>
#include <QtGui/QWidgetAction>

#include <KGlobal>
#include <KLocale>
#include <KCalendarSystem>
#include <KGuiItem>
#include <KDialog>
#include <KDebug>


class Nepomuk2::Utils::DateFacet::Private
{
public:
    Private()
        : m_currentRange( NoDateRange ) {
    }

    /// only used for code simplification
    QList<DateRangeCandidate> m_rangeCandidates;

    DateRangeCandidates m_enabledRanges;
    QList<DateRangeCandidate> m_ranges;

    DateRangeCandidate m_currentRange;
    DateRange m_customRange;

    void rebuild();

    bool setCurrentRange( const DateRange& range );

    QString titleForDateRangeCandidate( DateRangeCandidate range ) const;
    DateRange dateRangeCandidateToDateRange( DateRangeCandidate range ) const;

    DateFacet* q;
};


void Nepomuk2::Utils::DateFacet::Private::rebuild()
{
    m_ranges.clear();
    Q_FOREACH( DateRangeCandidate r, m_rangeCandidates ) {
        if ( m_enabledRanges&r ) {
            m_ranges << r;
        }
    }
    if ( !m_ranges.contains( m_currentRange ) ) {
        q->clearSelection();
    }
    q->setLayoutChanged();
}


bool Nepomuk2::Utils::DateFacet::Private::setCurrentRange( const DateRange& range )
{
    kDebug() << range;
    Q_FOREACH( DateRangeCandidate dr, m_ranges ) {
        if ( dateRangeCandidateToDateRange( dr ) == range ) {
            kDebug() << range << "is" << dr;
            m_currentRange = dr;
            q->setSelectionChanged();
            return true;
        }
    }
    // not one of the candicates -> use it as custom one
    if ( m_ranges.contains( CustomDateRange ) ) {
        m_currentRange = CustomDateRange;
        m_customRange = range;
        q->setSelectionChanged();
        return true;
    }
    else {
        return false;
    }
}


QString Nepomuk2::Utils::DateFacet::Private::titleForDateRangeCandidate( DateRangeCandidate range ) const
{
    switch( range ) {
    case Anytime:
        return i18nc( "referring to a filter on the modification and usage date of files/resources", "Anytime" );
    case Today:
        return i18nc( "referring to a filter on the modification and usage date of files/resources", "Today" );
    case Yesterday:
        return i18nc( "referring to a filter on the modification and usage date of files/resources", "Yesterday" );
    case ThisWeek:
        return i18nc( "referring to a filter on the modification and usage date of files/resources", "This Week" );
    case LastWeek:
        return i18nc( "referring to a filter on the modification and usage date of files/resources", "Last Week" );
    case ThisMonth:
        return i18nc( "referring to a filter on the modification and usage date of files/resources", "This Month" );
    case LastMonth:
        return i18nc( "referring to a filter on the modification and usage date of files/resources", "Last Month" );
    case ThisYear:
        return i18nc( "referring to a filter on the modification and usage date of files/resources", "This Year" );
    case LastYear:
        return i18nc( "referring to a filter on the modification and usage date of files/resources", "Last Year" );
    case CustomDateRange:
        return i18nc( "referring to a filter on the modification and usage date of files/resources that will open a dialog to choose a date range", "Custom..." );
    default:
        return QString();
    }

    // shut up gcc
    return QString();
}


DateRange Nepomuk2::Utils::DateFacet::Private::dateRangeCandidateToDateRange( DateRangeCandidate range ) const
{
    const QDate today = QDate::currentDate();

    switch( range ) {
    case Today:
        return DateRange::today();

    case Yesterday:
        return DateRange(today.addDays(-1), today.addDays(-1));

    case ThisWeek: {
        return DateRange::thisWeek( DateRange::NoDateRangeFlags );
    }

    case LastWeek: {
        return DateRange::weekOf(today.addDays( -KGlobal::locale()->calendar()->daysInWeek( today ) ));
    }

    case ThisMonth: {
        return DateRange::thisMonth( DateRange::NoDateRangeFlags );
    }

    case LastMonth: {
        return DateRange::monthOf(today.addDays( -KGlobal::locale()->calendar()->daysInMonth( today ) ));
    }

    case ThisYear: {
        return DateRange::thisYear();
    }

    case LastYear: {
        return DateRange::yearOf( QDate( today.year()-1, 1, 1 ) );
    }

    case CustomDateRange:
        return m_customRange;

    default:
        return DateRange();
    }

    // shut up gcc
    return DateRange();
}


Nepomuk2::Utils::DateFacet::DateFacet( QObject* parent )
    : Facet( parent ),
      d( new Private() )
{
    d->q = this;
    d->m_rangeCandidates << Anytime
                         << Today
                         << Yesterday
                         << ThisWeek
                         << LastWeek
                         << ThisMonth
                         << LastMonth
                         << ThisYear
                         << LastYear
                         << CustomDateRange;
    setDateRangeCandidates( DefaultRanges );
}


Nepomuk2::Utils::DateFacet::~DateFacet()
{
    delete d;
}


Nepomuk2::Query::Term Nepomuk2::Utils::DateFacet::queryTerm() const
{
    return createDateRangeTerm( d->dateRangeCandidateToDateRange( d->m_currentRange ) );
}


Nepomuk2::Utils::DateFacet::SelectionMode Nepomuk2::Utils::DateFacet::selectionMode() const
{
    return MatchOne;
}


int Nepomuk2::Utils::DateFacet::count() const
{
    return d->m_ranges.count();
}


KGuiItem Nepomuk2::Utils::DateFacet::guiItem( int index ) const
{
    if ( index < d->m_ranges.count() )
        return KGuiItem( d->titleForDateRangeCandidate( d->m_ranges[index] ) );
    else
        return KGuiItem();
}


bool Nepomuk2::Utils::DateFacet::isSelected( int index ) const
{
    return d->m_ranges.indexOf( d->m_currentRange ) == index;
}


void Nepomuk2::Utils::DateFacet::setDateRangeCandidates( DateRangeCandidates ranges )
{
    d->m_enabledRanges = ranges;
    d->rebuild();
}


Nepomuk2::Utils::DateFacet::DateRangeCandidates Nepomuk2::Utils::DateFacet::dateRangeCandidates() const
{
    return d->m_enabledRanges;
}


void Nepomuk2::Utils::DateFacet::clearSelection()
{
    d->m_currentRange = d->m_ranges.isEmpty() ? NoDateRange : d->m_ranges.first();
    setQueryTermChanged();
    setSelectionChanged();
}


void Nepomuk2::Utils::DateFacet::setSelected( int index, bool selected )
{
    if ( index == d->m_ranges.indexOf( CustomDateRange ) ) {
        bool ok = false;
        DateRange newRange = getCustomRange( &ok );
        if( ok ) {
            d->setCurrentRange( newRange );
        }
    }

    else if ( selected &&
             index < d->m_ranges.count() ) {
        kDebug() << d->m_ranges[index] << d->dateRangeCandidateToDateRange(d->m_ranges[index]);
        d->m_currentRange = d->m_ranges[index];
        setQueryTermChanged();
        setSelectionChanged();
    }

    else {
        clearSelection();
    }
}


bool Nepomuk2::Utils::DateFacet::selectFromTerm( const Nepomuk2::Query::Term& term )
{
    DateRange range = extractDateRange( term );
    return range.isValid() ? d->setCurrentRange( range ) : false;
}


Nepomuk2::Query::Term Nepomuk2::Utils::DateFacet::createDateRangeTerm( const DateRange& range ) const
{
    return Nepomuk2::Query::dateRangeQuery( range.start(), range.end() ).term();
}


namespace {
    bool isDateComparisonTerm( const Nepomuk2::Query::Term& term ) {
        return( term.isComparisonTerm() &&
                term.toComparisonTerm().subTerm().isLiteralTerm() &&
                term.toComparisonTerm().subTerm().toLiteralTerm().value().isDateTime() &&
                ( term.toComparisonTerm().comparator() == Nepomuk2::Query::ComparisonTerm::Greater ||
                  term.toComparisonTerm().comparator() == Nepomuk2::Query::ComparisonTerm::Smaller ) );
    }

    Nepomuk2::Types::Property isDateTerm( const Nepomuk2::Query::Term& term, QDate& start, QDate& end ) {
        if( term.isAndTerm() ) {
            if( term.toAndTerm().subTerms().count() != 2 ||
                !isDateComparisonTerm( term.toAndTerm().subTerms()[0] ) ||
                !isDateComparisonTerm( term.toAndTerm().subTerms()[1] ) ||
                term.toAndTerm().subTerms()[0].toComparisonTerm().property() != term.toAndTerm().subTerms()[1].toComparisonTerm().property() ||
                term.toAndTerm().subTerms()[0].toComparisonTerm().comparator() == term.toAndTerm().subTerms()[1].toComparisonTerm().comparator() ) {
                return Nepomuk2::Types::Property();
            }
            // at this point we have two ComparisonTerms with QDate subterms and similar properties and different comparators
            if( term.toAndTerm().subTerms()[0].toComparisonTerm().comparator() == Nepomuk2::Query::ComparisonTerm::Greater ) {
                start = term.toAndTerm().subTerms()[0].toComparisonTerm().subTerm().toLiteralTerm().value().toDateTime().toLocalTime().date();
                end = term.toAndTerm().subTerms()[1].toComparisonTerm().subTerm().toLiteralTerm().value().toDateTime().toLocalTime().date();
            }
            else {
                start = term.toAndTerm().subTerms()[1].toComparisonTerm().subTerm().toLiteralTerm().value().toDateTime().toLocalTime().date();
                end = term.toAndTerm().subTerms()[0].toComparisonTerm().subTerm().toLiteralTerm().value().toDateTime().toLocalTime().date();
            }
            return term.toAndTerm().subTerms()[0].toComparisonTerm().property();
        }
        else if( isDateComparisonTerm( term ) ) {
            if( term.toComparisonTerm().comparator() == Nepomuk2::Query::ComparisonTerm::Greater ) {
                start = term.toComparisonTerm().subTerm().toLiteralTerm().value().toDateTime().toLocalTime().date();
            }
            else {
                end = term.toComparisonTerm().subTerm().toLiteralTerm().value().toDateTime().toLocalTime().date();
            }
            return term.toComparisonTerm().property();
        }
        else {
            return Nepomuk2::Types::Property();
        }
    }
}

DateRange Nepomuk2::Utils::DateFacet::extractDateRange( const Query::Term& term ) const
{
    // this is ugly since creating the query is done in standardqueries.cpp
    // thus, if that impl is changed this code wont work anymore. :(
    if( !term.isOrTerm() ) {
        return DateRange();
    }
    Nepomuk2::Query::OrTerm orTerm = term.toOrTerm();
    if( orTerm.subTerms().count() != 3 ) {
        return DateRange();
    }
    QDate start, end;
    bool mtime = false;
    bool created = false;
    bool nuao = false;
    Q_FOREACH( const Nepomuk2::Query::Term& t, orTerm.subTerms() ) {
        Nepomuk2::Types::Property p = isDateTerm( t, start, end );
        if( p == Nepomuk2::Vocabulary::NIE::lastModified() ) {
            mtime = true;
        }
        else if( p == Nepomuk2::Vocabulary::NIE::contentCreated() ) {
            created = true;
        }
        else {
            // check if it is the nuao term
            if( !t.isComparisonTerm() ||
                !t.toComparisonTerm().isInverted() ||
                t.toComparisonTerm().property() != Nepomuk2::Vocabulary::NUAO::involves() ) {
                return DateRange();
            }
            Nepomuk2::Types::Property p = isDateTerm(t.toComparisonTerm().subTerm(), start, end);
            if( p == Nepomuk2::Vocabulary::NUAO::start() )
                nuao = true;
            else
                return DateRange();
        }
    }
    if( mtime && created && nuao ) {
        return DateRange(start, end);
    }
    else {
        return DateRange();
    }
}


DateRange Nepomuk2::Utils::DateFacet::getCustomRange( bool* ok ) const
{
    DateRangeSelectionWidget* drw = new DateRangeSelectionWidget();
    drw->setRange( d->dateRangeCandidateToDateRange(d->m_currentRange) );

    // we use QMenu for the look only
    QMenu menu;
    QWidgetAction* wa = new QWidgetAction( &menu );
    wa->setDefaultWidget(drw);
    menu.addAction(wa);

    connect( drw, SIGNAL(rangeChanged(DateRange)), &menu, SLOT(close()));

    menu.exec(QCursor::pos());

    if( ok )
        *ok = true;
    return drw->range();
}

#include "datefacet.moc"
