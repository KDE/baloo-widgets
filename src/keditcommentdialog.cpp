/*
    SPDX-FileCopyrightText: 2014 Felix Eisele

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "keditcommentdialog.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>

#include <KLocalizedString>

KEditCommentDialog::KEditCommentDialog(QWidget *parent, const QString &commentText, const QString &captionText)
    : QDialog(parent)
    , m_editor(new QTextEdit(this))
{
    setWindowTitle(captionText);

    auto layout = new QVBoxLayout(this);

    m_editor->setText(commentText);

    layout->addWidget(m_editor);

    auto buttonBox = new QDialogButtonBox(this);
    layout->addWidget(buttonBox);

    buttonBox->addButton(i18n("Save"), QDialogButtonBox::AcceptRole);
    buttonBox->addButton(QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    resize(sizeHint());
}

KEditCommentDialog::~KEditCommentDialog() = default;

QString KEditCommentDialog::getCommentText() const
{
    return m_editor->toPlainText();
}

#include "moc_keditcommentdialog.cpp"
