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

#ifndef __GROUPEDLINEEDIT_H__
#define __GROUPEDLINEEDIT_H__

#include <QtGui/QPlainTextEdit>

namespace Nepomuk2 {

class GroupedLineEdit : public QPlainTextEdit
{
    Q_OBJECT

    public:
        explicit GroupedLineEdit(QWidget *parent = 0);
        virtual ~GroupedLineEdit();

        QString text() const;
        int cursorPosition() const;

        void setCursorPosition(int position);
        void setText(const QString &text);

        void removeAllBlocks();
        void addBlock(int start, int end);

        virtual QSize sizeHint() const;
        virtual QSize minimumSizeHint() const;

    signals:
        void editingFinished();

    protected:
        virtual void paintEvent(QPaintEvent *e);
        virtual void keyPressEvent(QKeyEvent *e);

    private:
        struct Private;
        Private *d;
};

}

#endif