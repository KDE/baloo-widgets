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


#include "facetdelegate.h"
#include "facet.h"
#include "facetmodel.h"


Nepomuk::Utils::FacetDelegate::FacetDelegate( QObject* parent )
    : QStyledItemDelegate( parent )
{
}

Nepomuk::Utils::FacetDelegate::~FacetDelegate()
{
}

void Nepomuk::Utils::FacetDelegate::paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    // TODO: draw radio buttons instead of check boxes if Facet::selectionMode() == MatchOne, ie. if the facet is exclusive

    // hacky way to not paint anything on title items which we use as spacers for now
    if( !index.data(Qt::DisplayRole).toString().isEmpty() )
        QStyledItemDelegate::paint( painter, option, index );
}
