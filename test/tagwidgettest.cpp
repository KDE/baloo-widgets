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
#include "tagwidgettest.h"
#include "tagwidget.h"

#include <QVBoxLayout>
#include <QCheckBox>
#include <kdebug.h>


TagWidgetTest::TagWidgetTest()
    : QWidget()
{
    m_tagWidget = new Baloo::TagWidget(this);
    QVBoxLayout* lay = new QVBoxLayout(this);
    lay->addWidget(m_tagWidget);
    connect(m_tagWidget, SIGNAL(tagClicked(Baloo::Tag)),
            this, SLOT(slotTagClicked(Baloo::Tag)));
    connect(m_tagWidget, SIGNAL(selectionChanged(QList<Baloo::Tag>)),
            this, SLOT(slotSelectionChanged(QList<Baloo::Tag>)));

    QCheckBox* box = new QCheckBox( "Align Right", this );
    connect(box, SIGNAL(toggled(bool)), this, SLOT(alignRight(bool)));
    lay->addWidget(box);

    box = new QCheckBox( "Disable clicking", this );
    connect(box, SIGNAL(toggled(bool)), this, SLOT(disableClicking(bool)));
    lay->addWidget(box);

    box = new QCheckBox( "Read only", this );
    connect(box, SIGNAL(toggled(bool)), this, SLOT(setReadOnly(bool)));
    lay->addWidget(box);
}

TagWidgetTest::~TagWidgetTest()
{
}


void TagWidgetTest::slotTagClicked(const Baloo::Tag& tag)
{
    kDebug() << "Tag clicked:" << tag.id() << tag.name();
}


void TagWidgetTest::slotSelectionChanged( const QList<Baloo::Tag>& tags )
{
    QStringList ts;
    foreach(const Baloo::Tag& tag, tags)
        ts << tag.name();
    kDebug() << "Selection changed:" << ts;
}


void TagWidgetTest::alignRight( bool enable )
{
    if( enable )
        m_tagWidget->setAlignment( Qt::AlignRight );
    else
        m_tagWidget->setAlignment( Qt::AlignLeft );
}


void TagWidgetTest::disableClicking( bool enable )
{
    Baloo::TagWidget::ModeFlags flags = m_tagWidget->modeFlags();
    m_tagWidget->setModeFlags( enable ? flags | Baloo::TagWidget::DisableTagClicking : flags & ~Baloo::TagWidget::DisableTagClicking );
}


void TagWidgetTest::setReadOnly( bool enable )
{
    Baloo::TagWidget::ModeFlags flags = m_tagWidget->modeFlags();
    m_tagWidget->setModeFlags( enable ? flags | Baloo::TagWidget::ReadOnly : flags & ~Baloo::TagWidget::ReadOnly );
}

#include "tagwidgettest.moc"
