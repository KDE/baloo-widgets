/*
    Copyright (C) 2012-2014  Vishesh Handa <vhanda@kde.org>

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
#include "KRatingWidget"

#include <KFileMetaData/UserMetaData>

#include <QtWidgets/QLabel>
#include <QtCore/QTime>
#include <QtCore/QUrl>
#include <QtCore/QLocale>

#include <KJob>
#include <KFormat>
#include <KLocalizedString>

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
{
}

WidgetFactory::~WidgetFactory()
{
}

//
// Widget Creation
//
static QString toString(const QVariant& value)
{
    switch (value.type()) {
        case QVariant::Int:
            return QLocale().toString(value.toInt());
        case QVariant::Double:
            return QLocale().toString(value.toDouble());

        case QVariant::List: {
            QStringList list;
            for (const QVariant& var : value.toList()) {
                list << toString(var);
            }

            return list.join(", ");
        }

        default:
            return value.toString();
    }
}

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
        KFormat form;
        // vHanda: FIXME: Add links! Take m_noLinks into consideration
        QString valueString;

        if (prop == QStringLiteral("duration")) {
            valueString = form.formatDuration(value.toInt() * 1000);
        } else if (prop == QStringLiteral("bitRate")) {
            valueString = i18nc("@label bitrate (per second)", "%1/s", form.formatByteSize(value.toInt(), 1, KFormat::MetricBinaryDialect));
        } else {
            // Check if Date/DateTime

            QDateTime dt = QDateTime::fromString(value.toString(), Qt::ISODate);
            if (dt.isValid()) {
                QTime time = dt.time();
                if (!time.hour() && !time.minute() && !time.second()){
                    valueString = form.formatRelativeDate(dt.date(), QLocale::LongFormat);
                } else {
                    valueString = form.formatRelativeDateTime(dt, QLocale::LongFormat);
                }
            }
            else {
                valueString = toString(value);
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

    connect(tagWidget, &TagWidget::selectionChanged, this, &WidgetFactory::slotTagsChanged);
    connect(tagWidget, &TagWidget::tagClicked, this, &WidgetFactory::slotTagClicked);

    m_tagWidget = tagWidget;
    m_prevTags = tags;

    return tagWidget;
}

QWidget* WidgetFactory::createCommentWidget(const QString& comment, QWidget* parent)
{
    KCommentWidget* commentWidget = new KCommentWidget(parent);
    commentWidget->setText(comment);
    commentWidget->setReadOnly(m_readOnly);

    connect(commentWidget, &KCommentWidget::commentChanged, this, &WidgetFactory::slotCommentChanged);

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

    connect(ratingWidget, static_cast<void (KRatingWidget::*)(uint)>(&KRatingWidget::ratingChanged), this, &WidgetFactory::slotRatingChanged);

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
    connect(valueWidget, &ValueWidget::linkActivated, this, &WidgetFactory::slotLinkActivated);

    return valueWidget;
}

//
// Data Synchronization
//

void WidgetFactory::slotCommentChanged(const QString& comment)
{
    for (const QString& filePath : m_items) {
        KFileMetaData::UserMetaData md(filePath);
        md.setUserComment(comment);
    }
    emit dataChangeStarted();
    emit dataChangeFinished();
}

void WidgetFactory::slotRatingChanged(uint rating)
{
    for (const QString& filePath : m_items) {
        KFileMetaData::UserMetaData md(filePath);
        md.setRating(rating);
    }
    emit dataChangeStarted();
    emit dataChangeFinished();
}

void WidgetFactory::slotTagsChanged(const QStringList& tags)
{
    if (m_tagWidget) {
        for (const QString& filePath : m_items) {
            KFileMetaData::UserMetaData md(filePath);

            // When multiple tags are selected one doesn't want to loose the old tags
            // of any of the resources. Unless specifically removed.
            QStringList newTags = md.tags() + tags;
            newTags.removeDuplicates();

            for (const QString& tag : m_prevTags) {
                if (!tags.contains(tag)) {
                    newTags.removeAll(tag);
                }
            }
            md.setTags(newTags);
        }

        m_prevTags = tags;
        emit dataChangeStarted();
        emit dataChangeFinished();
    }
}

//
// Notifications
//

void WidgetFactory::slotLinkActivated(const QString& url)
{
    emit urlActivated(QUrl::fromUserInput(url));
}

void WidgetFactory::slotTagClicked(const QString& tag)
{
    QUrl url;
    url.setScheme("tags");
    url.setPath(tag);

    emit urlActivated(url);
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

