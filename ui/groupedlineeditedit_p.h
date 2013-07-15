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

#ifndef __GROUPEDLINEEDITEDIT_H__
#define __GROUPEDLINEEDITEDIT_H__

#include <QtGui/QLineEdit>

namespace Nepomuk2 {

class GroupedLineEditEdit : public QLineEdit
{
    Q_OBJECT

    public:
        explicit GroupedLineEditEdit(int start_pos, const QString &content, QWidget *parent = 0);

        virtual QSize sizeHint() const;
        virtual QSize minimumSizeHint() const;

    private slots:
        void contentChanged();
        void changeCursorPosition(int o, int n);

    signals:
        void cursorPositionChanged(int position);
        void cursorBeforeStart();
        void cursorAfterEnd();

    protected:
        virtual void keyPressEvent(QKeyEvent *e);

    private:
        int start_pos;
};

}

#endif