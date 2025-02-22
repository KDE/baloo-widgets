/*
    SPDX-FileCopyrightText: 2012-2014 Vishesh Handa <vhanda@kde.org>

    Code largely copied/adapted from KFileMetadataProvider
    SPDX-FileCopyrightText: 2010 Peter Penz <peter.penz@gmx.at>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "widgetfactory.h"
#include "KRatingWidget"
#include "filemetadatautil_p.h"
#include "kcommentwidget_p.h"
#include "tagwidget.h"
#include "widgetfactory_p.h"

#include <KFileMetaData/PropertyInfo>
#include <KFileMetaData/UserMetaData>

#include <QCollator>
#include <QLabel>
#include <QLocale>
#include <QMetaType>
#include <QTime>
#include <QUrl>

#include <KApplicationTrader>
#include <KFormat>
#include <KLocalizedString>
#include <KStringHandler>

using namespace Baloo;

WidgetFactory::WidgetFactory(QObject *parent)
    : QObject(parent)
    , m_dateFormat(QLocale::LongFormat)
{
    m_localTimeZone = QTimeZone(QTimeZone::LocalTime);
}

WidgetFactory::~WidgetFactory() = default;

//
// Widget Creation
//
QWidget *WidgetFactory::createWidget(const QString &prop, const QVariant &value, QWidget *parent)
{
    QWidget *widget = nullptr;
    const int maxUrlLength = 80;

    if (prop == QLatin1String("rating")) {
        widget = createRatingWidget(value.toInt(), parent);
    } else if (prop == QLatin1String("userComment")) {
        widget = createCommentWidget(value.toString(), parent);
    } else if (prop == QLatin1String("tags")) {
        widget = createTagWidget(Baloo::Private::sortTags(value.toStringList()), parent);
    } else if (prop == QLatin1String("gpsLocation")) {
        const auto pair = value.value<QPair<float, float>>();
        const auto latitude = pair.first;
        const auto longitude = pair.second;

        const QString latitudeStr = latitude < 0 ? i18nc("Latitude (South)", "%1°S", -latitude) : i18nc("Latitude (North)", "%1°N", latitude);
        const QString longitudeStr = longitude < 0 ? i18nc("Longitude (West)", "%1°W", -longitude) : i18nc("Longitude (East)", "%1°E", longitude);
        const QString gpsLocationStr = latitudeStr + QLatin1Char(' ') + longitudeStr;

        if (const auto geoService = KApplicationTrader::preferredService(QStringLiteral("x-scheme-handler/geo"))) {
            const QString geoUri = QStringLiteral("geo:%1,%2").arg(latitude).arg(longitude);
            QLabel *valueWidget = createLinkWidget(parent);

            valueWidget->setText(QStringLiteral("<a href='%1'>%2</a>").arg(geoUri, gpsLocationStr));
            valueWidget->setToolTip(i18nc("@info:tooltip Show location in map viewer", "Show location in %1", geoService->name()));
            widget = valueWidget;
        } else {
            QLabel *valueWidget = createValueWidget(parent);
            valueWidget->setText(gpsLocationStr);
            widget = valueWidget;
        }

    } else if (prop == QLatin1String("originUrl")) {
        QLabel *valueWidget = createLinkWidget(parent);
        QString valueString = value.toUrl().toString();
        // Shrink link label
        auto labelString = KStringHandler::csqueeze(valueString, maxUrlLength);
        valueString = QStringLiteral("<a href=\"%1\">%2</a>").arg(valueString, labelString);
        valueWidget->setText(valueString);
        widget = valueWidget;

    } else {
        QString valueString;
        QLabel *valueWidget = createValueWidget(parent);

        if (prop == QLatin1String("dimensions") && value.type() == QMetaType::QSize) {
            valueString = i18nc("width × height", "%1 × %2", value.toSize().width(), value.toSize().height());
        } else {
            auto pi = KFileMetaData::PropertyInfo::fromName(prop);
            if (pi.property() != KFileMetaData::Property::Empty) {
                if (pi.valueType() == QVariant::DateTime || pi.valueType() == QVariant::Date) {
                    valueString = formatDateTime(value, m_dateFormat, m_localTimeZone);
                } else {
                    valueString = pi.formatAsDisplayString(value);
                }
            } else {
                valueString = valuetoString(value, m_dateFormat);
            }
        }

        valueWidget->setText(valueString);
        widget = valueWidget;
    }
    if (QLabel *label = qobject_cast<QLabel *>(widget)) {
        // Allow keyboard focus to go to the label for accessibility software like screen readers.
        label->setTextInteractionFlags(Qt::TextBrowserInteraction | Qt::TextSelectableByKeyboard);
    }

    widget->setForegroundRole(parent->foregroundRole());
    widget->setFont(parent->font());
    widget->setObjectName(prop);
    return widget;
}

QWidget *WidgetFactory::createTagWidget(const QStringList &tags, QWidget *parent)
{
    auto tagWidget = new TagWidget(parent);
    tagWidget->setReadyOnly(m_readOnly);
    tagWidget->setSelectedTags(tags);

    connect(tagWidget, &TagWidget::selectionChanged, this, &WidgetFactory::slotTagsChanged);
    connect(tagWidget, &TagWidget::tagClicked, this, [this](const QString &tag) {
        QUrl url;
        url.setScheme(QStringLiteral("tags"));
        url.setPath(tag);
        Q_EMIT urlActivated(url);
    });

    m_tagWidget = tagWidget;
    m_prevTags = tags;

    return tagWidget;
}

QWidget *WidgetFactory::createCommentWidget(const QString &comment, QWidget *parent)
{
    auto commentWidget = new KCommentWidget(parent);
    commentWidget->setText(comment);
    commentWidget->setReadOnly(m_readOnly);

    connect(commentWidget, &KCommentWidget::commentChanged, this, &WidgetFactory::slotCommentChanged);

    m_commentWidget = commentWidget;

    return commentWidget;
}

QWidget *WidgetFactory::createRatingWidget(int rating, QWidget *parent)
{
    auto ratingWidget = new KRatingWidget(parent);
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
    auto valueWidget = new ValueWidget(parent);
    valueWidget->setWordWrap(true);
    valueWidget->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    valueWidget->setTextInteractionFlags(Qt::TextSelectableByMouse);
    valueWidget->setTextFormat(Qt::PlainText);

    return valueWidget;
}

QLabel *WidgetFactory::createLinkWidget(QWidget *parent)
{
    auto valueWidget = new ValueWidget(parent);
    valueWidget->setWordWrap(true);
    valueWidget->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    valueWidget->setTextInteractionFlags(Qt::TextBrowserInteraction);
    valueWidget->setTextFormat(Qt::RichText);
    connect(valueWidget, &ValueWidget::linkActivated, this, [this](const QString &url) {
        Q_EMIT this->urlActivated(QUrl::fromUserInput(url));
    });

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
    }
}

QString WidgetFactory::valuetoString(const QVariant &value, QLocale::FormatType dateFormat)
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
        return formatDateTime(value, dateFormat, m_localTimeZone);
    }
    case QVariant::List: {
        QStringList list;
        const auto valueList = value.toList();
        for (const QVariant &var : valueList) {
            list << valuetoString(var, dateFormat);
        }
        return list.join(i18nc("String list separator", ", "));
    }

    default:
        return value.toString();
    }
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

#include "moc_widgetfactory.cpp"
