#ifndef TAGWIDGETTEST_H
#define TAGWIDGETTEST_H

#include <QWidget>
#include <Nepomuk2/Tag>
#include "tagwidget.h"

class TagWidgetTest : public QWidget
{
    Q_OBJECT

public:
    TagWidgetTest();
    ~TagWidgetTest();

public slots:
    void slotTagClicked(const Nepomuk2::Tag&);
    void slotSelectionChanged( const QList<Nepomuk2::Tag>& tags );

private slots:
    void enableMinimode( bool enable );
    void alignRight( bool enable );
    void disableClicking( bool enable );
    void setReadOnly( bool enable );

private:
    Nepomuk2::TagWidget* m_tagWidget;
};

#endif
