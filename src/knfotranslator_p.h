/*****************************************************************************
 * Copyright (C) 2010 by Peter Penz <peter.penz@gmx.at>                      *
 * Copyright (C) 2013 by Vishesh Handa <me@vhanda.in>                        *
 *                                                                           *
 * This library is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Library General Public               *
 * License as published by the Free Software Foundation; either              *
 * version 2 of the License, or (at your option) any later version.          *
 *                                                                           *
 * This library is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Library General Public License for more details.                          *
 *                                                                           *
 * You should have received a copy of the GNU Library General Public License *
 * along with this library; see the file COPYING.LIB.  If not, write to      *
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 * Boston, MA 02110-1301, USA.                                               *
 *****************************************************************************/

#ifndef KNFOTRANSLATOR_H
#define KNFOTRANSLATOR_H

#include <QHash>
#include <QString>

/**
 * @brief Returns translations for Baloo Properties
 *
 */
class KNfoTranslator
{
public:
    static KNfoTranslator& instance();
    QString translation(const QString& propName) const;

protected:
    KNfoTranslator();
    virtual ~KNfoTranslator();
    friend class KNfoTranslatorSingleton;

private:
    QHash<QString, QString> m_hash;
};

#endif // KNFO_TRANSLATOR_H
