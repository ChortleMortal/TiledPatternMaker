/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CANVAS_H
#define CANVAS_H

#include <QtCore>
#include <QtWidgets>
#include "base/shared.h"
#include "base/cycler.h"
#include "base/canvasSettings.h"

class Configuration;
class Design;
class Workspace;
class Layer;
class CanvasSettings;

enum eMode
{
    MODE_NONE,
    MODE_LAYER,
    MODE_DEFAULT = MODE_LAYER,
    MODE_ZLEVEL,
    MODE_STEP,
    MODE_SEPARATION,
    MODE_ORIGIN,
    MODE_OFFSET,     // row/col offsets
    MODE_TRANSFORM,
    MODE_CYCLE
};

class TiledPatternMaker;

class Canvas : public QGraphicsScene
{
    Q_OBJECT

public:
    static Canvas * getInstance();
    static void     releaseInstance();

    void    update();

    void    addDesign(Design * design);

    void    addLayer(Layer * t);
    void    removeLayer(Layer *t);

    [[noreturn]]void    clear() { qFatal("Do not call this"); }
    void    clearCanvas();

    void    procKeyEvent(QKeyEvent * k);    // from view
    void    ProcKey(QKeyEvent *k);

    void    duplicate();

    void    setSceneRect(const QRectF & rect);
    void    setSceneRect(qreal x, qreal y, qreal w, qreal h);

    void    dump(bool force = false);
    void    dumpGraphicsInfo();

    void    saveImage();

    void    setMode(eMode mode);
    eMode   getMode() { return _mode2; }
    QString getModeStr();

    CanvasSettings   getCanvasSettings() { return settings; }
    void             writeCanvasSettings(CanvasSettings & info);
    void             writeBorderSettings(CanvasSettings & info);

    void    setMaxStep(int max);
    void    stopTimer();

protected:
    void drawForeground(QPainter *painter, const QRectF &rect) Q_DECL_OVERRIDE;
    void drawBackground(QPainter *painter, const QRectF &rect) Q_DECL_OVERRIDE;

    void reposition(qreal, qreal);
    void offset2(qreal, qreal);
    void origin(int, int);
    void selectLayer(int);
    void zPlus();
    void zMinus();
    void showLayer();
    void hideLayer();
    void deltaScale(int delta);
    void deltaRotate(int delta, bool cw);
    void deltaMoveV(int delta);
    void deltaMoveH(int delta);

    bool step(int delta);       // from keyboard

    void ProcKeyLeft(QKeyEvent *k);
    void ProcKeyRight(QKeyEvent *k);
    void ProcKeyDown(QKeyEvent *k);
    void ProcKeyUp(QKeyEvent *k);

signals:
    void sig_viewWS();
    void sig_figure_changed();
    void sig_unload();

    void sig_clearWorkspace();

    void sig_separation(int,int);
    void sig_offset(int,int);
    void sig_origin(int,int);
    void sig_startCycle();

    void sig_deltaRotate(int amount);
    void sig_deltaMoveV(int amount);
    void sig_deltaMoveH(int amount);
    void sig_deltaScale(int amount);

    void sig_cyclerQuit();
    void sig_cyclerKey(int key);

    void sig_forceUpdateStyles();
    void sig_raiseMenu();


public slots:
    void drainTheSwamp();   // for debug

    void slot_repositionAbs(qreal, qreal);
    void slot_offsetAbs2(qreal, qreal);
    void slot_originAbs(int, int);

    void slot_setStep(int step);
    void slot_startTimer();

    void slot_toggleDesignVisibility(int design);
    void slot_png(QString file, int row, int col);

    void slot_cycler_finished();
    void saveBMP(QString name);

private slots:
    void slot_nextStep();   // from timer

private:
    Canvas();
    ~Canvas() override;

    static Canvas * mpThis;
    Configuration * config;
    QTimer        * timer;
    Workspace     * workspace;
    BorderPtr       border;

    CanvasSettings  settings;

    int             maxStep;
    int             stepsTaken;

    int             selectedLayer;
    QPen            gridPen;
    bool            dragging;
    bool            paintBackground;

    eMode          _mode2;
};

#endif // CANVAS_H
