#include "gui/panels/panel_page.h"
#include "sys/qt/utilities.h"
#include "sys/sys.h"
#include "gui/top/controlpanel.h"
#include "sys/tiledpatternmaker.h"
#include "gui/top/system_view_controller.h"


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

    connect(this,   &panel_page::sig_reconstructView, viewControl, &SystemViewController::slot_reconstructView,Qt::QueuedConnection);
    connect(this,   &panel_page::sig_updateView,      viewControl, &SystemViewController::slot_updateView);
    connect(this,   &panel_page::sig_attachMe,        parent,      &ControlPanel::slot_reattachPage,     Qt::QueuedConnection);
    connect(panel,  &ControlPanel::sig_raiseDetached, this,        &panel_page::slot_raiseDetached);
}

// pressing 'x' to close detached/floating page is used to re-attach
void panel_page::closeEvent(QCloseEvent *event)
{
    qInfo() << "panel_page::closeEvent" << pageName;

    if (closed)
    {
        // prevents being called twice which would overwrite settings
        return;
    }

    closePage();

    floating = false;

    event->setAccepted(false);  // dont close

    // re-attach to stack
    emit sig_attachMe(this);
}

// called when closing the application
void panel_page::closePage()
{
    //qInfo() << "panel_page::closePage" << pageName;

    QSettings s;
    QString name = QString("panel2/%1/pageState").arg(pageName);
    s.setValue(name,sPageState[pageState]);

    switch (pageState)
    {
    case PAGE_DETACHED:
    {
        name = QString("panel2/%1/pagePos").arg(pageName);
        QPoint pt = pos();
        qDebug() << pageName << pt;
        s.setValue(name,pt);

        name = QString("panel2/%1/pageSize").arg(pageName);
        QSize sz = size();
        qDebug() << pageName << sz;
        s.setValue(name,sz);

    }    break;

    case PAGE_SUB_ATTACHED:
    case PAGE_ATTACHED:
        break;
    }

    closed = true;
}

bool panel_page::wasFloated()
{
    QSettings s;
    QString name       = QString("panel2/%1/pageState").arg(pageName);
    QString wasFloated = s.value(name,false).toString();
    return (wasFloated == "PAGE_DETACHED");
}

bool panel_page::wasSubAttached()
{
    QSettings s;
    QString name       = QString("panel2/%1/pageState").arg(pageName);
    QString wasFloated = s.value(name,false).toString();
    return (wasFloated == "PAGE_SUB_ATTACHED");
}

void panel_page::detach(QString tname)
{
    setParent(nullptr);     // this detaches

    setWindowTitle(tname);

    closed   = false;
    floating = true;

    QSettings s;

    QString name = QString("panel2/%1/pagePos").arg(pageName);
    QPoint pt    = s.value(name,QPointF()).toPoint();
    if (!pt.isNull())
    {
        qDebug() << "panel_page::floatMe()" << pageName << pt;
        QScreen *screenAtOrigin = QGuiApplication::screenAt(pt);
        QScreen *currentScreen = screen();
        if (screenAtOrigin != currentScreen)
        {
            pt = QCursor::pos();
        }
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
    else
    {
        adjustSize();
    }
    show();
}

void panel_page::slot_raiseDetached()
{
    if (isFloating())
    {
        raise();
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

void panel_page::enterEvent(QEnterEvent *event)
{
    Q_UNUSED(event)
    mouseEnter();
}

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

bool panel_page::pageBlocked()
{
    return (blockCount != 0);
}

void panel_page::setPageStatus()
{
    panel->setPageStatus(pageType,pageStatusString);
}

void panel_page::clearPageStatus()
{
    panel->clearPageStatus(pageType);
}
