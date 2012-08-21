/*
   This file is part of the Nepomuk KDE project.
   Copyright 2008-2009 Sebastian Trueg <trueg@kde.org>

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

#include "nepomukmassupdatejob.h"

#include "klocale.h"
#include "kdebug.h"

#include <Nepomuk2/Tag>


Nepomuk2::MassUpdateJob::MassUpdateJob( QObject* parent )
    : KJob( parent ),
      m_index( -1 )
{
    kDebug();
    setCapabilities( Killable|Suspendable );
    connect( &m_processTimer, SIGNAL(timeout()),
             this, SLOT(slotNext()) );
}


Nepomuk2::MassUpdateJob::~MassUpdateJob()
{
    kDebug();
}


void Nepomuk2::MassUpdateJob::setFiles( const KUrl::List& urls )
{
    m_resources.clear();
    foreach( const KUrl &url, urls ) {
        m_resources.append( Resource( url ) );
    }
    setTotalAmount( KJob::Files, m_resources.count() );
}


void Nepomuk2::MassUpdateJob::setResources( const QList<Nepomuk2::Resource>& rl )
{
    m_resources = rl;
    setTotalAmount( KJob::Files, m_resources.count() );
}


void Nepomuk2::MassUpdateJob::setProperties( const QList<QPair<QUrl,Nepomuk2::Variant> >& props )
{
    m_properties = props;
}


void Nepomuk2::MassUpdateJob::start()
{
    if ( m_index < 0 ) {
        kDebug();
        emit description( this,
                          i18n("Changing annotations") );
        m_index = 0;
        m_processTimer.start();
    }
    else {
        kDebug() << "Job has already been started";
    }
}


bool Nepomuk2::MassUpdateJob::doKill()
{
    if ( m_index > 0 ) {
        m_processTimer.stop();
        m_index = -1;
        return true;
    }
    else {
        return false;
    }
}


bool Nepomuk2::MassUpdateJob::doSuspend()
{
    m_processTimer.stop();
    return true;
}


bool Nepomuk2::MassUpdateJob::doResume()
{
    if ( m_index > 0 ) {
        m_processTimer.start();
        return true;
    }
    else {
        return false;
    }
}


void Nepomuk2::MassUpdateJob::slotNext()
{
    if ( !isSuspended() ) {
        if ( m_index < m_resources.count() ) {
            Nepomuk2::Resource& res = m_resources[m_index];
            for ( int i = 0; i < m_properties.count(); ++i ) {
                res.setProperty( m_properties[i].first, m_properties[i].second );
            }
            ++m_index;
            setProcessedAmount( KJob::Files, m_index );
        }
        else if ( m_index >= m_resources.count() ) {
            kDebug() << "done";
            m_index = -1;
            m_processTimer.stop();
            emitResult();
        }
    }
}


Nepomuk2::MassUpdateJob* Nepomuk2::MassUpdateJob::tagResources( const QList<Nepomuk2::Resource>& rl, const QList<Nepomuk2::Tag>& tags )
{
    Nepomuk2::MassUpdateJob* job = new Nepomuk2::MassUpdateJob();
    job->setResources( rl );
    //job->setProperties( QList<QPair<QUrl,Nepomuk2::Variant> >() << qMakePair( QUrl( Nepomuk2::Resource::tagUri() ), Nepomuk2::Variant( convertResourceList<Tag>( tags ) ) ) );
    return job;
}


Nepomuk2::MassUpdateJob* Nepomuk2::MassUpdateJob::rateResources( const QList<Nepomuk2::Resource>& rl, int rating )
{
    Nepomuk2::MassUpdateJob* job = new Nepomuk2::MassUpdateJob();
    job->setResources( rl );
    //job->setProperties( QList<QPair<QUrl,Nepomuk2::Variant> >() << qMakePair( QUrl( Nepomuk2::Resource::ratingUri() ), Nepomuk2::Variant( rating ) ) );
    return job;
}

Nepomuk2::MassUpdateJob* Nepomuk2::MassUpdateJob::commentResources( const QList<Nepomuk2::Resource>& rl, const QString& comment )
{
    Nepomuk2::MassUpdateJob* job = new Nepomuk2::MassUpdateJob();
    job->setResources( rl );
    //job->setProperties( QList<QPair<QUrl,Nepomuk2::Variant> >() << qMakePair( QUrl( Nepomuk2::Resource::descriptionUri() ), Nepomuk2::Variant( comment ) ) );
    return job;
}

#include "nepomukmassupdatejob.moc"
