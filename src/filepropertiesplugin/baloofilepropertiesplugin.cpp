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

#include "filemetadataprovider.h"
#include "filemetadatawidget.h"
#include "filemetadatawidget_p.h"
#include "metadatafilter.h"

K_PLUGIN_CLASS_WITH_JSON(BalooFilePropertiesPlugin, "baloofilepropertiesplugin.json")

// Filters out all kfileitem# properties since this information
// is already available on the "General" tab of the properties dialog.
class FilePropertiesMetadataFilter : public Baloo::MetadataFilter
{
public:
    FilePropertiesMetadataFilter()
        : Baloo::MetadataFilter()
    {
    }

    QVariantMap filter(const QVariantMap &data) override
    {
        QVariantMap filteredData = data;

        auto it = filteredData.begin();
        while (it != filteredData.end()) {
            if (it.key().startsWith(QLatin1String("kfileitem#"))) {
                it = filteredData.erase(it);
            } else {
                ++it;
            }
        }

        return Baloo::MetadataFilter::filter(filteredData);
    }
};

BalooFilePropertiesPlugin::BalooFilePropertiesPlugin(QObject *parent, const QList<QVariant> &args)
    : KPropertiesDialogPlugin(qobject_cast<KPropertiesDialog *>(parent))
{
    Q_UNUSED(args);

    auto widgetContainer = new QWidget();

    auto containerLayout = new QVBoxLayout(widgetContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);

    auto metaDataWidget = new Baloo::FileMetaDataWidget();
    // Shove in our own filter.
    Baloo::FileMetaDataWidgetPrivate::get(metaDataWidget)->m_filter.reset(new FilePropertiesMetadataFilter);
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
    metaDataArea->setWidgetResizable(true);
    metaDataArea->setFrameShape(QFrame::NoFrame);

    connect(metaDataWidget, &Baloo::FileMetaDataWidget::metaDataRequestFinished, this, [this, metaDataArea, metaDataWidget] {
        auto *metaDataWidgetPrivate = Baloo::FileMetaDataWidgetPrivate::get(metaDataWidget);

        // Only add "Details" page if we found any properties.
        const auto data = metaDataWidgetPrivate->m_filter->filter(metaDataWidgetPrivate->m_provider->data());
        if (!data.isEmpty()) {
            properties->addPage(metaDataArea, i18nc("Tab page with file meta data", "&Details"));
        }
    });
}

BalooFilePropertiesPlugin::~BalooFilePropertiesPlugin() = default;

#include "baloofilepropertiesplugin.moc"
