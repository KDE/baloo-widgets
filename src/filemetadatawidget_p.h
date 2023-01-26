/*
    SPDX-FileCopyrightText: 2012-2013 Vishesh Handa <me@vhanda.in>

    Adapted from KFileMetadataWidget
    SPDX-FileCopyrightText: 2008 Sebastian Trueg <trueg@kde.org>
    SPDX-FileCopyrightText: 2009-2010 Peter Penz <peter.penz@gmx.at>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _BALOO_FILE_METADATAWIDGET_P_H
#define _BALOO_FILE_METADATAWIDGET_P_H

#include "widgets_export.h"

#include <QGridLayout>
#include <QList>
#include <QMap>
#include <QScopedPointer>
#include <QStringList>

class QCheckBox;
class QLabel;
class QWidget;

namespace Baloo
{
class FileMetaDataProvider;
class FileMetaDataWidget;
class MetadataFilter;
class WidgetFactory;

/**
 * @internal
 */
class BALOO_WIDGETS_EXPORT FileMetaDataWidgetPrivate
{
public:
    struct Row {
        QCheckBox *checkBox;
        QLabel *label;
        QWidget *value;
    };

    explicit FileMetaDataWidgetPrivate(FileMetaDataWidget *parent);
    ~FileMetaDataWidgetPrivate();

    FileMetaDataWidgetPrivate(const FileMetaDataWidgetPrivate&) = delete;
    FileMetaDataWidget& operator=(const FileMetaDataWidgetPrivate&) = delete;

    static FileMetaDataWidgetPrivate *get(FileMetaDataWidget *q);

    void deleteRows();

    void slotLoadingFinished();
    void slotLinkActivated(const QString &link);
    void slotDataChangeStarted();
    void slotDataChangeFinished();

    QStringList sortedKeys(const QVariantMap &data) const;
    QLabel *createLabel(const QString &key, const QString &itemLabel, FileMetaDataWidget *parent);

    void saveConfig();

    QList<Row> m_rows;
    FileMetaDataProvider *m_provider = nullptr;
    QGridLayout *m_gridLayout = nullptr;

    MetadataFilter *m_filter = nullptr;
    WidgetFactory *m_widgetFactory = nullptr;

    QMap<QString, bool> m_visibilityChanged;
    bool m_configureVisibleProperties = false;

private:
    FileMetaDataWidget *const q;
};

} // namespace Baloo

#endif // _BALOO_FILE_METADATAWIDGET_P_H
