/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2012  Vishesh Handa <me@vhanda.in>

    Code largely copied/adapted from KFileMetadataProvider
    Copyright (C) 2010 by Peter Penz <peter.penz@gmx.at>

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


#include "widgetfactory.h"
#include "tagwidget.h"
#include "kcommentwidget_p.h"
#include "kratingwidget.h"

#include <QtGui/QLabel>

#include <KJob>
#include <KDebug>

#include <Nepomuk2/Variant>
#include <Nepomuk2/Resource>
#include <Nepomuk2/Tag>
#include <Nepomuk2/DataManagement>
#include <nepomuk2/utils.h>
#include <Nepomuk2/Types/Property>

#include <Nepomuk2/Vocabulary/NIE>
#include <Soprano/Vocabulary/NAO>

using namespace Soprano::Vocabulary;
using namespace Nepomuk2::Vocabulary;

namespace {
    static QString plainText(const QString& richText)
    {
        QString plainText;
        plainText.reserve(richText.length());

        bool skip = false;
        for (int i = 0; i < richText.length(); ++i) {
            const QChar c = richText.at(i);
            if (c == QLatin1Char('<')) {
                skip = true;
            } else if (c == QLatin1Char('>')) {
                skip = false;
            } else if (!skip) {
                plainText.append(c);
            }
        }

        return plainText;
    }
}

namespace Nepomuk2 {

WidgetFactory::WidgetFactory(QObject* parent): QObject(parent)
{
    m_readOnly = false;
}

WidgetFactory::~WidgetFactory()
{
}

//
// Widget Creation
//

QWidget* WidgetFactory::createWidget(const QUrl& prop, const Variant& value, QWidget* parent)
{
    QWidget* widget = 0;

    if( prop == NAO::numericRating() ) {
        widget = createRatingWidget( value.toInt(), parent );
    }
    else if( prop == NAO::description() ) {
        widget = createCommentWidget( value.toString(), parent );
    }
    else if( prop == NAO::hasTag() ) {
        QList<Tag> tags;
        foreach(const Resource& res, value.toResourceList())
            tags << Tag(res);

        widget = createTagWidget( tags, parent );
    }
    else {
        QList<Resource> resources;
        foreach(const QUrl& uri, m_uris)
            resources << uri;

        QString string = value.toString();
        if( !prop.toString().startsWith("kfileitem#") )
            string = Utils::formatPropertyValue( prop, value, resources, Utils::WithKioLinks );
        widget = createValueWidget( string, parent );
    }

    widget->setForegroundRole(parent->foregroundRole());
    widget->setFont(parent->font());

    return widget;
}

QWidget* WidgetFactory::createTagWidget(const QList<Tag>& tags, QWidget* parent)
{
    TagWidget* tagWidget = new TagWidget(parent);
    tagWidget->setModeFlags(m_readOnly
                            ? TagWidget::MiniMode | TagWidget::ReadOnly
                            : TagWidget::MiniMode);
    tagWidget->setSelectedTags(tags);

    connect(tagWidget, SIGNAL(selectionChanged(QList<Nepomuk2::Tag>)),
            this, SLOT(slotTagsChanged(QList<Nepomuk2::Tag>)));
    connect(tagWidget, SIGNAL(tagClicked(Nepomuk2::Tag)),
            this, SLOT(slotTagClicked(Nepomuk2::Tag)));

    m_tagWidget = tagWidget;

    return tagWidget;
}

QWidget* WidgetFactory::createCommentWidget(const QString& comment, QWidget* parent)
{
    KCommentWidget* commentWidget = new KCommentWidget(parent);
    commentWidget->setText(comment);
    commentWidget->setReadOnly(m_readOnly);

    connect(commentWidget, SIGNAL(commentChanged(QString)),
            this, SLOT(slotCommentChanged(QString)));

    m_commentWidget = commentWidget;

    return commentWidget;
}

QWidget* WidgetFactory::createRatingWidget(int rating, QWidget* parent)
{
    KRatingWidget* ratingWidget = new KRatingWidget(parent);
    const Qt::Alignment align = (ratingWidget->layoutDirection() == Qt::LeftToRight) ?
                                Qt::AlignLeft : Qt::AlignRight;
    ratingWidget->setAlignment(align);
    ratingWidget->setRating(rating);
    const QFontMetrics metrics(parent->font());
    ratingWidget->setPixmapSize(metrics.height());

    connect(ratingWidget, SIGNAL(ratingChanged(uint)),
            this, SLOT(slotRatingChanged(uint)));

    m_ratingWidget = ratingWidget;

    return ratingWidget;
}


// The default size hint of QLabel tries to return a square size.
// This does not work well in combination with layouts that use
// heightForWidth(): In this case it is possible that the content
// of a label might get clipped. By specifying a size hint
// with a maximum width that is necessary to contain the whole text,
// using heightForWidth() assures having a non-clipped text.
class ValueWidget : public QLabel
{
public:
    explicit ValueWidget(QWidget* parent = 0);
    virtual QSize sizeHint() const;
};

ValueWidget::ValueWidget(QWidget* parent) :
    QLabel(parent)
{
}

QSize ValueWidget::sizeHint() const
{
    QFontMetrics metrics(font());
    // TODO: QLabel internally provides already a method sizeForWidth(),
    // that would be sufficient. However this method is not accessible, so
    // as workaround the tags from a richtext are removed manually here to
    // have a proper size hint.
    return metrics.size(Qt::TextSingleLine, plainText(text()));
}

QWidget* WidgetFactory::createValueWidget(const QString& value, QWidget* parent)
{
    ValueWidget* valueWidget = new ValueWidget(parent);
    valueWidget->setWordWrap(true);
    valueWidget->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    valueWidget->setText(m_readOnly ? plainText(value) : value);
    connect(valueWidget, SIGNAL(linkActivated(QString)), this, SLOT(slotLinkActivated(QString)));

    return valueWidget;
}

//
// Data Synchronization
//

void WidgetFactory::slotCommentChanged(const QString& comment)
{
    KJob* job = Nepomuk2::setProperty( m_uris, NAO::description(), QVariantList() << comment );
    startChangeDataJob(job);
}

void WidgetFactory::slotRatingChanged(uint rating)
{
    KJob* job = Nepomuk2::setProperty( m_uris, NAO::numericRating(), QVariantList() << rating );
    startChangeDataJob(job);
}

void WidgetFactory::slotTagsChanged(const QList<Nepomuk2::Tag>& tags)
{
    if( m_tagWidget ) {
        m_tagWidget->setSelectedTags( tags );

        QVariantList tagUris;
        foreach( const Tag& tag, tags )
            tagUris << tag.uri();

        KJob* job = Nepomuk2::setProperty( m_uris, NAO::hasTag(), tagUris );
        startChangeDataJob(job);
    }
}

void WidgetFactory::startChangeDataJob(KJob* job)
{
    connect(job, SIGNAL(result(KJob*)),
            this, SIGNAL(dataChangeFinished()));

    emit dataChangeStarted();
    job->start();
}

//
// Notifications
//

void WidgetFactory::slotLinkActivated(const QString& url)
{
    emit urlActivated( url );
}

void WidgetFactory::slotTagClicked(const Nepomuk2::Tag& tag)
{
    emit urlActivated( tag.uri() );
}


//
// Accessor Methods
//
void WidgetFactory::setReadOnly(bool value)
{
    m_readOnly = value;
}

void WidgetFactory::setUris(const QList< QUrl >& uris)
{
    m_uris = uris;
    // Maybe we should invalidate some of the widgets?
}

QList< QUrl > WidgetFactory::uris()
{
    return m_uris;
}



}

