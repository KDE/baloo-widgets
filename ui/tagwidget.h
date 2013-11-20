/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2010 Sebastian Trueg <trueg@kde.org>
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

#include <QtGui/QWidget>
#include <baloo/tag.h>

namespace Baloo {
    class Tag;
    class TagWidgetPrivate;

    /**
     * \class TagWidget tagwidget.h
     *
     * \brief Allows to change a selection of tags.
     *
     * TagWidget provides a simple GUI interface to assign tags.
     * It has two basic modes:
     * \li If resources are set via setTaggedResource() or setTaggedResources()
     * the changes in the tag selection are automatically assigned to the
     * selected resources.
     * \li If no resources have been set the widget simply emits the selectionChanged()
     * signal.
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
        explicit TagWidget(QWidget* parent = 0);

        /**
         * Destructor
         */
        ~TagWidget();

        /**
         * The list of selected tags.
         *
         * \return The list of all tags that are currently selected. In case
         * resources to be tagged have been selected this list matches the
         * tags assigned to the resources.
         *
         * \sa setTaggedResource, taggedResource, Resource::getTags
         *
         * \since 4.5
         */
        QList<Tag> selectedTags() const;

        /**
         * The alignment of the tags in the widget.
         *
         * \since 4.5
         */
        Qt::Alignment alignment() const;

        /**
         * Flags to configure the widget.
         *
         * \since 4.5
         */
        enum ModeFlag {
            /**
             * Read only mode which prevents the changing
             * of tags by the user.
             */
            ReadOnly = 0x1,

            /**
             * Disable the clicking of the tags. This will
             * also disable the emitting of the tagClicked()
             * signal.
             */
            DisableTagClicking = 0x2
        };
        Q_DECLARE_FLAGS(ModeFlags, ModeFlag)

        /**
         * Flags the widget is configured with.
         *
         * \sa setModeFlags()
         *
         * \since 4.5
         */
        ModeFlags modeFlags() const;

    Q_SIGNALS:
        /**
         * This signal is emitted whenever a tag is clicked.
         */
        void tagClicked(const Baloo::Tag&);

        /**
         * Emitted whenever the selection of tags changes.
         *
         * \since 4.5
         */
        void selectionChanged(const QList<Baloo::Tag>& tags);

    public Q_SLOTS:
        /**
         * Set the list of selected tags. In case resources have been
         * set via setTaggedResource() or setTaggedResources() their
         * list of tags is changed automatically.
         *
         * \since 4.5
         */
        void setSelectedTags( const QList<Tag>& tags );

        /**
         * Set the alignment to use. Only horizontal alignment flags make a
         * difference.
         *
         * \since 4.5
         */
        void setAlignment( Qt::Alignment alignment );

        /**
         * Set flags to change the behaviour and look of the tag widget.
         *
         * \since 4.5
         */
        void setModeFlags( ModeFlags flags );

    private Q_SLOTS:
        void slotShowAll();
        void slotTagUpdateDone();
        void slotKEditTagDialogFinished( int result );

    private:
        TagWidgetPrivate* const d;
    };
}

Q_DECLARE_OPERATORS_FOR_FLAGS( Baloo::TagWidget::ModeFlags )

#endif
