/*
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2009 Peter Penz <peter.penz@gmx.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "filemetadataconfigwidget.h"
#include "filemetadataprovider.h"

#include <KConfig>
#include <KConfigGroup>

#include <QDebug>
#include <QEvent>
#include <QListWidget>
#include <QVBoxLayout>

using namespace Baloo;

class Baloo::FileMetaDataConfigWidgetPrivate
{
public:
    explicit FileMetaDataConfigWidgetPrivate(FileMetaDataConfigWidget *parent);
    ~FileMetaDataConfigWidgetPrivate();

    FileMetaDataConfigWidgetPrivate(const FileMetaDataConfigWidgetPrivate&) = delete;
    FileMetaDataConfigWidget& operator=(const FileMetaDataConfigWidgetPrivate&) = delete;

    void init();
    void loadMetaData();
    void addItem(const QString &property);

    /**
     * Is invoked after the meta data model has finished the loading of
     * meta data. The meta data labels will be added to the configuration
     * list.
     */
    void slotLoadingFinished();

    int m_visibleDataTypes;
    KFileItemList m_fileItems;
    FileMetaDataProvider *m_provider;
    QListWidget *m_metaDataList;

private:
    FileMetaDataConfigWidget *const q;
};

FileMetaDataConfigWidgetPrivate::FileMetaDataConfigWidgetPrivate(FileMetaDataConfigWidget *parent)
    : m_visibleDataTypes(0)
    , m_fileItems()
    , m_provider(nullptr)
    , m_metaDataList(nullptr)
    , q(parent)
{
    m_metaDataList = new QListWidget(q);
    m_metaDataList->setSelectionMode(QAbstractItemView::NoSelection);
    m_metaDataList->setSortingEnabled(true);

    auto layout = new QVBoxLayout(q);
    layout->addWidget(m_metaDataList);

    m_provider = new FileMetaDataProvider(q);
    m_provider->setReadOnly(true);
    QObject::connect(m_provider, SIGNAL(loadingFinished()), q, SLOT(slotLoadingFinished()));
}

FileMetaDataConfigWidgetPrivate::~FileMetaDataConfigWidgetPrivate() = default;

void FileMetaDataConfigWidgetPrivate::loadMetaData()
{
    m_metaDataList->clear();
    m_provider->setItems(m_fileItems);
}

void FileMetaDataConfigWidgetPrivate::addItem(const QString &key)
{
    // Meta information provided by Baloo that is already
    // available from KFileItem as "fixed item" (see above)
    // should not be shown as second entry.
    static const char *const hiddenProperties[] = {
        "comment", // = fixed item kfileitem#comment
        "contentSize", // = fixed item kfileitem#size

        nullptr // mandatory last entry
    };

    int i = 0;
    while (hiddenProperties[i] != nullptr) {
        if (key == QLatin1String(hiddenProperties[i])) {
            // the item is hidden
            return;
        }
        ++i;
    }

    // the item is not hidden, add it to the list
    KConfig config(QStringLiteral("baloofileinformationrc"), KConfig::NoGlobals);
    KConfigGroup settings = config.group("Show");

    const QString label = m_provider->label(key);

    auto item = new QListWidgetItem(label, m_metaDataList);
    item->setData(Qt::UserRole, key);
    const bool show = settings.readEntry(key, true);
    item->setCheckState(show ? Qt::Checked : Qt::Unchecked);
}

void FileMetaDataConfigWidgetPrivate::slotLoadingFinished()
{
    // Get all meta information labels that are available for
    // the currently shown file item and add them to the list.
    Q_ASSERT(m_provider != nullptr);

    m_metaDataList->clear();

    QVariantMap data = m_provider->data();
    // Always show these 3
    data.remove(QStringLiteral("rating"));
    data.remove(QStringLiteral("tags"));
    data.remove(QStringLiteral("userComment"));

    QVariantMap::const_iterator it = data.constBegin();
    while (it != data.constEnd()) {
        addItem(it.key());
        ++it;
    }

    addItem(QStringLiteral("rating"));
    addItem(QStringLiteral("tags"));
    addItem(QStringLiteral("userComment"));
}

FileMetaDataConfigWidget::FileMetaDataConfigWidget(QWidget *parent)
    : QWidget(parent)
    , d(new FileMetaDataConfigWidgetPrivate(this))
{
}

FileMetaDataConfigWidget::~FileMetaDataConfigWidget() = default;

void FileMetaDataConfigWidget::setItems(const KFileItemList &items)
{
    d->m_fileItems = items;
    d->loadMetaData();
}

KFileItemList FileMetaDataConfigWidget::items() const
{
    return d->m_fileItems;
}

void FileMetaDataConfigWidget::save()
{
    KConfig config(QStringLiteral("baloofileinformationrc"), KConfig::NoGlobals);
    KConfigGroup showGroup = config.group("Show");

    const int count = d->m_metaDataList->count();
    for (int i = 0; i < count; ++i) {
        QListWidgetItem *item = d->m_metaDataList->item(i);
        const bool show = (item->checkState() == Qt::Checked);
        const QString key = item->data(Qt::UserRole).toString();
        showGroup.writeEntry(key, show);
    }

    showGroup.sync();
}

bool FileMetaDataConfigWidget::event(QEvent *event)
{
    if (event->type() == QEvent::Polish) {
        qDebug() << "GOT POLISH EVENT!!!";
        // loadMetaData() must be invoked asynchronously, as the list
        // must finish it's initialization first
        QMetaObject::invokeMethod(this, "loadMetaData", Qt::QueuedConnection);
    }
    return QWidget::event(event);
    ;
}

QSize FileMetaDataConfigWidget::sizeHint() const
{
    return d->m_metaDataList->sizeHint();
}

#include "moc_filemetadataconfigwidget.cpp"
