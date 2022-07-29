#include "widgets/panel_page.h"
#include "panels/panel.h"
#include "tiledpatternmaker.h"
#include "misc/utilities.h"
#include "viewers/viewcontrol.h"
#include "makers/motif_maker/motif_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "settings/configuration.h"

panel_page::panel_page(ControlPanel * panel,  QString name) : QWidget()
{
    pageName    = name;

    this->panel     = panel;
    config          = Configuration::getInstance();
    view            = ViewControl::getInstance();
    motifMaker      = MotifMaker::getInstance();
    tilingMaker     = TilingMaker::getSharedInstance();
    mosaicMaker     = MosaicMaker::getInstance();

    vbox = new QVBoxLayout;
    vbox->setSizeConstraint(QLayout::SetFixedSize);

    newlySelected   = false;
    floated         = false;
    refresh         = true;
    blockCount      = 0;

    setLayout (vbox);

    connect(this,   &panel_page::sig_render,     theApp,   &TiledPatternMaker::slot_render);
    connect(this,   &panel_page::sig_refreshView,view,     &ViewControl::slot_refreshView);
    connect(this,   &panel_page::sig_attachMe,   panel,    &ControlPanel::slot_attachWidget, Qt::QueuedConnection);
}

// pressing 'x' to close detached/floating page is used to re-attach
void panel_page::closeEvent(QCloseEvent *event)
{
    setParent(nullptr);

    QSettings s;
    QString name = QString("panel2/%1/pagePos").arg(pageName);
    QPoint pt = pos();
    //qDebug() << "panel_page close event pos = " << pt;
    s.setValue(name,pt);

    event->setAccepted(false);

    floated = false;

    // re-attach to stack
    emit sig_attachMe(pageName);
}

// called when closing the application
void panel_page::closePage()
{
    QSettings s;
    QString name = QString("panel2/%1/floated").arg(pageName);
    if (floated)
    {
        s.setValue(name,true);

        name = QString("panel2/%1/pagePos").arg(pageName);
        QPoint pt = pos();
        //qDebug() << "panel_page close event pos = " << pt;
        s.setValue(name,pt);

    }
    else
    {
        s.setValue(name,false);
    }
}

bool panel_page::wasFloated()
{
    QSettings s;
    QString name = QString("panel2/%1/floated").arg(pageName);
    bool wasFloated = s.value(name,false).toBool();
    return wasFloated;
}

void panel_page::floatMe()
{
    QSettings s;
    QString name = QString("panel2/%1/pagePos").arg(pageName);
    QPoint pt    = s.value(name).toPoint();
    qDebug() << "panel_page::floatMe()" << pageName << pt;
    move(pt);
}

QString panel_page::addr(void * address)
{
    return Utils::addr(address);
}

QString panel_page::addr(const void * address)
{
    return QString::number(reinterpret_cast<uint64_t>(address),16);
}

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
void panel_page::enterEvent(QEnterEvent *event)
{
    Q_UNUSED(event)
    mouseEnter();
}
#else
void panel_page::enterEvent(QEvent *event)
{
    Q_UNUSED(event)
    mouseEnter();
}
#endif

void panel_page::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    mouseLeave();
}

void  panel_page::blockPage(bool block)
{
    if (block)
    {
        blockCount++;
    }
    else
    {
        blockCount--;
        Q_ASSERT(blockCount >= 0);
    }
}

bool  panel_page::pageBlocked()
{
    return (blockCount != 0);
}

void panel_page::updateView()
{
    view->update();
}
