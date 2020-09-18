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
#include "base/cycler.h"
#include "base/shared.h"

#define NUM_DESIGNS 30

#define E2STR(x) #x
#define Enum2Str(e)  {QString(#e)}

#define FPS    60     // frames per second
#define SECONDS(x)  (x * FPS)
#define RANGE(start,dur)  if (stepIndex >= SECONDS(start) && stepIndex <= (SECONDS(start) + SECONDS(dur)))
#define STEP(start)       if (stepIndex == SECONDS(start))
#define TICK(start)       if (stepIndex == start)

enum eDesign
{
    DESIGN_5,
    DESIGN_6,
    DESIGN_7,
    DESIGN_8,
    DESIGN_9,
    DESIGN_HU_INSERT,
    DESIGN_10,
    DESIGN_11,
    DESIGN_12,
    DESIGN_13,
    DESIGN_14,
    DESIGN_16,
    DESIGN_17,
    DESIGN_18,
    DESIGN_19,
    DESIGN_KUMIKO1,
    DESIGN_KUMIKO2,
    NO_DESIGN
};

static QString sDesign2[] =
{
    Enum2Str(DESIGN_0),
    Enum2Str(DESIGN_1),
    Enum2Str(DESIGN_2),
    Enum2Str(DESIGN_3),
    Enum2Str(DESIGN_4),
    Enum2Str(DESIGN_5),
    Enum2Str(DESIGN_6),
    Enum2Str(DESIGN_7),
    Enum2Str(DESIGN_8),
    Enum2Str(DESIGN_9),
    Enum2Str(DESIGN_10),
    Enum2Str(DESIGN_11),
    Enum2Str(DESIGN_12),
    Enum2Str(DESIGN_13),
    Enum2Str(DESIGN_14),
    Enum2Str(DESIGN_15),
    Enum2Str(DESIGN_16),
    Enum2Str(DESIGN_17),
    Enum2Str(DESIGN_18),
    Enum2Str(DESIGN_19),
    Enum2Str(DESIGN_KUMIKO1),
    Enum2Str(DESIGN_KUMIKO2),
    Enum2Str(DESIGN_HU_INSERT),
    Enum2Str(DESIGN_ROSETTE_MAKER),
    Enum2Str(DESIGN_STAR_MAKER),
    Enum2Str(NO_DESIGN)
};

enum eViewType
{
    VIEW_DESIGN,
    VIEW_START = VIEW_DESIGN,
    VIEW_MOSAIC,
    VIEW_PROTOTYPE,
    VIEW_DESIGN_ELEMENT,
    VIEW_FROTOTYPE_MAKER,
    VIEW_TILING,
    VIEW_TILING_MAKER,
    VIEW_MAP_EDITOR,
    VIEW_FACE_SET,
    VIEW_MAX = VIEW_FACE_SET,
    VIEW_UNDEFINED,
    NUM_VIEW_TYPES
};

static QString sViewerType[]
{
    E2STR(VIEW_DESIGN),
    E2STR(VIEW_MOSAIC),
    E2STR(VIEW_PROTOTYPE),
    E2STR(VIEW_DESIGN_ELEMENT),
    E2STR(VIEW_FROTOTYPE_MAKER),
    E2STR(VIEW_TILING),
    E2STR(VIEW_TILING_MAKER),
    E2STR(VIEW_MAP_EDITOR),
    E2STR(VIEW_FACE_SET),
    E2STR(VIEW_UNDEFINED)
};

enum eMapEditorMode
{
    MAP_MODE_MOSAIC,
    MAP_MODE_PROTO,
    MAP_MODE_FIGURE,
    MAP_MODE_LOCAL,
    MAP_MODE_TILING,
    MAP_MODE_MAX = MAP_MODE_TILING
};

