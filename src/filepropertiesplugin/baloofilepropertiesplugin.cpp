/*
    SPDX-FileCopyrightText: 2017 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "baloofilepropertiesplugin.h"

#include <QFrame>
#include <QScrollArea>
#include <QVBoxLayout>

#include <KIO/JobUiDelegate>
#include <KIO/OpenUrlJob>
#include <KLocalizedString>
#include <KPluginFactory>

#include "filemetadatawidget.h"

K_PLUGIN_CLASS_WITH_JSON(BalooFilePropertiesPlugin, "baloofilepropertiesplugin.json")

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

    connect(metaDataWidget, &Baloo::FileMetaDataWidget::metaDataRequestFinished, this, [this, metaDataArea, metaDataWidget] {
        properties->addPage(metaDataArea, i18nc("Tab page with file meta data", "&Details"));
    });
}

BalooFilePropertiesPlugin::~BalooFilePropertiesPlugin() = default;

#include "baloofilepropertiesplugin.moc"
