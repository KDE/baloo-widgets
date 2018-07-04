/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2012-2013 Vishesh Handa <me@vhanda.in>

    Adapated from KFileMetadataWidget
    Copyright (C) 2008 by Sebastian Trueg <trueg@kde.org>
    Copyright (C) 2009-2010 by Peter Penz <peter.penz@gmx.at>

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


#ifndef _BALOO_FILE_METADATAWIDGET_H
#define _BALOO_FILE_METADATAWIDGET_H

#include <QWidget>
#include <QUrl>

#include <KFileItem>
#include <QLocale>

#include "widgets_export.h"

namespace Baloo {

/**
 * Modify format of date display
 */
enum class DateFormats {
    LongFormat = QLocale::LongFormat,  ///< @see QLocale::LongFormat
    ShortFormat = QLocale::ShortFormat ///< @see QLocale::ShortFormat
};

class BALOO_WIDGETS_EXPORT FileMetaDataWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly)
    Q_PROPERTY(DateFormats dateFormat READ dateFormat WRITE setDateFormat)

public:
    explicit FileMetaDataWidget(QWidget* parent = nullptr);
    ~FileMetaDataWidget() override;

    /**
     * Sets the items for which the meta data should be shown.
     * The signal metaDataRequestFinished() will be emitted,
     * as soon as the meta data for the items has been received.
     */
    void setItems(const KFileItemList& items);
    KFileItemList items() const;

    /**
     * If set to true, data such as the comment, tag or rating cannot be
     * changed by the user. Per default read-only is disabled.
     */
    void setReadOnly(bool readOnly);
    bool isReadOnly() const;

    /**
     * Set date display format.
     * Per Default format is Long = @see QLocale::LongFormat
     */
    void setDateFormat(const DateFormats format);
    DateFormats dateFormat() const;

    /** @see QWidget::sizeHint() */
    QSize sizeHint() const override;

Q_SIGNALS:
    /**
     * Is emitted, if a meta data represents an URL that has
     * been clicked by the user.
     */
    void urlActivated(const QUrl& url);

    /**
     * Is emitted after the meta data has been received for the items
     * set by KFileMetaDataWidget::setItems().
     * @since 4.6
     */
    void metaDataRequestFinished(const KFileItemList& items);

private:
    class Private;
    Private* d;

    Q_PRIVATE_SLOT(d, void slotLoadingFinished())
    Q_PRIVATE_SLOT(d, void slotLinkActivated(QString))
    Q_PRIVATE_SLOT(d, void slotDataChangeStarted())
    Q_PRIVATE_SLOT(d, void slotDataChangeFinished())
};

}
Q_DECLARE_METATYPE(Baloo::DateFormats)

#endif // _BALOO_FILE_METADATAWIDGET_H
