/*****************************************************************************
 * Copyright (C) 2008 by Sebastian Trueg <trueg@kde.org>                     *
 * Copyright (C) 2009 by Peter Penz <peter.penz@gmx.at>                      *
 *                                                                           *
 * This library is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Library General Public               *
 * License as published by the Free Software Foundation; either              *
 * version 2 of the License, or (at your option) any later version.          *
 *                                                                           *
 * This library is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Library General Public License for more details.                          *
 *                                                                           *
 * You should have received a copy of the GNU Library General Public License *
 * along with this library; see the file COPYING.LIB.  If not, write to      *
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 * Boston, MA 02110-1301, USA.                                               *
 *****************************************************************************/

#include "kcommentwidget_p.h"
#include "keditcommentdialog.h"

#include <KLocalizedString>
#include <KConfigGui/KWindowConfig>
#include <KSharedConfig>

#include <QEvent>
#include <QPointer>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>

KCommentWidget::KCommentWidget(QWidget* parent) :
    QWidget(parent),
    m_readOnly(false),
    m_label(nullptr),
    m_sizeHintHelper(nullptr),
    m_comment()
{
    m_label = new QLabel(this);
    m_label->setWordWrap(true);
    m_label->setAlignment(Qt::AlignTop);
    connect(m_label, &QLabel::linkActivated, this, &KCommentWidget::slotLinkActivated);

    m_sizeHintHelper = new QLabel(this);
    m_sizeHintHelper->hide();

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_label);

    setText(m_comment);
}

KCommentWidget::~KCommentWidget()
{
}

void KCommentWidget::setText(const QString& comment)
{
    QString text;
    if (comment.isEmpty()) {
        if (m_readOnly) {
            text = QStringLiteral("-");
        } else {
            text = QStringLiteral("<a href=\"addComment\">") + i18nc("@label", "Add...") + QStringLiteral("</a>");
        }
    } else {
        if (m_readOnly) {
            text = comment.toHtmlEscaped();
        } else {
            text = QStringLiteral("<p>") + comment.toHtmlEscaped() + QStringLiteral(" <a href=\"editComment\">") + i18nc("@label", "Edit...") + QStringLiteral("</a></p>");
        }
    }

    m_label->setText(text);
    m_sizeHintHelper->setText(text);
    m_comment = comment;
}

QString KCommentWidget::text() const
{
    return m_comment;
}

void KCommentWidget::setReadOnly(bool readOnly)
{
    m_readOnly = readOnly;
    setText(m_comment);
}

bool KCommentWidget::isReadOnly() const
{
    return m_readOnly;
}

QSize KCommentWidget::sizeHint() const
{
    // Per default QLabel tries to provide a square size hint. This
    // does not work well for complex layouts that rely on a heightForWidth()
    // functionality with unclipped content. Use an unwrapped text label
    // as layout helper instead, that returns the preferred size of
    // the rich-text line.
    return m_sizeHintHelper->sizeHint();
}

bool KCommentWidget::event(QEvent* event)
{
    if (event->type() == QEvent::Polish) {
        m_label->setForegroundRole(foregroundRole());
    }
    return QWidget::event(event);
}

void KCommentWidget::slotLinkActivated(const QString& link)
{
    const QString caption = (link == QLatin1String("editComment")) ?
                            i18nc("@title:window", "Edit Comment") :
                            i18nc("@title:window", "Add Comment");

    QPointer<KEditCommentDialog> dialog = new KEditCommentDialog(this, m_comment, caption);

    KConfigGroup dialogConfig(KSharedConfig::openConfig(), "Baloo KEditCommentDialog");
    KWindowConfig::restoreWindowSize(dialog->windowHandle(), dialogConfig);

    if (dialog->exec() == QDialog::Accepted) {
        const QString oldText = m_comment;
        if (dialog != nullptr) {
            setText(dialog->getCommentText());
        }
        if (oldText != m_comment) {
            Q_EMIT commentChanged(m_comment);
        }
    }

    if (dialog != nullptr) {
        KWindowConfig::saveWindowSize(dialog->windowHandle(), dialogConfig);
        delete dialog;
        dialog = nullptr;
    }
}
