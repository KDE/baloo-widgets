/* This file is part of the Baloo widgets collection
   Copyright (c) 2013 Denis Steckelmacher <steckdenis@yahoo.fr>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2.1 as published by the Free Software Foundation,
   or any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "querybuildercompleter_p.h"

#include <baloo/completionproposal.h>
#include <klocalizedstring.h>

#include <QtGui/QListWidget>
#include <QtGui/QListWidgetItem>
#include <QtGui/QCalendarWidget>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QKeyEvent>
#include <QtGui/QTextDocument> // for Qt::escape

using namespace Baloo;

QueryBuilderCompleter::QueryBuilderCompleter(QWidget *parent)
: QListWidget(parent)
{
    // Display the completer in its own non-decorated popup
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_X11NetWmWindowTypeCombo);
    setAttribute(Qt::WA_ShowWithoutActivating);

    setFocusPolicy(Qt::NoFocus);
    setFocusProxy(parent);
    setFrameShape(NoFrame);
    setUniformItemSizes(true);

    parent->installEventFilter(this);

    connect(this, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(proposalActivated(QListWidgetItem*)));
}

QWidget *QueryBuilderCompleter::widgetForProposal(CompletionProposal *proposal,
                                                  const QString &value)
{
    // Create a label representing the pattern of the proposal
    QString proposal_text = QLatin1String("&nbsp; &nbsp; ");
    QStringList pattern = proposal->pattern();

    for (int i=0; i<pattern.count(); ++i) {
        const QString &part = pattern.at(i);

        if (i != 0) {
            proposal_text += QLatin1Char(' ');
        }

        if (part.at(0) == QLatin1Char('$')) {
            proposal_text += QLatin1String("<em>");

            if (!value.isEmpty()) {
                proposal_text += value;
            } else {
                switch (proposal->type()) {
                case CompletionProposal::NoType:
                    proposal_text += i18nc("Pattern placeholder having no specific type", "[something]");
                    break;

                case CompletionProposal::DateTime:
                    proposal_text += i18nc("Pattern placeholder of date-time type", "[date and time]");
                    break;

                case CompletionProposal::Tag:
                    proposal_text += i18nc("Pattern placeholder for a tag name", "[tag name]");
                    break;

                case CompletionProposal::Contact:
                    proposal_text += i18nc("Pattern placeholder for a contact identifier", "[contact]");
                    break;

                case CompletionProposal::Email:
                    proposal_text += i18nc("Pattern placeholder for an e-mail address", "[email address]");
                }
            }

            proposal_text += QLatin1String("</em>");
        } else if (i <= proposal->lastMatchedPart()) {
            proposal_text += QLatin1String("<strong>") + Qt::escape(part) + QLatin1String("</strong>");
        } else {
            proposal_text += Qt::escape(part);
        }
    }

    // Widget displaying the proposal
    QWidget *widget = new QWidget(this);
    QLabel *title_label = new QLabel(proposal->description().toString(), widget);
    QLabel *content_label = new QLabel(proposal_text);
    QVBoxLayout *vlayout = new QVBoxLayout(widget);

    QFont title_font(title_label->font());
    title_font.setBold(true);
    title_label->setFont(title_font);

    title_label->setTextFormat(Qt::PlainText);
    content_label->setTextFormat(Qt::RichText);

    vlayout->addWidget(title_label);
    vlayout->addWidget(content_label);

    return widget;
}

QString QueryBuilderCompleter::valueStartingWith(const QStringList &strings,
                                                 const QString &prefix) const
{
    QStringList::const_iterator it = qLowerBound(strings, prefix);

    if (it == strings.end() || !(*it).startsWith(prefix)) {
        return QString();
    } else {
        return QLatin1Char('"') + *it + QLatin1Char('"');
    }
}

void QueryBuilderCompleter::addProposal(CompletionProposal *proposal,
                                        const QString &prefix)
{
    QString value;
    QStringList pattern = proposal->pattern();

    // If the term the user is entering is a placeholder, pre-fill it
    if (!prefix.isEmpty() &&
        proposal->lastMatchedPart() < pattern.size() &&
        pattern.at(proposal->lastMatchedPart()).at(0) == QLatin1Char('$'))
    {
        switch (proposal->type()) {
#if 0
        case CompletionProposal::Contact:
            value = valueStartingWith(parser->allContacts(), prefix);
            break;
        case CompletionProposal::Tag:
            value = valueStartingWith(parser->allTags(), prefix);
            break;
        case CompletionProposal::Email:
            value = valueStartingWith(parser->allEmailAddresses(), prefix);
            break;
#endif
        case CompletionProposal::DateTime:
            value = QDate::currentDate().toString(Qt::DefaultLocaleShortDate);
            break;
        default:
            break;
        }
    }

    // Add a new item to the list
    QListWidgetItem *item = new QListWidgetItem(this);
    QWidget *widget = widgetForProposal(proposal, value);

    item->setData(Qt::UserRole, QVariant::fromValue(static_cast<void *>(proposal)));
    item->setData(Qt::UserRole + 1, value);
    item->setSizeHint(widget->sizeHint());

    addItem(item);
    setItemWidget(item, widget);

    if (count() == 1 || !value.isEmpty()) {
        // Select the first item, or an interesting completion if possible
        setCurrentRow(count() - 1);
    }
}

void QueryBuilderCompleter::open()
{
    if (count() == 0) {
        return;
    }

    QWidget *p = parentWidget();
    QPoint parent_position = p->mapToGlobal(QPoint(0, 0));

    // Display the popup just below the parent widget
    resize(p->width(), count() * item(0)->sizeHint().height());
    move(parent_position.x(), parent_position.y() + p->height());

    show();
}

void QueryBuilderCompleter::proposalActivated(QListWidgetItem *item)
{
    CompletionProposal *proposal =
        static_cast<CompletionProposal *>(
            item->data(Qt::UserRole).value<void *>()
        );

    emit proposalSelected(proposal, item->data(Qt::UserRole + 1).toString());
}

bool QueryBuilderCompleter::eventFilter(QObject *, QEvent *event)
{
    bool rs = false;

    if (!isVisible()) {
        return rs;       // Don't block events when the completer is not open
    }

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keypress = static_cast<QKeyEvent *>(event);

        switch (keypress->key()) {
        case Qt::Key_Up:
            if (currentRow() > 0) {
                setCurrentRow(currentRow() - 1);
            }
            break;

        case Qt::Key_Down:
            if (currentRow() < count() - 1) {
                setCurrentRow(currentRow() + 1);
            }
            break;

        case Qt::Key_Enter:
        case Qt::Key_Tab:
        case Qt::Key_Return:
            proposalActivated(currentItem());
            rs = true;  // In Dolphin, don't trigger a search when Enter is pressed in the auto-completion box
            break;

        case Qt::Key_Escape:
            hide();
            rs = true;
            break;

        default:
            break;
        }
    }

    return rs;
}

#include "querybuildercompleter_p.moc"
