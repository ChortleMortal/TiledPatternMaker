#pragma once
#ifndef VIEW_H
#define VIEW_H

#include <QWidget>
#include <QElapsedTimer>

#include "sys/qt/unique_qvector.h"
#include "sys/enums/eviewtype.h"
#include "sys/geometry/crop.h"
#include "gui/top/view_controller.h"

typedef std::shared_ptr<class Layer> LayerPtr;
typedef std::weak_ptr<Crop>          WeakCropPtr;

class ViewController;

#define NO_CHANGE       0x0
#define TOP_CHANGED     0x1
#define RIGHT_CHANGED   0x2
#define BOTTOM_CHANGED  0x4
#define LEFT_CHANGED    0x8

class ActiveLayers
{
public:
    void    add(Layer * layer);
    void    clear();
    bool    contains(eViewType type);
    Layer * get(eViewType type);
    int     size() { return layers.count(); }

    const QVector<Layer*> get();

    void    paint(QPainter & painter);

private:
    UniqueQVector<Layer*> layers;
    QMutex  mutex;
};

class View : public QWidget
{
    Q_OBJECT
    
    friend void addLayer(View * view, Layer *  layer);
    friend void addLayer(View * view, LayerPtr layer);
    friend void unloadView(View * view);
    friend void setPaintDisable(View * view, bool disable);
    friend void setViewBackgroundColor(View * view, QColor color);
    friend void setClip(View * view, CropPtr crop);
    friend void setFixedSize(View * view, QSize size);
    friend void setSize(View * view, QSize size);
    friend void clearLayout(View* view);

public:
    View();
    ~View() override;
    
    void    init(ViewController * parent);
    void    setWindowTitle(const QString & s);

    bool    isActiveLayer(eViewType type);  // not thread safe
    Layer * getActiveLayer(eViewType type);
    const QVector<Layer *> getActiveLayers(){ return activeLayers.get(); }

    QSize   getSize()                       { return _viewSize; }
    QColor  getViewBackgroundColor();

    bool    viewCanPaint();
    bool    splashCanPaint();
    void    setPaintDisable(bool disable);  // calls viewSuspendPaint
    void    appSuspendPaint(bool suspend);
    void    debugSuspendPaint(bool suspend);

    void    flash(QColor color);

protected:
    void    unloadView();

    void    setFixedSize(QSize sz);
    void    setSize(QSize sz);
    void    resize(QSize sz)                { Q_UNUSED(sz); qFatal("Dont call resize() directly - use setSize()"); }

    void    addLayer(LayerPtr layer);
    void    addLayer(Layer * layer);
    void    clearLayers()                   { activeLayers.clear(); }

    void    setViewBackgroundColor(QColor color);

    void    setClip(CropPtr crop)           { _painterCrop = crop; }
    CropPtr getClip()                       { return _painterCrop.lock(); }

    void    clearLayout(); // only used by cycler for pngs

    void    viewSuspendPaint(bool suspend);

    void    duplicateView();

public slots:
    void    slot_update();
    void    slot_repaint();
    void    slot_raiseView();

signals:
    void sig_close();
    void sig_viewSizeChanged(QSize oldSize, QSize newSize);
    void sig_viewMoved();

    void sig_mousePressed(QPointF pos,Qt::MouseButton);
    void sig_mouseDragged(QPointF pos);
    void sig_mouseTranslate(QPointF pt);
    void sig_mouseMoved(QPointF pos);
    void sig_mouseReleased(QPointF pos);
    void sig_mouseDoublePressed(QPointF pos,Qt::MouseButton);

    void sig_wheel_scale(qreal angle);
    void sig_wheel_rotate(qreal angle);

    void sig_deltaRotate(int amount);
    void sig_deltaMoveY(int amount);
    void sig_deltaMoveX(int amount);
    void sig_deltaScale(int amount);

    void sig_setCenter(QPointF pos);

    void sig_raiseMenu();
    void sig_rebuildMotif();

    void sig_stepperEnd();
    void sig_stepperPause();
    void sig_stepperKey(int key);

    void sig_saveImage();
    void sig_saveMenu();
    void sig_saveSVG();

    void sig_messageBox(QString msg, QString msg2);

protected:
    void paintEvent(QPaintEvent *event) override;

    void showEvent(QShowEvent * event) override;
    void closeEvent(QCloseEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent( QKeyEvent *k ) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent * event) override;
    void wheelEvent(QWheelEvent *event) override;

    void clearLayout(QLayout* layout, bool deleteWidgets = true);

    bool procKeyEvent(QKeyEvent * k);
    bool ProcKey(QKeyEvent *k);
    bool ProcNavKey(int key, int multiplier);

    void ProcKeyLeft( int delta);

    void ProcKeyRight(int delta);
    void ProcKeyDown( int delta);
    void ProcKeyUp(   int delta);

    class TilingMaker * tilingMaker;

    void    update()    { qFatal("Cannot call View::update directly"); }
    void    repaint()   { qFatal("Cannot call View::repaint directly"); }
    
private:
    class Configuration * config;
    class DesignMaker   * designMaker;
    class ControlPanel  * panel;
    ViewController      * viewControl;

    QSize               _viewSize;    // the main window size

    ActiveLayers      activeLayers;

    uint              _suspendPaintApp;
    uint              _suspendPaintDebug;
    uint              _suspendPaintView;

    bool              isShown;


    bool              dragging;
    QColor            backgroundColor;

    QPointF           sLast;    // used by pan

    QPointF           _tl;      // top left of view rect (screen pos)
    QPointF           _br;      // bottom right of view rect (scren pos)

    WeakCropPtr       _painterCrop;   // used for painter clipping
};

#endif // VIEW_H
