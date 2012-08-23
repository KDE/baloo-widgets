/*
   Copyright (c) 2010 Oszkar Ambrus <aoszkar@gmail.com>
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

#include "searchwidget.h"
#include "searchwidget_p.h"

#include "searchlineedit.h"
#include "facetwidget.h"
#include "simpleresourcemodel.h"
#include "proxyfacet.h"
#include "simplefacet.h"

#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QListView>
#include <QtGui/QSplitter>
#include <QtCore/QList>

#include <KDebug>
#include <KPushButton>
#include <KDialog>

#include <Nepomuk2/Query/Query>
#include <Nepomuk2/Query/QueryParser>
#include <Nepomuk2/Query/Result>
#include <Nepomuk2/Query/AndTerm>
#include <Nepomuk2/Query/OrTerm>
#include <Nepomuk2/Query/LiteralTerm>
#include <Nepomuk2/Query/ComparisonTerm>
#include <Nepomuk2/Query/ResourceTerm>
#include <Nepomuk2/Query/ResourceTypeTerm>
#include <Nepomuk2/Query/QueryServiceClient>
#include <Nepomuk2/Query/FileQuery>
#include <Nepomuk2/Query/NegationTerm>

#include <Nepomuk2/Resource>
#include <Nepomuk2/Variant>
#include <Nepomuk2/Vocabulary/NFO>

#include <Soprano/LiteralValue>
#include <Soprano/Vocabulary/NAO>

void Nepomuk2::Utils::SearchWidget::SearchWidgetPrivate::setupFacetWidget()
{
    m_facetWidget->clear();
    m_facetWidget->addFacet(Facet::createTypeFacet(m_facetWidget));
    m_facetWidget->addFacet(Facet::createDateFacet(m_facetWidget));
    m_facetWidget->addFacet(Facet::createPriorityFacet(m_facetWidget));
    m_facetWidget->addFacet(Facet::createTagFacet(m_facetWidget));
}


void Nepomuk2::Utils::SearchWidget::SearchWidgetPrivate::_k_queryComponentChanged()
{
    if ( !m_inQueryComponentChanged ) {
        m_inQueryComponentChanged = true;
        const Query::Query query = currentQuery();
        if( query != m_currentQuery ) {
            m_resourceModel->clear();
            // TODO: show busy indicator
            kDebug() << query;
            m_queryClient.close();
            if( query.isValid() ) {
                m_queryClient.query(query);
            }
            m_facetWidget->setClientQuery(query);
            m_currentQuery = query;
        }
        m_inQueryComponentChanged = false;
    }
    else {
        // we need to handle all component changes since one may trigger another
        QMetaObject::invokeMethod(q, "_k_queryComponentChanged", Qt::QueuedConnection);
    }
}


void Nepomuk2::Utils::SearchWidget::SearchWidgetPrivate::_k_listingFinished()
{
    // TODO: disable busy indicator
}

void Nepomuk2::Utils::SearchWidget::SearchWidgetPrivate::_k_forwardCurrentChanged(
        const QModelIndex & previous, const QModelIndex & current )
{
    Resource prevRes;
    Resource currRes;
    if ( previous.isValid() ) {
        prevRes = previous.data(Utils::ResourceModel::ResourceRole).value<Resource>();
    }
    if ( current.isValid() ) {
        currRes = current.data(Utils::ResourceModel::ResourceRole).value<Resource>();
    }

    emit q->currentResourceChanged(prevRes,currRes);
}

Nepomuk2::Query::Query Nepomuk2::Utils::SearchWidget::SearchWidgetPrivate::currentQuery( bool withBaseQuery ) const
{
    Query::Query query;
    if( withBaseQuery ) {
        kDebug() << "************ baseQuery:  " << m_baseQuery;
        query = m_baseQuery;
    }

    Query::Term facetTerm = m_facetWidget->queryTerm();
    Query::Term userQueryTerm = m_queryEdit->query().term();

    kDebug() << "************ userQuery:  " << userQueryTerm;
    kDebug() << "************ facetQuery: " << facetTerm;

    return query && facetTerm && userQueryTerm;
}


Nepomuk2::Utils::SearchWidget::SearchWidget(QWidget *parent)
    : QWidget(parent),
      d_ptr(new SearchWidgetPrivate())
{
    Q_D(SearchWidget);
    d->q = this;

    //query editor widget
    d->m_queryEdit = new SearchLineEdit(this);
    d->m_queryButton = new KPushButton(i18n("Search"), this);
    connect(d->m_queryEdit, SIGNAL(queryChanged(Nepomuk2::Query::Query)), this, SLOT(_k_queryComponentChanged()));
    connect(d->m_queryButton, SIGNAL(clicked()), this, SLOT(_k_queryComponentChanged()));

    //item widget
    d->m_itemWidget = new QListView(this);
    d->m_resourceModel = new Utils::SimpleResourceModel(this);
    d->m_itemWidget->setModel(d->m_resourceModel);
    connect(d->m_itemWidget->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SIGNAL(selectionChanged()));
    connect(d->m_itemWidget->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(_k_forwardCurrentChanged(QModelIndex,QModelIndex)));

    //facets widget
    d->m_facetWidget = new Nepomuk2::Utils::FacetWidget(this);
    d->setupFacetWidget();
    connect(d->m_facetWidget, SIGNAL(queryTermChanged(Nepomuk2::Query::Term)), this, SLOT(_k_queryComponentChanged()));

    //layout and config
    QSplitter* facetSplitter = new QSplitter(this);
    facetSplitter->addWidget(d->m_itemWidget);
    facetSplitter->addWidget(d->m_facetWidget);

    QGridLayout* layout = new QGridLayout( this );
    layout->setMargin(0);
    layout->addWidget( d->m_queryEdit, 0, 0 );
    layout->addWidget( d->m_queryButton, 0, 1 );
    layout->addWidget( facetSplitter, 1, 0, 1, 2 );
    layout->setRowStretch(1,1);

    // query client setup
    connect( &d->m_queryClient, SIGNAL(newEntries(QList<Nepomuk2::Query::Result>)),
            d->m_resourceModel, SLOT(addResults(QList<Nepomuk2::Query::Result>)) );
    connect( &d->m_queryClient, SIGNAL(finishedListing()),
            this, SLOT(_k_listingFinished()));

    setSelectionMode(QListView::ExtendedSelection);
    setConfigFlags( DefaultConfigFlags );
}

Nepomuk2::Utils::SearchWidget::~SearchWidget()
{
    delete d_ptr;
}

void Nepomuk2::Utils::SearchWidget::setConfigFlags( ConfigFlags flags )
{
    Q_D(SearchWidget);
    d->m_configFlags = flags;
    d->m_facetWidget->setShown( flags&ShowFacets );
    d->m_queryButton->setShown( !( flags&SearchWhileYouType ) );
    d->m_queryEdit->setSearchWhileTypingEnabled( flags&SearchWhileYouType );
}

Nepomuk2::Utils::SearchWidget::ConfigFlags Nepomuk2::Utils::SearchWidget::configFlags() const
{
    Q_D(const SearchWidget);
    return d->m_configFlags;
}

void Nepomuk2::Utils::SearchWidget::setSelectionMode ( QAbstractItemView::SelectionMode mode )
{
    Q_D(SearchWidget);

    d->m_itemWidget->setSelectionMode(mode);
}

QAbstractItemView::SelectionMode Nepomuk2::Utils::SearchWidget::selectionMode () const
{
    Q_D(const SearchWidget);

    return d->m_itemWidget->selectionMode();
}

Nepomuk2::Query::Query Nepomuk2::Utils::SearchWidget::setQuery( const Nepomuk2::Query::Query &query )
{
    Q_D(SearchWidget);

    // try to extract as much as possible from the query as facets
    Query::Query restQuery = d->m_facetWidget->extractFacetsFromQuery( query );

    // try to get the rest into the line edit
    restQuery.setTerm( d->m_queryEdit->extractUsableTerms( restQuery.term() ) );

    return restQuery;
}

void Nepomuk2::Utils::SearchWidget::setBaseQuery( const Query::Query& query )
{
    Q_D(SearchWidget);
    d->m_baseQuery = query;
    d->_k_queryComponentChanged();
}

Nepomuk2::Query::Query Nepomuk2::Utils::SearchWidget::baseQuery() const
{
    Q_D(const SearchWidget);
    return d->m_baseQuery;
}

Nepomuk2::Query::Query Nepomuk2::Utils::SearchWidget::query() const
{
    Q_D(const SearchWidget);
    return d->currentQuery();
}

Nepomuk2::Resource Nepomuk2::Utils::SearchWidget::currentResource() const
{
    Q_D(const SearchWidget);
    return d->m_itemWidget->currentIndex().data(Utils::ResourceModel::ResourceRole).value<Resource>();
}

QList<Nepomuk2::Resource> Nepomuk2::Utils::SearchWidget::selectedResources() const
{
    Q_D(const SearchWidget);
    QList<Nepomuk2::Resource> resourceList;
    foreach(const QModelIndex& index, d->m_itemWidget->selectionModel()->selectedIndexes()) {
        resourceList << index.data(Utils::ResourceModel::ResourceRole).value<Resource>();
    }
    return resourceList;
}


Nepomuk2::Utils::FacetWidget* Nepomuk2::Utils::SearchWidget::facetWidget() const
{
    Q_D(const SearchWidget);
    return d->m_facetWidget;
}


// static
Nepomuk2::Resource Nepomuk2::Utils::SearchWidget::searchResource( QWidget* parent,
                                                               const Nepomuk2::Query::Query& baseQuery,
                                                               SearchWidget::ConfigFlags flags )
{
    KDialog dlg( parent );
    dlg.setButtons(KDialog::Ok | KDialog::Cancel);
    SearchWidget* searchWidget = new SearchWidget(&dlg);
    dlg.setMainWidget(searchWidget);

    searchWidget->setBaseQuery( baseQuery );
    searchWidget->setConfigFlags( flags );
    searchWidget->setSelectionMode( QAbstractItemView::SingleSelection );

    if( dlg.exec() == QDialog::Accepted ) {
        return searchWidget->currentResource();
    }
    else {
        return Nepomuk2::Resource();
    }
}


// static
QList<Nepomuk2::Resource> Nepomuk2::Utils::SearchWidget::searchResources( QWidget* parent,
                                                                       const Nepomuk2::Query::Query& baseQuery,
                                                                       SearchWidget::ConfigFlags flags )
{
    KDialog dlg( parent );
    dlg.setButtons(KDialog::Ok | KDialog::Cancel);
    SearchWidget* searchWidget = new SearchWidget(&dlg);
    dlg.setMainWidget(searchWidget);

    searchWidget->setBaseQuery( baseQuery );
    searchWidget->setConfigFlags( flags );
    searchWidget->setSelectionMode( QAbstractItemView::ExtendedSelection );

    if( dlg.exec() == QDialog::Accepted ) {
        return searchWidget->selectedResources();
    }
    else {
        return QList<Nepomuk2::Resource>();
    }
}

#include "searchwidget.moc"
