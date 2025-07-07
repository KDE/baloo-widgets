/*
    SPDX-FileCopyrightText: 2008 Sebastian Trueg <trueg@kde.org>
    SPDX-FileCopyrightText: 2009 Peter Penz <peter.penz@gmx.at>
    SPDX-FileCopyrightText: 2025 Felix Ernst <felixernst@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOMMENT_WIDGET
#define KCOMMENT_WIDGET

#include <QString>
#include <QWidget>

class QPlainTextEdit;
class QToolButton;

/**
 * @brief Allows to edit and show a comment as part of KMetaDataWidget.
 */
class KCommentWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KCommentWidget(QWidget *parent = nullptr);
    ~KCommentWidget() override;

    void setText(const QString &comment);
    QString text() const;

    /**
     * If set to true, the comment cannot be changed by the user.
     * Per default read-only is disabled.
     */
    // TODO: provide common interface class for metadatawidgets
    void setReadOnly(bool readOnly);
    bool isReadOnly() const;

Q_SIGNALS:
    void commentChanged(const QString &comment);

protected:
    bool event(QEvent *event) override;

private:
    QPlainTextEdit *const m_plainTextEdit;
    QToolButton *const m_saveButton;
    QString m_comment;
};

#endif
