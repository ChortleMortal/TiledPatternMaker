#include <QThread>
#include <qscreen.h>
#include "gui/top/splash_screen.h"
#include "gui/top/system_view.h"
#include "gui/top/view_accessor.h"
#include "model/settings/configuration.h"
#include "sys/sys.h"

ViewAccessor::ViewAccessor()
{
    theView  = nullptr;
    taker    = nullptr;
}

ViewAccessor::~ViewAccessor()
{
    if (isAttached())
    {
        delete theView;
        theView = nullptr;
    }
    if (taker)
    {
        delete taker;
        taker = nullptr;
    }
}

void  ViewAccessor::attach(SystemView * view)
{
    theView = view;

    if (taker)
        delete taker;
    taker   = new Taker(view);
}

bool  ViewAccessor::isAttached()
{
    return (theView != nullptr) ;
}

const QVector<Layer *> ViewAccessor::getActiveLayers()
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

void ViewAccessor::addLayer(LayerPtr layer)
{
    if (theView && Sys::isGuiThread())
    {
        theView->addLayer(layer);
    }
}

void ViewAccessor::setPainterCrop(CropPtr crop)
{
    if (theView && Sys::isGuiThread())
    {
        theView->setPainterCrop(crop);
    }
}

qreal ViewAccessor:: viewWidth()
{
    if (theView && Sys::isGuiThread())
        return theView->width();
    else
        return 0;
}

qreal ViewAccessor::viewHeight()
{
    if (theView && Sys::isGuiThread())
        return theView->height();
    else
        return 0;
}

void  ViewAccessor::setTakerViewSize(QSize sz)
{
    if (theView && Sys::isGuiThread())
    {
        taker->useSize(sz);
    }
}

QSize ViewAccessor::getTakerViewSize()
{
    if (theView && Sys::isGuiThread())
    {
        return theView->size();
    }

    return QSize();
}

void ViewAccessor::clearLayout()
{
    if (theView && Sys::isGuiThread())
    {
        theView->clearLayout();
    }
}

void ViewAccessor::appSuspendPaint(bool suspend)
{
    if (theView && Sys::isGuiThread())
    {
        theView->appSuspendPaint(suspend);
    }
}

void ViewAccessor::debugSuspendPaint(bool suspend)
{
    if (theView && Sys::isGuiThread())
    {
        theView->debugSuspendPaint(suspend);
    }
}

void ViewAccessor::setPaintDisable(bool disable)
{
    if (theView && Sys::isGuiThread())
    {
        theView->setPaintDisable(disable);
    }
}

bool ViewAccessor::viewCanPaint()
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

QColor ViewAccessor::getViewBackgroundColor()
{
    if (theView && Sys::isGuiThread())
    {
        return theView->getWidgetBackground();
    }
    else
    {
        if (Sys::isDarkTheme)
            return Qt::black;
        else
            return Qt::white;
    }
}

QPoint ViewAccessor::mapToGlobal(const QPoint & pt)
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

void ViewAccessor::showView()
{
    if (theView && Sys::isGuiThread())
    {
        theView->show();
    }
}

void ViewAccessor::raiseView()
{
    if (theView && Sys::isGuiThread())
    {
        theView->show();
        theView->setWindowState((theView->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
        theView->raise();           // for MacOS
        theView->activateWindow();  // for Windows
    }
}

QRect ViewAccessor::getViewRect()
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

void ViewAccessor::setBkgdColor(Layer::Type vtype)
{
    setBkgdColor(Sys::config->getBkgdColor(vtype),vtype);
}

void ViewAccessor::setBkgdColor(QColor color, Layer::Type vtype)
{
    if (theView && Sys::isGuiThread())
    {
        taker->useColor(color,vtype);
    }
}

void ViewAccessor::setWindowTitle(const QString & s)
{
    if (theView && Sys::isGuiThread())
    {
        theView->setWindowTitle(s);
    }
}

QLayout * ViewAccessor::layout()
{
    if (theView && Sys::isGuiThread())
        return theView->layout();
    else
        return nullptr;
}

void ViewAccessor::move(const QPoint & pt)
{
    if (theView && Sys::isGuiThread())
    {
        theView->move(pt);
    }
}
void ViewAccessor::setWindowState(Qt::WindowStates ws)
{
    if (theView && Sys::isGuiThread())
    {
        theView->setWindowState(ws);
    }
}

Qt::WindowStates ViewAccessor::windowState()
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

void ViewAccessor::activateWindow()
{
    if (theView && Sys::isGuiThread())
    {
        theView->activateWindow();
    }
}

QWindow * ViewAccessor::windowHandle()
{
    if (theView && Sys::isGuiThread())
        return theView->windowHandle();
    else
        return  nullptr;
}

void ViewAccessor::updateView()
{
    if (theView && Sys::isGuiThread())
    {
        theView->updateView();
    }
}

void ViewAccessor::repaintView()
{
    if (theView && Sys::isGuiThread())
    {
        theView->repaintView();
    }
}

void ViewAccessor::flashView(QColor color)
{
    if (theView && Sys::isGuiThread())
    {
        auto oldColor = taker->getColor();
        auto oldUser  = taker->getColorSource();

        taker->useColor(color,oldUser);
        taker->take();
        theView->repaintView();

        auto thread = QThread::currentThread();
        thread->msleep(350);

        taker->useColor(oldColor,oldUser);
        taker->take();
        theView->repaintView();
    }
}

QImage ViewAccessor::paintToImage()
{
    if (theView && Sys::isGuiThread())
    {
        qDebug()<< "image size" << theView->size();
        QImage img(theView->size(), QImage::Format_ARGB32);
        img.setDevicePixelRatio(1.0);
        img.fill(taker->getColor());

        repaintView();  // neded for layer alignment

        QPainter painter(&img);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        theView->activeLayers.paint(&painter);

        return img;
    }
    else
    {
        return QImage();
    }
}

void ViewAccessor::unloadViewers()
{
    if (theView && Sys::isGuiThread())
    {
        theView->unloadViewers();
    }
}

void ViewAccessor::unloadContent()
{
    if (theView && Sys::isGuiThread())
    {
        theView->unloadLayerContent();
    }
}

void ViewAccessor::splash_display(QString txt)
{
    if (theView && Sys::isGuiThread())
    {
        theView->splasher->display(txt);
    }
}

void ViewAccessor::splash_replace(QString txt)
{
    if (theView && Sys::isGuiThread())
    {
        theView->splasher->replace(txt);
    }
}

void ViewAccessor::splash_remove()
{
    if (theView && Sys::isGuiThread())
    {
        theView->splasher->remove();
    }
}

void ViewAccessor::splash_disable(bool disable)
{
    if (theView && Sys::isGuiThread())
    {
        theView->splasher->disable(disable);
    }
}

Taker::Taker (SystemView * view)
{
    _view = view;
    size  = QSize(Sys::DEFAULT_WIDTH,Sys::DEFAULT_HEIGHT);
    color = Qt::black;
    colorSource = Layer::Type::Tiling;
}

void  Taker::take()
{
    _view->setSize(size);
    _view->setWidgetBackground(color);
}

void Taker::useColor(QColor c, Layer::Type type)
{
    color = c;
    colorSource = type;
}

void Taker::useSize(QSize sz)
{
    qInfo() << "Taker::useSize" << sz;
    size = sz;
}
