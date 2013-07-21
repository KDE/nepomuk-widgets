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

#ifndef __GROUPEDLINEEDITBLOCK_H__
#define __GROUPEDLINEEDITBLOCK_H__

#include <QtGui/QWidget>
#include <QtGui/QColor>

namespace Nepomuk2 {

class GroupedLineEdit;

class GroupedLineEditBlock : public QWidget
{
    friend class GroupedLineEdit;
    Q_OBJECT

    private:
        explicit GroupedLineEditBlock(int start_pos,
                                      const QString &content,
                                      const QColor &color,
                                      bool closable,
                                      QWidget *parent);

    public:
        virtual ~GroupedLineEditBlock();

        /**
         * \brief Content of this block and its sub-blocks
         */
        QString text() const;

        /**
         * \brief Mark a substring as a new nested block
         * \param start index of the first character part of the new block
         * \param end index of the last character part of the new block
         */
        void addBlock(int start, int end);

        /**
         * \brief Set the cursor position in the block
         * \param position the absolute position of the cursor (0 being the start
         *                 of a groupedlineedit)
         */
        void setCursorPosition(int position);

    private slots:
        void nestedBlockClosed();
        void nestedCursorBeforeStart();
        void nestedCursorAfterEnd();

    signals:
        void cursorPositionChanged(int position);
        void textChanged();
        void editingFinished();

        void cursorBeforeStart();
        void cursorAfterEnd();
        void closed();

    protected:
        virtual void paintEvent(QPaintEvent *e);

    private:
        struct Private;
        Private *d;
};

}

#endif