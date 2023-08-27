#pragma once
#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QMap>
#include <QColor>
#include <QRectF>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <memory>
#endif
#include <QSettings>

#include "enums/ecyclemode.h"
#include "enums/edesign.h"
#include "enums/efillmode.h"
#include "enums/efilesystem.h"
#include "enums/elogmode.h"
#include "enums/emapeditor.h"

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

////////////////////////////////////////////////////////////////
//
// Worklist
//
////////////////////////////////////////////////////////////////
class Worklist : public QStringList
{
public:
    Worklist() {}

    void                    set(QStringList & list);
    const QStringList &     get() { list.sort(); return list; }

    void                    setName(QString listName);
    const QString &         name() { return listname; }

    void                    add(QString designName);
    void                    clear();
    void                    removeuplicates() { list.removeDuplicates(); }

    void                    load(QSettings & s);
    void                    save(QSettings & s);

private:
    QString     listname;
    QStringList list;
};

////////////////////////////////////////////////////////////////
//
// Configuration
//
////////////////////////////////////////////////////////////////
class Configuration
{
public:
    static Configuration *  getInstance();
    static void             releaseInstance();

    void                    configurePaths();
    void                    save();

////////////////////////////////////////////////////////////////
//
// persistent
//
////////////////////////////////////////////////////////////////

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
    QString image0;
    QString image1;
    QString panelName;
    QString lastLoadedTileName; // used on startup
    QString lastLoadedXML;      // used on startup
    QString currentlyLoadedXML; // current status
    QString mosaicFilter;
    QString tileFilter;
    QString rootImageDir;
    QString rootMediaDir;
    QString baseLogName;
    QString lastCompareName;

    QStringList protoViewColors;
    QStringList viewColors;

    eRepeatType     repeatMode;
    eMapEditorMode  mapEditorMode;
    eLogTimer       logTime;
    eDesign         lastLoadedDesignId; // used on startup
    eGridUnits      gridUnits;
    eGridType       gridType;
    eCycleMode      genCycle;
    eCycleMode      viewCycle;
    eLoadType       fileFilter;
    eLoadType       versionFilter;
    eGridTilingAlgo gridTilingAlgo;

    int     cycleInterval;
    int     polySides;    // used by tiling maker
    int     protoViewMode;
    int     gridModelWidth;
    int     gridScreenWidth;
    int     gridTilingWidth;
    int     gridScreenSpacing;
    int     gridZLevel;

    bool    darkTheme;
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
    bool    showBackgroundImage;
    bool    stopIfDiff;
    bool    insightMode;
    bool    motifMultiView;
    bool    motifEnlarge;
    bool    limitViewSize;

    bool    verifyPopup;     // false pops up errors
    bool    verifyProtos;
    bool    verifyMaps;
    bool    verifyDump;     // TODO - make sure this flag work
    bool    verifyVerbose;  // TODO - make sure this flga works

    bool    mosaicFilterCheck;
    bool    mosaicWorklistCheck;
    bool    mosaicOrigCheck;
    bool    mosaicNewCheck;
    bool    mosaicTestCheck;
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

    bool    tm_showAllTiles;
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
    bool    generate_workList;
    bool    skipExisting;
    bool    cs_showBkgds;
    bool    cs_showFrameSettings;
    bool    showCenterDebug;
    bool    showGrid;
    bool    showGridLayerCenter;
    bool    showGridModelCenter;
    bool    showGridScreenCenter;

    bool    measure;

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

////////////////////////////////////////////////////////////////
//
// volatile
//
////////////////////////////////////////////////////////////////

    int     appInstance;

    Layer * selectedLayer;

    QString rootTileDir;
    QString originalTileDir;
    QString newTileDir;
    QString testTileDir;
    QString rootMosaicDir;
    QString originalMosaicDir;
    QString newMosaicDir;
    QString testMosiacDir;
    QString templateDir;
    QString examplesDir;
    QString mapsDir;
    QString worklistsDir;

    bool    primaryDisplay;
    bool    circleX;
    bool    hideCircles;
    bool    enableDetachedPages;
    bool    showCenterMouse;
    bool    updatePanel;
    bool    dontReplicate;
    bool    highlightUnit;
    bool    motifPropagate;
    bool    debugMapEnable;
    bool    dontTrapLog;
    bool    localCycle;

    QString getMediaRoot();
    QString getMediaRootLocal();
    QString getMediaRootAppdata();
    QString getImageRoot();

private:
    Configuration();
    ~Configuration();

    static Configuration * mpThis;
};

#endif // CONFIGURATION_H
