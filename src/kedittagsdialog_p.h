/*
 * Copyright (C) 2009 by Peter Penz <peter.penz@gmx.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KEDIT_TAGS_DIALOG_H
#define KEDIT_TAGS_DIALOG_H

#include <QtWidgets/QDialog>

class QLineEdit;
class KJob;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QTimer;

/**
 * @brief Dialog to edit a list of Nepomuk tags.
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
    KEditTagsDialog(const QStringList& tags,
                    QWidget* parent = 0);

    virtual ~KEditTagsDialog();

    QStringList tags() const;

private slots:
    void slotTextEdited(const QString& text);
    void slotAcceptedButtonClicked();


    void slotTagsLoaded(KJob* job);

private:
    void loadTags();
    void removeNewTagItem();

private:
    QStringList m_tags;
    QStringList m_allTags;

    QListWidget* m_tagsList;
    QListWidgetItem* m_newTagItem;
    QListWidgetItem* m_autoCheckedItem;
    QLineEdit* m_newTagEdit;
};

#endif
