/*
    SPDX-FileCopyrightText: 2006-2010 Sebastian Trueg <trueg@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _BALOO_TAG_WIDGET_P_H_
#define _BALOO_TAG_WIDGET_P_H_

#include "tagwidget.h"

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

    /// create clickable tag label
    void addTagLabel(const QString &tag);

    /// check the corresponding checkboxes and even
    /// add missing checkboxes
    void selectTags(const QStringList &tags);

    bool m_readOnly;

    QMap<QString, TagCheckBox *> m_tagLabels;
    QLabel *m_showAllLinkLabel;
    KBlockLayout *m_flowLayout;
    TagWidget *q;

    KEditTagsDialog *m_editTagsDialog;
};
}

#endif
