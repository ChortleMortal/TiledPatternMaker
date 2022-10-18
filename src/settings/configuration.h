#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QMap>
#include <QColor>
#include <QRectF>
#include "enums/ecyclemode.h"
#include "enums/edesign.h"
#include "enums/efilesystem.h"
#include "enums/elogmode.h"
#include "enums/emapeditor.h"
#include "enums/eviewtype.h"

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
    GRID_UNITS_MODEL,
    GRID_UNITS_TILE
};

enum  eGridType
{
    GRID_ORTHOGONAL,
    GRID_ISOMETRIC,
    GRID_RHOMBIC,
};

enum eProtoViewMode
{
    PROTO_DRAW_MAP     =  0x01,
    PROTO_DRAW_TILES   =  0x02,
    PROTO_DRAW_MOTIFS  =  0x04,
    PROTO_DEL_TILES    =  0x08,
    PROTO_DEL_MOTIFS   =  0x10,
    PROTO_DRAW_DESIGN_ELEMENT =  0x20
};

class Configuration
{
public:
    static Configuration * getInstance();
    static void            releaseInstance();

    void    configurePaths();
    void    save();

    DesignPtr getDesign(eDesign design) { return availableDesigns.value(design); }

    void setWorkList (QStringList & list);
    void addWorkList(QString name);
    void clearWorkList();
    const QStringList & getWorkList() { return workList; }

    void setViewerType(eViewType  viewerType);
    inline eViewType getViewerType() { return viewerType; }

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

    QString xmlTool;
    QString compareDir0;
    QString compareDir1;
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

    QStringList viewImages;
    QStringList protoViewColors;

    eRepeatType     repeatMode;
    eMapEditorMode  mapEditorMode;
    eLogTimer       logTime;
    eDesign         lastLoadedDesignId; // used on startup
    eGridUnits      gridUnits;
    eGridType       gridType;
    eCycleMode      genCycle;
    eCycleMode      viewCycle;
    eLoadType       fileFilter;

    int     cycleInterval;
    int     polySides;    // used by tiling maker
    int     protoViewMode;
    int     gridModelWidth;
    int     gridScreenWidth;
    int     gridScreenSpacing;

    bool    darkTheme;
    bool    autoLoadStyles;
    bool    autoLoadTiling;
    bool    autoLoadDesigns;
    bool    loadTilingMulti;
    bool    loadTilingModify;
    bool    scaleToView;
    bool    logToStderr;
    bool    logToDisk;
    bool    logToPanel;
    bool    logNumberLines;
    bool    logDebug;
    bool    mapedStatusBox;
    bool    showBackgroundImage;
    bool    stopIfDiff;
    bool    insightMode;
    bool    motifMultiView;
    bool    motifBkgdWhite;
    bool    motifEnlarge;

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
    bool    lockView;
    bool    splitScreen;

    bool    showMotif;
    bool    showTileBoundary;
    bool    showMotifBoundary;
    bool    showExtendedBoundary;

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
    bool    measure;

    bool    gridModelCenter;
    bool    gridScreenCenter;
    bool    snapToGrid;
    bool    defaultImageRoot;
    bool    defaultMediaRoot;

    QColor  transparentColor;            // used by some menus

////////////////////////////////////////////////////////////////
//
// volatile
//
////////////////////////////////////////////////////////////////

    QMap<eDesign,DesignPtr> availableDesigns;
    WeakLayerPtr            selectedLayer;

    QString rootTileDir;
    QString originalTileDir;
    QString newTileDir;
    QString testTileDir;
    QString rootDesignDir;
    QString originalDesignDir;
    QString newDesignDir;
    QString testDesignDir;
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

    QString getMediaRoot();
    QString getMediaRootLocal();
    QString getMediaRootAppdata();
    QString getImageRoot();

private:
    Configuration();
    ~Configuration() {}

    static Configuration * mpThis;

    eViewType   viewerType;
    QStringList workList;
};

#endif // CONFIGURATION_H
