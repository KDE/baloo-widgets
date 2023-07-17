/*
    SPDX-FileCopyrightText: 2010 Sebastian Trueg <trueg@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef _BALOO_TAG_CHECKBOX_H_
#define _BALOO_TAG_CHECKBOX_H_

#include <QLabel>

#include "tagwidget_p.h"

namespace Baloo
{
class TagCheckBox : public QLabel
{
    Q_OBJECT

public:
    explicit TagCheckBox(const QString &tag, QWidget *parent = nullptr);
    ~TagCheckBox() override;

    QString tag() const
    {
        return m_tag;
    }

Q_SIGNALS:
    void tagClicked(const QString &tag);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void enableUrlHover(bool enabled);

    QString m_tag;
    bool m_urlHover = false;
};
}

#endif
