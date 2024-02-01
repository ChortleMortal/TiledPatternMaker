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

enum eColorTheme
{
    AUTO_THEME,
    LITE_THEME,
    DARK_THEME
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

    void                    set(QString name, QStringList & list);
    const QStringList &     get()       { list.sort(); return list; }

    void                    setName(QString name) { listname = name; }
    const QString &         getName()   { return listname; }

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

    void    save();
    void    configurePaths();

    QString getMediaRoot();
    QString getMediaRootLocal();
    QString getMediaRootAppdata();
    QString getImageRoot();

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
    QString mosaicFilter;
    QString tileFilter;
    QString rootImageDir;
    QString rootMediaDir;
    QString baseLogName;
    QString lastCompareName;

    QString lastLoadedTiling;      // used on startup
    QString lastLoadedMosaic;      // used on startup
    QString lastLoadedLegacyDes;   // used on startup

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

    bool    tm_showAllTiles;
    bool    tm_hideTable;
    bool    tm_showDebug;
    bool    tm_autofill;
    bool    tm_showOverlaps;

    bool    compare_transparent;
    bool    compare_popup;
    bool    filterColor;
    bool    display_differences;

    bool    use_workListForCompare;
    bool    skipExisting;
    bool    cs_showBkgds;
    bool    showCenterDebug;
    bool    showGrid;
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
    Configuration();
    ~Configuration();

    static Configuration * mpThis;
};

#endif // CONFIGURATION_H
