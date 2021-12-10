/*
    SPDX-FileCopyrightText: 2012-2013 Vishesh Handa <me@vhanda.in>

    Adapted from KFileMetadataWidget
    SPDX-FileCopyrightText: 2008 Sebastian Trueg <trueg@kde.org>
    SPDX-FileCopyrightText: 2009-2010 Peter Penz <peter.penz@gmx.at>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _BALOO_FILE_METADATAWIDGET_H
#define _BALOO_FILE_METADATAWIDGET_H

#include <QUrl>
#include <QWidget>

#include <KFileItem>
#include <QLocale>

#include "widgets_export.h"

namespace Baloo
{
/**
 * Modify format of date display
 */
enum class DateFormats {
    LongFormat = QLocale::LongFormat, ///< @see QLocale::LongFormat
    ShortFormat = QLocale::ShortFormat, ///< @see QLocale::ShortFormat
};

enum class ConfigurationMode {
    ReStart = 0, /**< Switch into configuration mode. The selection is
                  *  initialized with the current configuration.
                  *  In case the widget is in configuration mode already,
                  *  the changes are discarded, and the mode is kept.
                  */
    Accept, /**< Save any changes, switch to regular mode */
    Cancel, /**< Discard any changes, switch to regular mode */
};

class BALOO_WIDGETS_EXPORT FileMetaDataWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly)
    Q_PROPERTY(DateFormats dateFormat READ dateFormat WRITE setDateFormat)

public:
    explicit FileMetaDataWidget(QWidget *parent = nullptr);
    ~FileMetaDataWidget() override;

    /**
     * Sets the items for which the meta data should be shown.
     * The signal metaDataRequestFinished() will be emitted,
     * as soon as the meta data for the items has been received.
     */
    void setItems(const KFileItemList &items);
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

    /**
     * Switch between regular (view) and configuration mode.
     * @since 19.08.00
     */
    void setConfigurationMode(ConfigurationMode mode);

Q_SIGNALS:
    /**
     * Is emitted, if a meta data represents an URL that has
     * been clicked by the user.
     */
    void urlActivated(const QUrl &url);

    /**
     * Is emitted after the meta data has been received for the items
     * set by KFileMetaDataWidget::setItems().
     * @since 4.6
     */
    void metaDataRequestFinished(const KFileItemList &items);

private:
    class Private;
    Private *d;

    Q_PRIVATE_SLOT(d, void slotLoadingFinished())
    Q_PRIVATE_SLOT(d, void slotLinkActivated(QString))
    Q_PRIVATE_SLOT(d, void slotDataChangeStarted())
    Q_PRIVATE_SLOT(d, void slotDataChangeFinished())
};

}
Q_DECLARE_METATYPE(Baloo::DateFormats)

#endif // _BALOO_FILE_METADATAWIDGET_H
