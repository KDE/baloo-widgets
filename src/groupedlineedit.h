/* This file is part of the Baloo widgets collection
   Copyright (c) 2013 Denis Steckelmacher <steckdenis@yahoo.fr>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2.1 as published by the Free Software Foundation,
   or any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __GROUPEDLINEEDIT_H__
#define __GROUPEDLINEEDIT_H__

#include <QtGui/QPlainTextEdit>

#include "widgets_export.h"

namespace Baloo {

/**
 * \class GroupedLineEdit groupedlineedit.h baloo/groupedlineedit.h
 * \brief Single-line text editor with grouped terms support
 *
 * This widget looks like a QLineEdit, but terms can be grouped in it. A group
 * consists of terms drawn inside a rounded rectangle. Rectangles have different
 * colors, that allow the user to recognize parts of the input. They can be
 * nested.
 *
 * This widget can be used to show how a line can be tokenized or parsed. For
 * instance, the QueryBuilder widget uses this widget to show the filters of a
 * query, but a mail client can also use the control to highlight e-mail
 * addresses.
 */
class BALOO_WIDGETS_EXPORT GroupedLineEdit : public QPlainTextEdit
{
    Q_OBJECT

    public:
        explicit GroupedLineEdit(QWidget *parent = 0);
        virtual ~GroupedLineEdit();

        /**
         * Text displayed in this control. Use this method instead of
         * toPlainText() as it is garanteed that this method will return exactly
         * what the user sees.
         */
        QString text() const;

        /**
         * Current cursor position. The cursor position starts at 0 when the
         * cursor is completely at the left of the control (for right-to-left
         * languages) and goes to text().length() when the cursor is completely
         * at the right of the control.
         */
        int cursorPosition() const;

        /**
         * Set the cursor position
         *
         * \sa cursorPosition
         */
        void setCursorPosition(int position);

        /**
         * Set the text dislayed by the control. Calling this method removes all
         * the blocks.
         *
         * \sa text
         * \sa removeAllBlocks
         */
        void setText(const QString &text);

        /**
         * Clear the text and the blocks contained in the widget.
         */
        void clear();

        /**
         * Select all the text present in the control.
         */
        void selectAll();

        /**
         * Remove all the blocks that were present in the control. The text is
         * not changed, but no block will be drawn any more.
         */
        void removeAllBlocks();

        /**
         * Add a block to the control. A block represents a group and is
         * identified by the indexes of the first and the last characters in it.
         * These limits are inclusive.
         *
         * The control maintains a list of block colors, that are cycled through
         * at each block insertion. If you want the blocks to always have the
         * same color even if they are removed and then re-added, always add
         * them in the same order.
         */
        void addBlock(int start, int end);

        virtual QSize sizeHint() const;
        virtual QSize minimumSizeHint() const;

    signals:
        /**
         * Signal triggered when the user presses Enter or Return in the control
         */
        void editingFinished();

    protected:
        virtual void paintEvent(QPaintEvent *e);
        virtual void keyPressEvent(QKeyEvent *e);

    private:
        struct Private;
        Private *d;
};

}

#endif