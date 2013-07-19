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
};

QueryBuilder::QueryBuilder(Query::QueryParser *parser, QWidget *parent)
: GroupedLineEdit(parent),
  d(new Private)
{
    d->parser = parser;
    d->completer = new QueryBuilderCompleter(this);

    connect(this, SIGNAL(textChanged()),
            this, SLOT(reparse()));
    connect(d->completer, SIGNAL(proposalSelected(Nepomuk2::Query::CompletionProposal*,QString)),
            this, SLOT(autoComplete(Nepomuk2::Query::CompletionProposal*,QString)));
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
    int position = cursorPosition();
    Query::Query query = d->parser->parse(text(), Query::QueryParser::DetectFilenamePattern, position);
    Query::Term term(query.term());

    // Highlight the input field
    removeAllBlocks();

    if (term.isAndTerm() || term.isOrTerm() || term.isNegationTerm()) {
        handleTerm(term);
    } else if (term.length() > 0) {
        // There is only one term (comparison, literal, etc) in the query, create
        // a block for it, handleTerm would not have done that
        addBlock(term.position(), term.position() + term.length() - 1);
    }

    setCursorPosition(position);

    // Build the list of auto-completions
    QList<Query::CompletionProposal *> proposals = d->parser->completionProposals();

    if (proposals.count() == 1) {
        // Only one proposal, the query is unambiguous, show a list of auto-completions
        switch (proposals.at(0)->type()) {
        case Query::CompletionProposal::Tag:
            d->completer->setMode(QueryBuilderCompleter::Tags);

            Q_FOREACH(const QString &tag, d->parser->allTags()) {
                d->completer->addTag(tag);
            }
            break;

        case Query::CompletionProposal::DateTime:
            d->completer->setMode(QueryBuilderCompleter::DateTime);
            break;

        default:
            // TODO: Handle contacts (Nepomuk or Akonadi contacts)
            d->completer->setMode(QueryBuilderCompleter::Proposals);
            break;
        }

        d->completer->addProposal(proposals.at(0));
        d->completer->open();
    } else if (proposals.count() > 1) {
        // More than one proposal, the user needs to clarify its input
        d->completer->setMode(QueryBuilderCompleter::Proposals);

        Q_FOREACH(Query::CompletionProposal *proposal, d->parser->completionProposals()) {
            d->completer->addProposal(proposal);
        }

        d->completer->open();
    } else {
        // No completion available
        d->completer->hide();
    }
}

void QueryBuilder::autoComplete(Query::CompletionProposal *proposal, const QString &placeholder_content)
{
    // Build the text that will be used to auto-complete the query
    QString replacement;
    int cursor_position = -1;

    Q_FOREACH(const QString &part, proposal->pattern()) {
        if (!replacement.isEmpty()) {
            replacement += QLatin1Char(' ');
        }

        if (part.at(0) == QLatin1Char('%')) {
            if (cursor_position == -1) {
                cursor_position = replacement.length() + placeholder_content.length();
            }

            replacement += placeholder_content;
        } else {
            // FIXME: This arbitrarily selects a term even if it does not fit
            //        what the user entered.
            replacement += part.section(QLatin1Char('|'), 0, 0);
        }
    }

    QString t = text();
    t.replace(proposal->startPosition(), proposal->endPosition() - proposal->startPosition() + 1, replacement);
    setText(t);

    setCursorPosition(proposal->startPosition() +
        (cursor_position >= 0 ? cursor_position : replacement.length()));
    reparse();
}

#include "querybuilder.moc"
