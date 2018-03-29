/*
   This file is part of the Baloo KDE project.
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
#include "tagwidgettest.h"
#include "tagwidget.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QCheckBox>
#include <QtCore/QDebug>

TagWidgetTest::TagWidgetTest()
    : QWidget()
{
    m_tagWidget = new Baloo::TagWidget(this);
    QVBoxLayout* lay = new QVBoxLayout(this);
    lay->addWidget(m_tagWidget);
    connect(m_tagWidget, SIGNAL(tagClicked(QString)),
            this, SLOT(slotTagClicked(QString)));
    connect(m_tagWidget, SIGNAL(selectionChanged(QStringList)),
            this, SLOT(slotSelectionChanged(QStringList)));

    QCheckBox* box = new QCheckBox( "Align Right", this );
    connect(box, SIGNAL(toggled(bool)), this, SLOT(alignRight(bool)));
    lay->addWidget(box);

    box = new QCheckBox( "Read only", this );
    connect(box, SIGNAL(toggled(bool)), this, SLOT(setReadOnly(bool)));
    lay->addWidget(box);
}

TagWidgetTest::~TagWidgetTest()
{
}


void TagWidgetTest::slotTagClicked(const QString& tag)
{
    qDebug() << "Tag clicked:" << tag;
}


void TagWidgetTest::slotSelectionChanged( const QStringList& tags )
{
    qDebug() << "Selection changed:" << tags;
}


void TagWidgetTest::alignRight( bool enable )
{
    if( enable )
        m_tagWidget->setAlignment( Qt::AlignRight );
    else
        m_tagWidget->setAlignment( Qt::AlignLeft );
}


void TagWidgetTest::setReadOnly( bool enable )
{
    m_tagWidget->setReadyOnly(enable);
}

