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

#include "querybuildercompleter_p.h"

#include <nepomuk2/completionproposal.h>
#include <klocalizedstring.h>

#include <QtGui/QListWidget>
#include <QtGui/QListWidgetItem>
#include <QtGui/QCalendarWidget>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QKeyEvent>

QueryBuilderCompleter::QueryBuilderCompleter(QWidget *parent)
: QStackedWidget(parent),
  page_proposals(new QListWidget(this)),
  page_strings(new QListWidget(this)),
  page_datetime(new QCalendarWidget(this))
{
    // Display the completer in its own non-decorated popup
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_X11NetWmWindowTypeCombo);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_WindowPropagation);

    setFocusPolicy(Qt::NoFocus);
    setFocusProxy(parent);

    // Configure the pages
    page_proposals->setFrameShape(QFrame::NoFrame);
    page_strings->setFrameShape(QFrame::NoFrame);

    page_datetime->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    page_datetime->setFirstDayOfWeek(QLocale::system().firstDayOfWeek());

    // Add them in the stack
    addWidget(page_proposals);
    addWidget(page_strings);
    addWidget(page_datetime);

    parent->installEventFilter(this);

    connect(page_proposals, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(valueSelected()));
    connect(page_strings, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(valueSelected()));
    connect(page_datetime, SIGNAL(activated(QDate)),
            this, SLOT(valueSelected()));
}

void QueryBuilderCompleter::setMode(Mode mode)
{
    page_proposals->clear();

    switch (mode) {
    case Proposals:
        setCurrentWidget(page_proposals);
        break;

    case Strings:
        setCurrentWidget(page_strings);
        page_strings->clear();
        break;

    case DateTime:
        setCurrentWidget(page_datetime);
        break;
    }
}

void QueryBuilderCompleter::addProposal(Nepomuk2::Query::CompletionProposal *proposal)
{
    // Create a label representing the pattern of the proposal
    QString proposal_text = QLatin1String("&nbsp; &nbsp; ");

    for (int i=0; i<proposal->pattern().count(); ++i) {
        const QString &part = proposal->pattern().at(i);

        if (i != 0) {
            proposal_text += QLatin1Char(' ');
        }

        if (part.at(0) == QLatin1Char('%')) {
            proposal_text += QLatin1String("<em>");

            switch (proposal->type()) {
            case Nepomuk2::Query::CompletionProposal::NoType:
                proposal_text += i18nc("Pattern placeholder having no specific type", "...");
                break;

            case Nepomuk2::Query::CompletionProposal::DateTime:
                proposal_text += i18nc("Pattern placeholder of date-time type", "[date and time]");
                break;

            case Nepomuk2::Query::CompletionProposal::Tag:
                proposal_text += i18nc("Pattern placeholder being a tag name", "[tag name]");
                break;

            case Nepomuk2::Query::CompletionProposal::Contact:
                proposal_text += i18nc("Pattern placeholder being a contact identifier", "[contact]");
                break;
            }

            proposal_text += QLatin1String("</em>");
        } else if (i <= proposal->lastMatchedPart()) {
            proposal_text += QLatin1String("<strong>") + part + QLatin1String("</strong>");
        } else {
            proposal_text += part;
        }
    }

    // Widget displaying the proposal
    QWidget *widget = new QWidget(this);
    QLabel *title_label = new QLabel(proposal->description().toString(), widget);
    QLabel *content_label = new QLabel(proposal_text);
    QVBoxLayout *vlayout = new QVBoxLayout(widget);

    QFont title_font(title_label->font());
    title_font.setBold(true);
    title_label->setFont(title_font);

    title_label->setTextFormat(Qt::PlainText);
    content_label->setTextFormat(Qt::RichText);

    vlayout->addWidget(title_label);
    vlayout->addWidget(content_label);

    // Add a new item to the list
    QListWidgetItem *item = new QListWidgetItem(page_proposals);

    item->setData(Qt::UserRole, QVariant::fromValue(static_cast<void *>(proposal)));
    item->setSizeHint(widget->sizeHint());

    page_proposals->addItem(item);
    page_proposals->setItemWidget(item, widget);
    page_proposals->setCurrentRow(0);
}

void QueryBuilderCompleter::setStrings(const QStringList &strings, const QString &preselect_prefix)
{
    Q_FOREACH(const QString &s, strings) {
        page_strings->addItem(s);
    }

    page_strings->sortItems(Qt::AscendingOrder);

    // Preselect the first prefix that is just after preselect_prefix in the
    // alphabetic order
    page_strings->setCurrentRow(0);

    for (int i=0; i<page_strings->count(); ++i) {
        if (page_strings->item(i)->text().toLower() > preselect_prefix.toLower()) {
            page_strings->setCurrentRow(i);
            break;
        }
    }
}

void QueryBuilderCompleter::open()
{
    if (page_proposals->count() == 0) {
        return;
    }

    QWidget *p = parentWidget();
    QPoint parent_position = p->mapToGlobal(QPoint(0, 0));

    // Display the popup just below the parent widget
    resize(p->width(), sizeHint().height());
    move(parent_position.x(), parent_position.y() + p->height());

    show();
}

void QueryBuilderCompleter::valueSelected()
{
    Nepomuk2::Query::CompletionProposal *proposal =
        static_cast<Nepomuk2::Query::CompletionProposal *>(
            page_proposals->currentItem()->data(Qt::UserRole).value<void *>()
        );

    QString placeholder_content;

    if (currentWidget() == page_strings) {
        placeholder_content = page_strings->currentItem()->text();
    } else if (currentWidget() == page_datetime) {
        placeholder_content = page_datetime->selectedDate().toString(QLatin1String("yyyy-MM-dd"));
    }

    emit proposalSelected(proposal, placeholder_content);
    hide();
}

bool QueryBuilderCompleter::eventFilter(QObject *, QEvent *event)
{
    if (!isVisible()) {
        return false;       // Don't block events when the completer is not open
    }

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keypress = static_cast<QKeyEvent *>(event);
        QListWidget *list = qobject_cast<QListWidget *>(currentWidget());    // The event is never filtered when the date-time page is opened

        switch (keypress->key()) {
        case Qt::Key_Up:
            if (list && list->currentRow() > 0) {
                list->setCurrentRow(list->currentRow() - 1);
            }
            break;

        case Qt::Key_Down:
            if (list && list->currentRow() < list->count() - 1) {
                list->setCurrentRow(list->currentRow() + 1);
            }
            break;

        case Qt::Key_Enter:
        case Qt::Key_Tab:
        case Qt::Key_Return:
            valueSelected();
            break;

        case Qt::Key_Escape:
            hide();
            break;

        default:
            break;
        }
    } else if (event->type() == QEvent::FocusOut) {
        hide();
    }

    return false;
}

#include "querybuildercompleter_p.moc"
