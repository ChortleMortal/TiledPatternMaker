#pragma once
#ifndef VIEW_H
#define VIEW_H

#include <QWidget>
#include <QElapsedTimer>

#include "sys/enums/eviewtype.h"
#include "sys/geometry/crop.h"
#include "gui/top/system_view_controller.h"

typedef std::shared_ptr<class Layer> LayerPtr;
typedef std::weak_ptr<class Layer>   WeakLayerPtr;
typedef std::weak_ptr<Crop>          WeakCropPtr;

class SystemViewController;

#define NO_CHANGE       0x0
#define TOP_CHANGED     0x1
#define RIGHT_CHANGED   0x2
#define BOTTOM_CHANGED  0x4
#define LEFT_CHANGED    0x8

class ActiveLayers
{
public:
    void    add(LayerPtr layer);
    void    clear();
    void    unloadContent();
    bool    contains(eViewType type);
    Layer * get(eViewType type);
    int     size() { return layers.count(); }

    const QVector<Layer *> get();

    void    paint(QPainter & painter);

private:
    QVector<WeakLayerPtr> layers;
};


class SystemView : public QWidget
{
    Q_OBJECT
    
    friend SystemViewAccessor;
    friend SystemViewController;

public:
    SystemView();
    ~SystemView() override;
    
    void    init(SystemViewController * parent);

    void    flash(QColor color); // called by tiling editor (gui thread)

signals:
    void sig_close();
    void sig_viewSizeChanged(QSize oldSize, QSize newSize);
    void sig_viewMoved();
    void sig_testSize();

    void sig_mousePressed(QPointF pos,Qt::MouseButton);
    void sig_mouseDragged(QPointF pos);
    void sig_mouseMoved(QPointF pos);
    void sig_mouseReleased(QPointF pos);
    void sig_mouseDoublePressed(QPointF pos,Qt::MouseButton);

    void sig_mouseTranslate(uint sigid, QPointF pt);
    void sig_wheel_scale( uint sigid, qreal angle);
    void sig_wheel_rotate(uint sigid, qreal angle);

    void sig_deltaRotate(uint sigid, int amount);
    void sig_deltaMoveY( uint sigid, int amount);
    void sig_deltaMoveX( uint sigid, int amount);
    void sig_deltaScale( uint sigid,  int amount);

    void sig_raiseMenu();
    void sig_rebuildMotif();

    void sig_stepperEnd();
    void sig_stepperPause();
    void sig_stepperKey(int key);

    void sig_saveImage();
    void sig_saveMenu();
    void sig_saveSVG();
    void sig_print();

    void sig_loadComplete();

    void sig_messageBox(QString msg, QString msg2);

public slots:
    void    testSize();

protected:
    // called by system controller - these are not threaad safe
    void    addLayer(LayerPtr layer)        { activeLayers.add(layer); }
    void    unloadLayerContent()            { activeLayers.unloadContent(); }
    bool    isActiveLayer(eViewType type)   { return activeLayers.contains(type); }
    Layer * getActiveLayer(eViewType type)  { return activeLayers.get(type); }
    const QVector<Layer*> getActiveLayers() { return activeLayers.get(); }

    void    appSuspendPaint(bool suspend);
    void    setPaintDisable(bool disable);  // calls viewSuspendPaint
    bool    viewCanPaint();
    void    debugSuspendPaint(bool suspend);
    bool    splashCanPaint();
    void    viewSuspendPaint(bool suspend);

    QColor  getBackgroundColor();
    void    setBackgroundColor(QColor color);

    void    updateView()                            { QWidget::update(); }
    void    repaintView()                           { QWidget::repaint(); }
    void    repaint()                               { qFatal("Dont call repaint() directly - use repaintView()"); }
    void    unloadViewers();
    void    raiseView();

    void    setFixedSize(QSize sz);
    void    setSize(QSize sz);

    void    setPainterClip(CropPtr crop)            { _painterCrop = crop; }
    CropPtr getPainterClip()                        { return _painterCrop.lock(); }

    void    clearLayout(); // only used by cycler for pngs
    void    clearLayout(QLayout* layout, bool deleteWidgets = true);
    void    clearLayers()                           { activeLayers.clear(); }

    void    processLoadState(class LoadUnit * loadUnit);

    void    setWindowTitle(const QString & s);
    QPoint  mapToGlobal(const QPoint & pt) const    { return QWidget::mapToGlobal(pt); }
    QRect   rect()                                  { return QWidget::rect();}
    void    show()                                  { QWidget::show(); }
    void    raise()                                 { QWidget::raise(); }
    qreal   width()                                 { return QWidget::width(); }
    qreal   height()                                { return QWidget::height(); }
    QSize   size()                                  { return QWidget::size(); }
    void    resize(QSize sz)                        { Q_UNUSED(sz); qFatal("Dont call resize() directly - use setSize()"); }

    QPixmap grab()                                  { return QWidget::grab(); }
    QLayout *  layout()                             { return QWidget::layout(); }
    void    move(const QPoint & pt)                 { QWidget::move(pt); }
    void    setWindowState(Qt::WindowStates ws)     { QWidget::setWindowState(ws); }
    Qt::WindowStates windowState()                  { return QWidget::windowState(); }
    void    activateWindow()                        { QWidget::activateWindow(); }
    QWindow * windowHandle()                        { return QWidget::windowHandle(); }


    void    duplicateView();        // FIXME move and better hookup

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

    bool procKeyEvent(      QKeyEvent * k);
    bool procLegacyKeyEvent(QKeyEvent * k);
    bool procCommonKey(     QKeyEvent * k);
    bool procNavigationKey( QKeyEvent * k);

    void procKeyLeft( int delta);
    void procKeyRight(int delta);
    void procKeyDown( int delta);
    void procKeyUp(   int delta);

private:
    SystemViewController * parent;

    ActiveLayers    activeLayers;
    WeakCropPtr     _painterCrop;   // used for painter clipping

    uint        _suspendPaintApp;
    uint        _suspendPaintDebug;
    uint        _suspendPaintView;
    QPointF     _tl;                // top left of view rect (screen pos)
    QPointF     _br;                // bottom right of view rect (scren pos)

    QPointF     sLast;              // used by pan
    bool        isShown;
    bool        dragging;
    QColor      backgroundColor;
    QSize       requestedSize;
    bool        constrained;        // constrained to fixed size
};

#endif // VIEW_H
