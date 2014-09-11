/*****************************************************************************
 * Copyright (C) 2009 by Peter Penz <peter.penz@gmx.at>                      *
 * Copyright (C) 2014 by Vishesh Handa <me@vhanda.in>                        *
 *                                                                           *
 * This library is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Library General Public               *
 * License version 2 as published by the Free Software Foundation.           *
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

#include "kedittagsdialog_p.h"

#include <KLocalizedString>

#include <QtWidgets/QLineEdit>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtCore/QTimer>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <QtCore/QUrl>
#include <QtWidgets/QDialogButtonBox>

#include <Baloo/TagListJob>

KEditTagsDialog::KEditTagsDialog(const QStringList& tags,
                                 QWidget *parent) :
    QDialog(parent),
    m_tags(tags),
    m_tagsList(0),
    m_newTagItem(0),
    m_autoCheckedItem(0),
    m_newTagEdit(0)
{
    const QString captionText = (tags.count() > 0) ?
                                i18nc("@title:window", "Change Tags") :
                                i18nc("@title:window", "Add Tags");
    setWindowTitle(captionText);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(this);

    buttonBox->addButton(i18n("Save"), QDialogButtonBox::AcceptRole);
    buttonBox->addButton(QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &KEditTagsDialog::slotAcceptedButtonClicked);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout* topLayout = new QVBoxLayout;
    setLayout(topLayout);

    QLabel* label = new QLabel(i18nc("@label:textbox",
                                     "Configure which tags should "
                                     "be applied."), this);

    m_tagsList = new QListWidget();
    m_tagsList->setSortingEnabled(true);
    m_tagsList->setSelectionMode(QAbstractItemView::NoSelection);

    QLabel* newTagLabel = new QLabel(i18nc("@label", "Create new tag:"));
    m_newTagEdit = new QLineEdit(this);
    m_newTagEdit->setClearButtonEnabled(true);
    connect(m_newTagEdit, SIGNAL(textEdited(QString)),
            this, SLOT(slotTextEdited(QString)));

    QHBoxLayout* newTagLayout = new QHBoxLayout();
    newTagLayout->addWidget(newTagLabel);
    newTagLayout->addWidget(m_newTagEdit, 1);

    topLayout->addWidget(label);
    topLayout->addWidget(m_tagsList);
    topLayout->addLayout(newTagLayout);
    topLayout->addWidget(buttonBox);

    resize(sizeHint());

    loadTags();
}

KEditTagsDialog::~KEditTagsDialog()
{
}

QStringList KEditTagsDialog::tags() const
{
    return m_tags;
}

void KEditTagsDialog::slotAcceptedButtonClicked()
{
    m_tags.clear();

    const int count = m_tagsList->count();
    for (int i = 0; i < count; ++i) {
        QListWidgetItem* item = m_tagsList->item(i);
        if (item->checkState() == Qt::Checked) {
            m_tags << item->text();
        }
    }

    accept();
}

void KEditTagsDialog::slotTextEdited(const QString& text)
{
    // Remove unnecessary spaces from a new tag is
    // mandatory, as the user cannot see the difference
    // between a tag "Test" and "Test ".
    const QString tagText = text.simplified();
    if (tagText.isEmpty()) {
        removeNewTagItem();
        return;
    }

    // Check whether the new tag already exists. If this
    // is the case, remove the new tag item.
    const int count = m_tagsList->count();
    for (int i = 0; i < count; ++i) {
        QListWidgetItem* item = m_tagsList->item(i);
        const bool remove = (item->text() == tagText) &&
                            ((m_newTagItem == 0) || (m_newTagItem != item));
        if (remove) {
            m_tagsList->scrollToItem(item);
            if (item->checkState() == Qt::Unchecked) {
                item->setCheckState(Qt::Checked);
                // Remember the checked item, so that it can be unchecked
                // again if the user changes the tag-text.
                m_autoCheckedItem = item;
            }
            removeNewTagItem();
            return;
        }
    }

    // There is no tag in the list with the the passed text.
    if (m_newTagItem == 0) {
        m_newTagItem = new QListWidgetItem(tagText, m_tagsList);
    } else {
        m_newTagItem->setText(tagText);
    }

    if (m_autoCheckedItem != 0) {
        m_autoCheckedItem->setCheckState(Qt::Unchecked);
        m_autoCheckedItem = 0;
    }

    m_newTagItem->setData(Qt::UserRole, QUrl());
    m_newTagItem->setCheckState(Qt::Checked);
    m_tagsList->scrollToItem(m_newTagItem);
}

void KEditTagsDialog::loadTags()
{
    Baloo::TagListJob* job = new Baloo::TagListJob();
    connect(job, SIGNAL(finished(KJob*)), this, SLOT(slotTagsLoaded(KJob*)));

    job->start();
}

void KEditTagsDialog::slotTagsLoaded(KJob* job)
{
    Baloo::TagListJob* tjob = static_cast<Baloo::TagListJob*>(job);

    m_allTags = tjob->tags();
    qSort(m_allTags.begin(), m_allTags.end());

    foreach (const QString &tag, m_allTags) {
        QListWidgetItem* item = new QListWidgetItem(tag, m_tagsList);

        const bool check = m_tags.contains(tag);
        item->setCheckState(check ? Qt::Checked : Qt::Unchecked);
    }
}

void KEditTagsDialog::removeNewTagItem()
{
    if (m_newTagItem != 0) {
        const int row = m_tagsList->row(m_newTagItem);
        m_tagsList->takeItem(row);
        delete m_newTagItem;
        m_newTagItem = 0;
    }
}
