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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QtCore>
#include <QColor>
#include "design.h"
#include "base/cycler.h"

#define NUM_DESIGNS 30

#define E2STR(x) #x
#define Enum2Str(e)  {QString(#e)}

#define FPS    60     // frames per second
#define SECONDS(x)  (x * FPS)
#define RANGE(start,dur)  if (stepIndex >= SECONDS(start) && stepIndex <= (SECONDS(start) + SECONDS(dur)))
#define STEP(start)       if (stepIndex == SECONDS(start))
#define TICK(start)       if (stepIndex == start)

enum eViewType
{
    VIEW_DESIGN,
    VIEW_START = VIEW_DESIGN,
    VIEW_PROTO,
    VIEW_PROTO_FEATURE,
    VIEW_DEL,
    VIEW_FIGURE_MAKER,
    VIEW_TILING,
    VIEW_TILIING_MAKER,
    VIEW_MAP_EDITOR,
    VIEW_FACE_SET,
    VIEW_MAX = VIEW_FACE_SET
};

static QString sViewerType[]
{
    E2STR(VIEW_DESIGN),
    E2STR(VIEW_PROTO),
    E2STR(VIEW_PROTO_FEATURE),
    E2STR(VIEW_DEL),
    E2STR(VIEW_FIGURE_MAKER),
    E2STR(VIEW_TILING),
    E2STR(VIEW_TILIING_MAKER),
    E2STR(VIEW_MAP_EDITOR),
    E2STR(VIEW_FACE_SET)
};

enum eDesignViewer
{
    DV_LOADED_STYLE,
    DV_WS_STYLE,
    DV_SHAPES,
    DV_MAX = DV_SHAPES
};

static QString sDesignViewer[] =
{
    E2STR(DV_LOADED_STYLE),
    E2STR(DV_WS_STYLE),
    E2STR(DV_SHAPES),
};

enum eProtoViewer
{
    PV_STYLE,
    PV_WS,
    PV_MAX = PV_WS
};

static QString sProtoViewer[] =
{
    E2STR(PV_STYLE),
    E2STR(PV_WS)
};

enum eProtoFeatureViewer
{
    PVF_STYLE,
    PVF_WS,
    PVF_MAX = PVF_WS
};

static QString sProtoFeatureViewer[] =
{
    E2STR(PVF_STYLE),
    E2STR(PVF_WS)
};

enum eDELViewer
{
    DEL_STYLES,
    DEL_WS,
    DEL_MAX = DEL_WS
};

static QString sDELViewer[] =
{
    E2STR(DEL_STYLES),
    E2STR(DEL_WS)
};

enum eFigureViewer
{
    FV_STYLE,
    FV_WS,
    FV_MAX = FV_WS
};

static QString sFigureViewer[] =
{
    E2STR(FV_STYLE),
    E2STR(FV_WS),
};

enum eTilingViewer
{
    TV_STYLE,
    TV_WORKSPACE,
    TV_MAX = TV_WORKSPACE
};

static QString sTilingViewer[] =
{
    E2STR(TV_STYLE),
    E2STR(TV_WORKSPACE)
};

enum eTilingMakerView
{
    TD_STYLE,
    TD_WORKSPACE,
    TD_MAX = TD_WORKSPACE
};

static QString sTilingMakerView[] =
{
    E2STR(TD_STYLE),
    E2STR(TD_WORKSPACE)
};


enum eMapEditorView
{
    ME_STYLE_MAP,
    ME_PROTO_MAP,
    ME_FIGURE_MAP,
    ME_MAX = ME_FIGURE_MAP
};

static QString sMapEditorView[] =
{
    E2STR(ME_STYLE_MAP),
    E2STR(ME_PROTO_MAP),
    E2STR(ME_FIGURE_MAP)
};


enum eRepeatType
{
    REPEAT_SINGLE,
    REPEAT_PACK,
    REPEAT_DEFINED,
    REPEAT_MAX = REPEAT_DEFINED
};

static QString sRepeatType[4]  = {
    E2STR(REPEAT_SINGLE),
    E2STR(REPEAT_PACK),
    E2STR(REPEAT_DEFINED)
};

enum ePushTarget
{
    TARGET_LOADED_STYLES,
    TARGET_WS_STYLES,
    TARGET_MAX = TARGET_WS_STYLES
};

enum eCSSelect  // canvas settings
{
    CS_STYLE,
    CS_WS,
    CS_CANVAS
};

class Configuration
{
public:
    static Configuration * getInstance();
    static void            releaseInstance();

    void    reconfigurePaths();
    void    save();

    DesignPtr getDesign(eDesign design) { return availableDesigns.value(design); }

    ///
    /// persistent
    ///

    eRepeatType           repeatMode;

    eViewType             viewerType;
    eDesignViewer         designViewer;
    eProtoViewer          protoViewer;
    eProtoFeatureViewer   protoFeatureViewer;
    eTilingViewer         tilingViewer;
    eTilingMakerView      tilingMakerViewer;
    eFigureViewer         figureViewer;
    eDELViewer            delViewer;
    eMapEditorView        mapEditorView;

    ePushTarget           pushTarget;
    eCSSelect             canvasSettings;
    eCycleMode            cycleMode;
    int                   cycleInterval;

    QString rootMediaDir;
    QString rootTileDir;
    QString newTileDir;
    QString rootDesignDir;
    QString newDesignDir;
    QString templateDir;
    QString rootImageDir;
    QString examplesDir;
    QString compareDir0;
    QString compareDir1;

    QString image0;
    QString image1;

    QString panelName;

    eDesign lastLoadedDesignId; // used on startup
    QString lastLoadedTileName; // used on startup
    QString lastLoadedXML;      // used on startup
    QString currentlyLoadedXML; // current status

    bool    firstBirthday;
    bool    autoClear;
    bool    autoLoadStyles;
    bool    autoLoadTiling;
    bool    autoLoadDesigns;
    bool    autoCycle;
    bool    stopIfDiff;
    bool    verifyMaps;
    bool    wsStatusBox;

    bool    designFilterCheck;
    bool    tileFilterCheck;
    bool    showAllFeatures;
    bool    lockView;
    bool    screenIsSplit;
    bool    transparentCompare;

    QString designFilter;
    QString tileFilter;

    ///
    /// volatile
    ///


    bool    circleX;
    bool    sceneGrid;
    bool    boundingRects;
    bool    hideCircles;

    bool    fgdGridModel;       // true= model, false = screen
    int     fgdGridStepScreen;
    qreal   fgdGridStepModel;

    bool    updatePanel;

    bool    debugReplicate;
    bool    debugMapEnable;

    FaceSet * faceSet;
    FacePtr   selectedFace;                     // used by FaceSetView;

    Feature * selectedDesignElementFeature;     // set by menu
    QColor    figureViewBkgdColor;                // used by some menus

    QMap<eDesign,DesignPtr>  availableDesigns;

protected:
    QString getMediaRoot();
    QString getMediaRootLocal();
    QString getMediaRootAppdata();
    QString getImageRoot();

private:
    Configuration();
    ~Configuration() {}

    static Configuration * mpThis;
};

#endif // CONFIGURATION_H
