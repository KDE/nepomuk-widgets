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

#include "groupedlineedit.h"
#include "groupedlineeditblock_p.h"

#include <QtGui/QStyleOptionFrameV3>
#include <QtGui/QHBoxLayout>

using namespace Nepomuk2;

struct GroupedLineEdit::Private
{
    QHBoxLayout *layout;
    GroupedLineEditBlock *main_block;

    int cursor_position;
};

GroupedLineEdit::GroupedLineEdit(QWidget* parent)
: QScrollArea(parent),
  d(new Private)
{
    // Make the query builder look like a line edit
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget *widget = new QWidget(this);

    widget->setBackgroundRole(QPalette::Base);

    setWidget(widget);
    setWidgetResizable(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Create the main block
    d->main_block = NULL;
    d->cursor_position = 0;

    d->layout = new QHBoxLayout(widget);
    d->layout->setContentsMargins(2, 2, 2, 2);
    d->layout->addStretch();

    removeAllBlocks();
}

GroupedLineEdit::~GroupedLineEdit()
{
    delete d;
}

QString GroupedLineEdit::text() const
{
    if (d->main_block) {
        return d->main_block->text();
    } else {
        return QString();
    }
}

int GroupedLineEdit::cursorPosition() const
{
    return d->cursor_position;
}

void GroupedLineEdit::addBlock(int start, int end)
{
    if (d->main_block) {
        d->main_block->addBlock(start, end);
    }
}

void GroupedLineEdit::setCursorPosition(int position)
{
    if (d->main_block) {
        d->main_block->setCursorPosition(position);
    }
}

void GroupedLineEdit::setText(const QString& text)
{
    // Re-create the main block
    if (d->main_block) {
        delete d->main_block;
    }

    d->main_block = new GroupedLineEditBlock(0, text, QColor(), false, widget());
    d->layout->insertWidget(0, d->main_block);

    connect(d->main_block, SIGNAL(textChanged()),
            this, SIGNAL(textChanged()), Qt::QueuedConnection);
    connect(d->main_block, SIGNAL(cursorPositionChanged(int)),
            this, SLOT(emitCursorPositionChanged(int)));
}

void GroupedLineEdit::removeAllBlocks()
{
    setText(text());
}

QSize GroupedLineEdit::sizeHint() const
{
    QSize hint(widget()->sizeHint());

    hint.setHeight(hint.height() + 8);

    return hint;
}

void GroupedLineEdit::emitCursorPositionChanged(int position)
{
    d->cursor_position = position;

    if (focusWidget()) {
        ensureWidgetVisible(focusWidget(), 0, 0);
    }

    emit cursorPositionChanged(position);
}

#include "groupedlineedit.moc"
