/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2010 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _BALOO_TAG_WIDGET_P_H_
#define _BALOO_TAG_WIDGET_P_H_

#include "tagwidget.h"

#include <QtCore/QList>
#include <QtCore/QMap>

class QLabel;
class KBlockLayout;
class KEditTagsDialog;

namespace Baloo {

class TagCheckBox;

class TagWidgetPrivate
{
public:
    void init( TagWidget* parent );
    void rebuild();
    void buildTagHash( const QList<Tag>& tags );
    QList<Tag> intersectResourceTags();

    /// lookup (and if necessary create) checkbox for tag
    TagCheckBox* getTagCheckBox( const Tag& tag );

    /// check the corresponding checkboxes and even
    /// add missing checkboxes
    void selectTags( const QList<Tag>& tags );

    bool m_readOnly;

    QMap<Tag, TagCheckBox*> m_checkBoxHash;
    QLabel* m_showAllLinkLabel;
    KBlockLayout* m_flowLayout;
    TagWidget* q;

    KEditTagsDialog* m_editTagsDialog;
};
}

#endif
