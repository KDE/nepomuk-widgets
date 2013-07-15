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

#include "groupedlineeditedit_p.h"

#include <QtGui/QPalette>
#include <QtGui/QStyleOptionFrameV3>
#include <QtGui/QFontMetrics>
#include <QtGui/QKeyEvent>

using namespace Nepomuk2;

GroupedLineEditEdit::GroupedLineEditEdit(int start_pos, const QString &content, QWidget *parent)
: QLineEdit(content, parent),
  start_pos(start_pos)
{
    QPalette pal(palette());

    pal.setColor(QPalette::Base, Qt::transparent);

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setFrame(false);
    setPalette(pal);

    connect(this, SIGNAL(textChanged(QString)),
            this, SLOT(contentChanged()));
    connect(this, SIGNAL(cursorPositionChanged(int,int)),
            this, SLOT(changeCursorPosition(int,int)));
}

void GroupedLineEditEdit::contentChanged()
{
    updateGeometry();
}

void GroupedLineEditEdit::changeCursorPosition(int o, int n)
{
    (void) o;

    emit cursorPositionChanged(start_pos + n);
}

QSize GroupedLineEditEdit::sizeHint() const
{
    QStyleOptionFrameV3 opt;
    QFontMetrics fm(fontMetrics());
    QMargins margins(textMargins());

    int h = qMax(fm.height(), 14) + margins.top() + margins.bottom();
    int w = fm.width(text()) + margins.left() + margins.right() + 8;

    opt.initFrom(this);

    return style()->sizeFromContents(
        QStyle::CT_LineEdit,
        &opt,
        QSize(w, h),
        this
    );
}

QSize GroupedLineEditEdit::minimumSizeHint() const
{
    return QSize();
}

void GroupedLineEditEdit::keyPressEvent(QKeyEvent *e)
{
    if (cursorPosition() == 0 && e->key() == Qt::Key_Left) {
        emit cursorBeforeStart();
    } else if (cursorPosition() == text().length() && e->key() == Qt::Key_Right) {
        emit cursorAfterEnd();
    }

    QLineEdit::keyPressEvent(e);
}

