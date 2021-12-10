/*
    SPDX-FileCopyrightText: 2012 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "filemetadataconfigwidget.h"

#include <QApplication>
#include <QCheckBox>
#include <QFileDialog>
#include <QPushButton>
#include <QVBoxLayout>

class FileMetadataWidgetTest : public QWidget
{
    Q_OBJECT
public:
    explicit FileMetadataWidgetTest(QWidget *parent = nullptr, Qt::WindowFlags f = {});

private Q_SLOTS:
    void slotChooseFiles();

private:
    Baloo::FileMetaDataConfigWidget *m_metadataWidget;
    QPushButton *m_button;
};

FileMetadataWidgetTest::FileMetadataWidgetTest(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    m_metadataWidget = new Baloo::FileMetaDataConfigWidget(this);

    m_button = new QPushButton(QLatin1String("Select files"), this);
    connect(m_button, SIGNAL(clicked(bool)), this, SLOT(slotChooseFiles()));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_button);
    layout->addWidget(m_metadataWidget);
}

void FileMetadataWidgetTest::slotChooseFiles()
{
    const QList<QUrl> urlList = QFileDialog::getOpenFileUrls();
    KFileItemList list;
    for (const QUrl &url : urlList)
        list << KFileItem(url, QString(), mode_t());

    m_metadataWidget->setItems(list);
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("FileMetaDataConfigWidgetApp"));
    FileMetadataWidgetTest test;
    test.show();
    return app.exec();
}

#include "metadataconfigwidgetapp.moc"
