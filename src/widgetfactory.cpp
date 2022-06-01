/*
    SPDX-FileCopyrightText: 2012-2014 Vishesh Handa <vhanda@kde.org>

    Code largely copied/adapted from KFileMetadataProvider
    SPDX-FileCopyrightText: 2010 Peter Penz <peter.penz@gmx.at>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "widgetfactory.h"
#include "KRatingWidget"
#include "kcommentwidget_p.h"
#include "tagwidget.h"

#include <KFileMetaData/PropertyInfo>
#include <KFileMetaData/UserMetaData>

#include <QCollator>
#include <QLabel>
#include <QLocale>
#include <QTime>
#include <QUrl>

#include <KFormat>
#include <KJob>
#include <KLocalizedString>
#include <KStringHandler>

using namespace Baloo;

WidgetFactory::WidgetFactory(QObject *parent)
    : QObject(parent)
    , m_readOnly(false)
    , m_dateFormat(QLocale::LongFormat)
{
}

WidgetFactory::~WidgetFactory()
{
}

//
// Widget Creation
//
static QString formatDateTime(const QVariant &value, QLocale::FormatType dateFormat)
{
    const QString valueString = value.toString();
    QDateTime dt = QDateTime::fromString(valueString, Qt::ISODate);

    if (dt.isValid()) {
        KFormat form;
        QTime time = dt.time();
        // Check if Date/DateTime
        if (!time.hour() && !time.minute() && !time.second()) {
            return form.formatRelativeDate(dt.date(), dateFormat);
        } else {
            return form.formatRelativeDateTime(dt, dateFormat);
        }
    }

    return valueString;
}

static QString toString(const QVariant &value, QLocale::FormatType dateFormat)
{
    switch (value.type()) {
    case QVariant::Int:
        return QLocale().toString(value.toInt());
    case QVariant::Double:
        return QLocale().toString(value.toDouble());
    case QVariant::StringList:
        return value.toStringList().join(i18nc("String list separator", ", "));
    case QVariant::Date:
    case QVariant::DateTime: {
        return formatDateTime(value, dateFormat);
    }
    case QVariant::List: {
        QStringList list;
        const auto valueList = value.toList();
        for (const QVariant &var : valueList) {
            list << toString(var, dateFormat);
        }
        return list.join(i18nc("String list separator", ", "));
    }

    default:
        return value.toString();
    }
}

QWidget *WidgetFactory::createWidget(const QString &prop, const QVariant &value, QWidget *parent)
{
    QWidget *widget = nullptr;
    const int maxUrlLength = 80;

    if (prop == QLatin1String("rating")) {
        widget = createRatingWidget(value.toInt(), parent);
    } else if (prop == QLatin1String("userComment")) {
        widget = createCommentWidget(value.toString(), parent);
    } else if (prop == QLatin1String("tags")) {
        QStringList tags = value.toStringList();
        QCollator coll;
        coll.setNumericMode(true);
        std::sort(tags.begin(), tags.end(), [&](const QString &s1, const QString &s2) {
            return coll.compare(s1, s2) < 0;
        });
        widget = createTagWidget(tags, parent);
    } else {
        QString valueString;
        QLabel *valueWidget = createValueWidget(parent);

        auto pi = KFileMetaData::PropertyInfo::fromName(prop);
        if (pi.name() == QLatin1String("originUrl")) {
            auto url = value.toUrl();
            valueString = url.toString();
            // Shrink link label
            auto labelString = KStringHandler::csqueeze(valueString, maxUrlLength);
            valueString = QStringLiteral("<a href=\"%1\">%2</a>").arg(valueString, labelString);
            valueWidget->setTextFormat(Qt::RichText);
            valueWidget->setTextInteractionFlags(Qt::TextBrowserInteraction);

        } else if (pi.name() != QLatin1String("empty")) {
            if (pi.valueType() == QVariant::DateTime || pi.valueType() == QVariant::Date) {
                valueString = formatDateTime(value, m_dateFormat);
            } else {
                valueString = pi.formatAsDisplayString(value);
            }
        } else {
            valueString = toString(value, m_dateFormat);
        }

        valueWidget->setText(valueString);
        widget = valueWidget;
    }

    widget->setForegroundRole(parent->foregroundRole());
    widget->setFont(parent->font());
    widget->setObjectName(prop);
    return widget;
}

QWidget *WidgetFactory::createTagWidget(const QStringList &tags, QWidget *parent)
{
    TagWidget *tagWidget = new TagWidget(parent);
    tagWidget->setReadyOnly(m_readOnly);
    tagWidget->setSelectedTags(tags);

    connect(tagWidget, &TagWidget::selectionChanged, this, &WidgetFactory::slotTagsChanged);
    connect(tagWidget, &TagWidget::tagClicked, this, &WidgetFactory::slotTagClicked);

    m_tagWidget = tagWidget;
    m_prevTags = tags;

    return tagWidget;
}

QWidget *WidgetFactory::createCommentWidget(const QString &comment, QWidget *parent)
{
    KCommentWidget *commentWidget = new KCommentWidget(parent);
    commentWidget->setText(comment);
    commentWidget->setReadOnly(m_readOnly);

    connect(commentWidget, &KCommentWidget::commentChanged, this, &WidgetFactory::slotCommentChanged);

    m_commentWidget = commentWidget;

    return commentWidget;
}

QWidget *WidgetFactory::createRatingWidget(int rating, QWidget *parent)
{
    KRatingWidget *ratingWidget = new KRatingWidget(parent);
    const Qt::Alignment align = (ratingWidget->layoutDirection() == Qt::LeftToRight) ? Qt::AlignLeft : Qt::AlignRight;
    ratingWidget->setAlignment(align);
    ratingWidget->setRating(rating);
    const QFontMetrics metrics(parent->font());
    ratingWidget->setPixmapSize(metrics.height());

    connect(ratingWidget, static_cast<void (KRatingWidget::*)(int)>(&KRatingWidget::ratingChanged), this, &WidgetFactory::slotRatingChanged);

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
    explicit ValueWidget(QWidget *parent = nullptr);
    QSize sizeHint() const override;
};

ValueWidget::ValueWidget(QWidget *parent)
    : QLabel(parent)
{
}

QSize ValueWidget::sizeHint() const
{
    QFontMetrics metrics(font());
    // TODO: QLabel internally provides already a method sizeForWidth(),
    // that would be sufficient. However this method is not accessible.
    return metrics.size(Qt::TextSingleLine, text());
}

QLabel *WidgetFactory::createValueWidget(QWidget *parent)
{
    ValueWidget *valueWidget = new ValueWidget(parent);
    valueWidget->setTextInteractionFlags(Qt::TextSelectableByMouse);
    valueWidget->setTextFormat(Qt::PlainText);
    valueWidget->setWordWrap(true);
    valueWidget->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    connect(valueWidget, &ValueWidget::linkActivated, this, &WidgetFactory::slotLinkActivated);

    return valueWidget;
}

//
// Data Synchronization
//

void WidgetFactory::slotCommentChanged(const QString &comment)
{
    for (const KFileItem &item : std::as_const(m_items)) {
        QUrl url = item.targetUrl();
        if (!url.isLocalFile()) {
            continue;
        }
        KFileMetaData::UserMetaData md(url.toLocalFile());
        md.setUserComment(comment);
    }
    Q_EMIT dataChangeStarted();
    Q_EMIT dataChangeFinished();
}

void WidgetFactory::slotRatingChanged(int rating)
{
    for (const KFileItem &item : std::as_const(m_items)) {
        QUrl url = item.targetUrl();
        if (!url.isLocalFile()) {
            continue;
        }
        KFileMetaData::UserMetaData md(url.toLocalFile());
        md.setRating(rating);
    }
    Q_EMIT dataChangeStarted();
    Q_EMIT dataChangeFinished();
}

void WidgetFactory::slotTagsChanged(const QStringList &tags)
{
    if (m_tagWidget) {
        for (const KFileItem &item : std::as_const(m_items)) {
            QUrl url = item.targetUrl();
            if (!url.isLocalFile()) {
                continue;
            }
            KFileMetaData::UserMetaData md(url.toLocalFile());

            // When multiple tags are selected one doesn't want to loose the old tags
            // of any of the resources. Unless specifically removed.
            QStringList newTags = md.tags() + tags;
            newTags.removeDuplicates();

            for (const QString &tag : std::as_const(m_prevTags)) {
                if (!tags.contains(tag)) {
                    newTags.removeAll(tag);
                }
            }
            md.setTags(newTags);
        }

        m_prevTags = tags;
        Q_EMIT dataChangeStarted();
        Q_EMIT dataChangeFinished();
    }
}

//
// Notifications
//

void WidgetFactory::slotLinkActivated(const QString &url)
{
    Q_EMIT urlActivated(QUrl::fromUserInput(url));
}

void WidgetFactory::slotTagClicked(const QString &tag)
{
    QUrl url;
    url.setScheme(QStringLiteral("tags"));
    url.setPath(tag);

    Q_EMIT urlActivated(url);
}

//
// Accessor Methods
//
void WidgetFactory::setReadOnly(bool value)
{
    m_readOnly = value;
}

void WidgetFactory::setItems(const KFileItemList &items)
{
    m_items = items;
}

Baloo::DateFormats WidgetFactory::dateFormat() const
{
    return static_cast<Baloo::DateFormats>(m_dateFormat);
}

void Baloo::WidgetFactory::setDateFormat(const Baloo::DateFormats format)
{
    m_dateFormat = static_cast<QLocale::FormatType>(format);
}
