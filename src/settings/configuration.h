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
#include "enums/ecyclemode.h"
#include "enums/edesign.h"
#include "enums/eviewtype.h"
#include "enums/ekeyboardmode.h"
#include "base/qtapplog.h"

typedef std::shared_ptr<class Design> DesignPtr;
typedef std::weak_ptr<class Layer>    WeakLayerPtr;

#define NUM_DESIGNS 30



#define FPS    60     // frames per second
#define SECONDS(x)  (x * FPS)
#define RANGE(start,dur)  if (stepIndex >= SECONDS(start) && stepIndex <= (SECONDS(start) + SECONDS(dur)))
#define STEP(start)       if (stepIndex == SECONDS(start))
#define TICK(start)       if (stepIndex == start)

#define TileBlue    "#39426d"
#define TileGreen   "#34554a"
#define TileBlack   "#10100e"
#define TileWhite   "#c2bcb0"
#define TileBrown   "#382310"
#define TileBrown2  "#632E1C"
#define TileGold    "#ffe05b"
#define AlhambraBrown "#a35807"
#define AlhambraBlue  "#1840b2"
#define AlhambraGreen "#234b30"
#define AlhambraGold  "#c59c0c"

#define E2STR(x) #x

enum eMapEditorMode
{
    MAPED_MODE_MOSAIC,
    MAPED_MODE_PROTO,
    MAPED_MODE_FIGURE,
    MAPED_MODE_LOCAL,
    MAPED_MODE_TILING,
    MAPED_MODE_FILLED_TILING,
    MAPED_MODE_DCEL,
    MAPED_MODE_NONE,
    MAPED_MODE_MAX = MAPED_MODE_NONE
};

static QString sMapEditorMode[] =
{
    E2STR(MAPED_MODE_MOSAIC),
    E2STR(MAPED_MODE_PROTO),
    E2STR(MAPED_MODE_FIGURE),
    E2STR(MAPED_MODE_LOCAL),
    E2STR(MAPED_MODE_TILING),
    E2STR(MAPED_MODE_FILLED_TILING),
    E2STR(MAPED_MODE_DCEL),
    E2STR(MAPED_MODE_NONE)
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

enum eGridUnits
{
    GRID_UNITS_SCREEN,
    GRID_UNITS_MODEL
};

enum  eGridType
{
    GRID_ORTHOGONAL,
    GRID_ISOMETRIC,
    GRID_RHOMBIC
};

enum eProtoViewMode
{
    PROTO_DRAW_MAP            =  0x01,
    PROTO_DRAW_FEATURES       =  0x02,
    PROTO_DRAW_FIGURES        =  0x04,
    PROTO_HIGHLIGHT_FEATURES  =  0x08,
    PROTO_HIGHLIGHT_FIGURES   =  0x10,
    PROTO_DRAW_DESIGN_ELEMENT =  0x20
};

enum  eAspectRatio
{
    ASPECT_UNCONSTRAINED,
    ASPECT_SQRT_2,
    ASPECT_SQRT_3,
    ASPECT_SQRT_4,
    ASPECT_SQRT_5,
    ASPECT_SQRT_6,
    ASPECT_SQRT_7,
    ASPECT_SQUARE,
    ASPECT_SD,
    ASPECT_HD
};

class Configuration
{
public:
    static Configuration * getInstance();
    static void            releaseInstance();

    void    configurePaths();
    void    save();

    DesignPtr getDesign(eDesign design) { return availableDesigns.value(design); }

    void setViewerType(eViewType  viewerType);
    inline eViewType getViewerType() { return viewerType; }

////////////////////////////////////////////////////////////////
//
// persistent
//
////////////////////////////////////////////////////////////////

    eRepeatType     repeatMode;
    eMapEditorMode  mapEditorMode;
    eCycleMode      cycleMode;
    eLogTimer       logTime;

    int     cycleInterval;
    int     polySides;    // used by tiling maker
    int     protoViewMode;

    QString xmlTool;

    QString compareDir0;
    QString compareDir1;
    QString image0;
    QString image1;
    QString viewImage;

    QString panelName;

    eDesign lastLoadedDesignId; // used on startup
    QString lastLoadedTileName; // used on startup
    QString lastLoadedXML;      // used on startup
    QString currentlyLoadedXML; // current status

    bool    autoLoadStyles;
    bool    autoLoadTiling;
    bool    autoLoadDesigns;
    bool    loadTilingMulti;
    bool    loadTilingModify;
    bool    scaleToView;
    bool    stopIfDiff;
    bool    logToStderr;
    bool    logToDisk;
    bool    logToPanel;
    bool    logNumberLines;
    bool    logWarningsOnly;
    bool    mapedStatusBox;
    bool    showBackgroundImage;
    bool    highlightUnit;
    bool    insightMode;

    bool    verifyMaps;
    bool    verifyDump;     // TODO - make sure this flag work
    bool    verifyVerbose;  // TODO - make sure this flga works

    bool    mosaicFilterCheck;
    bool    tileFilterCheck;
    bool    lockView;
    bool    splitScreen;

    bool    tm_showAllFeatures;
    bool    tm_hideTable;
    bool    tm_showDebug;
    bool    tm_autofill;
    bool    tm_showOverlaps;

    bool    compare_transparent;
    bool    compare_popup;
    bool    view_transparent;
    bool    view_popup;
    bool    filter_transparent;
    bool    display_differences;

    bool    use_workListForCompare;
    bool    use_workListForGenerate;
    bool    generate_workList;
    bool    skipExisting;

    QStringList workList;

    bool    cs_showBkgds;
    bool    cs_showFrameSettings;

    QString mosaicFilter;
    QString tileFilter;

    bool        showCenterDebug;
    bool        showGrid;
    eGridUnits  gridUnits;
    eGridType   gridType;
    int         gridModelWidth;
    bool        gridModelCenter;
    qreal       gridModelSpacing;
    int         gridScreenWidth;
    bool        gridScreenCenter;
    int         gridScreenSpacing;
    qreal       gridAngle;
    bool        snapToGrid;

    QString     rootImageDir;
    QString     rootMediaDir;
    bool        defaultMediaRoot;
    bool        defaultImageRoot;

    QColor      transparentColor;            // used by some menus

////////////////////////////////////////////////////////////////
//
// volatile
//
////////////////////////////////////////////////////////////////

    QString rootTileDir;
    QString originalTileDir;
    QString newTileDir;
    QString rootDesignDir;
    QString originalDesignDir;
    QString newDesignDir;
    QString templateDir;
    QString examplesDir;

    bool    circleX;
    bool    hideCircles;
    bool    enableDetachedPages;
    bool    showCenterMouse;
    bool    updatePanel;
    bool    debugReplicate;
    bool    debugMapEnable;

    WeakLayerPtr selectedLayer;

    eKbdMode    kbdMode;
    QColor      figureViewBkgdColor;            // used by some menus

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

    eViewType   viewerType;
};

#endif // CONFIGURATION_H
