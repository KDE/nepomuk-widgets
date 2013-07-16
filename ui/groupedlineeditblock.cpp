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

#include "groupedlineeditblock_p.h"
#include "groupedlineeditedit_p.h"

#include <QtGui/QHBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QColor>
#include <QtGui/QBrush>
#include <QtGui/QPainter>

using namespace Nepomuk2;

struct GroupedLineEditBlock::Private
{
    GroupedLineEditBlock *block;
    QColor color;
    int start_pos;
    int sub_color_index;

    QHBoxLayout *layout;
    bool closable;

    void addLineEdit(int offset, const QString &content, int index);
    void addBlock(int offset, const QString& content, int index);
    void setWidgetCursorPosition(QWidget *widget, int widget_start, int position);

    int indexOfWidget(QWidget *widget);
    QWidget *widgetAtPosition(int position, int &widget_start);
};

GroupedLineEditBlock::GroupedLineEditBlock(int start_pos,
                                           const QString &content,
                                           const QColor &color,
                                           bool closable,
                                           QWidget *parent)
: QWidget(parent),
  d(new Private)
{
    d->layout = new QHBoxLayout(this);
    d->layout->setContentsMargins(3, 0, 3, 0);

    d->block = this;
    d->color = color;
    d->closable = closable;
    d->sub_color_index = 0;
    d->start_pos = start_pos;

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // Add a line edit containing the whole content
    d->addLineEdit(0, content, 0);
    d->layout->setSpacing(0);

    if (closable) {
        // Add a close button
        QPushButton *close_button = new QPushButton(QString::fromUtf8("×"), this);

        close_button->setFlat(true);
        close_button->setStyleSheet(QLatin1String("margin:0px; padding:2px;"));

        connect(close_button, SIGNAL(clicked(bool)),
                this, SIGNAL(closed()));

        d->layout->addWidget(close_button);
    }
}

GroupedLineEditBlock::~GroupedLineEditBlock()
{
    delete d;
}

QString GroupedLineEditBlock::text() const
{
    QString rs;

    for (int i=0; i<d->layout->count(); ++i) {
        QLayoutItem *item = d->layout->itemAt(i);

        if (item->widget()) {
            GroupedLineEditEdit *as_edit = qobject_cast<GroupedLineEditEdit *>(item->widget());
            GroupedLineEditBlock *as_block = qobject_cast<GroupedLineEditBlock *>(item->widget());

            if (as_edit) {
                rs.append(as_edit->text());
            } else if (as_block) {
                rs.append(as_block->text());
            }
        }
    }

    return rs;
}

void GroupedLineEditBlock::addBlock(int start, int end)
{
    // The block must be entirely contained in an existing widget
    int widget_start, unused;
    QWidget *widget = d->widgetAtPosition(start, widget_start);

    if (!widget || widget != d->widgetAtPosition(end, unused)) {
        return;
    }

    GroupedLineEditEdit *as_edit = qobject_cast<GroupedLineEditEdit *>(widget);
    GroupedLineEditBlock *as_block = qobject_cast<GroupedLineEditBlock *>(widget);

    if (as_block) {
        as_block->addBlock(start - widget_start, end - widget_start);
    } else if (as_edit) {
        // Replace the edit with an edit (if not empty), a block and then another edit
        int index = d->indexOfWidget(as_edit);
        QString content = as_edit->text();

        int left_particle_len = start - widget_start;
        int mid_particle_len = end - start + 1;
        int right_particle_len = content.length() - left_particle_len - mid_particle_len;

        if (left_particle_len == 0 && right_particle_len == 0) {
            // Perfectly overlapping blocks
            return;
        }

        delete as_edit;

        if (right_particle_len > 0) {
            d->addLineEdit(end + 1, content.right(right_particle_len), index);
        }

        d->addBlock(start, content.mid(left_particle_len, mid_particle_len), index);

        if (left_particle_len > 0) {
            d->addLineEdit(widget_start, content.left(left_particle_len), index);
        }
    }
}

void GroupedLineEditBlock::setCursorPosition(int position)
{
    // Find the widget that must contains the new cursor position
    int widget_start;
    int character_position = qMax(0, position - d->start_pos - 1);  // Character just before the cursor
    QWidget *widget = d->widgetAtPosition(character_position, widget_start);

    if (!widget) {
        return;
    }

    // Set the cursor position of the widget
    d->setWidgetCursorPosition(widget, widget_start, position);
}

void GroupedLineEditBlock::nestedBlockClosed()
{
    sender()->deleteLater();
    emit textChanged();
}

void GroupedLineEditBlock::nestedCursorBeforeStart()
{
    int index = d->indexOfWidget(static_cast<QWidget *>(sender()));

    if (index == 0) {
        emit cursorBeforeStart();
    } else if (index != -1) {
        // Put the cursor at the end of the previous control
        d->setWidgetCursorPosition(d->layout->itemAt(index - 1)->widget(), 0, INT_MAX);
    }
}

