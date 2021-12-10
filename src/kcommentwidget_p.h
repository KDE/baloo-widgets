/*
    SPDX-FileCopyrightText: 2008 Sebastian Trueg <trueg@kde.org>
    SPDX-FileCopyrightText: 2009 Peter Penz <peter.penz@gmx.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOMMENT_WIDGET
#define KCOMMENT_WIDGET

#include <QString>
#include <QWidget>

class QLabel;

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

    QSize sizeHint() const override;

Q_SIGNALS:
    void commentChanged(const QString &comment);

protected:
    bool event(QEvent *event) override;

private Q_SLOTS:
    void slotLinkActivated(const QString &link);

private:
    bool m_readOnly;
    QLabel *m_label;
    QLabel *m_sizeHintHelper; // see comment in KCommentWidget::sizeHint()
    QString m_comment;
};

#endif
