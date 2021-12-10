/*
    SPDX-FileCopyrightText: 2006-2007 Sebastian Trueg <trueg@kde.org>

    KBlockLayout is based on the FlowLayout example from QT4.
    SPDX-FileCopyrightText: 2004-2006 Trolltech ASA.

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KBLOCKLAYOUT_H
#define KBLOCKLAYOUT_H

#include <QLayout>
#include <QLayoutItem>

/**
 * The KBlockLayout arranges widget in rows and columns like a text
 * editor does.
 */
class KBlockLayout : public QLayout
{
public:
    explicit KBlockLayout(QWidget *parent, int margin = 0, int hSpacing = -1, int vSpacing = -1);
    explicit KBlockLayout(int margin = 0, int hSpacing = -1, int vSpacing = -1);
    ~KBlockLayout() override;

    /**
     * Set the alignment to use. It can be a combination of a horizontal and
     * a vertical alignment flag. The vertical flag is used to arrange widgets
     * that do not fill the complete height of a row.
     *
     * The default alignment is Qt::AlignLeft|Qt::AlignTop
     */
    void setAlignment(Qt::Alignment);
    Qt::Alignment alignment() const;

    int horizontalSpacing() const;
    int verticalSpacing() const;

    void setSpacing(int h, int v);

    void addItem(QLayoutItem *item) override;
    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int) const override;
    int count() const override;
    QLayoutItem *itemAt(int index) const override;
    QSize minimumSize() const override;
    void setGeometry(const QRect &rect) override;
    QSize sizeHint() const override;
    QLayoutItem *takeAt(int index) override;

private:
    int doLayout(const QRect &rect, bool testOnly) const;

    class Private;
    Private *const d;
};

#endif
