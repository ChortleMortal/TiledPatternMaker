#pragma once
#include <QMap>
#include <QColor>
#include <QRectF>
#include <QSettings>

#include "gui/map_editor/map_editor.h"
#include "gui/viewers/layer.h"
#include "gui/viewers/motif_maker_view.h"
#include "gui/viewers/tiling_maker_view.h"
#include "sys/enums/ecyclemode.h"
#include "sys/enums/efillmode.h"
#include "sys/enums/efilesystem.h"
#include "sys/enums/elogmode.h"
#include "sys/sys/load_unit.h"
#include "sys/sys/versioning.h"
#include "model/motifs/motif.h"
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
    GRID_UNITS_TILE,
    GRID_UNITS_OFF,
	GRID_UNITS_MAX = GRID_UNITS_OFF
};

enum  eGridType
{
    GRID_ORTHOGONAL,
    GRID_ISOMETRIC,
    GRID_RHOMBIC,
    GRID_TYPE_MAX = GRID_RHOMBIC
};

enum eGridPosition
{
    GRID_POS_CENTERED,
    GRID_POS_OFFSET,
    GRID_POS_FIXED,
    GRID_POS_MAX = GRID_POS_FIXED
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
    SHOW_MAP                =  0x01,
    SHOW_TILES              =  0x02,
    SHOW_MOTIFS             =  0x04,
    SHOW_TU_TILES           =  0x08,
    SHOW_TU_MOTIFS          =  0x10,
    SHOW_TILING_UNIT        =  0x20,

    SHOW_ALL_TU_TILES       =  0x40,
    SHOW_SELECTED_TU_TILES  =  0x80,
    SHOW_SELECTED_TU_MOTIFS =  0x100,
    SHOW_EXT_BOUNDARY       =  0x200
};

class Configuration : public QObject
{
    Q_OBJECT

public:
    Configuration();
    ~Configuration();

    void    save();

    void    configureMediaPaths(bool emit);
    void    configureImagePaths(bool emit);
    void    configureLogPaths(bool emit);

    void    loadBackgroundColors();
    void    saveBackgroundColors();

    static QStringList defaultProtoViewColors();

    QColor  getBkgdColor(Layer::Type vtype);
    void    setBkgdColor(Layer::Type vtype, QColor color);

    void    setAlternateRootMediaDir(QString dir)   { _alternateRootMediaDir = dir;}
    void    setAlternateRootImageDir(QString dir)   { _alternateRootImageDir = dir;}
    void    setAlternateRootLogDir(QString dir)     { _alternateRootLogDir = dir;}


    // all settings here should persist
    // volatile settings are in Sys::

    qreal   gridAngle;
    qreal   gridModelSpacing;

    qreal   mapedAngle;     // degrees
    qreal   mapedRadius;
    qreal   mapedLen;
    qreal   mapedMergeSensitivity;

    qreal   protoviewWidth;
    qreal   motifViewWidth;

    QString BMPCompare0;
    QString BMPCompare1;
    QString pageName;
    QString mosaicFilter;
    QString tileFilter;

    QString lastCompareName;

    VersionedName lastLoadedTiling;      // used on startup
    VersionedName lastLoadedMosaic;      // used on startup
    VersionedName lastLoadedLegacy;      // used on startup
    eLoadUnitType lastLoadedType;

    QStringList protoViewColors;

    MapEditor::EditorMode    mapEditorMode;
    MotifMakerView::ViewType motifMakerViewType;

    eRepeatType     repeatMode;
    eLogTimer       logTime;
    eGridUnits      gridUnits;
    eGridType       gridType;

    eLoadType       imageFileFilter;

    eGridTilingAlgo gridTilingAlgo;
    eColorTheme     colorTheme;

    int     cycleInterval;
    int     protoViewMode;
    int     gridModelWidth;
    int     gridScreenWidth;
    int     gridTilingWidth;
    int     gridScreenSpacing;
    Layer::ZLevel  gridZLevel;

    uint    debugViewConfig;
    uint    mapedCleanseLevel;
    uint    debugTabIndex;

    bool    firstBirthday;

    bool    defaultImageRoot;
    bool    defaultMediaRoot;
    bool    defaultLogRoot;

    bool    autoLoadLast;
    bool    scaleToView;
    bool    logToStderr;
    bool    logToDisk;
    bool    logToPanel;
    bool    logNumberLines;
    bool    logDebug;
    bool    enableLog2;
    bool    mapedStatusBox;
    bool    mapedLoadCopies;
    bool    insightMode;         // fixed for the session - the UI is built for it
    bool    pendingInsightMode;  // persisted, takes effect on restart
    bool    limitViewSize;
    bool    disableSplash;
    bool    disableResizeNotify;

    bool    verifyPopup;         // false pops up errors
    bool    forceVerifyProtos;
    bool    verifyMaps;
    bool    verifyDump;
    bool    verifyVerbose;
    bool    buildEmptyNmaps;    // rebuild empty neighbour maps
    bool    slowCleanseMapMerges;   // de-duplicates edges after merge

    bool    mosaicFilterCheck;
    bool    mosaicWorklistCheck;
    bool    mosaicFilterXCheck;
    bool    mosaicWorklistXCheck;
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
    bool    tilingWorklistXCheck;
    bool    lockView;
    bool    multiView;
    bool    integratedScreen;
    bool    bigScreen;

    bool    showMotif;
    bool    showTileBoundary;
    bool    showMotifBoundary;
    bool    showExtendedBoundary;
    bool    showMotifCenter;
    bool    showTileCenter;

    bool    tm_excludesInTable;
    bool    tm_fillOnLoad;
    bool    tm_snapToGrid;
    bool    tm_snapOnly;
    bool    tm_tileAnchorMode;      // when enabled, adjusting the selected tile re-snaps tiles linked to it by tile anchors
    bool    tm_vectorAnchorMode;    // when enabled, translation-vector ends re-bind to whatever tile they land on
    int     tm_tileOpacity;        // 0-100, opacity of tile fill colors in the tiling maker view
    int     tm_poly_side_count;    // number of selected vertices when drawing polygons.
    qreal   tm_poly_rotation;      // regular polygon tile rotation

    bool    compare_transparent;
    bool    compare_popup;
    bool    compare_filterColor;
    bool    highContrastDiffs;

    bool    use_workListForCompare;
    bool    skipExisting;
    bool    cs_showBkgds;
    bool    showGridModelCenter;
    bool    showGridViewCenter;
    eImageType    imageType;
    bool    multithreadedGeneration;
    bool    includeBkgdGeneration;

    eGridPosition gridModelPos;
    eGridPosition gridScreenPos;
    Xform         gridModelXform;
    Xform         gridScreenXform;

    bool    saveMosaicTest;
    bool    saveTilingTest;

    bool    vCompLock;
    bool    vCompXML;

    QColor   transparentColor;      // used by some menus
    QString  gridColorTiling;
    QString  gridColorModel;
    QString  gridColorScreen;

    Worklist worklist;

signals:
    void    sig_mediaPathsChanged();
    void    sig_imagesPathChanged();
    void    sig_logPathChanged();

protected:
    void    configurePaths();
    QString getDefaultMediaDir();
    QString getMediaDirDevelopment();
    QString climbAndFindMediaRoot(const QString & appDirPath, int levels);

private:
    QString _alternateRootMediaDir;
    QString _alternateRootImageDir;
    QString _alternateRootLogDir;

    QColor   bkgdColors[NUM_VIEW_TYPES];

    bool    developmentBuild;
};
