/*
    SPDX-FileCopyrightText: 2009 Peter Penz <peter.penz@gmx.at>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KEDIT_TAGS_DIALOG_H
#define KEDIT_TAGS_DIALOG_H

#include "widgets_export.h"

#include <QDialog>
#include <QHash>

class QLineEdit;
class KJob;
class QTreeView;
class QTreeWidgetItem;
class QPushButton;
class QTimer;
class QStandardItemModel;
class QSortFilterProxyModel;
class QStandardItem;

/**
 * @brief Dialog to edit a list of Baloo tags.
 *
 * It is possible for the user to add existing tags,
 * create new tags or to remove tags.
 *
 * @see KMetaDataConfigurationDialog
 */
class BALOO_WIDGETS_EXPORT KEditTagsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit KEditTagsDialog(const QStringList &tags, QWidget *parent = nullptr);

    ~KEditTagsDialog() override;

    QStringList tags() const;

private Q_SLOTS:
    void slotTextEdited(const QString &text);
    void slotAcceptedButtonClicked();

    void slotItemActivated(const QModelIndex &index);

private:
    void setupModel(const QStringList &allTags, const QStringList &selectedTags);
    QStandardItem *ensureItemForTagExists(const QString tag);
    QStandardItem *addTag(QStandardItem *parentItem, const QString &cannonicalTagPath, const QString &tagName);
    QStandardItem *findTag(const QString tag);
    QStandardItem *findSubItem(QString split, QStandardItem *parentItem);

    QStringList m_tags;
    QStringList m_allTags;
    QTreeView *m_treeView = nullptr;
    QLineEdit *m_newTagEdit = nullptr;
    QStandardItemModel *m_model = nullptr;
    QStandardItem *m_newItem = nullptr;
};

#endif
