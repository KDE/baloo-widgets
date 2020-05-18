/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2017 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "baloofilepropertiesplugin.h"

#include <QFrame>
#include <QVBoxLayout>
#include <QScrollArea>

#include <KLocalizedString>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KIO/JobUiDelegate>
#include <KIO/OpenUrlJob>

#include "filemetadatawidget.h"

K_PLUGIN_FACTORY(BalooFilePropertiesPluginFactory, registerPlugin<BalooFilePropertiesPlugin>();)

BalooFilePropertiesPlugin::BalooFilePropertiesPlugin(QObject *parent, const QList<QVariant> &args)
    : KPropertiesDialogPlugin(qobject_cast<KPropertiesDialog *>(parent))
{
    Q_UNUSED(args);

    QWidget *widgetContainer = new QWidget();

    QVBoxLayout *containerLayout = new QVBoxLayout(widgetContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);

    auto *metaDataWidget = new Baloo::FileMetaDataWidget();
    metaDataWidget->setItems(properties->items());
    connect(metaDataWidget, &Baloo::FileMetaDataWidget::urlActivated, this, [this](const QUrl &url) {
        KIO::OpenUrlJob *job = new KIO::OpenUrlJob(url);
        job->setUiDelegate(new KIO::JobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, properties));
        job->start();
    });

    containerLayout->addWidget(metaDataWidget);
    containerLayout->addStretch(1);

    QScrollArea *metaDataArea = new QScrollArea();

    metaDataArea->setWidget(widgetContainer);
    metaDataArea->setWidgetResizable(true);
    metaDataArea->setFrameShape(QFrame::NoFrame);

    properties->addPage(metaDataArea, i18nc("Tab page with file meta data", "&Details"));
}

BalooFilePropertiesPlugin::~BalooFilePropertiesPlugin() = default;

#include "baloofilepropertiesplugin.moc"
