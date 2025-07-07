/*
    SPDX-FileCopyrightText: 2008 Sebastian Trueg <trueg@kde.org>
    SPDX-FileCopyrightText: 2009 Peter Penz <peter.penz@gmx.at>
    SPDX-FileCopyrightText: 2025 Felix Ernst <felixernst@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcommentwidget_p.h"
#include "widgetsdebug.h"

#include <KLocalizedString>

#include <QEvent>
#include <QFontMetrics>
#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QPointer>
#include <QTimer>
#include <QToolButton>

namespace
{
    void setHeightToFitContent(QPlainTextEdit *plainTextEdit)
    {
        // `plainTextEdit->document()->size()` is reported wrongly until it is shown the first time, so we delay the height change.
        QTimer::singleShot(1, plainTextEdit, [plainTextEdit](){
            const int numberOfLinesToShow(plainTextEdit->document()->size().height() + 1);
            plainTextEdit->setFixedHeight(numberOfLinesToShow * QFontMetrics(plainTextEdit->font()).lineSpacing() + plainTextEdit->contentsMargins().top() + plainTextEdit->contentsMargins().bottom());
        });
    };
}

KCommentWidget::KCommentWidget(QWidget *parent)
    : QWidget(parent)
    , m_plainTextEdit{new QPlainTextEdit(this)}
    , m_saveButton{new QToolButton{this}}
{
    setFocusProxy(m_plainTextEdit);
    m_plainTextEdit->setAccessibleName(i18nc("accessible name for file metadata comment text box", "Comment"));
    m_plainTextEdit->setTabChangesFocus(true);
    m_plainTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_plainTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(m_plainTextEdit, &QPlainTextEdit::textChanged, this, [this](){
        const bool canSave{!m_plainTextEdit->isReadOnly() && m_plainTextEdit->toPlainText() != m_comment};
        m_saveButton->setEnabled(canSave);
        if (canSave) {
            m_saveButton->setVisible(true);
        }
        setHeightToFitContent(m_plainTextEdit);
    });

    m_saveButton->setText(i18nc("@action:button file metadata comment", "Save Comment"));
    m_saveButton->hide();
    connect(m_saveButton, &QAbstractButton::clicked, this, [this](){
        m_comment = m_plainTextEdit->toPlainText();
        Q_EMIT commentChanged(m_plainTextEdit->toPlainText());
        m_saveButton->setDisabled(true);
    });

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_plainTextEdit);
    layout->addWidget(m_saveButton);

    setText(m_comment);
}

KCommentWidget::~KCommentWidget() = default;

void KCommentWidget::setText(const QString &comment)
{
    m_comment = comment;
    m_plainTextEdit->setPlainText(comment);
    setHeightToFitContent(m_plainTextEdit);
    m_saveButton->hide();
}

QString KCommentWidget::text() const
{
    return m_comment;
}

void KCommentWidget::setReadOnly(bool readOnly)
{
    m_plainTextEdit->setReadOnly(readOnly);
    setText(m_comment);
}

bool KCommentWidget::isReadOnly() const
{
    return m_plainTextEdit->isReadOnly();
}

bool KCommentWidget::event(QEvent *event)
{
    if (event->type() == QEvent::Show) {
        setHeightToFitContent(m_plainTextEdit);
    }
    return QWidget::event(event);
}

#include "moc_kcommentwidget_p.cpp"
