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

#include <QLabel>
#include <QTime>

#include <KUrl>
#include <KJob>
#include <KDebug>
#include <KGlobal>
#include <KLocale>

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

using namespace Baloo;

WidgetFactory::WidgetFactory(QObject* parent)
    : QObject(parent)
    , m_readOnly( false )
    , m_noLinks( false )
    , m_tagWidget(0)
    , m_ratingWidget(0)
    , m_commentWidget(0)
{
}

WidgetFactory::~WidgetFactory()
{
}

//
// Widget Creation
//

QWidget* WidgetFactory::createWidget(const QString& prop, const QVariant& value, QWidget* parent)
{
    QWidget* widget = 0;

    if (prop == QLatin1String("rating")) {
        widget = createRatingWidget( value.toInt(), parent );
    }
    else if (prop == QLatin1String("userComment")) {
        widget = createCommentWidget( value.toString(), parent );
    }
    else if (prop == QLatin1String("tags")) {
        QStringList tags = value.toStringList();
        widget = createTagWidget( tags, parent );
    }
    else {
        // vHanda: FIXME: Add links! Take m_noLinks into consideration
        QString valueString;

        if (prop == "duration") {
            QTime time = QTime().addSecs(value.toInt());
            valueString = KGlobal::locale()->formatTime(time, true, true);
        }
        else {
            // Check if Date/DateTime
            QDateTime dt = QDateTime::fromString(value.toString(), Qt::ISODate);
            if (dt.isValid()) {
                QTime time = dt.time();
                if (!time.hour() && !time.minute() && !time.second())
                    valueString = KGlobal::locale()->formatDate(dt.date(), KLocale::FancyLongDate);
                else
                    valueString = KGlobal::locale()->formatDateTime(dt, KLocale::FancyLongDate);
            }
            else {
                switch (value.type()) {
                case QVariant::Int:
                    valueString = KGlobal::locale()->formatLong(value.toInt());
                    break;
                case QVariant::Double:
                    valueString = KGlobal::locale()->formatNumber(value.toDouble());
                    break;
                default:
                    valueString = value.toString();
                    break;
                }
            }
        }
        widget = createValueWidget(valueString, parent);
    }

    widget->setForegroundRole(parent->foregroundRole());
    widget->setFont(parent->font());

    return widget;
}

QWidget* WidgetFactory::createTagWidget(const QStringList& tags, QWidget* parent)
{
    TagWidget* tagWidget = new TagWidget(parent);
    tagWidget->setReadyOnly(m_readOnly);
    tagWidget->setSelectedTags(tags);

    connect(tagWidget, SIGNAL(selectionChanged(QStringList)),
            this, SLOT(slotTagsChanged(QStringList)));
    connect(tagWidget, SIGNAL(tagClicked(QString)),
            this, SLOT(slotTagClicked(QString)));

    if (m_tagWidget) {
        delete m_tagWidget;
    }
    m_tagWidget = tagWidget;
    m_prevTags = tags;

    return tagWidget;
}

QWidget* WidgetFactory::createCommentWidget(const QString& comment, QWidget* parent)
{
    KCommentWidget* commentWidget = new KCommentWidget(parent);
    commentWidget->setText(comment);
    commentWidget->setReadOnly(m_readOnly);

    connect(commentWidget, SIGNAL(commentChanged(QString)),
            this, SLOT(slotCommentChanged(QString)));

    if (m_commentWidget) {
        delete m_commentWidget;
    }
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

    if (m_ratingWidget) {
        delete m_ratingWidget;
    }

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
    // FIXME: Save the comments!!
    //KJob* job = Nepomuk2::setProperty( m_uris, NAO::description(), QVariantList() << comment );
    //startChangeDataJob(job);
}

void WidgetFactory::slotRatingChanged(uint rating)
{
    // FIXME: Save the ratings!
    //KJob* job = Nepomuk2::setProperty( m_uris, NAO::numericRating(), QVariantList() << rating );
    //startChangeDataJob(job);
}

void WidgetFactory::slotTagsChanged(const QStringList& tags)
{
    if (m_tagWidget) {
        // FIXME: vHanda: SAVE THE TAGS!!
        /*
        TagRelationCreateJob* job = new TagRelationCreateJob(m_items, tags);

        // FIXME: vHanda : Remove the tags that are no longer applicable
        // When multiple tags are selected one doesn't want to loose the old tags
        // of any of the resources. Unless specifically removed.
        // QSet<Tag> removedTags = m_prevTags.toSet().subtract( tags.toSet() );
        // Remove these tags!

        m_prevTags = tags;
        startChangeDataJob(job);
        */
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
    emit urlActivated(KUrl(url));
}

void WidgetFactory::slotTagClicked(const QString& tag)
{
    // vHanda: FIXME: Create a link for this tag!!
    // emit urlActivated( tag.uri() );
}


//
// Accessor Methods
//
void WidgetFactory::setReadOnly(bool value)
{
    m_readOnly = value;
}

void WidgetFactory::setNoLinks(bool value)
{
    m_noLinks = value;
}

void WidgetFactory::setItems(const QStringList& items)
{
    m_items = items;
}
