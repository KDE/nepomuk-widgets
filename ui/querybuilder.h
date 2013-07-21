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

#ifndef __QUERYEDITOR_H__
#define __QUERYEDITOR_H__

#include <QPlainTextEdit>

namespace Nepomuk2 {

namespace Query {
    class QueryParser;
}

class QueryBuilder : public QPlainTextEdit
{
    public:
        explicit QueryBuilder(Query::QueryParser *parser, QWidget *parent = 0);

        virtual QSize sizeHint() const;

    protected:
        virtual void keyPressEvent(QKeyEvent *e);
};

}

#endif