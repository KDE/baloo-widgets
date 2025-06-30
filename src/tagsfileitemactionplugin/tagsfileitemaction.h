/*
    SPDX-FileCopyrightText: 2018 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef TAGSFILEITEMACTION_H
#define TAGSFILEITEMACTION_H

#include <KAbstractFileItemActionPlugin>
#include <KCoreDirLister>
#include <KFileItemListProperties>
#include <KFileMetaData/UserMetaData>

class QAction;
class QWidget;

class TagsFileItemAction : public KAbstractFileItemActionPlugin
{
    Q_OBJECT
public:
    TagsFileItemAction(QObject *parent, const QVariantList &args);
    ~TagsFileItemAction() override;
    QList<QAction *> actions(const KFileItemListProperties &fileItemInfos, QWidget *parentWidget) override;

private:
    KFileMetaData::UserMetaData *m_metaData = nullptr;
    KCoreDirLister m_tagsLister;
};

#endif // TAGSFILEITEMACTION_H
