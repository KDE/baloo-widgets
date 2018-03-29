/*
   This file is part of the Baloo KDE project.
   Copyright (C) 2013 Vishesh Handa <me@vhanda.in>
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

#include "tagcheckbox.h"
#include "tagwidget.h"
#include "tagwidget_p.h"

#include <QtGui/QMouseEvent>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>

using namespace Baloo;

TagCheckBox::TagCheckBox(const QString& tag, QWidget* parent)
    : QWidget( parent ),
      m_label(0),
      m_tag(tag),
      m_urlHover(false)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setMargin(0);

    m_label = new QLabel(tag.split("/", QString::SkipEmptyParts).last(), this);
    m_label->setToolTip(tag);
    m_label->setMouseTracking(true);
    m_label->setTextFormat(Qt::PlainText);
    m_label->setForegroundRole(parent->foregroundRole());
    m_child = m_label;

    m_child->installEventFilter( this );
    m_child->setMouseTracking(true);
    layout->addWidget( m_child );
}

TagCheckBox::~TagCheckBox()
{
}

void TagCheckBox::leaveEvent( QEvent* event )
{
    QWidget::leaveEvent( event );
    enableUrlHover( false );
}


bool TagCheckBox::eventFilter( QObject* watched, QEvent* event )
{
    if( watched == m_child ) {
        switch( event->type() ) {
        case QEvent::MouseMove: {
            QMouseEvent* me = static_cast<QMouseEvent*>(event);
            enableUrlHover( tagRect().contains(me->pos()) );
            break;
        }

        case QEvent::MouseButtonRelease: {
            QMouseEvent* me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::LeftButton && tagRect().contains(me->pos())) {
                emit tagClicked( m_tag );
                return true;
            }
            break;
        }

        default:
            // do nothing
            break;
        }
    }

    return QWidget::eventFilter( watched, event );
}


QRect TagCheckBox::tagRect() const
{
    return QRect(QPoint(0, 0), m_label->size());
}


void TagCheckBox::enableUrlHover( bool enable )
{
    if( m_urlHover != enable ) {
        m_urlHover = enable;
        QFont f = font();
        if(enable)
            f.setUnderline(true);
        m_child->setFont(f);
        m_child->setCursor( enable ? Qt::PointingHandCursor : Qt::ArrowCursor );
    }
}

