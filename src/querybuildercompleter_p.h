/* This file is part of the Baloo widgets collection
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

#include <QListWidget>

class QListWidgetItem;

namespace Baloo {

class CompletionProposal;

class QueryBuilderCompleter : public QListWidget
{
Q_OBJECT

public:
    explicit QueryBuilderCompleter(QWidget *parent);

    void addProposal(CompletionProposal *proposal, const QString &prefix);

public Q_SLOTS:
    void open();

private Q_SLOTS:
    void proposalActivated(QListWidgetItem *item);

protected:
    virtual bool eventFilter(QObject *, QEvent *event);

Q_SIGNALS:
    void proposalSelected(CompletionProposal *proposal,
                          const QString &value);

private:
    QString valueStartingWith(const QStringList &strings,
                              const QString &prefix) const;
    QWidget *widgetForProposal(CompletionProposal *proposal,
                               const QString &value);

private:
    QStringList validPropertyNames;
};

}

#endif