static QString sMapEditorMode[] =
{
    E2STR(MAP_MODE_MOSAIC),
    E2STR(MAP_MODE_PROTO),
    E2STR(MAP_MODE_FIGURE),
    E2STR(MAP_MODE_LOCAL),
    E2STR(MAP_MODE_TILING),
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

enum eKbdMode
{
    KBD_MODE_UNDEFINED,
    KBD_MODE_XFORM_VIEW,
    KBD_MODE_XFORM_BKGD,
    KBD_MODE_XFORM_TILING,
    KBD_MODE_XFORM_FEATURE,
    KBD_MODE_POS,
    KBD_MODE_LAYER,
    KBD_MODE_ZLEVEL,
    KBD_MODE_STEP,
    KBD_MODE_SEPARATION,
    KBD_MODE_ORIGIN,
    KBD_MODE_OFFSET,
    KBD_MODE_CENTER
};

const QString sKbdMode[]  = {
    E2STR(KBD_MODE_UNDEFINED),
    E2STR(KBD_MODE_XFORM_VIEW),
    E2STR(KBD_MODE_XFORM_BKGD),
    E2STR(KBD_MODE_XFORM_TILING),
    E2STR(KBD_MODE_XFORM_FEATURE),
    E2STR(KBD_MODE_POS),
    E2STR(KBD_MODE_LAYER),
    E2STR(KBD_MODE_ZLEVEL),
    E2STR(KBD_MODE_STEP),
    E2STR(KBD_MODE_SEPARATION),
    E2STR(KBD_MODE_ORIGIN),
    E2STR(KBD_MODE_OFFSET),
    E2STR(KBD_MODE_CENTER)
};

enum eMouseMode
{
    MOUSE_MODE_NONE,
    MOUSE_MODE_TRANSLATE,
    MOUSE_MODE_ROTATE,
    MOUSE_MODE_SCALE,
};

enum eGridType
{
    GRID_SCREEN,
    GRID_MODEL
};

class Configuration
{
public:
    static Configuration * getInstance();
    static void            releaseInstance();

    void    configurePaths();
    void    save();

    DesignPtr getDesign(eDesign design) { return availableDesigns.value(design); }


////////////////////////////////////////////////////////////////
//
// persistent
//
////////////////////////////////////////////////////////////////

    eRepeatType           repeatMode;

    eViewType             viewerType;
    eMapEditorMode        mapEditorMode;

    eCycleMode            cycleMode;

    int                   cycleInterval;
    int                   polySides;    // used by tiling maker

    QString compareDir0;
    QString compareDir1;
    QString xmlTool;

    QString image0;
    QString image1;

    QString panelName;

    eDesign lastLoadedDesignId; // used on startup
    QString lastLoadedTileName; // used on startup
    QString lastLoadedXML;      // used on startup
    QString currentlyLoadedXML; // current status

    bool    autoLoadStyles;
    bool    autoLoadTiling;
    bool    autoLoadDesigns;
    bool    scaleToView;
    bool    autoCycle;
    bool    stopIfDiff;
    bool    logToStderr;
    bool    logToDisk;
    bool    logToPanel;
    bool    logNumberLines;
    bool    logWarningsOnly;
    bool    logElapsedTime;
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
    bool    display_differences;
    bool    compare_ping_pong;
    bool    compare_side_by_side;

    QString mosaicFilter;
    QString tileFilter;

    bool        showGrid;
    eGridType   gridType;
    int         gridModelWidth;
    bool        gridModelCenter;
    qreal       gridModelSpacing;
    int         gridScreenWidth;
    bool        gridScreenCenter;
    int         gridScreenSpacing;


////////////////////////////////////////////////////////////////
//
// volatile
//
////////////////////////////////////////////////////////////////

    QString rootMediaDir;
    QString rootTileDir;
    QString newTileDir;
    QString rootDesignDir;
    QString newDesignDir;
    QString templateDir;
    QString examplesDir;
    QString rootImageDir;

    bool    circleX;
    bool    hideCircles;
    bool    enableDetachedPages;
    bool    showCenter;

    bool    updatePanel;

    bool    debugReplicate;
    bool    debugMapEnable;

    eKbdMode    kbdMode;

    WeakFacesPtr   faces;                           // used by FaceSetView
    WeakFacePtr    selectedFace;                    // used by FaceSetView

    QColor    figureViewBkgdColor;                  // used by some menus

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
