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

#include "widgets_export.h"
#include "groupedlineedit.h"

namespace Baloo {

class CompletionProposal;
class NaturalQueryParser;
class Term;

class BALOO_WIDGETS_EXPORT QueryBuilder : public GroupedLineEdit
{
    Q_OBJECT

    public:
        explicit QueryBuilder(NaturalQueryParser *parser, QWidget *parent = 0);

        /**
         * @brief Parse the user query and provide syntax-highlighting and auto-completion
         *
         * If parsing is disabled, the query builder acts like a simple
         * QLineEdit without any fancy coloring. If parsing is enabled, all the
         * features are exposed to the user.
         *
         * By default, parsing is enabled.
         */
        void setParsingEnabled(bool enable);

        /**
         * @return whether parsing is enabled
         */
        bool parsingEnabled() const;

    private:
        void handleTerm(const Term &term);

    private Q_SLOTS:
        void reparse();
        void proposalSelected(CompletionProposal *proposal,
                              const QString &value);

    private:
        struct Private;
        Private *d;
};

}

#endif
