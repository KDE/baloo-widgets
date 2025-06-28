/*
    SPDX-FileCopyrightText: 2017 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "baloofilepropertiesplugin.h"

#include <QFrame>
#include <QScrollArea>
#include <QVBoxLayout>

#include <kio_version.h>
#if KIO_VERSION >= QT_VERSION_CHECK(5, 98, 0)
#include <KIO/JobUiDelegateFactory>
#else
#include <KIO/JobUiDelegate>
#endif

#include <KIO/OpenUrlJob>
#include <KLocalizedString>
#include <KPluginFactory>

#include "filemetadatawidget.h"

K_PLUGIN_CLASS_WITH_JSON(BalooFilePropertiesPlugin, "baloofilepropertiesplugin.json")

BalooFilePropertiesPlugin::BalooFilePropertiesPlugin(QObject *parent, const QList<QVariant> &args)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    : KPropertiesDialogPlugin(qobject_cast<KPropertiesDialog *>(parent))
#else
    : KPropertiesDialogPlugin(parent)
#endif
{
    Q_UNUSED(args);

    auto widgetContainer = new QWidget();

    auto containerLayout = new QVBoxLayout(widgetContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);

    auto metaDataWidget = new Baloo::FileMetaDataWidget();
    metaDataWidget->setItems(properties->items());
    connect(metaDataWidget, &Baloo::FileMetaDataWidget::urlActivated, this, [this](const QUrl &url) {
        auto job = new KIO::OpenUrlJob(url);
#if KIO_VERSION >= QT_VERSION_CHECK(5, 98, 0)
        job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, properties));
#else
        job->setUiDelegate(new KIO::JobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, properties));
#endif
        job->start();
    });

    containerLayout->addWidget(metaDataWidget);
    containerLayout->addStretch(1);

    auto metaDataArea = new QScrollArea();

    metaDataArea->setWidget(widgetContainer);
    metaDataArea->setFocusProxy(widgetContainer);
    metaDataArea->setWidgetResizable(true);
    metaDataArea->setFrameShape(QFrame::NoFrame);

    connect(metaDataWidget, &Baloo::FileMetaDataWidget::metaDataRequestFinished, this, [this, metaDataArea] {
        properties->addPage(metaDataArea, i18nc("Tab page with file meta data", "&Details"));
    });
}

BalooFilePropertiesPlugin::~BalooFilePropertiesPlugin() = default;

#include "baloofilepropertiesplugin.moc"

#include "moc_baloofilepropertiesplugin.cpp"
