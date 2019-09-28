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

#include "base/shared.h"
#include "base/cycler.h"
#include "base/canvasSettings.h"
#include "base/scene.h"

class Configuration;
class Design;
class Workspace;
class Layer;
class CanvasSettings;
class WorkspaceViewer;

enum eKbdMode
{
    KBD_MODE_TRANSFORM,
    KBD_MODE_LAYER,
    KBD_MODE_ZLEVEL,
    KBD_MODE_STEP,
    KBD_MODE_SEPARATION,
    KBD_MODE_ORIGIN,
    KBD_MODE_OFFSET,     // row/col offsets
    KBD_MODE_BKGD,
    KBD_MODE_DATA,
    KBD_MODE_DEFAULT = KBD_MODE_TRANSFORM
};

class TiledPatternMaker;

class Canvas : public QObject
{
    Q_OBJECT

public:
    static Canvas * getInstance();
    static void     releaseInstance();

    void    init();
    void    update();
    void    invalidate();

    Scene * swapScenes();

    void    addDesign(Design * design);

    void    clearCanvas();

    void    duplicate();

    void    setSceneRect(const QRectF & rect);
    void    setSceneRect(qreal x, qreal y, qreal w, qreal h);

    void    dump(bool force = false);
    void    dumpGraphicsInfo();

    void    saveImage();

    void     procKeyEvent(QKeyEvent * k);    // not from View
    void     setKbdMode(eKbdMode mode);
    eKbdMode getKbdMode() { return kbdMode; }
    QString  getKbdModeStr();

    void           setCanvasSettings(CanvasSettings info);
    CanvasSettings getCanvasSettings() { return settings; }

    void    setMaxStep(int max);
    void    stopTimer();

    Scene  * scene;

signals:
    void sig_viewWS();
    void sig_figure_changed();
    void sig_unload();

    void sig_clearWorkspace();

    void sig_deltaRotate(int amount);
    void sig_deltaMoveY(int amount);
    void sig_deltaMoveX(int amount);
    void sig_deltaScale(int amount);

    void sig_cyclerStart();
    void sig_cyclerQuit();
    void sig_cyclerKey(int key);

    void sig_forceUpdateStyles();
    void sig_raiseMenu();

public slots:
    void slot_procKeyEvent(QKeyEvent * k);    // from view

    void slot_designReposition(qreal, qreal);
    void slot_designOffset(qreal, qreal);
    void slot_designOrigin(int, int);
    void slot_designToggleVisibility(int design);

    void slot_startTimer();
    void slot_setStep(int step);

    void slot_png(QString file, int row, int col);
    void saveBMP(QString name);

    void slot_cycler_finished();

    void drainTheSwamp();   // for debug

private slots:
    void slot_nextStep();   // from timer

protected:
    void ProcKey(QKeyEvent *k, bool isALT);
    bool ProcNavKey(int key, int multiplier, bool isALT);
    void ProcKeyLeft( int delta, bool isALT);
    void ProcKeyRight(int delta, bool isALT);
    void ProcKeyDown( int delta, bool isALT);
    void ProcKeyUp(   int delta, bool isALT);

    void designReposition(qreal, qreal);
    void designOffset(qreal, qreal);
    void designOrigin(int, int);
    void designLayerSelect(int);
    void designLayerZPlus();
    void designLayerZMinus();
    void designLayerShow();
    void designLayerHide();

    void designScale(int delta);
    void designRotate(int delta, bool cw);
    void designMoveY(int delta);
    void designMoveX(int delta);

    bool step(int delta);       // from keyboard

private:
    Canvas();
    virtual ~Canvas();

    static Canvas   * mpThis;
    Configuration   * config;
    QTimer          * timer;
    Workspace       * workspace;
    WorkspaceViewer * viewer;
    Scene           * sceneA;
    Scene           * sceneB;

    CanvasSettings  settings;

    int maxStep;
    int stepsTaken;
    int selectedLayer;

    bool dragging;

    eKbdMode kbdMode;
};

#endif // CANVAS_H
