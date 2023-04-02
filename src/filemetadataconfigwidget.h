/*
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2009 Peter Penz <peter.penz@gmx.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef BALOO_FILEMETADATACONFIGWIDGET_H
#define BALOO_FILEMETADATACONFIGWIDGET_H

#include "widgets_export.h"
#include <KFileItem>

#include <QWidget>

#include <memory>

namespace Baloo
{
#if BALOO_WIDGETS_ENABLE_DEPRECATED_SINCE(23, 8)
class FileMetaDataConfigWidgetPrivate;

/**
 * @brief Widget which allows to configure which meta data should be shown
 *        in the FileMetadataWidget
 * @deprecated Since 23.08, use FileMetaDataWidget::setConfigurationMode()
 *             instead.
 */
class BALOO_WIDGETS_EXPORT FileMetaDataConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileMetaDataConfigWidget(QWidget *parent = nullptr);
    ~FileMetaDataConfigWidget() override;

    /**
     * Sets the items, for which the visibility of the meta data should
     * be configured. Note that the visibility of the meta data is not
     * bound to the items itself, the items are only used to determine
     * which meta data should be configurable. For example when a JPEG image
     * is set as item, it will be configurable which EXIF data should be
     * shown. If an audio file is set as item, it will be configurable
     * whether the artist, album name, ... should be shown.
     */
    void setItems(const KFileItemList &items);
    KFileItemList items() const;

    /**
     * Saves the modified configuration.
     */
    void save();

    /** @see QWidget::sizeHint() */
    QSize sizeHint() const override;

protected:
    bool event(QEvent *event) override;

private:
    friend class FileMetaDataConfigWidgetPrivate;
    std::unique_ptr<FileMetaDataConfigWidgetPrivate> const d;

    Q_PRIVATE_SLOT(d, void loadMetaData())
    Q_PRIVATE_SLOT(d, void slotLoadingFinished())
};
#endif
}
#endif
