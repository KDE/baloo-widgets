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

#include "tagwidget.h"
#include "tagwidget_p.h"
#include "kblocklayout.h"
#include "kedittagsdialog_p.h"
#include "tagcheckbox.h"

#include <Nepomuk2/Tag>
#include <Nepomuk2/ResourceManager>
#include <Nepomuk2/DataManagement>

#include <karrowbutton.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <kdebug.h>
#include <KJob>

#include <QtGui/QPushButton>
#include <QtGui/QBoxLayout>
#include <QtGui/QLabel>
#include <QtCore/QSet>

#include <Soprano/QueryResultIterator>
#include <Soprano/Node>
#include <Soprano/Model>
#include <Soprano/Vocabulary/NAO>

using namespace Soprano::Vocabulary;

void Nepomuk2::TagWidgetPrivate::init( TagWidget* parent )
{
    q = parent;
    m_maxTags = 10;
    m_flags = TagWidget::StandardMode;
    m_blockSelectionChangedSignal = false;
    m_showAllLinkLabel = 0;

    QGridLayout* mainLayout = new QGridLayout( q );
    mainLayout->setMargin(0);
    m_flowLayout = new KBlockLayout( 0, KDialog::spacingHint()*3 );
    m_showAllButton = new QPushButton( i18nc("@label", "Show all tags..."), q );
    q->connect( m_showAllButton, SIGNAL(clicked()), SLOT(slotShowAll()) );
    mainLayout->addLayout( m_flowLayout, 0, 0, 1, 2 );
    mainLayout->addWidget( m_showAllButton, 1, 1, 1, 1 );
    mainLayout->setColumnStretch( 0, 1 );

    rebuild();
}


void Nepomuk2::TagWidgetPrivate::rebuild()
{
    if( m_flags & TagWidget::MiniMode ) {
        buildTagHash( q->selectedTags() );
    }
    else {
        QList<Tag> tags = q->selectedTags();
        buildTagHash( loadTags( m_maxTags ) );
        selectTags( tags );
    }
}


void Nepomuk2::TagWidgetPrivate::buildTagHash( const QList<Tag>& tags )
{
    qDeleteAll(m_checkBoxHash);
    m_checkBoxHash.clear();

    foreach( const Nepomuk2::Tag& tag, tags ) {
        getTagCheckBox( tag );
    }

    delete m_showAllLinkLabel;
    m_showAllLinkLabel = 0;

    if( m_flags & TagWidget::MiniMode ) {
        m_showAllButton->hide();
        if ( (m_flags&TagWidget::ReadOnly) && !tags.isEmpty() ) {
            return;
        }

        m_showAllLinkLabel = new QLabel( q );
        m_flowLayout->addWidget( m_showAllLinkLabel );
        if( m_flags&TagWidget::ReadOnly ) {
            m_showAllLinkLabel->setText("-");
        }
        else {
            QFont f(q->font());
            f.setUnderline(true);
            m_showAllLinkLabel->setFont(f);
            m_showAllLinkLabel->setText( QLatin1String("<a href=\"add_tags\">") +
                                         ( m_checkBoxHash.isEmpty() ? i18nc("@label", "Add Tags...") : i18nc("@label", "Change...") ) +
                                         QLatin1String("</a>") );
            q->connect( m_showAllLinkLabel, SIGNAL(linkActivated(QString)), SLOT(slotShowAll()) );
        }
    }
    else {
        m_showAllButton->setShown( !(m_flags&TagWidget::ReadOnly) );
    }
}


QList<Nepomuk2::Tag> Nepomuk2::TagWidgetPrivate::loadTags( int max )
{
    // get the "max" first tags with the most resources
    QString query = QString::fromLatin1("select ?r count(distinct ?f) as ?c where { "
                                        "?r a %1 . "
                                        "?f %2 ?r . "
                                        "} ORDER BY DESC(?c) LIMIT %3")
                    .arg( Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::Tag()),
                          Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::hasTag()))
                    .arg( max );
    QList<Nepomuk2::Tag> tags;
    Soprano::QueryResultIterator it = ResourceManager::instance()->mainModel()->executeQuery( query, Soprano::Query::QueryLanguageSparql );
    while( it.next() ) {
        // workaround for bug in Virtuoso where resources are returned as strings if a count() is in the select clause
        tags << Tag( KUrl(it["r"].toString()) );
    }
    return tags;
}


