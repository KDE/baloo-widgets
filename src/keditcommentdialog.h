/*
    SPDX-FileCopyrightText: 2014 Felix Eisele

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KEDITCOMMENTDIALOG_H
#define KEDITCOMMENTDIALOG_H

#include <QDialog>

class QWidget;
class QTextEdit;

class KEditCommentDialog : public QDialog
{
    Q_OBJECT
public:
    KEditCommentDialog(QWidget *parent, const QString &commentText, const QString &captionText);
    ~KEditCommentDialog() override;

    QString getCommentText() const;

private:
    QTextEdit *m_editor;
};

#endif
