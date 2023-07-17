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
    : QLabel(parent)
    , m_tag(tag)
{
    auto lastSegment = tag.split(QLatin1Char('/'), Qt::SkipEmptyParts).last();
    setText(lastSegment);

    setToolTip(tag);
    setTextFormat(Qt::PlainText);
    setForegroundRole(parent->foregroundRole());

    installEventFilter(this);
}

TagCheckBox::~TagCheckBox() = default;

bool TagCheckBox::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this) {
        switch (event->type()) {
        case QEvent::Enter: {
            enableUrlHover(true);
            break;
        }
        case QEvent::Leave: {
            enableUrlHover(false);
            break;
        }
        case QEvent::MouseMove: {
            auto me = static_cast<QMouseEvent *>(event);
            enableUrlHover(rect().contains(me->pos()));
            break;
        }

        case QEvent::MouseButtonRelease: {
            auto me = static_cast<QMouseEvent *>(event);
            if (me->button() == Qt::LeftButton && rect().contains(me->pos())) {
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

void TagCheckBox::enableUrlHover(bool enable)
{
    if (m_urlHover != enable) {
        m_urlHover = enable;
        QFont f = font();
        f.setUnderline(enable);
        setFont(f);
        setCursor(enable ? Qt::PointingHandCursor : Qt::ArrowCursor);
    }
}

#include "moc_tagcheckbox.cpp"
