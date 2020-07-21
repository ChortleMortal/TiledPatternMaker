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

#include <QSvgGenerator>
#include "base/shared.h"
#include "base/cycler.h"
#include "base/canvas_settings.h"
#include "base/configuration.h"

class Design;
class Workspace;
class Layer;
class CanvasSettings;
class WorkspaceViewer;
class TiledPatternMaker;

class Canvas : public QObject
{
    Q_OBJECT

public:
    static Canvas * getInstance();
    static void     releaseInstance();

    void    init();

    void    duplicate();

    void    savePixmap(QString name);

    bool     procKeyEvent(QKeyEvent * k);    // from View
    void     setKbdMode(eKbdMode mode);
    QString  getKbdModeStr();

    QSvgGenerator * getSvgGenerator() { return &generator; }

    void    setMaxStep(int max);
    void    stopTimer();

signals:
    void sig_viewWS();
    void sig_figure_changed();
    void sig_unload();

    void sig_clearWorkspace();

    void sig_deltaRotate(int amount);
    void sig_deltaMoveY(int amount);
    void sig_deltaMoveX(int amount);
    void sig_deltaScale(int amount);

    void sig_cyclerStart(eCycleMode);
    void sig_cyclerQuit();
    void sig_cyclerKey(int key);

    void sig_forceUpdateStyles();
    void sig_raiseMenu();

    void sig_kbdMode(eKbdMode);

public slots:
    void saveImage();
    void saveSvg();

    void slot_designReposition(qreal, qreal);
    void slot_designOffset(qreal, qreal);
    void slot_designOrigin(int, int);
    void slot_designToggleVisibility(int design);

    void slot_startTimer();
    void slot_setStep(int step);

    void slot_show_png(QString file, int row, int col);

    void slot_cycler_finished();

    void drainTheSwamp();   // for debug

private slots:
    void slot_nextStep();   // from timer

protected:
    bool ProcKey(QKeyEvent *k, bool isALT);
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
    View            * view;

    QSvgGenerator   generator;

    int maxStep;
    int stepsTaken;
    int selectedLayer;

    bool dragging;
};

#endif // CANVAS_H
