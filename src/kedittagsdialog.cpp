/*****************************************************************************
 * Copyright (C) 2009 by Peter Penz <peter.penz@gmx.at>                      *
 * Copyright (C) 2014 by Vishesh Handa <me@vhanda.in>                        *
 * Copyright (C) 2017 by James D. Smith <smithjd15@gmail.com                    *
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

#include <QDebug>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QTreeWidget>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

#include <Baloo/TagListJob>

KEditTagsDialog::KEditTagsDialog(const QStringList &tags, QWidget *parent)
    : QDialog(parent)
    , m_tags(tags)
    , m_tagTree(nullptr)
    , m_newTagEdit(nullptr)
{
    const QString captionText = (tags.count() > 0) ? i18nc("@title:window", "Edit Tags") : i18nc("@title:window", "Add Tags");
    setWindowTitle(captionText);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);

    buttonBox->addButton(i18n("Save"), QDialogButtonBox::AcceptRole);
    buttonBox->addButton(QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &KEditTagsDialog::slotAcceptedButtonClicked);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout *topLayout = new QVBoxLayout;
    setLayout(topLayout);

    QLabel *label = new QLabel(i18nc("@label:textbox",
                                     "Configure which tags should "
                                     "be applied."),
                               this);

    m_tagTree = new QTreeWidget();
    m_tagTree->setSortingEnabled(true);
    m_tagTree->setSelectionMode(QAbstractItemView::NoSelection);
    m_tagTree->setHeaderHidden(true);

    QLabel *newTagLabel = new QLabel(i18nc("@label", "Create new tag:"));
    m_newTagEdit = new QLineEdit(this);
    m_newTagEdit->setClearButtonEnabled(true);
    m_newTagEdit->setFocus();
    connect(m_newTagEdit, &QLineEdit::textEdited, this, &KEditTagsDialog::slotTextEdited);
    connect(m_tagTree, &QTreeWidget::itemActivated, this, &KEditTagsDialog::slotItemActivated);

    QHBoxLayout *newTagLayout = new QHBoxLayout();
    newTagLayout->addWidget(newTagLabel);
    newTagLayout->addWidget(m_newTagEdit, 1);

    topLayout->addWidget(label);
    topLayout->addWidget(m_tagTree);
    topLayout->addLayout(newTagLayout);
    topLayout->addWidget(buttonBox);

    resize(sizeHint());

    Baloo::TagListJob *job = new Baloo::TagListJob();
    connect(job, &Baloo::TagListJob::finished, [this](KJob *job) {
        Baloo::TagListJob *tjob = static_cast<Baloo::TagListJob *>(job);
        m_allTags = tjob->tags();
        loadTagWidget();
    });

    job->start();
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

    for (const QTreeWidgetItem *item : m_allTagTreeItems.values()) {
        if (item->checkState(0) == Qt::Checked) {
            m_tags << qvariant_cast<QString>(item->data(0, Qt::UserRole));
        }
    }

    accept();
}

void KEditTagsDialog::slotItemActivated(const QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)

    const QString tag = qvariant_cast<QString>(item->data(0, Qt::UserRole));
    m_newTagEdit->setText(tag + QLatin1Char('/'));
    m_newTagEdit->setFocus();
}

void KEditTagsDialog::slotTextEdited(const QString &text)
{
    // Remove unnecessary spaces from a new tag is
    // mandatory, as the user cannot see the difference
    // between a tag "Test" and "Test ".
    QString tagText = text.simplified();
    while (tagText.endsWith(QLatin1String("//"))) {
        tagText.chop(1);
        m_newTagEdit->setText(tagText);
        return;
    }

    // Remove all tree items related to the previous new tag
    const QStringList splitTag = m_newTag.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    for (int i = splitTag.size() - 1; i >= 0 && i < splitTag.size(); --i) {
        QString itemTag = m_newTag.section(QLatin1Char('/'), 0, i, QString::SectionSkipEmpty);
        QTreeWidgetItem *item = m_allTagTreeItems.value(itemTag);

        if (!m_allTags.contains(m_newTag) && (item->childCount() == 0)) {
            if (i != 0) {
                QTreeWidgetItem *parentItem = item->parent();
                parentItem->removeChild(item);
            } else {
                const int row = m_tagTree->indexOfTopLevelItem(item);
                m_tagTree->takeTopLevelItem(row);
            }

            m_allTagTreeItems.remove(itemTag);
        }

        if (!m_tags.contains(itemTag)) {
            item->setCheckState(0, Qt::Unchecked);
        }

        item->setExpanded(false);
    }

    if (!tagText.isEmpty()) {
        m_newTag = tagText;
        modifyTagWidget(tagText);
        m_tagTree->sortItems(0, Qt::SortOrder::AscendingOrder);
    } else {
        m_newTag.clear();
        m_allTagTreeItems.clear();
        m_tagTree->clear();
        loadTagWidget();
    }
}

void KEditTagsDialog::loadTagWidget()
{
    for (const QString &tag : m_tags) {
        modifyTagWidget(tag);
    }

    for (const QString &tag : m_allTags) {
        modifyTagWidget(tag);
    }

    m_tagTree->sortItems(0, Qt::SortOrder::AscendingOrder);
}

void KEditTagsDialog::modifyTagWidget(const QString &tag)
{
    const QStringList splitTag = tag.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    for (int i = 0; i < splitTag.size(); ++i) {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        QString itemTag = tag.section(QLatin1Char('/'), 0, i, QString::SectionSkipEmpty);

        if (!m_allTagTreeItems.contains(itemTag)) {
            item->setText(0, splitTag.at(i));
            item->setIcon(0, QIcon::fromTheme(QLatin1String("tag")));
            item->setData(0, Qt::UserRole, itemTag);
            m_allTagTreeItems.insert(itemTag, item);
            QString parentTag = tag.section(QLatin1Char('/'), 0, (i - 1), QString::SectionSkipEmpty);
            QTreeWidgetItem *parentItem = m_allTagTreeItems.value(parentTag);

            if (i != 0) {
                parentItem->addChild(item);
            } else {
                m_tagTree->addTopLevelItem(item);
            }
        } else {
            item = m_allTagTreeItems.value(itemTag);
        }

        if (!m_allTags.contains(tag)) {
            m_tagTree->scrollToItem(item, QAbstractItemView::PositionAtCenter);
        }

        if (((item->childCount() != 0) && m_tags.contains(tag)) || (m_newTag == tag)) {
            item->setExpanded(true);
        } else if (item->parent() && m_tags.contains(tag)) {
            item->parent()->setExpanded(true);
        }

        const bool check = (m_tags.contains(itemTag) || (m_newTag == itemTag));
        item->setCheckState(0, check ? Qt::Checked : Qt::Unchecked);
    }
}
