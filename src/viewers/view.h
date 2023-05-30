#pragma once
#ifndef VIEW_H
#define VIEW_H

#include <QWidget>

#include "misc/unique_qvector.h"
#include "settings/frame_settings.h"
#include "enums/ekeyboardmode.h"
#include "enums/emousemode.h"

typedef std::shared_ptr<class Layer> LayerPtr;

class LoadUnit
{
public:
    QString         name;
    QElapsedTimer   loadTimer;
};

class View : public QWidget
{
    Q_OBJECT

public:
    View();
    ~View() override;

    void    init();
    void    unloadView();
    void    paintEnable(bool enable);

    void    resize(QSize sz);

    void    addLayer(LayerPtr layer);
    void    addLayer(Layer * layer);
    void    addTopLayer(LayerPtr layer);
    void    clearLayers()           { activeLayers.clear(); }
    int     numLayers()             { return activeLayers.size(); }

    bool    isActiveLayer(Layer * l);
    QVector<Layer *> getActiveLayers();

    void    setKbdMode(eKbdMode mode);
    bool    getKbdMode(eKbdMode mode);
    QString getKbdModeStr();
    void    resetKbdMode();

    void    setMouseMode(eMouseMode newMode, bool set);
    bool    getMouseMode(eMouseMode mode);

    void    setBackgroundColor(QColor color);
    QColor  getBackgroundColor();

    void    setViewTitle(const QString & s);

    void    clearLayout(); // only used by cycler for pngs

    LoadUnit & getLoadUnit() { return loadUnit; }

    void    dumpRefs();

    FrameSettings frameSettings;


signals:
    void sig_viewSizeChanged(QSize oldSize, QSize newSize);

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
    void sig_cyclerQuit();
    void sig_cyclerKey(int key);
    void sig_rebuildMotif();

    void sig_saveImage();
    void sig_saveMenu();
    void sig_saveSVG();

public slots:
    virtual void slot_refreshView() {}

protected:
    void showEvent(QShowEvent * event) override;
    void closeEvent(QCloseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
#if 0
    void moveEvent(QMoveEvent *event) override;
#endif
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

    void duplicateView();

    class TilingMaker * tilingMaker;

private:
    Configuration     * config;
    class DesignMaker * designMaker;
    class ControlPanel* panel;

    UniqueQVector<Layer*> activeLayers;

    LoadUnit          loadUnit;
    bool              canPaint;
    bool              isShown;
    unsigned int      iMouseMode;
    unsigned int      iLastMouseMode;
    eKbdMode          keyboardMode;

    bool              dragging;
    QColor            backgroundColor;

    QPointF           sLast;    // used by pan

    QRect             _geometry;
};

#endif // VIEW_H
