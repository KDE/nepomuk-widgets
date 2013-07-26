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

#include <nepomuk2/completionproposal.h>
#include <nepomuk2/queryparser.h>
#include <nepomuk2/query.h>
#include <nepomuk2/comparisonterm.h>
#include <nepomuk2/negationterm.h>
#include <nepomuk2/andterm.h>
#include <nepomuk2/orterm.h>
#include <nepomuk2/resourceterm.h>
#include <nepomuk2/resourcetypeterm.h>

using namespace Nepomuk2;

struct QueryBuilder::Private
{
    Query::QueryParser *parser;
    QueryBuilderCompleter *completer;
    bool parsing_enabled;
};

QueryBuilder::QueryBuilder(Query::QueryParser *parser, QWidget *parent)
: GroupedLineEdit(parent),
  d(new Private)
{
    d->parser = parser;
    d->completer = new QueryBuilderCompleter(parser, this);
    d->parsing_enabled = true;

    connect(this, SIGNAL(textChanged()),
            this, SLOT(reparse()));
    connect(d->completer, SIGNAL(proposalSelected(Nepomuk2::Query::CompletionProposal*,QString)),
            this, SLOT(proposalSelected(Nepomuk2::Query::CompletionProposal*,QString)));
}

void QueryBuilder::setParsingEnabled(bool enable)
{
    d->parsing_enabled = enable;
}

bool QueryBuilder::parsingEnabled() const
{
    return d->parsing_enabled;
}

void QueryBuilder::handleTerm(const Query::Term &term)
{
    QList<Query::Term> subterms;
    bool block_for_each_term = false;

    switch (term.type()) {
    case Query::Term::Comparison:
        subterms.append(term.toComparisonTerm().subTerm());
        break;

    case Query::Term::Negation:
        subterms.append(term.toNegationTerm().subTerm());
        break;

    case Query::Term::And:
        subterms = term.toAndTerm().subTerms();
        block_for_each_term = true;
        break;

    case Query::Term::Or:
        subterms = term.toOrTerm().subTerms();
        block_for_each_term = true;
        break;

    default:
        break;
    }

    Q_FOREACH(const Query::Term &term, subterms) {
        if (block_for_each_term && (term.isComparisonTerm() || term.isResourceTypeTerm())) {
            addBlock(term.position(), term.position() + term.length() - 1);
        }

        handleTerm(term);
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

    Query::Query query = d->parser->parse(t, Query::QueryParser::DetectFilenamePattern, position);
    Query::Term term(query.term());

    // Extract the term just before the cursor
    QString term_before_cursor;

    for (int i=position-1; i>=0 && !t.at(i).isSpace(); --i) {
        term_before_cursor.prepend(t.at(i));
    }

    // Highlight the input field
    removeAllBlocks();

    if (term.isAndTerm() || term.isOrTerm() || term.isNegationTerm()) {
        handleTerm(term);
    } else if (term.isComparisonTerm() || term.isResourceTypeTerm()) {
        // There is only one term (comparison, literal, etc) in the query, create
        // a block for it, handleTerm would not have done that
        addBlock(term.position(), term.position() + term.length() - 1);
    }

    setCursorPosition(position);

    // Build the list of auto-completions
    QList<Query::CompletionProposal *> proposals = d->parser->completionProposals();

    if (proposals.count() > 0) {
        d->completer->clear();

        Q_FOREACH(Query::CompletionProposal *proposal, d->parser->completionProposals()) {
            d->completer->addProposal(proposal, term_before_cursor);
        }

        d->completer->open();
    } else {
        // No completion available
        d->completer->hide();
    }
}

void QueryBuilder::proposalSelected(Query::CompletionProposal *proposal,
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

        if (part.at(0) == QLatin1Char('%')) {
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