QList<Nepomuk2::Tag> Nepomuk2::TagWidgetPrivate::intersectResourceTags()
{
    if ( m_resources.count() == 1 ) {
        return m_resources.first().tags();
    }
    else if ( !m_resources.isEmpty() ) {
        // determine the tags used for all resources
        QSet<Tag> tags = QSet<Tag>::fromList( m_resources.first().tags() );
        QList<Resource>::const_iterator it = m_resources.constBegin();
        for ( ++it; it != m_resources.constEnd(); ++it ) {
            tags.intersect( QSet<Tag>::fromList( (*it).tags() ) );
        }
        return tags.values();
    }
    else {
        return QList<Tag>();
    }
}


Nepomuk2::TagCheckBox* Nepomuk2::TagWidgetPrivate::getTagCheckBox( const Tag& tag )
{
    QMap<Tag, TagCheckBox*>::iterator it = m_checkBoxHash.find(tag);
    if( it == m_checkBoxHash.end() ) {
        kDebug() << "Creating checkbox for" << tag.genericLabel();
        TagCheckBox* checkBox = new TagCheckBox( tag, this, q );
        q->connect( checkBox, SIGNAL(tagClicked(Nepomuk2::Tag)), SIGNAL(tagClicked(Nepomuk2::Tag)) );
        q->connect( checkBox, SIGNAL(tagStateChanged(Nepomuk2::Tag,int)), SLOT(slotTagStateChanged(Nepomuk2::Tag,int)) );
        m_checkBoxHash.insert( tag, checkBox );
        m_flowLayout->addWidget( checkBox );
        return checkBox;
    }
    else {
        return it.value();
    }
}

namespace Nepomuk2 {
/// operator necessary for QMap::erase
bool operator<(const Tag& t1, const Tag& t2) {
    return t1.uri() < t2.uri();
}
}

void Nepomuk2::TagWidgetPrivate::selectTags( const QList<Tag>& tags )
{
    m_blockSelectionChangedSignal = true;

    if( m_flags & TagWidget::MiniMode ) {
        buildTagHash( tags );
    }
    else {
        foreach( TagCheckBox* checkBox, m_checkBoxHash ) {
            checkBox->setChecked( false );
        }

        foreach( const Tag& tag, tags ) {
            getTagCheckBox(tag)->setChecked( true );
        }

        // remove the previously added check boxes which are not amongst
        // the top 10 or remove as many check boxes as possible to get down to
        // m_maxTags
        if( !m_checkBoxHash.isEmpty() ) {
            int r = qMax( m_checkBoxHash.count() - m_maxTags, 0 );
            QMap<Tag, TagCheckBox*>::iterator it = m_checkBoxHash.end();
            --it;
            while( r > 0 ) {

                // stop the loop when we reach the beginning of the hash
                bool end = ( it == m_checkBoxHash.begin() );

                // remove unused checkboxes
                if( !it.value()->isChecked() ) {
                    delete it.value();
                    it = m_checkBoxHash.erase(it);
                    --r;
                }

                if( end ) {
                    break;
                }
                else {
                    --it;
                }
            }
        }
    }

    m_blockSelectionChangedSignal = false;
}

namespace {
    QList<QUrl> resourcesToUrlList(const QList<Nepomuk2::Resource>& resList) {
        QList<QUrl> urlList;
        foreach(const Nepomuk2::Resource& res, resList) {
            urlList << res.uri();
        }
        return urlList;
    }

    QList<QVariant> tagsToVariantList(const QList<Nepomuk2::Tag>& resList) {
        QList<QVariant> list;
        foreach(const Nepomuk2::Tag& res, resList) {
            list << res.uri();
        }
        return list;
    }
}

