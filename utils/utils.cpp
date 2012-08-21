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

#include "utils.h"

#include "variant.h"
#include "resourcemanager.h"
#include "resource.h"

#include "filequery.h"
#include "comparisonterm.h"
#include "andterm.h"
#include "resourceterm.h"
#include "resourcetypeterm.h"
#include "optionalterm.h"

#include "nie.h"
#include "nfo.h"
#include "nuao.h"
#include "ndo.h"

#include <kglobal.h>
#include <klocale.h>
#include <kmimetype.h>

#include <Soprano/Model>
#include <Soprano/QueryResultIterator>
#include <Soprano/NodeIterator>


QString Nepomuk2::Utils::formatPropertyValue( const Nepomuk2::Types::Property& property,
                                             const Nepomuk2::Variant& value,
                                             const QList<Nepomuk2::Resource>& resources,
                                             PropertyFormatFlags flags )
{
    // first handle lists
    if( value.isList() ) {
        QList<Variant> values = value.toVariantList();
        QStringList valueStrings;
        Q_FOREACH( const Variant& v, values ) {
            valueStrings << formatPropertyValue( property, v, resources, flags );
        }
        return valueStrings.join( QLatin1String(", ") );
    }

    //
    // We handle the one special case of referrer URLs of downloads
    // TODO: put stuff like this in a generic rule-based framework
    //
    if( property == Nepomuk2::Vocabulary::NDO::copiedFrom() &&
        !resources.isEmpty() ) {
        Nepomuk2::Query::Query query(
            Nepomuk2::Query::AndTerm(
                Nepomuk2::Query::ResourceTypeTerm(
                    Nepomuk2::Vocabulary::NDO::DownloadEvent()
                    ),
                Nepomuk2::Query::ComparisonTerm(
                    Nepomuk2::Vocabulary::NUAO::involves(),
                    Nepomuk2::Query::ResourceTerm(resources.first())
                    )
                )
            );
        query.setLimit(1);

        QList<Soprano::Node> results =
            Nepomuk2::ResourceManager::instance()->mainModel()->executeQuery(
                query.toSparqlQuery(),
                Soprano::Query::QueryLanguageSparql).iterateBindings(0).allNodes();
        if( !results.isEmpty() ) {
            Nepomuk2::Resource dlRes(results.first().uri());
            KUrl url;
            QString label;
            if( dlRes.hasProperty(Nepomuk2::Vocabulary::NDO::referrer()) ) {
                Nepomuk2::Resource referrerWebPage = dlRes.property(Nepomuk2::Vocabulary::NDO::referrer()).toResource();
                url = referrerWebPage.property(Nepomuk2::Vocabulary::NIE::url()).toUrl();
                KUrl referrerDomain(url);
                referrerDomain.setPath(QString());
                referrerDomain.setQuery(QString());
                label = referrerDomain.prettyUrl();
            }
            else {
                Nepomuk2::Resource res(value.toResource());
                url = res.uri();
                label = res.genericLabel();
            }

            if( flags & WithKioLinks ) {
                return QString::fromLatin1("<a href=\"%1\">%2</a>")
                    .arg(url.url(), label);
            }
            else {
                return label;
            }
        }
    }

    // do not use else here since the above code might fall through

    QString valueString;
    if (value.isDateTime()) {
        valueString = KGlobal::locale()->formatDateTime(value.toDateTime().toLocalTime(), KLocale::FancyLongDate);
    }

    else if(value.isDouble()) {
        valueString = KGlobal::locale()->formatNumber(value.toDouble());
    }

    else if(value.isInt() && property == Vocabulary::NFO::duration() ) {
        QTime time = QTime().addSecs( value.toInt() );
        valueString = KGlobal::locale()->formatTime( time, true, true );
    }

    else if(value.isResource() &&
            value.toResource().exists()) {
        valueString = value.toResource().genericLabel();
    }

    else if(property == Vocabulary::NIE::contentSize()) {
        valueString = KGlobal::locale()->formatByteSize(value.toDouble());
    }

    else if(property == Vocabulary::NIE::mimeType()) {
        KMimeType::Ptr mimeType = KMimeType::mimeType(value.toString());
        valueString = (mimeType ? mimeType->comment() : value.toString());
    }

    else {
        valueString = value.toString();
    }

    if( flags & WithKioLinks ) {
        // for all property/value pairs we create a default query
        Nepomuk2::Query::FileQuery query( Nepomuk2::Query::Term::fromProperty(property, value) );
        return QString::fromLatin1("<a href=\"%1\">%2</a>")
            .arg(query.toSearchUrl(property.label() + QLatin1String(": '") + valueString + '\'').url(),
                 valueString);
    }
    else {
        return valueString;
    }
}


Nepomuk2::Resource Nepomuk2::Utils::createCopyEvent( const KUrl& srcUrl, const KUrl& destUrl, const QDateTime& startTime, const KUrl& referrer )
{
    //
    // Remember where a file was downloaded from the semantic way:
    // We have two file resources:
    //   one for the source file (which in most cases is a remote file)
    //   and one for the destination file (which will be or is already indexed)
    // the latter is marked as being copied from the former
    // and then there is the download event which links to the referrer.
    //

    QUrl srcType;
    QUrl destType;
    if(srcUrl.isLocalFile()) {
        srcType = Nepomuk2::Vocabulary::NFO::FileDataObject();
    }
    else {
        srcType = Nepomuk2::Vocabulary::NFO::RemoteDataObject();
    }
    if(destUrl.isLocalFile()) {
        destType = Nepomuk2::Vocabulary::NFO::FileDataObject();
    }
    else {
        destType = Nepomuk2::Vocabulary::NFO::RemoteDataObject();
    }

    // source and dest resources
    Nepomuk2::Resource srcFileRes(srcUrl, srcType);
    Nepomuk2::Resource destFileRes(destUrl, destType);
    srcFileRes.setProperty(Nepomuk2::Vocabulary::NIE::url(), srcUrl);
    destFileRes.setProperty(Nepomuk2::Vocabulary::NIE::url(), destUrl);

    // relate src and dest
    destFileRes.setProperty(Nepomuk2::Vocabulary::NDO::copiedFrom(), srcFileRes);

    // details in the download event
    Nepomuk2::Resource downloadEventRes(QUrl(), Nepomuk2::Vocabulary::NDO::DownloadEvent());
    downloadEventRes.addProperty(Nepomuk2::Vocabulary::NUAO::involves(), destFileRes);
    downloadEventRes.addProperty(Nepomuk2::Vocabulary::NUAO::start(), startTime);

    // set the referrer
    if(referrer.isValid()) {
        // TODO: we could at this point index the referrer site via strigi
        Nepomuk2::Resource referrerRes(referrer, Nepomuk2::Vocabulary::NFO::Website());
        downloadEventRes.addProperty(Nepomuk2::Vocabulary::NDO::referrer(), referrerRes);
    }

    return downloadEventRes;
}


void Nepomuk2::Utils::finishCopyEvent( Resource& /*eventResource*/, const QDateTime& /*endTime*/ )
{
    // FIXME: NUAO doesn't have end() yet.
    // eventResource.setProperty(Nepomuk2::Vocabulary::NUAO::end(), endTime);
}
