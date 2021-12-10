/*
    SPDX-FileCopyrightText: 2006-2010 Sebastian Trueg <trueg@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _BALOO_TAG_WIDGET_P_H_
#define _BALOO_TAG_WIDGET_P_H_

#include "tagwidget.h"

#include <QList>
#include <QMap>

class QLabel;
class KBlockLayout;
class KEditTagsDialog;

namespace Baloo
{
class TagCheckBox;

class TagWidgetPrivate
{
public:
    void init(TagWidget *parent);
    void rebuild();
    void buildTagHash(const QStringList &tags);

    /// lookup (and if necessary create) checkbox for tag
    TagCheckBox *getTagCheckBox(const QString &tag);

    /// check the corresponding checkboxes and even
    /// add missing checkboxes
    void selectTags(const QStringList &tags);

    bool m_readOnly;

    QMap<QString, TagCheckBox *> m_checkBoxHash;
    QLabel *m_showAllLinkLabel;
    KBlockLayout *m_flowLayout;
    TagWidget *q;

    KEditTagsDialog *m_editTagsDialog;
};
}

#endif
