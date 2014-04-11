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
#include <kcomponentdata.h>
#include <kfiledialog.h>
#include <KFileItem>
#include <KPushButton>
#include <KUrl>

#include <QVBoxLayout>
#include <QCheckBox>

class FileMetadataWidgetTest : public QWidget
{
    Q_OBJECT
public:
    explicit FileMetadataWidgetTest(QWidget* parent = 0, Qt::WindowFlags f = 0);

private slots:
    void slotChooseFiles();

private:
    Baloo::FileMetaDataConfigWidget* m_metadataWidget;
    KPushButton* m_button;
};

FileMetadataWidgetTest::FileMetadataWidgetTest(QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    m_metadataWidget = new Baloo::FileMetaDataConfigWidget( this );

    m_button = new KPushButton( QLatin1String("Select files"), this );
    connect( m_button, SIGNAL(clicked(bool)), this, SLOT(slotChooseFiles()) );

    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->addWidget( m_button );
    layout->addWidget( m_metadataWidget );
}

void FileMetadataWidgetTest::slotChooseFiles()
{
    KUrl::List urlList = KFileDialog::getOpenUrls();
    KFileItemList list;
    foreach(const KFileItem& item, urlList)
        list << KFileItem( item.url(), QString(), mode_t() );

    m_metadataWidget->setItems( list );
}

int main( int argc, char** argv )
{
    QApplication app( argc, argv );
    KComponentData data( "FileMetaDataConfigWidgetApp" );
    FileMetadataWidgetTest test;
    test.show();
    return app.exec();
}

#include "metadataconfigwidgetapp.moc"
