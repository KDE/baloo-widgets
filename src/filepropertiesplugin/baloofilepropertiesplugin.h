/*
    SPDX-FileCopyrightText: 2017 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include <KPropertiesDialog>

class BalooFilePropertiesPlugin : public KPropertiesDialogPlugin
{
    Q_OBJECT

public:
    BalooFilePropertiesPlugin(QObject *parent, const QList<QVariant> &args);
    ~BalooFilePropertiesPlugin() override;
};
