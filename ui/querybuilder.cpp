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
#include "querysyntaxhighlighter_p.h"

#include <QFont>
#include <QFontMetrics>
#include <QStyle>
#include <QStyleOptionFrameV3>
#include <QApplication>
#include <QKeyEvent>

using namespace Nepomuk2;

QueryBuilder::QueryBuilder(Nepomuk2::Query::QueryParser *parser, QWidget *parent)
: QPlainTextEdit(parent)
{
    new QuerySyntaxHighlighter(parser, document());

    setWordWrapMode(QTextOption::NoWrap);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setTabChangesFocus(true);
    setFixedHeight(sizeHint().height());

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    document()->setMaximumBlockCount(1);
}

QSize QueryBuilder::sizeHint() const
{
    QFontMetrics fm(font());
    QStyleOptionFrameV3 opt;
    QString text = document()->toPlainText();

    int h = qMax(fm.height(), 14) + 4;
    int w = fm.width(text) + 4;

    opt.initFrom(this);

    return style()->sizeFromContents(
        QStyle::CT_LineEdit,
        &opt,
        QSize(w, h).expandedTo(QApplication::globalStrut()),
        this
    );
}

void QueryBuilder::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
        e->ignore();
    } else {
        QPlainTextEdit::keyPressEvent(e);
    }
}
