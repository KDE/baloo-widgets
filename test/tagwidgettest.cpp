/*
    SPDX-FileCopyrightText: 2010 Sebastian Trueg <trueg@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "tagwidgettest.h"

#include <QCheckBox>
#include <QDebug>
#include <QVBoxLayout>

TagWidgetTest::TagWidgetTest()
    : QWidget()
{
    m_tagWidget = new Baloo::TagWidget(this);
    auto lay = new QVBoxLayout(this);
    lay->addWidget(m_tagWidget);
    connect(m_tagWidget, SIGNAL(tagClicked(QString)), this, SLOT(slotTagClicked(QString)));
    connect(m_tagWidget, SIGNAL(selectionChanged(QStringList)), this, SLOT(slotSelectionChanged(QStringList)));

    auto box = new QCheckBox(QStringLiteral("Align Right"), this);
    connect(box, SIGNAL(toggled(bool)), this, SLOT(alignRight(bool)));
    lay->addWidget(box);

    box = new QCheckBox(QStringLiteral("Read only"), this);
    connect(box, SIGNAL(toggled(bool)), this, SLOT(setReadOnly(bool)));
    lay->addWidget(box);
}

TagWidgetTest::~TagWidgetTest() = default;

void TagWidgetTest::slotTagClicked(const QString &tag)
{
    qDebug() << "Tag clicked:" << tag;
}

void TagWidgetTest::slotSelectionChanged(const QStringList &tags)
{
    qDebug() << "Selection changed:" << tags;
}

void TagWidgetTest::alignRight(bool enable)
{
    if (enable)
        m_tagWidget->setAlignment(Qt::AlignRight);
    else
        m_tagWidget->setAlignment(Qt::AlignLeft);
}

void TagWidgetTest::setReadOnly(bool enable)
{
    m_tagWidget->setReadyOnly(enable);
}
