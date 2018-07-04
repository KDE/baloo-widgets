/* This file is part of the KDE libraries
   Copyright (C) 2014 Felix Eisele

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "keditcommentdialog.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QWidget>
#include <QLayout>
#include <QLabel>
#include <QTextEdit>

#include <KLocalizedString>

KEditCommentDialog::KEditCommentDialog(QWidget* parent, const QString& commentText, const QString& captionText)
    : QDialog(parent)
{
    setWindowTitle(captionText);

    QVBoxLayout* layout = new QVBoxLayout;
    setLayout(layout);

    m_editor = new QTextEdit(this);
    m_editor->setText(commentText);

    layout->addWidget(m_editor);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
    layout->addWidget(buttonBox);

    buttonBox->addButton(i18n("Save"), QDialogButtonBox::AcceptRole);
    buttonBox->addButton(QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    resize(sizeHint());
}

KEditCommentDialog::~KEditCommentDialog()
{
}

QString KEditCommentDialog::getCommentText() const
{
    return m_editor->toPlainText();
}
