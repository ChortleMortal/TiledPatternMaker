#pragma once
#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QMap>
#include <QColor>
#include <QRectF>
#include <QSettings>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <memory>
#endif

#include "sys/enums/ecyclemode.h"
#include "sys/enums/edesign.h"
#include "sys/enums/efillmode.h"
#include "sys/enums/efilesystem.h"
#include "sys/enums/elogmode.h"
#include "sys/enums/emapeditor.h"
#include "sys/sys/versioning.h"
#include "model/settings/worklist.h"

typedef std::shared_ptr<class Design> DesignPtr;
typedef std::weak_ptr<class Layer>    WeakLayerPtr;

#define NUM_DESIGNS 30

#define FPS    60     // frames per second
#define SECONDS(x)  (x * FPS)
#define RANGE(start,dur)  if (stepIndex >= SECONDS(start) && stepIndex <= (SECONDS(start) + SECONDS(dur)))
#define STEP(start)       if (stepIndex == SECONDS(start))
#define TICK(start)       if (stepIndex == start)

#define E2STR(x) #x

enum eGridUnits
{
    GRID_UNITS_SCREEN,
    GRID_UNITS_MODEL,
    GRID_UNITS_TILE
};

enum  eGridType
{
    GRID_ORTHOGONAL,
    GRID_ISOMETRIC,
    GRID_RHOMBIC,
};

enum  eGridTilingAlgo
{
    FLOOD,
    REGION
};

enum eColorTheme
{
    AUTO_THEME,
    LITE_THEME,
    DARK_THEME
};

enum eProtoViewMode
{
    PROTO_DRAW_MAP      =  0x01,
    PROTO_ALL_TILES     =  0x02,
    PROTO_ALL_MOTIFS    =  0x04,
    PROTO_DEL_TILES     =  0x08,
    PROTO_DEL_MOTIFS    =  0x10,
    PROTO_DRAW_PROTO    =  0x20,
    PROTO_ALL_VISIBLE   =  0x40,
    PROTO_VISIBLE_TILE  =  0x80,
    PROTO_VISIBLE_MOTIF =  0x100
};

class Configuration
{
public:
    Configuration();
    ~Configuration();

    void    save();
    void    configurePaths();

    QString getMediaRoot();
    QString getMediaRootLocal();
    QString getMediaRootAppdata();
    QString getImageRoot();

    qreal   gridAngle;
    qreal   gridModelSpacing;

    qreal   mapedAngle;     // degrees
    qreal   mapedRadius;
    qreal   mapedLen;
    qreal   mapedMergeSensitivity;

    qreal   protoviewWidth;
    qreal   motifViewWidth;

    QString xmlTool;
    QString diffTool;
    QString BMPCompare0;
    QString BMPCompare1;
    QString panelName;
    QString mosaicFilter;
    QString tileFilter;
    QString rootImageDir;
    QString rootMediaDir;
    QString baseLogName;
    QString lastCompareName;

    VersionedName lastLoadedTiling;      // used on startup
    VersionedName lastLoadedMosaic;      // used on startup
    VersionedName lastLoadedLegacy;      // used on startup

    QStringList protoViewColors;
    QStringList viewColors;

    eRepeatType     repeatMode;
    eMapEditorMode  mapEditorMode;
    eLogTimer       logTime;
    eGridUnits      gridUnits;
    eGridType       gridType;

    eCycleMode      viewCycle2;

    eLoadType       genFileFilter;
    eLoadType       viewFileFilter;
    eLoadType       versionFilter;

    eGridTilingAlgo gridTilingAlgo;
    eColorTheme     colorTheme;

    int     cycleInterval;
    int     polySides;    // used by tiling maker
    int     protoViewMode;
    int     gridModelWidth;
    int     gridScreenWidth;
    int     gridTilingWidth;
    int     gridScreenSpacing;
    int     gridZLevel;

    uint    debugViewConfig;
    uint    mapedCleanseLevel;

    bool    autoLoadStyles;
    bool    autoLoadTiling;
    bool    autoLoadDesigns;
    bool    scaleToView;
    bool    logToStderr;
    bool    logToDisk;
    bool    logToAppDir;
    bool    logToPanel;
    bool    logNumberLines;
    bool    logDebug;
    bool    mapedStatusBox;
    bool    mapedLoadCopies;
    bool    showBackgroundImage;
    bool    insightMode;
    bool    motifMultiView;
    bool    motifEnlarge;
    bool    limitViewSize;

    bool    verifyPopup;         // false pops up errors
    bool    verifyProtos;
    bool    verifyMaps;
    bool    verifyDump;         // TODO - make sure this flag work
    bool    verifyVerbose;      // TODO - make sure this flag work
    bool    buildEmptyNmaps;    // rebuild empty neighbour maps
    bool    unDuplicateMerge;   // de-duplicates edges after merge

    bool    mosaicFilterCheck;
    bool    mosaicWorklistCheck;
    bool    mosaicOrigCheck;
    bool    mosaicNewCheck;
    bool    mosaicTestCheck;
    bool    mosaicSortCheck;
    bool    showWithBkgds;
    bool    tilingOrigCheck;
    bool    tilingNewCheck;
    bool    tilingTestCheck;
    bool    tileFilterCheck;
    bool    tilingWorklistCheck;
    bool    lockView;
    bool    splitScreen;
    bool    bigScreen;

    bool    showMotif;
    bool    showTileBoundary;
    bool    showMotifBoundary;
    bool    showExtendedBoundary;
    bool    showMotifCenter;
    bool    showTileCenter;

    bool    tm_showExcludes;
    bool    tm_hideTable;
    bool    tm_hideTranslations;
    bool    tm_showDebug;
    bool    tm_loadFill;
    bool    tm_showOverlaps;

    bool    compare_transparent;
    bool    compare_popup;
    bool    compare_filterColor;

    bool    use_workListForCompare;
    bool    skipExisting;
    bool    cs_showBkgds;
    bool    showCenterDebug;
    bool    showGrid;
    bool    lockGridToView;
    bool    showGridLayerCenter;
    bool    showGridModelCenter;
    bool    showGridViewCenter;
    bool    genCycleMosaic;
    bool    multithreadedGeneration;

    bool    gridModelCenter;
    bool    gridScreenCenter;
    bool    snapToGrid;
    bool    defaultImageRoot;
    bool    defaultMediaRoot;
    bool    saveMosaicTest;
    bool    saveTilingTest;

    bool    vCompLock;
    bool    vCompXML;
    bool    vCompTile;

    QColor  transparentColor;            // used by some menus
    QString  gridColorTiling;
    QString  gridColorModel;
    QString  gridColorScreen;

    Worklist worklist;

private:

};

#endif // CONFIGURATION_H
