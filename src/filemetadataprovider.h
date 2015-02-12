/*****************************************************************************
 * Copyright (C) 2010 by Peter Penz <peter.penz@gmx.at>                      *
 *                                                                           *
 * This library is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Library General Public               *
 * License as published by the Free Software Foundation; either              *
 * version 2 of the License, or (at your option) any later version.          *
 *                                                                           *
 * This library is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Library General Public License for more details.                          *
 *                                                                           *
 * You should have received a copy of the GNU Library General Public License *
 * along with this library; see the file COPYING.LIB.  If not, write to      *
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 * Boston, MA 02110-1301, USA.                                               *
 *****************************************************************************/

#ifndef _BALOO_FILEMETADATAMODEL_H
#define _BALOO_FILEMETADATAMODEL_H

#include <QObject>
#include <QString>
#include <QVariant>

#include <KFileItem>
#include <Baloo/IndexerConfig>

namespace Baloo {

/**
 * @brief Provides the data for the MetaDataWidget.
 *
 * The default implementation provides all meta data
 * that are available due to Nepomuk. If custom
 * meta data should be added, the method KFileMetaDataProvider::loadData()
 * must be overwritten.
 *
 * @see FileMetaDataWidget
 */
class FileMetaDataProvider : public QObject
{
    Q_OBJECT

public:
    explicit FileMetaDataProvider(QObject* parent = 0);
    virtual ~FileMetaDataProvider();

    /**
     * Sets the items, where the meta data should be
     * requested. The loading of the meta data is done
     * asynchronously. The signal loadingFinished() is
     * emitted, as soon as the loading has been finished.
     * The meta data can be retrieved by
     * KFileMetaDataProvider::data() afterwards. The label for
     * each item can be retrieved by KFileMetaDataProvider::label().
     */
    void setItems(const KFileItemList& items);
    KFileItemList items() const;

    /**
     * If set to true, data such as the comment, tag or rating cannot be changed by the user.
     * Per default read-only is disabled. The method readOnlyChanged() can be overwritten
     * to react on the change.
     */
    void setReadOnly(bool readOnly);
    bool isReadOnly() const;

    /**
     * @return Translated string for the label of the meta data represented
     *         by \p metaDataUri. If no custom translation is provided, the
     *         base implementation must be invoked.
     */
    virtual QString label(const QString& metaDataLabel) const;

    /**
     * Meta data items are sorted alphabetically by their translated
     * label per default. However it is possible to provide an internal
     * prefix to the label, so that specific items are grouped together.
     * For example it makes sense that the meta data for 'width' and 'height'
     * of an image are shown below each other. By adding a common prefix,
     * a grouping is done.
     * @return Returns the name of the group the meta data indicated
     *         by \p label belongs to. Per default an empty string
     *         is returned.
     */
    virtual QString group(const QString& label) const;

    /**
     * @return Meta data for the items that have been set by
     *         KFileMetaDataProvider::setItems(). The method should
     *         be invoked after the signal loadingFinished() has
     *         been received (otherwise no data will be returned).
     */
    QVariantMap data() const;

    /**
     * Returns true if the items do not exist in the database and
     * have just been indexed. This means, that we should not allow
     * others to search through these items cause they do not exist
     * in the database.
     */
    bool realTimeIndexing();

Q_SIGNALS:
    /**
     * Is emitted after the loading triggered by KFileMetaDataProvider::setItems()
     * has been finished.
     *
     * Can be emitted multiple times to indicate data changes
     */
    void loadingFinished();

private Q_SLOTS:
    void slotLoadingFinished(KJob* job);
    void slotFileFetchFinished(KJob* job);

    void insertBasicData();
    void insertEditableData();

private:
    /**
     * Checks for the existance of \p uri in \p allProperties, and accordingly
     * inserts the total integer value of that property in m_data. On completion
     * it removes \p uri from \p allProperties
     */
    void totalPropertyAndInsert(const QString& prop, const QList<QVariantMap>& resources,
                                QSet<QString>& allProperties);

    /*
     * @return The number of subdirectories for the directory \a path.
     */
    static int subDirectoriesCount(const QString &path);

    bool m_readOnly;

    /// Set to true when the file has been specially indexed and does not exist in the db
    bool m_realTimeIndexing;
    QList<KFileItem> m_fileItems;

    QVariantMap m_data;
    Baloo::IndexerConfig m_config;
};

}
#endif