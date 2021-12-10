/*
    SPDX-FileCopyrightText: 2009 Peter Penz <peter.penz@gmx.at>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KEDIT_TAGS_DIALOG_H
#define KEDIT_TAGS_DIALOG_H

#include <QDialog>
#include <QHash>

class QLineEdit;
class KJob;
class QTreeWidget;
class QTreeWidgetItem;
class QPushButton;
class QTimer;

/**
 * @brief Dialog to edit a list of Baloo tags.
 *
 * It is possible for the user to add existing tags,
 * create new tags or to remove tags.
 *
 * @see KMetaDataConfigurationDialog
 */
class KEditTagsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit KEditTagsDialog(const QStringList &tags, QWidget *parent = nullptr);

    ~KEditTagsDialog() override;

    QStringList tags() const;

private Q_SLOTS:
    void slotTextEdited(const QString &text);
    void slotAcceptedButtonClicked();

    void slotItemActivated(const QTreeWidgetItem *item, int column);

private:
    void loadTagWidget();
    void modifyTagWidget(const QString &tag);

private:
    QHash<QString, QTreeWidgetItem *> m_allTagTreeItems;
    QStringList m_tags;
    QStringList m_allTags;
    QString m_newTag;

    QTreeWidget *m_tagTree;
    QLineEdit *m_newTagEdit;
};

#endif
