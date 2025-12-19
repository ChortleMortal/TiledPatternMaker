#include "gui/top/system_view_accessor.h"
#include "sys/sys.h"
#include "gui/top/system_view.h"

SystemViewAccessor::SystemViewAccessor() {}

qreal SystemViewAccessor:: viewWidth()
{
    if (theView && Sys::isGuiThread())
        return theView->width();
    else
        return 0;
}

qreal SystemViewAccessor::viewHeight()
{
    if (theView && Sys::isGuiThread())
        return theView->height();
    else
        return 0;
}

void  SystemViewAccessor::setSize(QSize sz)
{
    if (theView && Sys::isGuiThread())
    {
        theView->setSize(sz);
    }
}

void  SystemViewAccessor::setFixedSize(QSize sz)
{
    if (theView && Sys::isGuiThread())
    {
        theView->setFixedSize(sz);
    }
}

void SystemViewAccessor::clearLayout()
{
    if (theView && Sys::isGuiThread())
    {
        theView->clearLayout();
    }
}

const QVector<Layer *> SystemViewAccessor::getActiveLayers()
{
    if (theView && Sys::isGuiThread())
    {
        return theView->getActiveLayers();
    }
    else
    {
        return QVector<Layer*>();
    }
}

void SystemViewAccessor::appSuspendPaint(bool suspend)
{
    if (theView && Sys::isGuiThread())
    {
        theView->appSuspendPaint(suspend);
    }
}

void SystemViewAccessor::debugSuspendPaint(bool suspend)
{
    if (theView && Sys::isGuiThread())
    {
        theView->debugSuspendPaint(suspend);
    }
}

void SystemViewAccessor::setPaintDisable(bool disable)
{
    if (theView && Sys::isGuiThread())
    {
        theView->setPaintDisable(disable);
    }
}

bool SystemViewAccessor::viewCanPaint()
{
    if (theView && Sys::isGuiThread())
    {
        return theView->viewCanPaint();
    }
    else
    {
        return false;
    }
}

bool SystemViewAccessor::splashCanPaint()
{
    if (theView && Sys::isGuiThread())
    {
        return  theView->splashCanPaint();
    }
    else
    {
        return false;
    }
}

QColor SystemViewAccessor::getViewBackgroundColor()
{
    if (theView && Sys::isGuiThread())
    {
        return theView->getBackgroundColor();
    }
    else
    {
        if (Sys::isDarkTheme)
            return Qt::black;
        else
            return Qt::white;
    }
}

QPixmap SystemViewAccessor::grabView()
{
    if (theView && Sys::isGuiThread())
        return theView->grab();
    else
        return QPixmap();
}



QPoint SystemViewAccessor::mapToGlobal(const QPoint & pt)
{
    if (theView && Sys::isGuiThread())
    {
        return theView->mapToGlobal(pt);
    }
    else
    {
        return pt;
    }
}

void SystemViewAccessor::showView()
{
    if (theView && Sys::isGuiThread())
    {
        theView->show();
    }
}

void SystemViewAccessor::raiseView()
{
    if (theView && Sys::isGuiThread())
    {
        theView->raise();
    }
}

QRect SystemViewAccessor::viewRect()
{
    if (theView && Sys::isGuiThread())
    {
        return  theView->rect();
    }
    else
    {
        return QRect();
    }
}

void SystemViewAccessor::setWindowTitle(const QString & s)
{
    if (theView && Sys::isGuiThread())
    {
        theView->setWindowTitle(s);
    }
}

QLayout * SystemViewAccessor::layout()
{
    if (theView && Sys::isGuiThread())
        return theView->layout();
    else
        return nullptr;
}

void SystemViewAccessor::move(const QPoint & pt)
{
    if (theView && Sys::isGuiThread())
    {
        theView->move(pt);
    }
}
void SystemViewAccessor::setWindowState(Qt::WindowStates ws)
{
    if (theView && Sys::isGuiThread())
    {
        theView->setWindowState(ws);
    }
}

Qt::WindowStates SystemViewAccessor::windowState()
{
    if (theView && Sys::isGuiThread())
    {
        return  theView->windowState();
    }
    else
    {
        return Qt::WindowNoState;
    }
}

void SystemViewAccessor::activateWindow()
{
    if (theView && Sys::isGuiThread())
    {
        theView->activateWindow();
    }
}

QWindow * SystemViewAccessor::windowHandle()
{
    if (theView && Sys::isGuiThread())
        return theView->windowHandle();
    else
        return  nullptr;
}

void SystemViewAccessor::repaintView()
{
    if (theView && Sys::isGuiThread())
    {
        theView->repaintView();
    }
}

