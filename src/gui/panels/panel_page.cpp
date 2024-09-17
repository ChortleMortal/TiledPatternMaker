#include "gui/panels/panel_page.h"
#include "sys/qt/utilities.h"
#include "sys/sys.h"
#include "gui/top/controlpanel.h"
#include "sys/tiledpatternmaker.h"
#include "gui/top/view_controller.h"

panel_page::panel_page(ControlPanel * parent, ePanelPage page,  QString name) : QWidget()
{
    pageName        = name;
    pageType        = page;
    panel           = parent;

    config          = Sys::config;
    viewControl     = Sys::viewController;
    prototypeMaker  = Sys::prototypeMaker;
    tilingMaker     = Sys::tilingMaker;
    mosaicMaker     = Sys::mosaicMaker;
    designMaker     = Sys::designMaker;

    closed          = false;
    floating        = false;
    newlySelected   = false;
    refresh         = true;
    blockCount      = 0;

    vbox            = new QVBoxLayout();
    //setFixedWidth(PANEL_RHS_WIDTH);
    setLayout (vbox);

    connect(this,   &panel_page::sig_render,          theApp,     &TiledPatternMaker::slot_render,      Qt::QueuedConnection);
    connect(this,   &panel_page::sig_reconstructView, viewControl,&ViewController::slot_reconstructView,Qt::QueuedConnection);
    connect(this,   &panel_page::sig_attachMe,        parent,     &ControlPanel::slot_reattachPage,     Qt::QueuedConnection);
    connect(this,   &panel_page::sig_updateView,      Sys::view,  &View::slot_update);
}

// pressing 'x' to close detached/floating page is used to re-attach
void panel_page::closeEvent(QCloseEvent *event)
{
    qInfo() << __FUNCTION__ << pageName;

    if (closed)
    {
        // prevents being called twice which would overwrite settings
        return;
    }

    closePage(floating);

    floating = false;

    event->setAccepted(false);

    // re-attach to stack
    emit sig_attachMe(this);
}

// called when closing the application
void panel_page::closePage(bool detached)
{
    qInfo() << __FUNCTION__ << pageName;

    QSettings s;
    QString name = QString("panel2/%1/floated").arg(pageName);
    if (detached)
    {
        s.setValue(name,true);

        name = QString("panel2/%1/pagePos").arg(pageName);
        QPoint pt = pos();
        qDebug() << pageName << pt;
        s.setValue(name,pt);

        name = QString("panel2/%1/pageSize").arg(pageName);
        QSize sz = size();
        qDebug() << pageName << sz;
        s.setValue(name,sz);
    }
    else
    {
        s.setValue(name,false);
    }
    closed = true;
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
    closed   = false;
    floating = true;

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
