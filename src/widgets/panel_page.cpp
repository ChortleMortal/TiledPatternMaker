#include "widgets/panel_page.h"
#include "legacy/design_maker.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "makers/prototype_maker/prototype_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "misc/utilities.h"
#include "panels/panel.h"
#include "settings/configuration.h"
#include "tiledpatternmaker.h"
#include "viewers/viewcontrol.h"

panel_page::panel_page(ControlPanel * panel,  QString name) : QWidget()
{
    pageName    = name;

    this->panel     = panel;
    config          = Configuration::getInstance();
    view            = ViewControl::getInstance();
    prototypeMaker  = PrototypeMaker::getInstance();
    tilingMaker     = TilingMaker::getSharedInstance();
    mosaicMaker     = MosaicMaker::getInstance();
    designMaker     = DesignMaker::getInstance();

    newlySelected   = false;
    floated         = false;
    refresh         = true;
    blockCount      = 0;

    vbox            = new QVBoxLayout;
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
    qDebug() << "panel_page closeEvent pos = " << pt;
    s.setValue(name,pt);

    name = QString("panel2/%1/pageSize").arg(pageName);
    QSize sz = size();
    qDebug() << "panel_page closeEvent size = " << sz;
    s.setValue(name,sz);

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
        qDebug() << "panel_page closePage pos = " << pt;
        s.setValue(name,pt);

        name = QString("panel2/%1/pageSize").arg(pageName);
        QSize sz = size();
        qDebug() << "panel_page closePage size = " << sz;
        s.setValue(name,sz);
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
    QPoint pt    = s.value(name,QPointF()).toPoint();
    if (!pt.isNull())
    {
        qDebug() << "panel_page::floatMe()" << pageName << pt;
    }
    else
    {
        pt = QCursor::pos();
        qDebug() << "floating to cursor pos =" << pt;
    }
    move(pt);

    name = QString("panel2/%1/pageSize").arg(pageName);
    QSize sz = s.value(name,QSize()).toSize();
    if (sz.isValid())
    {
        qDebug() << "resizing to" << sz;
        resize(sz);
    }
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