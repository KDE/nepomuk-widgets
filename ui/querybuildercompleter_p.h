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

#ifndef __QUERYBUILDERCOMPLETER_H__
#define __QUERYBUILDERCOMPLETER_H__

#include <QtGui/QStackedWidget>

class QListWidget;
class QListWidgetItem;
class QCalendarWidget;
class QDate;

namespace Nepomuk2 { namespace Query { class CompletionProposal; }}

class QueryBuilderCompleter : public QStackedWidget
{
    Q_OBJECT

    public:
        enum Mode {
            Proposals,
            Strings,
            DateTime,
        };

        explicit QueryBuilderCompleter(QWidget *parent);

        void setMode(Mode mode);

        void addProposal(Nepomuk2::Query::CompletionProposal *proposal);
        void setStrings(const QStringList &strings, const QString &preselect_prefix);

    public slots:
        void open();

    private slots:
        void valueSelected();

    protected:
        virtual bool eventFilter(QObject *, QEvent *event);

    signals:
        void proposalSelected(Nepomuk2::Query::CompletionProposal *proposal,
                              const QString &placeholder_content);

    private:
        QListWidget *page_proposals;
        QListWidget *page_strings;
        QCalendarWidget *page_datetime;
};

#endif