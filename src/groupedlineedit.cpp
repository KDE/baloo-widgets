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

#include "groupedlineedit.h"

#include <QStyleOptionFrameV3>
#include <QFontMetrics>
#include <QApplication>
#include <QScrollBar>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextLayout>
#include <QTextLine>
#include <QPainter>
#include <QPainterPath>
#include <QBrush>
#include <QColor>
#include <QPalette>

using namespace Baloo;

struct GroupedLineEdit::Private
{
    struct Block {
        int start;
        int end;
    };

    QVector<Block> blocks;
    QBrush base;
};

GroupedLineEdit::GroupedLineEdit(QWidget* parent)
: QPlainTextEdit(parent),
  d(new Private)
{
    setWordWrapMode(QTextOption::NoWrap);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    document()->setMaximumBlockCount(1);

    // Use a transparent base, to make the rectangle visible
    QPalette pal = palette();

    d->base = pal.base();
    pal.setBrush(QPalette::Base, Qt::transparent);

    setPalette(pal);
}

GroupedLineEdit::~GroupedLineEdit()
{
    delete d;
}

QString GroupedLineEdit::text() const
{
    // Remove the block crosses from the text
    return toPlainText();
}

int GroupedLineEdit::cursorPosition() const
{
    return textCursor().positionInBlock();
}

void GroupedLineEdit::addBlock(int start, int end)
{
    Private::Block block;

    block.start = start;
    block.end = end;

    d->blocks.append(block);
    viewport()->update();
}

void GroupedLineEdit::setCursorPosition(int position)
{
    QTextCursor c = textCursor();

    c.setPosition(position, QTextCursor::MoveAnchor);

    setTextCursor(c);
}

void GroupedLineEdit::setText(const QString &text)
{
    setPlainText(text);
}

void GroupedLineEdit::clear()
{
    QPlainTextEdit::clear();
    removeAllBlocks();
}

void GroupedLineEdit::selectAll()
{
    QTextCursor c = textCursor();

    c.select(QTextCursor::LineUnderCursor);

    setTextCursor(c);
}

void GroupedLineEdit::removeAllBlocks()
{
    d->blocks.clear();
    viewport()->update();
}

QSize GroupedLineEdit::sizeHint() const
{
    QSize rs(
        40,
        document()->findBlock(0).layout()->lineAt(0).height() +
            document()->documentMargin() * 2 +
            frameWidth() * 2
    );

    return rs;
}

QSize GroupedLineEdit::minimumSizeHint() const
{
    return sizeHint();
}

void GroupedLineEdit::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        emit editingFinished();
        return;
    }

    QPlainTextEdit::keyPressEvent(e);
}

void GroupedLineEdit::paintEvent(QPaintEvent *e)
{
    static unsigned char colors[] = {
        0  , 87 , 174,
        243, 195, 0  ,
        0  , 179, 119,
        235, 115, 49 ,
        139, 179, 0  ,
        85 , 87 , 83 ,
        0  , 140, 0  ,
        117, 81 , 26
    };

    QTextLine line = document()->findBlock(0).layout()->lineAt(0);
    QPainter painter(viewport());
    int color_index = 0;

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::HighQualityAntialiasing, true);

    painter.fillRect(0, 0, viewport()->width(), viewport()->height(), d->base);

    Q_FOREACH(const Private::Block &block, d->blocks) {
        qreal start_x = line.cursorToX(block.start, QTextLine::Leading);
        qreal end_x = line.cursorToX(block.end + 1, QTextLine::Leading);
        QPainterPath path;
        QRectF rectangle(
            start_x - 1.0 - double(horizontalScrollBar()->value()),
            1.0,
            end_x - start_x + 2.0,
            double(viewport()->height() - 2)
        );

        unsigned char *c = colors + (color_index * 3);
        QColor color(c[0], c[1], c[2]);

        path.addRoundedRect(rectangle, 5.0, 5.0);
        painter.setPen(color);
        painter.setBrush(color.lighter(180));
        painter.drawPath(path);

        color_index = (color_index + 1) & 0x7; // Increment color_index, modulo 8 so that it does not overflow colors
    }

    QPlainTextEdit::paintEvent(e);
}

