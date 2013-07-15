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
};

QueryBuilder::QueryBuilder(Query::QueryParser *parser, QWidget *parent)
: GroupedLineEdit(parent),
  d(new Private)
{
    d->parser = parser;

    connect(this, SIGNAL(textChanged()),
            this, SLOT(reparse()));
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
    Query::Query query = d->parser->parse(text(), Query::QueryParser::DetectFilenamePattern);
    Query::Term term(query.term());
    int position = cursorPosition();

    removeAllBlocks();

    if (term.isAndTerm() || term.isOrTerm() || term.isNegationTerm()) {
        handleTerm(term);
    } else if (term.length() > 0) {
        // There is only one term (comparison, literal, etc) in the query, create
        // a block for it, handleTerm would not have done that
        addBlock(term.position(), term.position() + term.length() - 1);
    }

    setCursorPosition(position);
}
