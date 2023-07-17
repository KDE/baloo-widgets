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
    using TagWidget = Baloo::TagWidget;

    m_tagWidget = new TagWidget(this);
    auto lay = new QVBoxLayout(this);
    lay->addWidget(m_tagWidget);
    connect(m_tagWidget, &TagWidget::tagClicked, this, &TagWidgetTest::slotTagClicked);
    connect(m_tagWidget, &TagWidget::selectionChanged, this, &TagWidgetTest::slotSelectionChanged);

    auto box = new QCheckBox(QStringLiteral("Align Right"), this);
    connect(box, &QCheckBox::toggled, this, &TagWidgetTest::alignRight);
    lay->addWidget(box);

    box = new QCheckBox(QStringLiteral("Read only"), this);
    connect(box, &QCheckBox::toggled, m_tagWidget, &TagWidget::setReadyOnly);
    lay->addWidget(box);

    m_tagWidget->setSelectedTags({});
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

#include "moc_tagwidgettest.cpp"