void Nepomuk2::TagWidgetPrivate::updateResources()
{
    if ( !m_resources.isEmpty() ) {
        KJob* job = setProperty( resourcesToUrlList(m_resources), NAO::hasTag(),
                                 tagsToVariantList(q->selectedTags()) );
        q->connect( job, SIGNAL(result(KJob*)),
                    SLOT(slotTagUpdateDone()) );
        q->setEnabled( false ); // no updates during execution
    }
}


Nepomuk2::TagWidget::TagWidget( const Resource& resource, QWidget* parent )
    : QWidget( parent ),
      d( new TagWidgetPrivate() )
{
    d->init( this );
    setTaggedResource( resource );
}


Nepomuk2::TagWidget::TagWidget( QWidget* parent )
    : QWidget( parent ),
      d( new TagWidgetPrivate() )
{
    d->init( this );
}


Nepomuk2::TagWidget::~TagWidget()
{
    delete d;
}


QList<Nepomuk2::Resource> Nepomuk2::TagWidget::taggedResources() const
{
    return d->m_resources;
}


#ifndef KDE_NO_DEPRECATED
QList<Nepomuk2::Tag> Nepomuk2::TagWidget::assignedTags() const
{
    return selectedTags();
}
#endif


QList<Nepomuk2::Tag> Nepomuk2::TagWidget::selectedTags() const
{
    QList<Nepomuk2::Tag> tags;
    QMapIterator<Tag, TagCheckBox*> it( d->m_checkBoxHash );
    while( it.hasNext() ) {
        it.next();
        if( it.value()->isChecked() )
            tags << it.key();
    }
    return tags;
}


int Nepomuk2::TagWidget::maxTagsShown() const
{
    return d->m_maxTags;
}


Qt::Alignment Nepomuk2::TagWidget::alignment() const
{
    return d->m_flowLayout->alignment();
}


Nepomuk2::TagWidget::ModeFlags Nepomuk2::TagWidget::modeFlags() const
{
    return d->m_flags;
}


void Nepomuk2::TagWidget::setTaggedResource( const Resource& resource )
{
    QList<Resource> l;
    l.append( resource );
    setTaggedResources( l );
}


void Nepomuk2::TagWidget::setTaggedResources( const QList<Resource>& resources )
{
    // reset selection to a sensible default
    d->buildTagHash( d->loadTags( d->m_maxTags ) );

    d->m_resources = resources;
    if ( !resources.isEmpty() ) {
        d->selectTags( d->intersectResourceTags() );
    }
    else {
        d->selectTags( QList<Tag>() );
    }
}


#ifndef KDE_NO_DEPRECATED
void Nepomuk2::TagWidget::setAssignedTags( const QList<Tag>& tags )
{
    setSelectedTags( tags );
}
#endif


void Nepomuk2::TagWidget::setSelectedTags( const QList<Nepomuk2::Tag>& tags )
{
    d->selectTags( tags );
    d->updateResources();
}


void Nepomuk2::TagWidget::setMaxTagsShown( int max )
{
    d->m_maxTags = max;
    setTaggedResources( d->m_resources );
}


void Nepomuk2::TagWidget::setAlignment( Qt::Alignment alignment )
{
    d->m_flowLayout->setAlignment( alignment );
}


void Nepomuk2::TagWidget::setModeFlags( ModeFlags flags )
{
    d->m_flags = flags;
    d->rebuild();
}


void Nepomuk2::TagWidget::slotTagUpdateDone()
{
    setEnabled( true );
}


void Nepomuk2::TagWidget::slotShowAll()
{
    KEditTagsDialog dlg( selectedTags(), this );
    if( dlg.exec() ) {
        setSelectedTags( dlg.tags() );
        emit selectionChanged( selectedTags() );
    }
}


void Nepomuk2::TagWidget::slotTagStateChanged( const Nepomuk2::Tag&, int )
{
    if( !d->m_blockSelectionChangedSignal ) {
        d->updateResources();
        emit selectionChanged( selectedTags() );
    }
}

#include "tagwidget.moc"
