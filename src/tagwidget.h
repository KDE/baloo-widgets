/*
 * This file is part of the Baloo KDE project.
 * Copyright (C) 2006-2010 Sebastian Trueg <trueg@kde.org>
 * Copyright (C) 2013      Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _BALOO_TAG_WIDGET_H_
#define _BALOO_TAG_WIDGET_H_

#include "widgets_export.h"

#include <QWidget>

namespace Baloo
{
class TagWidgetPrivate;

/**
 * \class TagWidget tagwidget.h
 *
 * \brief Allows to change a selection of tags.
 *
 * TagWidget provides a simple GUI interface to assign tags.
 *
 * \author Sebastian Trueg <trueg@kde.org>
 */
class BALOO_WIDGETS_EXPORT TagWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    explicit TagWidget(QWidget *parent = nullptr);

    /**
     * Destructor
     */
    ~TagWidget() override;

    /**
     * The list of selected tags.
     *
     * \return The list of all tags that are currently selected. In case
     * resources to be tagged have been selected this list matches the
     * tags assigned to the resources.
     *
     * \since 4.5
     */
    QStringList selectedTags() const;

    /**
     * The alignment of the tags in the widget.
     *
     * \since 4.5
     */
    Qt::Alignment alignment() const;

    /**
     * If the widget is read only
     */
    bool readOnly() const;

Q_SIGNALS:
    /**
     * This signal is emitted whenever a tag is clicked.
     */
    void tagClicked(const QString &);

    /**
     * Emitted whenever the selection of tags changes.
     *
     * \since 4.5
     */
    void selectionChanged(const QStringList &tags);

public Q_SLOTS:
    /**
     * Set the list of selected tags. In case resources have been
     * set via setTaggedResource() or setTaggedResources() their
     * list of tags is changed automatically.
     *
     * \since 4.5
     */
    void setSelectedTags(const QStringList &tags);

    /**
     * Set the alignment to use. Only horizontal alignment flags make a
     * difference.
     *
     * \since 4.5
     */
    void setAlignment(Qt::Alignment alignment);

    /**
     * Set the TagWidget as read only
     */
    void setReadyOnly(bool readOnly = true);

private Q_SLOTS:
    void slotShowAll();
    void slotTagUpdateDone();
    void slotKEditTagDialogFinished(int result);

private:
    TagWidgetPrivate *const d;
};
}

#endif
