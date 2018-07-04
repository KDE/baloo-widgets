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


#ifndef WIDGETFACTORY_H
#define WIDGETFACTORY_H

#include "filemetadatawidget.h"

#include <QtCore/QObject>
#include <QtCore/QStringList>

class KJob;
class QUrl;
class KCommentWidget;
class KRatingWidget;

namespace Baloo {

    class Tag;
    class TagWidget;

    class WidgetFactory : public QObject
    {
        Q_OBJECT
    public:
        explicit WidgetFactory(QObject* parent = nullptr);
        virtual ~WidgetFactory();

        void setItems(const QStringList& items);

        void setReadOnly(bool value);
        void setNoLinks(bool value);

        void setDateFormat(const DateFormats format);
        DateFormats dateFormat() const;

        QWidget* createWidget(const QString& prop, const QVariant& value, QWidget* parent);

    Q_SIGNALS:
        void urlActivated(const QUrl& url);
        void dataChangeStarted();
        void dataChangeFinished();

    private Q_SLOTS:
        void slotTagsChanged(const QStringList& tags);
        void slotCommentChanged(const QString& comment);
        void slotRatingChanged(uint rating);

        void slotTagClicked(const QString& tag);
        void slotLinkActivated(const QString& url);

    private:
        QWidget* createRatingWidget(int rating, QWidget* parent);
        QWidget* createTagWidget(const QStringList& tags, QWidget* parent);
        QWidget* createCommentWidget(const QString& comment, QWidget* parent);
        QWidget* createValueWidget(const QString& value, QWidget* parent);

        TagWidget* m_tagWidget;
        KRatingWidget* m_ratingWidget;
        KCommentWidget* m_commentWidget;

        QStringList m_items;
        QStringList m_prevTags;
        bool m_readOnly;
        bool m_noLinks;
        QLocale::FormatType m_dateFormat;
    };
}

#endif // WIDGETFACTORY_H
