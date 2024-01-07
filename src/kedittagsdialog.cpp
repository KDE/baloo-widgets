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
#include <QSortFilterProxyModel>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWidget>

#include <Baloo/TagListJob>

namespace
{
const int canonicalTagPathRole = Qt::UserRole + 1;
}

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

    m_model = new QStandardItemModel(this);

    m_treeView = new QTreeView(this);
    m_treeView->setSortingEnabled(true);
    m_treeView->setSelectionMode(QAbstractItemView::NoSelection);
    m_treeView->setHeaderHidden(true);
    m_treeView->setExpandsOnDoubleClick(true);
    m_treeView->setModel(m_model);
    connect(m_treeView, &QTreeView::clicked, this, &KEditTagsDialog::slotItemActivated);

    auto newTagLabel = new QLabel(i18nc("@label", "Create new tag:"));
    m_newTagEdit = new QLineEdit(this);
    m_newTagEdit->setClearButtonEnabled(true);
    m_newTagEdit->setFocus();
    connect(m_newTagEdit, &QLineEdit::textEdited, this, &KEditTagsDialog::slotTextEdited);

    auto newTagLayout = new QHBoxLayout();
    newTagLayout->addWidget(newTagLabel);
    newTagLayout->addWidget(m_newTagEdit, 1);

    topLayout->addWidget(label);
    topLayout->addWidget(m_treeView);
    topLayout->addLayout(newTagLayout);
    topLayout->addWidget(buttonBox);

    resize(sizeHint());

    auto job = new Baloo::TagListJob();
    connect(job, &Baloo::TagListJob::finished, this, [this](KJob *job) {
        auto tjob = static_cast<Baloo::TagListJob *>(job);
        m_allTags = tjob->tags();
        setupModel(m_allTags, m_tags);
    });

    job->start();
}

void KEditTagsDialog::setupModel(const QStringList &allTags, const QStringList &selectedTags)
{
    for (const auto &tag : allTags) {
        ensureItemForTagExists(tag);
    }

    for (const auto &tag : selectedTags) {
        auto currentItem = ensureItemForTagExists(tag);
        currentItem->setCheckState(Qt::Checked);
    }

    m_treeView->expandAll();
}

QStandardItem *KEditTagsDialog::addTag(QStandardItem *parentItem, const QString &cannonicalTagPath, const QString &tagName)
{
    auto newItem = new QStandardItem(tagName);
    newItem->setIcon(QIcon::fromTheme(QLatin1String("tag")));
    newItem->setCheckable(true);
    newItem->setData(cannonicalTagPath, canonicalTagPathRole);
    parentItem->appendRow(newItem);
    return newItem;
}

QStandardItem *KEditTagsDialog::findSubItem(QString currentName, QStandardItem *parentItem)
{
    for (int i = 0; i < parentItem->rowCount(); ++i) {
        auto child = parentItem->child(i);
        if (child->text() == currentName) {
            return child;
        }
    }
    return nullptr;
}

QStandardItem *KEditTagsDialog::findTag(const QString tag)
{
    QStandardItem *subItem = m_model->invisibleRootItem();
    const QStringList splitTags = tag.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    for (int i = 0; i < splitTags.size() && subItem != nullptr; ++i) {
        auto currentName = splitTags[i];
        subItem = findSubItem(currentName, subItem);
    }

    return subItem;
}

QStandardItem *KEditTagsDialog::ensureItemForTagExists(const QString tag)
{
    QStandardItem *parentItem = m_model->invisibleRootItem();
    QStandardItem *currentItem = nullptr;
    const QStringList splitTags = tag.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    for (int i = 0; i < splitTags.size(); ++i) {
        auto currentName = splitTags[i];

        currentItem = findSubItem(currentName, parentItem);
        if (currentItem == nullptr) {
            QString tagPath = QLatin1Char('/') + splitTags.mid(0, i + 1).join(QLatin1Char('/'));
            currentItem = addTag(parentItem, tagPath, currentName);
        }

        parentItem = currentItem;
    }

    return currentItem;
}

KEditTagsDialog::~KEditTagsDialog() = default;

QStringList KEditTagsDialog::tags() const
{
    return m_tags;
}

void KEditTagsDialog::slotAcceptedButtonClicked()
{
    m_tags.clear();

    std::function<void(QStandardItem *)> recurseInTree;
    recurseInTree = [this, &recurseInTree](QStandardItem *item) {
        QString canonicalPath = item->data(canonicalTagPathRole).toString();
        if (item->checkState() == Qt::Checked) {
            m_tags << canonicalPath;
        }

        for (int i = 0; i < item->rowCount(); ++i) {
            recurseInTree(item->child(i));
        }
    };
    recurseInTree(m_model->invisibleRootItem());

    accept();
}

void KEditTagsDialog::slotItemActivated(const QModelIndex &index)
{
    const auto tag = m_treeView->model()->data(index, canonicalTagPathRole).toString();
    m_newTagEdit->setText(tag + QLatin1Char('/'));
    m_newTagEdit->setFocus();
}

void KEditTagsDialog::slotTextEdited(const QString &text)
{
    // Remove unnecessary spaces from a new tag is
    // mandatory, as the user cannot see the difference
    // between a tag "Test" and "Test ".
    QString tagText = text.simplified();
    while (tagText.endsWith(QLatin1Char('/'))) {
        tagText.chop(1);
    }

    while(m_newItem != nullptr) {
        auto parent = m_newItem->parent();
        if (!parent) {
            parent = m_model->invisibleRootItem();
        }
        parent->removeRow(m_newItem->row());
        QString canonicalPath = parent->data(canonicalTagPathRole).toString();
        if (m_allTags.contains(canonicalPath) || m_tags.contains(canonicalPath) || parent == m_model->invisibleRootItem()) {
            m_newItem = nullptr;
        } else {
            m_newItem = parent;
        }
    }

    if (tagText.isEmpty()) {
        m_treeView->setModel(m_model);
        m_treeView->expandAll();
        return;
    }

    if (!findTag(text)) {
        m_newItem = ensureItemForTagExists(text);
        m_newItem->setIcon(QIcon::fromTheme(QStringLiteral("tag-new")));
        m_newItem->setCheckState(Qt::Checked);
    }

    m_treeView->expandAll();
}

#include "moc_kedittagsdialog_p.cpp"
