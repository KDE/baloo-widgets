/*
    SPDX-FileCopyrightText: 2006-2010 Sebastian Trueg <trueg@kde.org>
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
