#pragma once

#include <QMessageBox>
#include <QWidget>
#include <QElapsedTimer>

#include "gui/viewers/layer.h"
#include "sys/geometry/crop.h"

class Canvas;
typedef std::shared_ptr<class Layer>  LayerPtr;
typedef std::weak_ptr<class Layer>    WeakLayerPtr;
typedef std::weak_ptr<Crop>           WeakCropPtr;

class ViewController;
class SystemView;
class SplashScreen;

#define NO_CHANGE       0x0
#define TOP_CHANGED     0x1
#define RIGHT_CHANGED   0x2
#define BOTTOM_CHANGED  0x4
#define LEFT_CHANGED    0x8

class ActiveLayers
{
public:
    void    paint(QPainter * painter);

    void    add(LayerPtr layer);
    void    clear();
    void    unloadContent();
    bool    contains(Layer::Type type);
    Layer * get(Layer::Type type);
    int     size() { return layers.count(); }

    const QVector<Layer *> get();
    void  setPainterCrop(CropPtr crop)   { painterCrop = crop; }

private:
    QVector<WeakLayerPtr> layers;
    WeakCropPtr           painterCrop;
};


class SystemView : public QWidget
{
    Q_OBJECT

    friend class ViewAccessor;
    friend class SplashScreen;
    friend class Taker;

public:
    SystemView();
    ~SystemView() override;
    
    void init(ViewController * parent);

    static QSize enforceAspectRatio(QSize sz);

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
    void sig_cycleLayers();

    void sig_saveImage();
    void sig_saveMenu();
    void sig_saveSVG();
    void sig_print();

    void sig_loadComplete();

    void sig_messageBox(QString msg, QString info, QMessageBox::Icon icon, bool modal);

    void sig_prepare();
    void sig_enlarge(uint sigid, Xform xf);
    void sig_restore(uint sigid);

public slots:
    void    testSize();

protected:
    // setters - called by view accessor - these are not threaad safe
    void    setSize(QSize sz);
    void    setPainterCrop(CropPtr crop)    { activeLayers.setPainterCrop(crop); }
    void    setWindowTitle(const QString & s);

    void    addLayer(LayerPtr layer)        { activeLayers.add(layer); }
    void    unloadLayerContent()            { activeLayers.unloadContent(); }
    void    unloadViewers();

    void    appSuspendPaint(bool suspend);
    void    setPaintDisable(bool disable);  // calls viewSuspendPaint
    void    debugSuspendPaint(bool suspend);
    void    viewSuspendPaint(bool suspend);

    void    updateView()                            { QWidget::update(); }
    void    repaintView()                           { QWidget::repaint(); }
    void    repaint()                               { qFatal("Dont call repaint() directly - use repaintView()"); }
    void    raise()                                 { QWidget::raise(); }

    void    move(const QPoint & pt)                 { QWidget::move(pt); }
    void    setWindowState(Qt::WindowStates ws)     { QWidget::setWindowState(ws); }
    void    show()                                  { QWidget::show(); }
    void    activateWindow()                        { QWidget::activateWindow(); }

    void    clearLayout(); // only used by cycler for pngs
    void    clearLayout(QLayout* layout, bool deleteWidgets = true);
    void    clearLayers()                           { activeLayers.clear(); }

    void    processLoadState(class LoadUnit * loadUnit);

    void    resize(QSize sz)                        { Q_UNUSED(sz); qFatal("Dont call resize() directly - use setSize()"); }
    void    duplicateView();

    // geters

    bool    isActiveLayer(Layer::Type type)         { return activeLayers.contains(type); }
    Layer * getActiveLayer(Layer::Type type)        { return activeLayers.get(type); }
    const QVector<Layer*> getActiveLayers()         { return activeLayers.get(); }

    bool    viewCanPaint();

    QRect   rect()                                  { return QWidget::rect();}
    qreal   width()                                 { return QWidget::width(); }
    qreal   height()                                { return QWidget::height(); }
    QSize   size()                                  { return QWidget::size(); }
    QPoint  mapToGlobal(const QPoint & pt) const    { return QWidget::mapToGlobal(pt); }
    QColor  getWidgetBackground()                   { return palette().color(QPalette::Window); }

    QPixmap grab()                                  { return QWidget::grab(); }
    QLayout *  layout()                             { return QWidget::layout(); }
    Qt::WindowStates windowState()                  { return QWidget::windowState(); }
    QWindow * windowHandle()                        { return QWidget::windowHandle(); }

    // event handlers

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
    void    setFixedSize(QSize sz);
    void    setWidgetBackground(QColor color);
    QSize   targetSize(QSize sz) const;

    ViewController * parent;
    SplashScreen   * splasher;

    ActiveLayers    activeLayers;

    uint        _suspendPaintApp;
    uint        _suspendPaintDebug;
    uint        _suspendPaintView;
    QPointF     _tl;                // top left of view rect (screen pos)
    QPointF     _br;                // bottom right of view rect (scren pos)

    QPointF     sLast;              // used by pan
    bool        isShown;
    bool        dragging;
    bool        awaitingSizeConfirm; // waiting for the window system to confirm/reject our last requested size
    bool        constrained;        // constrained to fixed size
    bool        adjustingAspectRatio;
};
