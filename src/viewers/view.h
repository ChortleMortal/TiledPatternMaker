#pragma once
#ifndef VIEW_H
#define VIEW_H

#include <QWidget>
#include <QElapsedTimer>

#include "misc/unique_qvector.h"
#include "enums/ekeyboardmode.h"
#include "enums/emousemode.h"
#include "enums/eviewtype.h"

#define fakeqWarning qWarning

typedef std::shared_ptr<class Layer> LayerPtr;

class ViewController;

enum eLoadState
{
    LOADING_NONE,
    LOADING_TILING,
    LOADING_MOSAIC,
    LOADING_LEGACY
};

#define NO_CHANGE       0x0
#define TOP_CHANGED     0x1
#define RIGHT_CHANGED   0x2
#define BOTTOM_CHANGED  0x4
#define LEFT_CHANGED    0x8

class LoadUnit
{
public:
    LoadUnit();

    eLoadState  getLoadState()      { return loadState; }
    QString     getLoadName()       { return loadName; }
    eLoadState  getLastLoadState()  { return lastState; }
    QString     getLastLoadName()   { return lastName; }

    void        setLoadState(eLoadState state, QString name);
    void        resetLoadState() { loadState = LOADING_NONE; }

    QElapsedTimer   loadTimer;

private:
    eLoadState      loadState;
    QString         loadName;
    QString         lastName;
    eLoadState      lastState;

    class Configuration * config;
};

class View : public QWidget
{
    Q_OBJECT
    
    friend class ViewController;

public:
    View();
    ~View() override;
    
    void    init(ViewController * parent);
    void    unloadView();

    void    resize(QSize sz);
    QSize   getCurrentSize() { return viewSize; }

    void    addLayer(LayerPtr layer);
    void    addLayer(Layer * layer);
    void    addTopLayer(LayerPtr layer);
    void    clearLayers()           { activeLayers.clear(); }

    bool    isActiveLayer(Layer * l);
    QVector<Layer *> getActiveLayers();
    Layer * getActiveLayer(eViewType type);

    void    setKbdMode(eKbdMode mode);
    bool    getKbdMode(eKbdMode mode);
    QString getKbdModeStr();
    void    resetKbdMode();

    void    setMouseMode(eMouseMode newMode, bool set);
    bool    getMouseMode(eMouseMode mode);

    void    setViewBackgroundColor(QColor color);
    QColor  getViewBackgroundColor();

    void    setViewTitle(const QString & s);

    void    clearLayout(); // only used by cycler for pngs

    LoadUnit & getLoadUnit() { return loadUnit; }

    void setPaintEnable(bool enable);

    void setAppPaint(bool enb)      { _appPaint =  enb; }
    bool getAppPaint()              { return _appPaint; }

    void update();
    void repaint();

signals:
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

    void sig_kbdMode(eKbdMode);

    void sig_raiseMenu();
    void sig_rebuildMotif();

    void sig_stepperEnd();
    void sig_stepperPause();
    void sig_stepperKey(int key);

    void sig_saveImage();
    void sig_saveMenu();
    void sig_saveSVG();

protected:
    void setCanPaint(bool enb)      { _canPaint = enb;  }
    bool getCanPaint()              { return _canPaint; }

    void duplicateView();

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
    
private:
    class Configuration * config;
    class DesignMaker   * designMaker;
    class ControlPanel  * panel;
    ViewController      * viewControl;

    QSize               viewSize;    // the main window size

    UniqueQVector<Layer*> activeLayers;

    LoadUnit          loadUnit;
    bool              _canPaint;
    bool              _appPaint;
    bool              isShown;
    unsigned int      iMouseMode;
    unsigned int      iLastMouseMode;
    eKbdMode          keyboardMode;

    bool              dragging;
    QColor            backgroundColor;

    QPointF           sLast;    // used by pan

    QPointF           _tl;      // top left of view rect (screen pos)
    QPointF           _br;      // bottom right of view rect (scren pos)
};

#endif // VIEW_H
