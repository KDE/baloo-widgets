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

#include "kedittagsdialog_p.h"

K_PLUGIN_FACTORY_WITH_JSON(TagsFileItemActionFactory, "tagsfileitemaction.json", registerPlugin<TagsFileItemAction>();)

TagsFileItemAction::TagsFileItemAction(QObject *parent, const QVariantList &)
    : KAbstractFileItemActionPlugin(parent)
    , m_tagsLister()
{
}

TagsFileItemAction::~TagsFileItemAction()
{
    delete m_metaData;
}

QList<QAction *> TagsFileItemAction::actions(const KFileItemListProperties &fileItemInfos, QWidget *parentWidget)
{
    if (fileItemInfos.urlList().size() > 1) {
        return {};
    }

    const QString filePath = fileItemInfos.urlList().constFirst().toLocalFile();
    if (!QFileInfo(filePath).isWritable()) {
        return {};
    }

    m_metaData = new KFileMetaData::UserMetaData(filePath);
    if (!m_metaData->isSupported()) {
        return {};
    }

    QMenu *menu = new QMenu(i18n("Assign Tags"), parentWidget);
    menu->setIcon(QIcon::fromTheme(QStringLiteral("tag")));

    connect(&m_tagsLister, &KCoreDirLister::itemsAdded, this, [this, menu](const QUrl &, const KFileItemList &items) {
        const QStringList fileTags = m_metaData->tags();

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

        for (const QString &name : std::as_const(allTags)) {
            QAction *action = menu->addAction(QIcon::fromTheme(QStringLiteral("tag")), name);
            action->setCheckable(true);
            action->setChecked(fileTags.contains(name));

            connect(action, &QAction::triggered, this, [this, name](bool isChecked) {
                QStringList newTags = m_metaData->tags();
                if (isChecked) {
                    newTags.append(name);
                } else {
                    newTags.removeAll(name);
                }
                m_metaData->setTags(newTags);
            });
        }
    });

    QAction *newAction = new QAction(i18n("Edit tags..."), menu);
    newAction->setIcon(QIcon::fromTheme(QStringLiteral("tag-new")));

    connect(newAction, &QAction::triggered, this, [this, parentWidget] {
        // Use KEditTagsDialog so the user can create and select hierarchical tags
        KEditTagsDialog *editDialog = new KEditTagsDialog(m_metaData->tags(), parentWidget);
        editDialog->setWindowModality(Qt::WindowModal);
        connect(editDialog, &QDialog::finished, this, [this, editDialog](int result) {
            if (result == QDialog::Accepted) {
                m_metaData->setTags(editDialog->tags());
            }
            editDialog->deleteLater();
        });
        editDialog->open();
    });

    m_tagsLister.openUrl(QUrl(QStringLiteral("tags:/")), KCoreDirLister::OpenUrlFlag::Reload);
    menu->addAction(newAction);
    menu->addSeparator();
    menu->setParent(parentWidget, Qt::Popup);

    connect(menu, &QObject::destroyed, this, [this]() {
        disconnect(&m_tagsLister, &KCoreDirLister::itemsAdded, this, nullptr);
        m_tagsLister.stop();
    });

    return {menu->menuAction()};
}

#include "tagsfileitemaction.moc"

#include "moc_tagsfileitemaction.cpp"
