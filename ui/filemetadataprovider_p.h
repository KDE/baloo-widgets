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

#ifndef _NEPOMUK_FILEMETADATAMODEL_H
#define _NEPOMUK_FILEMETADATAMODEL_H

#include <KUrl>

#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QString>

#include <Nepomuk2/Variant>

class KFileItemList;
class KProcess;
class QWidget;

namespace Nepomuk2 {

class ResourceLoader;
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
    virtual QString label(const KUrl& metaDataUri) const;

    /**
     * Meta data items are sorted alphabetically by their translated
     * label per default. However it is possible to provide an internal
     * prefix to the label, so that specific items are grouped together.
     * For example it makes sense that the meta data for 'width' and 'height'
     * of an image are shown below each other. By adding a common prefix,
     * a grouping is done.
     * @return Returns the name of the group the meta data indicated
     *         by \p metaDataUri belongs to. Per default an empty string
     *         is returned.
     */
    virtual QString group(const KUrl& metaDataUri) const;

    /**
     * @return Meta data for the items that have been set by
     *         KFileMetaDataProvider::setItems(). The method should
     *         be invoked after the signal loadingFinished() has
     *         been received (otherwise no data will be returned).
     */
    QHash<QUrl, Variant> data() const;


Q_SIGNALS:
    /**
     * Is emitted after the loading triggered by KFileMetaDataProvider::setItems()
     * has been finished.
     *
     * Can be emitted multiple times to indicate data changes
     */
    void loadingFinished();

private:
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT(d, void slotLoadingFinished(ResourceLoader* loader))
    Q_PRIVATE_SLOT(d, void slotLoadingFinished(KJob* job))
    Q_PRIVATE_SLOT(d, void insertBasicData());
};

}
#endif
