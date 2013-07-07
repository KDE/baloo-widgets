/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2012  Vishesh Handa <me@vhanda.in>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "resourceloader.h"
#include <KDebug>

#include <QtCore/QThread>
#include <Nepomuk2/Variant>
#include <Nepomuk2/ResourceManager>
#include <Nepomuk2/Types/Property>

using namespace Nepomuk2;

class ResourceLoader::LoadingThread : public QThread {
public:
    LoadingThread(const QList<QUrl>& uriList, QObject* parent = 0)
        : QThread(parent)
        , m_uriList(uriList)
    {}

    virtual void run() {
        m_shouldExit = false;

        if( !Nepomuk2::ResourceManager::instance()->initialized() )
            return;

        m_resourceList.reserve( m_uriList.size() );
        foreach(const QUrl& uri, m_uriList) {
            if( m_shouldExit )
                return;

            Resource res( uri );
            const QHash<QUrl, Variant> data = res.properties();

            // Load all the associated properties as well so that we do not block in the main thread
            QHash< QUrl, Variant >::const_iterator it = data.constBegin();
            for(; it != data.constEnd(); it++) {
                Types::Property( it.key() ).userVisible();
            }

            m_resourceList.append( res );
        }
    }

    QList<QUrl> m_uriList;
    QList<Resource> m_resourceList;

    bool m_shouldExit;
};

ResourceLoader::ResourceLoader(const QList< QUrl >& uriList, QObject* parent)
    : QObject(parent)
{
    m_thread = new LoadingThread( uriList, this );
    connect( m_thread, SIGNAL(finished()), this, SLOT(slotFinished()) );
}

ResourceLoader::~ResourceLoader()
{
    m_thread->m_shouldExit = true;
    m_thread->wait();

    delete m_thread;
}


QList< Resource > ResourceLoader::resources()
{
    return m_resources;
}

void ResourceLoader::start()
{
    m_thread->start();
}

void ResourceLoader::slotFinished()
{
    m_resources = m_thread->m_resourceList;
    emit finished( this );
}
