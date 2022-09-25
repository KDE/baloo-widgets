/*
    SPDX-FileCopyrightText: 2006-2010 Sebastian Trueg <trueg@kde.org>
    SPDX-FileCopyrightText: 2011-2013 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "tagwidget.h"
#include "kblocklayout.h"
#include "kedittagsdialog_p.h"
#include "tagcheckbox.h"
#include "tagwidget_p.h"

#include <KLocalizedString>

#include <QLabel>
#include <QPushButton>

using namespace Baloo;

void TagWidgetPrivate::init(TagWidget *parent)
{
    q = parent;
    m_readOnly = false;
    m_showAllLinkLabel = nullptr;
    m_editTagsDialog = nullptr;

    auto mainLayout = new QGridLayout(q);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    // TODO spacingHint should be declared. Old code  m_flowLayout = new KBlockLayout( 0, KDialog::spacingHint()*3 );
    m_flowLayout = new KBlockLayout(0);
    mainLayout->addLayout(m_flowLayout, 0, 0, 1, 2);
    mainLayout->setColumnStretch(0, 1);
}

void TagWidgetPrivate::rebuild()
{
    buildTagHash(q->selectedTags());
}

void TagWidgetPrivate::buildTagHash(const QStringList &tags)
{
    qDeleteAll(m_checkBoxHash);
    m_checkBoxHash.clear();

    for (const QString &tag : tags) {
        getTagCheckBox(tag);
    }

    delete m_showAllLinkLabel;
    m_showAllLinkLabel = nullptr;

    if (m_readOnly && !tags.isEmpty()) {
        return;
    }

    m_showAllLinkLabel = new QLabel(q);
    m_flowLayout->addWidget(m_showAllLinkLabel);
    if (m_readOnly) {
        m_showAllLinkLabel->setText(QStringLiteral("-"));
    } else {
        QFont f(q->font());
        f.setUnderline(true);
        m_showAllLinkLabel->setFont(f);
        m_showAllLinkLabel->setText(QLatin1String("<a href=\"add_tags\">") + (m_checkBoxHash.isEmpty() ? i18nc("@label", "Add...") : i18nc("@label", "Edit..."))
                                    + QLatin1String("</a>"));
        q->connect(m_showAllLinkLabel, SIGNAL(linkActivated(QString)), SLOT(slotShowAll()));
    }
}

TagCheckBox *TagWidgetPrivate::getTagCheckBox(const QString &tag)
{
    QMap<QString, TagCheckBox *>::iterator it = m_checkBoxHash.find(tag);
    if (it == m_checkBoxHash.end()) {
        // kDebug() << "Creating checkbox for" << tag.genericLabel();
        auto checkBox = new TagCheckBox(tag, q);
        q->connect(checkBox, SIGNAL(tagClicked(QString)), SIGNAL(tagClicked(QString)));
        m_checkBoxHash.insert(tag, checkBox);
        m_flowLayout->addWidget(checkBox);
        return checkBox;
    } else {
        return it.value();
    }
}

void TagWidgetPrivate::selectTags(const QStringList &tags)
{
    buildTagHash(tags);
}

TagWidget::TagWidget(QWidget *parent)
    : QWidget(parent)
    , d(new TagWidgetPrivate())
{
    setForegroundRole(parent->foregroundRole());
    d->init(this);
}

TagWidget::~TagWidget() = default;

QStringList TagWidget::selectedTags() const
{
    QStringList tags;
    QMapIterator<QString, TagCheckBox *> it(d->m_checkBoxHash);
    while (it.hasNext()) {
        tags << it.next().key();
    }
    return tags;
}

Qt::Alignment TagWidget::alignment() const
{
    return d->m_flowLayout->alignment();
}

bool TagWidget::readOnly() const
{
    return d->m_readOnly;
}

void TagWidget::setSelectedTags(const QStringList &tags)
{
    d->selectTags(tags);
}

void TagWidget::setAlignment(Qt::Alignment alignment)
{
    d->m_flowLayout->setAlignment(alignment);
}

void TagWidget::setReadyOnly(bool readOnly)
{
    d->m_readOnly = readOnly;
    d->rebuild();
}

void TagWidget::slotTagUpdateDone()
{
    setEnabled(true);
}

void TagWidget::slotShowAll()
{
    d->m_editTagsDialog = new KEditTagsDialog(selectedTags(), this);
    d->m_editTagsDialog->setWindowModality(Qt::ApplicationModal);
    connect(d->m_editTagsDialog, SIGNAL(finished(int)), this, SLOT(slotKEditTagDialogFinished(int)));
    d->m_editTagsDialog->open();
}

void TagWidget::slotKEditTagDialogFinished(int result)
{
    if (result == QDialog::Accepted) {
        setSelectedTags(d->m_editTagsDialog->tags());
        Q_EMIT selectionChanged(selectedTags());
    }

    d->m_editTagsDialog->deleteLater();
    d->m_editTagsDialog = nullptr;
}
