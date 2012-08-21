#include "tagwidgettest.h"
#include "tagwidget.h"

#include <QVBoxLayout>
#include <QCheckBox>
#include <kdebug.h>


TagWidgetTest::TagWidgetTest()
    : QWidget()
{
    m_tagWidget = new Nepomuk2::TagWidget(this);
    m_tagWidget->setMaxTagsShown(8);
    QVBoxLayout* lay = new QVBoxLayout(this);
    lay->addWidget(m_tagWidget);
    connect(m_tagWidget, SIGNAL(tagClicked(Nepomuk2::Tag)),
            this, SLOT(slotTagClicked(Nepomuk2::Tag)));
    connect(m_tagWidget, SIGNAL(selectionChanged(QList<Nepomuk2::Tag>)),
            this, SLOT(slotSelectionChanged(QList<Nepomuk2::Tag>)));

    QCheckBox* box = new QCheckBox( "Minimode", this );
    connect(box, SIGNAL(toggled(bool)), this, SLOT(enableMinimode(bool)));
    lay->addWidget(box);

    box = new QCheckBox( "Align Right", this );
    connect(box, SIGNAL(toggled(bool)), this, SLOT(alignRight(bool)));
    lay->addWidget(box);

    box = new QCheckBox( "Disable clicking", this );
    connect(box, SIGNAL(toggled(bool)), this, SLOT(disableClicking(bool)));
    lay->addWidget(box);

    box = new QCheckBox( "Read only", this );
    connect(box, SIGNAL(toggled(bool)), this, SLOT(setReadOnly(bool)));
    lay->addWidget(box);
}

TagWidgetTest::~TagWidgetTest()
{
}


void TagWidgetTest::slotTagClicked(const Nepomuk2::Tag& tag)
{
    kDebug() << "Tag clicked:" << tag.uri() << tag.genericLabel();
}


void TagWidgetTest::slotSelectionChanged( const QList<Nepomuk2::Tag>& tags )
{
    QStringList ts;
    foreach(const Nepomuk2::Tag& tag, tags)
        ts << tag.genericLabel();
    kDebug() << "Selection changed:" << ts;
}


void TagWidgetTest::enableMinimode( bool enable )
{
    Nepomuk2::TagWidget::ModeFlags flags = m_tagWidget->modeFlags();
    if( enable ) {
        flags |= Nepomuk2::TagWidget::MiniMode;
        flags &= ~Nepomuk2::TagWidget::StandardMode;
    }
    else {
        flags |= Nepomuk2::TagWidget::StandardMode;
        flags &= ~Nepomuk2::TagWidget::MiniMode;
    }
    m_tagWidget->setModeFlags( flags );
}


void TagWidgetTest::alignRight( bool enable )
{
    if( enable )
        m_tagWidget->setAlignment( Qt::AlignRight );
    else
        m_tagWidget->setAlignment( Qt::AlignLeft );
}


void TagWidgetTest::disableClicking( bool enable )
{
    Nepomuk2::TagWidget::ModeFlags flags = m_tagWidget->modeFlags();
    m_tagWidget->setModeFlags( enable ? flags | Nepomuk2::TagWidget::DisableTagClicking : flags & ~Nepomuk2::TagWidget::DisableTagClicking );
}


void TagWidgetTest::setReadOnly( bool enable )
{
    Nepomuk2::TagWidget::ModeFlags flags = m_tagWidget->modeFlags();
    m_tagWidget->setModeFlags( enable ? flags | Nepomuk2::TagWidget::ReadOnly : flags & ~Nepomuk2::TagWidget::ReadOnly );
}

#include "tagwidgettest.moc"