void GroupedLineEditBlock::nestedCursorAfterEnd()
{
    int index = d->indexOfWidget(static_cast<QWidget *>(sender()));

    if (index == d->layout->count() - (d->closable ? 2 : 1)) {
        emit cursorAfterEnd();
    } else if (index != -1) {
        // Put the cursor at the end of the previous control
        d->setWidgetCursorPosition(d->layout->itemAt(index + 1)->widget(), 0, 0);
    }
}

void GroupedLineEditBlock::paintEvent(QPaintEvent *e)
{
    if (d->color.isValid()) {
        QPainter painter(this);

        // Draw a rounded rectangle around the content of this block
        QPainterPath path;
        path.addRoundedRect(1.0, 1.0, width() - 2, height() - 2, 3.0, 3.0);

        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
        painter.setPen(d->color);
        painter.setBrush(d->color.lighter(180));
        painter.drawPath(path);
    }

    QWidget::paintEvent(e);
}

/*
 * Private
 */
void GroupedLineEditBlock::Private::addLineEdit(int offset, const QString &content, int index)
{
    GroupedLineEditEdit *edit = new GroupedLineEditEdit(start_pos + offset, content, block);

    layout->insertWidget(index, edit);

    QObject::connect(edit, SIGNAL(textChanged(QString)),
                     block, SIGNAL(textChanged()));
    QObject::connect(edit, SIGNAL(cursorBeforeStart()),
                     block, SLOT(nestedCursorBeforeStart()));
    QObject::connect(edit, SIGNAL(cursorAfterEnd()),
                     block, SLOT(nestedCursorAfterEnd()));
    QObject::connect(edit, SIGNAL(cursorPositionChanged(int)),
                     block, SIGNAL(cursorPositionChanged(int)));
}

void GroupedLineEditBlock::Private::addBlock(int offset, const QString &content, int index)
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

    QColor color(this->color);

    while (color == this->color) {
        unsigned char *c = &colors[0] + (sub_color_index * 4);

        color.setRgb(c[0], c[1], c[2]);
        sub_color_index = (sub_color_index + 1) & 7;
    }

    GroupedLineEditBlock *b = new GroupedLineEditBlock(start_pos + offset, content, color, true, block);

    layout->insertWidget(index, b);

    QObject::connect(b, SIGNAL(closed()),
                     block, SLOT(nestedBlockClosed()));
    QObject::connect(b, SIGNAL(textChanged()),
                     block, SIGNAL(textChanged()));
    QObject::connect(b, SIGNAL(cursorBeforeStart()),
                     block, SLOT(nestedCursorBeforeStart()));
    QObject::connect(b, SIGNAL(cursorAfterEnd()),
                     block, SLOT(nestedCursorAfterEnd()));
    QObject::connect(b, SIGNAL(cursorPositionChanged(int)),
                     block, SIGNAL(cursorPositionChanged(int)));
}

void GroupedLineEditBlock::Private::setWidgetCursorPosition(QWidget *widget, int widget_start, int position)
{
    GroupedLineEditEdit *as_lineedit = qobject_cast<GroupedLineEditEdit *>(widget);
    GroupedLineEditBlock *as_block = qobject_cast<GroupedLineEditBlock *>(widget);

    if (as_lineedit) {
        as_lineedit->setCursorPosition(
            qMax(0, qMin(position - start_pos - widget_start, as_lineedit->text().length()))
        );
        as_lineedit->setFocus();
    } else if (as_block) {
        as_block->setCursorPosition(position);
    }
}

int GroupedLineEditBlock::Private::indexOfWidget(QWidget* widget)
{
    for (int i=0; i<layout->count(); ++i) {
        QLayoutItem *item = layout->itemAt(i);

        if (item->widget() == widget) {
            return i;
        }
    }

    return -1;
}

QWidget *GroupedLineEditBlock::Private::widgetAtPosition(int position, int &widget_start)
{
    widget_start = 0;

    for (int i=0; i<layout->count(); ++i) {
        QLayoutItem *item = layout->itemAt(i);
        GroupedLineEditEdit *as_edit = qobject_cast<GroupedLineEditEdit *>(item->widget());
        GroupedLineEditBlock *as_block = qobject_cast<GroupedLineEditBlock *>(item->widget());
        int len = 0;

        if (as_edit) {
            len = as_edit->text().length();
        } else if (as_block) {
            len = as_block->text().length();
        }

        if (widget_start + len > position) {
            return item->widget();
        } else {
            widget_start += len;
        }
    }

    return layout->itemAt(layout->count() - (closable ? 2 : 1))->widget();
}

#include "groupedlineeditblock_p.moc"
