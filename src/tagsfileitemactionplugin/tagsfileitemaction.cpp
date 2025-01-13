/*
    SPDX-FileCopyrightText: 2018 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "tagsfileitemaction.h"

#include <QAction>
#include <QFileInfo>
#include <QIcon>
#include <QInputDialog>
#include <QList>
#include <QMenu>
#include <QUrl>
#include <QVariantList>
#include <QWidget>

#include <KLocalizedString>
#include <KPluginFactory>

K_PLUGIN_FACTORY_WITH_JSON(TagsFileItemActionFactory, "tagsfileitemaction.json", registerPlugin<TagsFileItemAction>();)

TagsFileItemAction::TagsFileItemAction(QObject *parent, const QVariantList &)
    : KAbstractFileItemActionPlugin(parent)
    , m_tagsLister()
{
    m_menu = new QMenu(i18n("Manage Tags"));
    m_menu->setIcon(QIcon::fromTheme(QStringLiteral("tag")));

    connect(&m_tagsLister, &KCoreDirLister::itemsAdded, this, [this](const QUrl &, const KFileItemList &items) {
        if (m_metaDataList.count() == 1) {
            manageTagsForSingleFile(items);
        } else {
            manageTagsForMultipleFiles(items);
        }
    });

    newAction = new QAction(i18n("Create New..."));
    newAction->setIcon(QIcon::fromTheme(QStringLiteral("tag-new")));

    connect(newAction, &QAction::triggered, this, [this] {
        QString newTag = QInputDialog::getText(m_menu, i18n("New tag"), i18n("New tag:"), QLineEdit::Normal);
        for (auto metaData : std::as_const(m_metaDataList)) {
            QStringList tags = metaData.tags();
            if (!tags.contains(newTag)) {
                tags.append(newTag);
                metaData.setTags(tags);
            }
        }
    });
}

TagsFileItemAction::~TagsFileItemAction()
{
    m_metaDataList.clear();
}

QList<QAction *> TagsFileItemAction::actions(const KFileItemListProperties &fileItemInfos, QWidget *parentWidget)
{
    m_metaDataList.clear();
    const auto urls = fileItemInfos.urlList();
    for (const auto &url : urls) {
        const QString filePath = url.toLocalFile();
        if (!QFileInfo(filePath).isWritable()) {
            continue;
        }

        KFileMetaData::UserMetaData metadata(filePath);
        if (!metadata.isSupported()) {
            continue;
        }
        m_metaDataList.append(metadata);
    }

    if (m_metaDataList.isEmpty()) {
        return {};
    }

    m_tagsLister.openUrl(QUrl(QStringLiteral("tags:/")), KCoreDirLister::OpenUrlFlag::Reload);

    m_menu->clear();
    m_menu->addAction(newAction);
    m_menu->addSeparator();
    m_menu->setParent(parentWidget, Qt::Popup);

    return {m_menu->menuAction()};
}

void TagsFileItemAction::manageTagsForSingleFile(const KFileItemList &items)
{
    const QStringList fileTags = m_metaDataList.constFirst().tags();

    // The file may be located outside an indexed path, or is not indexed yet
    // Show the complete tag list, i.e. the union of file and index DB tags
    QStringList allTags;
    allTags.reserve(fileTags.size() + items.size());
    allTags.append(fileTags);
    for (const KFileItem &item : items) {
        allTags.append(item.name());
    }
    allTags.sort(Qt::CaseInsensitive);
    allTags.removeDuplicates();

    auto icon = QIcon::fromTheme(QStringLiteral("tag"));
    for (const QString &name : std::as_const(allTags)) {
        QAction *action = m_menu->addAction(icon, name);
        action->setCheckable(true);
        action->setChecked(fileTags.contains(name));

        connect(action, &QAction::triggered, this, [this, name](bool isChecked) {
            QStringList newTags = m_metaDataList.first().tags();
            if (isChecked) {
                newTags.append(name);
            } else {
                newTags.removeAll(name);
            }
            m_metaDataList.first().setTags(newTags);
        });
    }
}

void TagsFileItemAction::manageTagsForMultipleFiles(const KFileItemList &items)
{
    // tags common between the selected files
    QSet<QString> commonTags;
    // all tags of the selected files
    QSet<QString> filesTags;
    int i{0};
    for (const auto &metaData : std::as_const(m_metaDataList)) {
        QSet<QString> currentFileTags;
        for (const auto &tag : metaData.tags()) {
            filesTags.insert(tag);
            currentFileTags.insert(tag);
        }
        if (i == 0) {
            commonTags = currentFileTags;
        }
        commonTags = commonTags.intersect(currentFileTags);
        ++i;
    }

    // The file may be located outside an indexed path, or is not indexed yet
    // Show the complete tag list, i.e. the union of files and index DB tags
    QStringList allTags;
    allTags.reserve(filesTags.size() + items.size());
    allTags.append(filesTags.values());
    for (const KFileItem &item : items) {
        allTags.append(item.name());
    }
    allTags.sort(Qt::CaseInsensitive);
    allTags.removeDuplicates();

    // create submenu for adding tags
    auto addTagsMenu = new QMenu(QStringLiteral("Add"), m_menu);
    auto icon = QIcon::fromTheme(QStringLiteral("list-add"));
    for (const QString &name : std::as_const(allTags)) {
        if (commonTags.contains(name)) {
            continue;
        }
        QAction *action = addTagsMenu->addAction(icon, name);

        connect(action, &QAction::triggered, this, [this, name]() {
            // add tag to all selected files/folders
            for (auto metaData : std::as_const(m_metaDataList)) {
                QStringList tags = metaData.tags();
                if (!tags.contains(name)) {
                    tags.append(name);
                    metaData.setTags(tags);
                }
            }
        });
    }
    addTagsMenu->setEnabled(!addTagsMenu->isEmpty());
    m_menu->addMenu(addTagsMenu);

    // create submenu for removing tags
    auto removeTagsMenu = new QMenu(QStringLiteral("Remove"), m_menu);
    icon = QIcon::fromTheme(QStringLiteral("list-remove"));
    removeTagsMenu->setEnabled(!filesTags.isEmpty());
    for (const QString &name : std::as_const(filesTags)) {
        QAction *action = removeTagsMenu->addAction(icon, name);

        connect(action, &QAction::triggered, this, [this, name]() {
            // remove tag from all selected files/folders
            for (auto metaData : std::as_const(m_metaDataList)) {
                QStringList tags = metaData.tags();
                if (tags.contains(name)) {
                    tags.removeAll(name);
                    metaData.setTags(tags);
                }
            }
        });
    }
    m_menu->addMenu(removeTagsMenu);
}

#include "tagsfileitemaction.moc"

#include "moc_tagsfileitemaction.cpp"
