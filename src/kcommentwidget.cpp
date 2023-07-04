/*
    SPDX-FileCopyrightText: 2008 Sebastian Trueg <trueg@kde.org>
    SPDX-FileCopyrightText: 2009 Peter Penz <peter.penz@gmx.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcommentwidget_p.h"
#include "keditcommentdialog.h"
#include "widgetsdebug.h"

#include <KLocalizedString>
#include <KSharedConfig>
#include <KWindowConfig>

#include <QEvent>
#include <QLabel>
#include <QPointer>
#include <QVBoxLayout>

KCommentWidget::KCommentWidget(QWidget *parent)
    : QWidget(parent)
    , m_label(new QLabel(this))
    , m_sizeHintHelper(new QLabel(this))
{
    m_label->setWordWrap(true);
    m_label->setAlignment(Qt::AlignTop);
    connect(m_label, &QLabel::linkActivated, this, &KCommentWidget::slotLinkActivated);

    m_sizeHintHelper->hide();

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_label);

    setText(m_comment);
}

KCommentWidget::~KCommentWidget() = default;

void KCommentWidget::setText(const QString &comment)
{
    QString content;
    if (comment.isEmpty()) {
        if (m_readOnly) {
            content = QStringLiteral("-");
        } else {
            content = QStringLiteral("<a href=\"addComment\">%1</a>").arg(i18nc("@label", "Add..."));
        }
    } else {
        if (m_readOnly) {
            content = comment.toHtmlEscaped();
        } else {
            content = QStringLiteral("<p>%1 <a href=\"editComment\">%2</a></p>").arg(comment.toHtmlEscaped(), i18nc("@label", "Edit..."));
        }
    }

    m_label->setText(content);
    m_sizeHintHelper->setText(content);
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

bool KCommentWidget::event(QEvent *event)
{
    if (event->type() == QEvent::Polish) {
        m_label->setForegroundRole(foregroundRole());
    }
    return QWidget::event(event);
}

void KCommentWidget::slotLinkActivated(const QString &link)
{
    const QString caption = (link == QLatin1String("editComment")) ? i18nc("@title:window", "Edit Comment") : i18nc("@title:window", "Add Comment");

    QPointer<KEditCommentDialog> dialog = new KEditCommentDialog(this, m_comment, caption);

    KConfigGroup dialogConfig(KSharedConfig::openConfig(), "Baloo KEditCommentDialog");
    KWindowConfig::restoreWindowSize(dialog->windowHandle(), dialogConfig);

    dialog->exec();
    if (dialog.isNull()) {
        qCWarning(Baloo::WIDGETS) << "Comment dialog destroyed while running";
        Q_ASSERT(!dialog.isNull());
        return;
    }

    if (dialog->result() == QDialog::Accepted) {
        const QString oldText = m_comment;
        setText(dialog->getCommentText());

        if (oldText != m_comment) {
            Q_EMIT commentChanged(m_comment);
        }
    }

    KWindowConfig::saveWindowSize(dialog->windowHandle(), dialogConfig);
    delete dialog;
}

#include "moc_kcommentwidget_p.cpp"
