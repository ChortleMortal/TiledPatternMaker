#pragma once

#include <QColor>
#include <QSize>
#include <QPixmap>
#include <QLayout>
#include <qdebug.h>
#include "gui/viewers/layer.h"

class SystemView;
class Taker;

typedef std::shared_ptr<class Layer>  LayerPtr;
typedef std::shared_ptr<class Crop>   CropPtr;
class Canvas;

class ViewAccessor
{
public:
    void    attach(SystemView * view);
    bool    isAttached();

    QWidget    * getWidget() { return (QWidget*)theView; }   // NOTE this bypasses everything to get at the widget
    SystemView * getView()   { return theView; }             // NOTE this bypasses everything to oget at the widget
    const Taker * getTaker() { return taker; }

    void    showView();
    void    raiseView();
    void    updateView();
    void    repaintView();
    void    flashView(QColor color);

    QImage  paintToImage();

    void    setTakerViewSize(QSize sz);
    void    setWindowTitle(const QString & s);

    QSize   getTakerViewSize();
    QRect   getViewRect();

    void    setBkgdColor(Layer::Type vtype);
    void    setBkgdColor(QColor color, Layer::Type vtype);
    QColor  getViewBackgroundColor();

    const QVector<Layer *> getActiveLayers();

    QLayout *  layout();        // used for PNGs
    void    clearLayout();      // used for PNGS
    void    activateWindow();

    QPoint  mapToGlobal(const QPoint & pt);

    void    appSuspendPaint(bool suspend);
    void    debugSuspendPaint(bool suspend);

    void    splash_display(QString txt);
    void    splash_replace(QString txt);
    void    splash_remove();
    void    splash_disable(bool disable);

protected:
    ViewAccessor();
    ~ViewAccessor();

    void  addLayer(LayerPtr layer);
    void  setPainterCrop(CropPtr crop);

    void  setPaintDisable(bool disable);
    bool  viewCanPaint();

    void  unloadViewers();
    void  unloadContent();

    qreal viewWidth();
    qreal viewHeight();

    void  move(const QPoint & pt);
    void  setWindowState(Qt::WindowStates ws);
    Qt::WindowStates windowState();
    QWindow * windowHandle();

    Taker      * taker;

private:
    SystemView * theView;
};

class Taker
{
public:
    Taker(SystemView * view);

    void        take();

    void        useColor(QColor c, Layer::Type type);
    void        useSize(QSize sz);

    QSize       getSize()  const        { return size; }
    QColor      getColor() const        { return color; }
    Layer::Type getColorSource()  const { return colorSource; }

private:
    SystemView * _view;

    QSize       size;
    QColor      color;
    Layer::Type colorSource;
};