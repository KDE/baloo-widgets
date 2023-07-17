/*
    SPDX-FileCopyrightText: 2006-2010 Sebastian Trueg <trueg@kde.org>
    SPDX-FileCopyrightText: 2011-2013 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "tagwidget.h"
#include "kblocklayout.h"
#include "kedittagsdialog_p.h"
#include "tagcheckbox.h"

#include <KLocalizedString>

#include <QLabel>
#include <QMap>

using namespace Baloo;

class Baloo::TagWidgetPrivate
{
public:
    void init(TagWidget *parent);
    void rebuild();
    void buildTagHash(const QStringList &tags);

    /// create clickable tag label
    void addTagLabel(const QString &tag);

    /// check the corresponding checkboxes and even
    /// add missing checkboxes
    void selectTags(const QStringList &tags);

    bool m_readOnly = false;

    QMap<QString, TagCheckBox *> m_tagLabels;
    QLabel *m_showAllLinkLabel = nullptr;
    KBlockLayout *m_flowLayout = nullptr;
    TagWidget *q;

    void showEditDialog();
    void kEditTagDialogFinished(int result);
    KEditTagsDialog *m_editTagsDialog = nullptr;
};

void TagWidgetPrivate::init(TagWidget *parent)
{
    q = parent;

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
    qDeleteAll(m_tagLabels);
    m_tagLabels.clear();

    for (const QString &tag : tags) {
        addTagLabel(tag);
    }

    delete m_showAllLinkLabel;
    m_showAllLinkLabel = nullptr;

    if (m_readOnly && !tags.isEmpty()) {
        return;
    }

    m_showAllLinkLabel = new QLabel(q);
    m_flowLayout->addWidget(m_showAllLinkLabel);
    if (m_readOnly) {
        m_showAllLinkLabel->setTextFormat(Qt::PlainText);
        m_showAllLinkLabel->setText(QStringLiteral("-"));
    } else {
        m_showAllLinkLabel->setTextFormat(Qt::RichText);
        m_showAllLinkLabel->setText(QStringLiteral("<a href=\"add_tags\">%1</a>").arg(m_tagLabels.isEmpty() ? i18nc("@label", "Add...") : i18nc("@label", "Edit...")));
        q->connect(m_showAllLinkLabel, &QLabel::linkActivated, [this]{showEditDialog();});
    }
}

void TagWidgetPrivate::addTagLabel(const QString &tag)
{
    const auto it = m_tagLabels.find(tag);
    if (it == m_tagLabels.end()) {
        auto label = new TagCheckBox(tag, q);
        q->connect(label, &TagCheckBox::tagClicked, q, &TagWidget::tagClicked);
        m_tagLabels.insert(tag, label);
        m_flowLayout->addWidget(label);
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
    return d->m_tagLabels.keys();
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

void TagWidgetPrivate::showEditDialog()
{
    m_editTagsDialog = new KEditTagsDialog(m_tagLabels.keys(), q);
    m_editTagsDialog->setWindowModality(Qt::ApplicationModal);
    q->connect(m_editTagsDialog, &QDialog::finished, q, [this](int result) {kEditTagDialogFinished(result);});
    m_editTagsDialog->open();
}

void TagWidgetPrivate::kEditTagDialogFinished(int result)
{
    if (result == QDialog::Accepted) {
        selectTags(m_editTagsDialog->tags());
        Q_EMIT q->selectionChanged(m_tagLabels.keys());
    }

    m_editTagsDialog->deleteLater();
    m_editTagsDialog = nullptr;
}

#include "moc_tagwidget.cpp"
