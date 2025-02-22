/*
    SPDX-FileCopyrightText: 2012 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef WIDGETFACTORY_H
#define WIDGETFACTORY_H

#include "filemetadatawidget.h"

#include <QObject>
#include <QStringList>
#include <QTimeZone>

class QLabel;
class QUrl;
class KCommentWidget;
class KRatingWidget;

namespace Baloo
{
class TagWidget;

class WidgetFactory : public QObject
{
    Q_OBJECT
public:
    explicit WidgetFactory(QObject *parent = nullptr);
    ~WidgetFactory() override;

    void setItems(const KFileItemList &items);

    void setReadOnly(bool value);

    void setDateFormat(const DateFormats format);
    DateFormats dateFormat() const;

    QWidget *createWidget(const QString &prop, const QVariant &value, QWidget *parent);

Q_SIGNALS:
    void urlActivated(const QUrl &url);

private Q_SLOTS:
    void slotTagsChanged(const QStringList &tags);
    void slotCommentChanged(const QString &comment);
    void slotRatingChanged(int rating);

private:
    QWidget *createRatingWidget(int rating, QWidget *parent);
    QWidget *createTagWidget(const QStringList &tags, QWidget *parent);
    QWidget *createCommentWidget(const QString &comment, QWidget *parent);
    QLabel *createValueWidget(QWidget *parent);
    QLabel *createLinkWidget(QWidget *parent);

    QString valuetoString(const QVariant &value, QLocale::FormatType dateFormat);

    TagWidget *m_tagWidget = nullptr;
    KRatingWidget *m_ratingWidget = nullptr;
    KCommentWidget *m_commentWidget = nullptr;

    KFileItemList m_items;
    QStringList m_prevTags;
    bool m_readOnly = false;
    QLocale::FormatType m_dateFormat;
    QTimeZone m_localTimeZone;
};
}

#endif // WIDGETFACTORY_H
