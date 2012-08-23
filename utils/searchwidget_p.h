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

#ifndef RESOURCESEARCHWIDGET_P_H
#define RESOURCESEARCHWIDGET_P_H

#include "searchwidget.h"

#include <Nepomuk2/Query/Query>
#include <Nepomuk2/Query/QueryServiceClient>

class QListView;
namespace Nepomuk2 {
    namespace Utils {
        class SimpleResourceModel;
        class FacetWidget;
        class SearchLineEdit;
    }
}
class KPushButton;

/**
  * WARNING: This class is just an implementation detail.
  */
class Nepomuk2::Utils::SearchWidget::SearchWidgetPrivate
{
public:
    SearchWidgetPrivate()
        : m_inQueryComponentChanged( false ) {
    }

    /// creates the default set of facets
    void setupFacetWidget();

    /**
     * Get the current query.
     * \param withBaseQuery If \p true the base query is included, otherwise it is not included.
     * \return The currently selected query.
     */
    Query::Query currentQuery( bool withBaseQuery = true ) const;

    void _k_queryComponentChanged();
    void _k_listingFinished();
    void _k_forwardCurrentChanged( const QModelIndex &, const QModelIndex &);

    SearchLineEdit *m_queryEdit;
    KPushButton *m_queryButton;
    QListView *m_itemWidget;
    Utils::SimpleResourceModel* m_resourceModel;
    Nepomuk2::Utils::FacetWidget *m_facetWidget;

    Nepomuk2::Query::QueryServiceClient m_queryClient;
    Nepomuk2::Query::Query m_currentQuery;

    Nepomuk2::Query::Query m_baseQuery;

    SearchWidget::ConfigFlags m_configFlags;

    bool m_inQueryComponentChanged;

    SearchWidget* q;
};

#endif // RESOURCESEARCHWIDGET_P_H
