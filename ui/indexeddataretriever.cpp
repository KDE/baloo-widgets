/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2012  Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "indexeddataretriever.h"

#include <QtCore/QProcess>
#include <QFileInfo>

#include <KStandardDirs>
#include <KDebug>

using namespace Baloo;

IndexedDataRetriever::IndexedDataRetriever(const QString& fileUrl, QObject* parent): KJob(parent)
{
    m_url = fileUrl;

    // Point to the actual file in the case of a system link
    QFileInfo fileInfo(m_url);
    if( fileInfo.isSymLink() )
        m_url = fileInfo.canonicalFilePath();
}

IndexedDataRetriever::~IndexedDataRetriever()
{
}

void IndexedDataRetriever::start()
{
    const QString exe = KStandardDirs::findExe(QLatin1String("nepomukindexer"));

    m_process = new QProcess( this );
    m_process->setReadChannel( QProcess::StandardOutput );

    QStringList args;
    args << "--data" << m_url;

    connect( m_process, SIGNAL(finished(int)), this, SLOT(slotIndexedFile(int)) );
    m_process->start( exe, args );
}

void IndexedDataRetriever::slotIndexedFile(int)
{
    QByteArray data = QByteArray::fromBase64(m_process->readAllStandardOutput());
    QDataStream in( &data, QIODevice::ReadOnly );

    /* vHanda: FIXME
    Nepomuk2::SimpleResourceGraph graph;
    in >> graph;

    QList< SimpleResource > list = graph.toList();
    foreach( const SimpleResource& res, list ) {
        if( !res.contains(NIE::url()) ) {
            continue;
        }

        QMultiHash<QUrl, QVariant> hash = res.properties();
        foreach(const QUrl& prop, hash.uniqueKeys()) {
            QVariantList varList = hash.values( prop );

            Nepomuk2::Variant variant;
            foreach( const QVariant& var, varList ) {
                variant.append( Variant(var) );
            }

            // In this case we want to extract the data from the blank node
            if( variant.toString().startsWith("_:") ) {
                SimpleResource tempRes = graph[ variant.toUrl() ];
                if( tempRes.isValid() ) {
                    PropertyHash ph = tempRes.properties();
                    QString value = ph.value( NCO::fullname() ).toString();
                    if( value.isEmpty() )
                        value = ph.value( NIE::title() ).toString();
                    if( value.isEmpty() )
                        value = ph.value( NAO::identifier() ).toString();

                    if( !value.isEmpty() )
                        m_data.insert( prop, value );
                }
            }
            else {
                m_data.insert( prop, variant );
            }
        }
    }
    */

    emitResult();
}

/* vHanda: FIXME
QHash< QUrl, Variant > IndexedDataRetriever::data()
{
    return m_data;
}
*/
