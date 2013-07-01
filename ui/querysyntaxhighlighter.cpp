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

#include "querysyntaxhighlighter_p.h"

#include <nepomuk2/queryparser.h>
#include <nepomuk2/query.h>
#include <nepomuk2/comparisonterm.h>
#include <nepomuk2/negationterm.h>
#include <nepomuk2/andterm.h>
#include <nepomuk2/orterm.h>
#include <nepomuk2/resourceterm.h>
#include <nepomuk2/resourcetypeterm.h>

#include <QTextCharFormat>

using namespace Nepomuk2;

QuerySyntaxHighlighter::QuerySyntaxHighlighter(Nepomuk2::Query::QueryParser *parser,
                                               QTextDocument *parent)
: QSyntaxHighlighter(parent),
  _parser(parser)
{
}

void QuerySyntaxHighlighter::highlightBlock(const QString &text)
{
    _color = 0;
    highlightTerm(_parser->parse(text).term());

    // Quotes are never part of the terms they enclose. This can cause glitches
    // as comparison and and/or terms don't count them : [sent by "Me]" shows a
    // trailing quote uncolored (the ComparisonTerm is shown by the square brackets)
    // The problem is solved by not highlighting quotes
    QTextCharFormat default_format;

    for (int i=0; i<text.size(); ++i) {
        if (text.at(i) == QLatin1Char('"')) {
            setFormat(i, 1, default_format);
        }
    }
}

void QuerySyntaxHighlighter::highlightTerm(const Nepomuk2::Query::Term &term)
{
    static unsigned char colors[] = {
        0  , 87 , 174, 0,
        243, 195, 0  , 0,
        0  , 179, 119, 0,
        235, 115, 49 , 0,
        139, 179, 0  , 0,
        85 , 87 , 83 , 0,
        0  , 140, 0  , 0,
        117, 81 , 26 , 0
    };

    unsigned char *color = &colors[0] + (_color * 4);
    QTextCharFormat fmt = format(term.position());

    switch (term.type()) {
        case Nepomuk2::Query::Term::Comparison:
            fmt.setForeground(QColor(color[0], color[1], color[2]));

            // Underline nested queries
            if (term.toComparisonTerm().subTerm().isAndTerm() ||
                term.toComparisonTerm().subTerm().isOrTerm()) {
                fmt.setFontUnderline(true);
            }

        case Query::Term::ResourceType:
        case Query::Term::Literal:
        case Query::Term::Resource:
            fmt.setFontItalic(term.isComparisonTerm() || term.isResourceTypeTerm());

            setFormat(term.position(), term.length(), fmt);

            if (term.isComparisonTerm()) {
                _color = (_color + 1) & 7;
                highlightTerm(term.toComparisonTerm().subTerm());
            }
            break;

        case Nepomuk2::Query::Term::Negation:
            highlightTerm(term.toNegationTerm().subTerm());
            break;

        case Nepomuk2::Query::Term::And:
            Q_FOREACH(const Nepomuk2::Query::Term &t, term.toAndTerm().subTerms()) {
                highlightTerm(t);
            }
            break;

        case Nepomuk2::Query::Term::Or:
            Q_FOREACH(const Nepomuk2::Query::Term &t, term.toOrTerm().subTerms()) {
                highlightTerm(t);
            }
            break;

        default:
            break;
    }
}
