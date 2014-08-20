/* This file is part of the Nepomuk widgets collection
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

#include "querybuilder.h"
#include "groupedlineedit.h"
#include "querybuildercompleter_p.h"

#include <baloo/completionproposal.h>
#include <baloo/naturalqueryparser.h>
#include <baloo/term.h>

using namespace Baloo;

struct QueryBuilder::Private
{
    NaturalQueryParser *parser;
    QueryBuilderCompleter *completer;

    bool parsing_enabled;
};

static int termStart(const Baloo::Term &term)
{
    return term.userData(QLatin1String("start_position")).toInt();
}

static int termEnd(const Baloo::Term &term)
{
    return term.userData(QLatin1String("end_position")).toInt();
}

QueryBuilder::QueryBuilder(NaturalQueryParser *parser, QWidget *parent)
: GroupedLineEdit(parent),
  d(new Private)
{
    d->parser = parser;
    d->completer = new QueryBuilderCompleter(this);
    d->parsing_enabled = true;

    connect(this, SIGNAL(textChanged()),
            this, SLOT(reparse()));
    connect(d->completer, SIGNAL(proposalSelected(CompletionProposal*,QString)),
            this, SLOT(proposalSelected(CompletionProposal*,QString)));
}

void QueryBuilder::setParsingEnabled(bool enable)
{
    d->parsing_enabled = enable;
}

bool QueryBuilder::parsingEnabled() const
{
    return d->parsing_enabled;
}

void QueryBuilder::handleTerm(const Term &term)
{
    // If the term has subterms (AND or OR), highlight these
    if (term.subTerms().count() > 0) {
        Q_FOREACH (const Term &subterm, term.subTerms()) {
            addBlock(termStart(subterm), termEnd(subterm));
            handleTerm(subterm);
        }
    }
}

void QueryBuilder::reparse()
{
    if (!parsingEnabled()) {
        d->completer->hide();
        return;
    }

    int position = cursorPosition();
    QString t = text();

    Query query = d->parser->parse(t, NaturalQueryParser::DetectFilenamePattern, position);
    Term term(query.term());

    // Extract the term just before the cursor
    QString term_before_cursor;

    for (int i=position-1; i>=0 && !t.at(i).isSpace(); --i) {
        term_before_cursor.prepend(t.at(i));
    }

    // Highlight the input field
    removeAllBlocks();
    handleTerm(term);

    setCursorPosition(position);

    // Build the list of auto-completions
    QList<CompletionProposal *> proposals = d->parser->completionProposals();

    if (proposals.count() > 0) {
        d->completer->clear();

        Q_FOREACH(CompletionProposal *proposal, d->parser->completionProposals()) {
            d->completer->addProposal(proposal, term_before_cursor);
        }

        d->completer->open();
    } else {
        // No completion available
        d->completer->hide();
    }
}

void QueryBuilder::proposalSelected(CompletionProposal *proposal,
                                    const QString &value)
{
    QString t = text();

    // Term before the cursor (if any)
    int term_before_cursor_pos = cursorPosition();
    QString term_before_cursor;

    while (term_before_cursor_pos > 0 && !t.at(term_before_cursor_pos - 1).isSpace()) {
        term_before_cursor.prepend(t.at(term_before_cursor_pos - 1));
        --term_before_cursor_pos;
    }

    // Build the text that will be used to auto-complete the query
    QStringList pattern = proposal->pattern();
    QString replacement;
    int first_unmatched_part = proposal->lastMatchedPart() + 1;
    int cursor_offset = -1;

    if (!term_before_cursor.isEmpty()) {
        // The last matched part will be replaced by value, so count it
        // as unmatched to have it replaced
        --first_unmatched_part;
    }

    for (int i=first_unmatched_part; i<pattern.count(); ++i) {
        const QString &part = pattern.at(i);

        if (!replacement.isEmpty()) {
            replacement += QLatin1Char(' ');
        }

        if (part.at(0) == QLatin1Char('$')) {
            cursor_offset = replacement.length() + value.length();
            replacement += value;
        } else {
            // FIXME: This arbitrarily selects a term even if it does not fit
            //        gramatically.
            replacement += part.section(QLatin1Char('|'), 0, 0);
        }
    }

    // setText() will cause a reparse(), that will invalidate proposal
    int put_cursor_at = term_before_cursor_pos +
        (cursor_offset >= 0 ? cursor_offset : replacement.length());

    // Auto-complete, setText() triggers a reparse
    t.replace(term_before_cursor_pos, term_before_cursor.length(), replacement);

    setText(t);
    setCursorPosition(put_cursor_at);
}

#include "querybuilder.moc"
