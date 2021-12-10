/*
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2010 Sebastian Trueg <trueg@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "tagcheckbox.h"
#include "tagwidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>

using namespace Baloo;

TagCheckBox::TagCheckBox(const QString &tag, QWidget *parent)
    : QWidget(parent)
    , m_label(nullptr)
    , m_tag(tag)
    , m_urlHover(false)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_label = new QLabel(tag.split(QLatin1Char('/'), Qt::SkipEmptyParts).last(), this);
    m_label->setToolTip(tag);
    m_label->setMouseTracking(true);
    m_label->setTextFormat(Qt::PlainText);
    m_label->setForegroundRole(parent->foregroundRole());
    m_child = m_label;

    m_child->installEventFilter(this);
    m_child->setMouseTracking(true);
    layout->addWidget(m_child);
}

TagCheckBox::~TagCheckBox()
{
}

void TagCheckBox::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    enableUrlHover(false);
}

bool TagCheckBox::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_child) {
        switch (event->type()) {
        case QEvent::MouseMove: {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            enableUrlHover(tagRect().contains(me->pos()));
            break;
        }

        case QEvent::MouseButtonRelease: {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            if (me->button() == Qt::LeftButton && tagRect().contains(me->pos())) {
                Q_EMIT tagClicked(m_tag);
                return true;
            }
            break;
        }

        default:
            // do nothing
            break;
        }
    }

    return QWidget::eventFilter(watched, event);
}

QRect TagCheckBox::tagRect() const
{
    return QRect(QPoint(0, 0), m_label->size());
}

void TagCheckBox::enableUrlHover(bool enable)
{
    if (m_urlHover != enable) {
        m_urlHover = enable;
        QFont f = font();
        if (enable)
            f.setUnderline(true);
        m_child->setFont(f);
        m_child->setCursor(enable ? Qt::PointingHandCursor : Qt::ArrowCursor);
    }
}
