/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2012  Vishesh Handa <me@vhanda.in>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "filemetadataconfigwidget.h"

#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QFileDialog>

class FileMetadataWidgetTest : public QWidget
{
    Q_OBJECT
public:
    explicit FileMetadataWidgetTest(QWidget* parent = nullptr, Qt::WindowFlags f = {});

private Q_SLOTS:
    void slotChooseFiles();

private:
    Baloo::FileMetaDataConfigWidget* m_metadataWidget;
    QPushButton* m_button;
};

FileMetadataWidgetTest::FileMetadataWidgetTest(QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    m_metadataWidget = new Baloo::FileMetaDataConfigWidget( this );

    m_button = new QPushButton( QLatin1String("Select files"), this );
    connect( m_button, SIGNAL(clicked(bool)), this, SLOT(slotChooseFiles()) );

    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->addWidget( m_button );
    layout->addWidget( m_metadataWidget );
}

void FileMetadataWidgetTest::slotChooseFiles()
{
    const QList<QUrl> urlList = QFileDialog::getOpenFileUrls();
    KFileItemList list;
    for (const QUrl& url : urlList)
        list << KFileItem( url, QString(), mode_t() );

    m_metadataWidget->setItems( list );
}

int main( int argc, char** argv )
{
    QApplication app( argc, argv );
    app.setApplicationName(QStringLiteral("FileMetaDataConfigWidgetApp"));
    FileMetadataWidgetTest test;
    test.show();
    return app.exec();
}

#include "metadataconfigwidgetapp.moc"
