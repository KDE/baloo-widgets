/*
    SPDX-FileCopyrightText: 2010 Sebastian Trueg <trueg@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef TAGWIDGETTEST_H
#define TAGWIDGETTEST_H

#include "tagwidget.h"
#include <QWidget>

class TagWidgetTest : public QWidget
{
    Q_OBJECT

public:
    TagWidgetTest();
    ~TagWidgetTest() override;

public Q_SLOTS:
    void slotTagClicked(const QString &);
    void slotSelectionChanged(const QStringList &tags);

private Q_SLOTS:
    void alignRight(bool enable);
    void setReadOnly(bool enable);

private:
    Baloo::TagWidget *m_tagWidget;
};

#endif
