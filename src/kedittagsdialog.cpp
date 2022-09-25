/*
    SPDX-FileCopyrightText: 2009 Peter Penz <peter.penz@gmx.at>
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2017 James D. Smith <smithjd15@gmail.com

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kedittagsdialog_p.h"

#include <KLocalizedString>

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWidget>

#include <Baloo/TagListJob>

KEditTagsDialog::KEditTagsDialog(const QStringList &tags, QWidget *parent)
    : QDialog(parent)
    , m_tags(tags)
{
    const QString captionText = (tags.count() > 0) ? i18nc("@title:window", "Edit Tags") : i18nc("@title:window", "Add Tags");
    setWindowTitle(captionText);
    auto buttonBox = new QDialogButtonBox(this);

    buttonBox->addButton(i18n("Save"), QDialogButtonBox::AcceptRole);
    buttonBox->addButton(QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &KEditTagsDialog::slotAcceptedButtonClicked);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto topLayout = new QVBoxLayout;
    setLayout(topLayout);

    auto label = new QLabel(i18nc("@label:textbox",
                                  "Configure which tags should "
                                  "be applied."),
                            this);

    m_tagTree = new QTreeWidget();
    m_tagTree->setSortingEnabled(true);
    m_tagTree->setSelectionMode(QAbstractItemView::NoSelection);
    m_tagTree->setHeaderHidden(true);

    auto newTagLabel = new QLabel(i18nc("@label", "Create new tag:"));
    m_newTagEdit = new QLineEdit(this);
    m_newTagEdit->setClearButtonEnabled(true);
    m_newTagEdit->setFocus();
    connect(m_newTagEdit, &QLineEdit::textEdited, this, &KEditTagsDialog::slotTextEdited);
    connect(m_tagTree, &QTreeWidget::itemActivated, this, &KEditTagsDialog::slotItemActivated);

    auto newTagLayout = new QHBoxLayout();
    newTagLayout->addWidget(newTagLabel);
    newTagLayout->addWidget(m_newTagEdit, 1);

    topLayout->addWidget(label);
    topLayout->addWidget(m_tagTree);
    topLayout->addLayout(newTagLayout);
    topLayout->addWidget(buttonBox);

    resize(sizeHint());

    auto job = new Baloo::TagListJob();
    connect(job, &Baloo::TagListJob::finished, [this](KJob *job) {
        auto tjob = static_cast<Baloo::TagListJob *>(job);
        m_allTags = tjob->tags();
        loadTagWidget();
    });

    job->start();
}

KEditTagsDialog::~KEditTagsDialog() = default;

QStringList KEditTagsDialog::tags() const
{
    return m_tags;
}

void KEditTagsDialog::slotAcceptedButtonClicked()
{
    m_tags.clear();

    for (const QTreeWidgetItem *item : std::as_const(m_allTagTreeItems)) {
        if (item->checkState(0) == Qt::Checked) {
            m_tags << qvariant_cast<QString>(item->data(0, Qt::UserRole));
        }
    }

    accept();
}

void KEditTagsDialog::slotItemActivated(const QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)

    const auto tag = qvariant_cast<QString>(item->data(0, Qt::UserRole));
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
    for (const QString &tag : std::as_const(m_tags)) {
        modifyTagWidget(tag);
    }

    for (const QString &tag : std::as_const(m_allTags)) {
        modifyTagWidget(tag);
    }

    m_tagTree->sortItems(0, Qt::SortOrder::AscendingOrder);
}

void KEditTagsDialog::modifyTagWidget(const QString &tag)
{
    const QStringList splitTag = tag.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    for (int i = 0; i < splitTag.size(); ++i) {
        QTreeWidgetItem *item;
        QString itemTag = tag.section(QLatin1Char('/'), 0, i, QString::SectionSkipEmpty);

        if (!m_allTagTreeItems.contains(itemTag)) {
            item = new QTreeWidgetItem();
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
