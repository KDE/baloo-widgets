/*
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2010 Sebastian Trueg <trueg@kde.org>
    SPDX-FileCopyrightText: 2025 Felix Ernst <felixernst@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "tagcheckbox.h"

#include <KLocalizedString>

#include <QStyle>

using namespace Baloo;

TagCheckBox::TagCheckBox(const QString &tag, QWidget *parent)
    : QToolButton(parent)
    , m_tag(tag)
{
    auto lastSegment = tag.split(QLatin1Char('/'), Qt::SkipEmptyParts).last();
    setText(lastSegment);
    setAccessibleName(i18nc("accessible name for tag, %1 is the tag label", "%1 tag", lastSegment));
    // i18n: A tag is a keyword to add to files and folders for organization and to quickly find all items with the same tag later. "Everything" refers to
    // files and folders, but this is obvious to the user because the user usually has tagged those items previously. This is about clicking a tag in the UI
    // to show the list of items with the same tag %1.
    setToolTip(i18nc("@info:tooltip", "Show everything tagged “%1”", tag));
    connect(this, &QAbstractButton::clicked, this, [this](){ Q_EMIT tagClicked(m_tag); });
}

TagCheckBox::~TagCheckBox() = default;

QSize TagCheckBox::minimumSizeHint() const
{
    return sizeHint();
}

QSize TagCheckBox::sizeHint() const
{
    /// We reduce the margins to be half of that of a normal button, so the tags need less space in the list of metadata and to give some uniqueness to this
    /// component so it isn't just seen as a normal button. The main use of this component is marking items with a tag, clicking the tag is secondary.
    const int halfAButtonMargin = style()->pixelMetric(QStyle::PM_ButtonMargin)/2;
    return QToolButton::sizeHint().shrunkBy({halfAButtonMargin, halfAButtonMargin, halfAButtonMargin, halfAButtonMargin});
}

#include "moc_tagcheckbox.cpp"
