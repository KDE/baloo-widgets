/*
 * This file is part of the KDE Baloo Project
 * Copyright 2018  Michael Heidelbach <ottwolt@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FILEMETADATAWIDGETTEST_H
#define FILEMETADATAWIDGETTEST_H

#include <filemetadatawidget.h>

class FileMetadataWidgetTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();

    void shouldSignalOnceWithoutFile();
    void shouldSignalOnceFile();
    void shouldSignalOnceFiles();
    void shouldShowProperties();
    void shouldShowCommonProperties();
    void shouldShowMultiValueProperties();

private:
     Baloo::FileMetaDataWidget*  m_widget;
     bool m_mayTestRating = false;
};

#endif // FILEMETADATAWIDGETTEST_H
