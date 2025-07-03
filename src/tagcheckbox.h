/*
    SPDX-FileCopyrightText: 2010 Sebastian Trueg <trueg@kde.org>
    SPDX-FileCopyrightText: 2025 Felix Ernst <felixernst@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef _BALOO_TAG_CHECKBOX_H_
#define _BALOO_TAG_CHECKBOX_H_

#include <QToolButton>

namespace Baloo
{
class TagCheckBox : public QToolButton
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
    QSize minimumSizeHint() const override;
    /** Reduce the preferred size so this component is not visually identical to a normal button. */
    QSize sizeHint() const override;

private:
    QString m_tag;
};
}

#endif
