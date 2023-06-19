#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QApplication>
#include "settings/configuration.h"
#include "tiledpatternmaker.h"

extern TiledPatternMaker * theApp;

Configuration * Configuration::mpThis = nullptr;

Configuration * Configuration::getInstance()
{
    if (mpThis == nullptr)
    {
        mpThis = new Configuration;
    }
    return mpThis;
}

void Configuration::releaseInstance()
{
    if (mpThis != nullptr)
    {
        delete mpThis;
        mpThis = nullptr;
    }
}

Configuration::Configuration()
{
    // peersist
    QSettings s;

    lastLoadedDesignId  = static_cast<eDesign>(s.value("design2",5).toInt());
    cycleInterval       = s.value("cycleInterval",4).toInt();
    polySides           = s.value("polySides",8).toInt();
    protoViewMode       = s.value("protoViewMode2",0x06).toInt();
    lastLoadedTileName  = s.value("lastLoadedTileName","").toString();
    lastLoadedXML       = s.value("lastLoadedXML","").toString();
    rootMediaDir        = s.value("rootMediaDir",getMediaRoot()).toString();
    rootImageDir        = s.value("rootImageDir",getImageRoot()).toString();
    image0              = s.value("image0","").toString();
    image1              = s.value("image1","").toString();
    panelName           = s.value("panelName","Control").toString();
    mosaicFilter        = s.value("designFilter","").toString();
    tileFilter          = s.value("tileFilter","").toString();
    xmlTool             = s.value("xmlTool","").toString();
    diffTool            = s.value("diffTool","").toString();
    transparentColor    = s.value("transparentColor","black").toString();
    gridColorTiling     = s.value("gridColorTiling2", QColor(Qt::red).name(QColor::HexArgb)).toString();
    gridColorModel      = s.value("gridColorModel2", QColor(Qt::green).name(QColor::HexArgb)).toString();
    gridColorScreen     = s.value("gridColorScreen2", QColor(Qt::blue).name(QColor::HexArgb)).toString();
    darkTheme           = s.value("darkTheme",false).toBool();
    autoLoadStyles      = s.value("autoLoadStyles",false).toBool();
    autoLoadTiling      = s.value("autoLoadTiling",false).toBool();
    autoLoadDesigns     = s.value("autoLoadDesigns",false).toBool();
    scaleToView         = s.value("scaleToView",true).toBool();
    stopIfDiff          = s.value("stopIfDiff",true).toBool();
    verifyMaps          = s.value("verifyMaps",false).toBool();
    verifyProtos        = s.value("verifyProtos",false).toBool();
    verifyPopup         = s.value("verifyPopup",false).toBool();
    verifyDump          = s.value("verifyDump",false).toBool();
    verifyVerbose       = s.value("verifyVerbose",false).toBool();
    baseLogName         = s.value("baseLogName","tiledPatternMakerLog").toString();
    logToStderr         = s.value("logToStderr",true).toBool();
    logToDisk           = s.value("logToDisk",true).toBool();
    logToAppDir         = s.value("logToAppDir",false).toBool();
    logToPanel          = s.value("logToPanel",true).toBool();
    logNumberLines      = s.value("logNumberLines",true).toBool();
    logDebug            = s.value("logDebug",false).toBool();
    logTime             = static_cast<eLogTimer>(s.value("logTimer",LOGT_NONE).toUInt());
    mapedStatusBox      = s.value("mapedStatusBox",false).toBool();
    mosaicFilterCheck   = s.value("designFilterCheck",false).toBool();
    mosaicWorklistCheck = s.value("mosaicWorklistCheck",false).toBool();
    mosaicOrigCheck     = s.value("mosaicOrigCheck",true).toBool();
    mosaicNewCheck      = s.value("mosaicNewCheck",true).toBool();
    mosaicTestCheck     = s.value("mosaicTestCheck",true).toBool();
    tilingOrigCheck     = s.value("tilingOrigCheck",true).toBool();
    tilingNewCheck      = s.value("tilingNewCheck",true).toBool();
    tilingTestCheck     = s.value("tilingTestCheck",true).toBool();
    tileFilterCheck     = s.value("tileFilterCheck",false).toBool();
    tilingWorklistCheck = s.value("tilingWorklistCheck",false).toBool();
    tm_showAllTiles     = s.value("tm_showAllTiles",false).toBool();
    tm_hideTable        = s.value("tm_hideTable",true).toBool();
    tm_showDebug        = s.value("tm_showDebug",false).toBool();
    tm_autofill         = s.value("tm_autofill",false).toBool();
    tm_showOverlaps     = s.value("tm_showOverlaps",true).toBool();
    lockView            = s.value("lockView",false).toBool();
    splitScreen         = s.value("screenIsSplit",false).toBool();
    showTileBoundary    = s.value("showTileBoundary",true).toBool();
    showMotifBoundary  = s.value("showMotifBoundary",true).toBool();
    showMotif          = s.value("showMotif",true).toBool();
    showExtendedBoundary= s.value("showExtendedBoundary",true).toBool();;
    showMotifCenter     = s.value("showMotifCenter",false).toBool();;
    showTileCenter      = s.value("showTileCenter",false).toBool();;

    compare_transparent = s.value("compare_transparent",false).toBool();
    compare_popup       = s.value("compare_popup",true).toBool();
    view_popup          = s.value("view_popup",true).toBool();
    view_transparent    = s.value("view_transparent",false).toBool();
    filter_transparent  = s.value("filter_transparent",false).toBool();
    display_differences = s.value("compare_differences",true).toBool();
    use_workListForCompare  = s.value("use_workListForCompare",false).toBool();
    generate_workList   = s.value("generate_workList",false).toBool();
    skipExisting        = s.value("skipExisting",false).toBool();
    showBackgroundImage = s.value("showBackgroundImage",true).toBool();
    motifMultiView      = s.value("motifMultiView",false).toBool();
    motifBkgdWhite      = s.value("motifBkgdWhite",false).toBool();
    motifEnlarge        = s.value("motifEnlarge",true).toBool();
    insightMode         = s.value("insightMode",false).toBool();
    cs_showBkgds        = s.value("cs_showBkgds",false).toBool();
    cs_showFrameSettings = s.value("cs_showFrameSettings",false).toBool();
    defaultImageRoot    = s.value("defaultImageRoot",true).toBool();
    defaultMediaRoot    = s.value("defaultMediaRoot",true).toBool();
    saveMosaicTest      = s.value("saveMosaicTest",false).toBool();
    saveTilingTest      = s.value("saveTilingTest",false).toBool();
    vCompLock           = s.value("vCompLock",true).toBool();
    vCompXML            = s.value("vCompXML",true).toBool();
    vCompTile           = s.value("vCompTile",false).toBool();

    viewerType          = static_cast<eViewType>(s.value("viewerType",VIEW_MOSAIC).toUInt());
    mapEditorMode       = static_cast<eMapEditorMode>(s.value("mapEditorMode",MAPED_MODE_MAP).toUInt());
    repeatMode          = static_cast<eRepeatType>(s.value("repeat",REPEAT_DEFINED).toUInt());

    showCenterDebug     = s.value("showCenterDebug",false).toBool();
    showGrid            = s.value("showGrid",false).toBool();
    showGridLayerCenter= s.value("showGridLayerCenter",false).toBool();
    showGridModelCenter = s.value("showGridModelCenter",false).toBool();
    showGridScreenCenter= s.value("showGridScreenCenter",false).toBool();
    gridUnits           = static_cast<eGridUnits>(s.value("gridUnits",GRID_UNITS_SCREEN).toUInt());
    gridType            = static_cast<eGridType>(s.value("gridType2",GRID_ORTHOGONAL).toUInt());
    gridTilingAlgo      = static_cast<eGridTilingAlgo>(s.value("gridTilingAlgo",REGION).toUInt());
    gridModelWidth      = s.value("gridModelWidth",3).toInt();
    gridModelCenter     = s.value("gridModelCenter",false).toBool();
    gridModelSpacing    = s.value("gridModelSpacing",1.0).toDouble();
    gridScreenWidth     = s.value("gridScreenWidth",3).toInt();
    gridScreenSpacing   = s.value("gridScreenSpacing",100).toInt();
    gridScreenCenter    = s.value("gridScreenCenter",false).toBool();
    gridTilingWidth     = s.value("gridTilingWidth",3).toInt();
    gridZLevel          = s.value("gridZLevel",5).toInt();
    snapToGrid          = s.value("snapToGrid",true).toBool();
    gridAngle           = s.value("gridAngle",30.0).toDouble();
    mapedAngle          = s.value("mapedAngle",30.0).toDouble();
    mapedRadius         = s.value("mapedRadius",0.25).toDouble();
    mapedLen            = s.value("mapedLen",1.0).toDouble();
    mapedMergeSensitivity = s.value("mapedMergeSensitivity",1e-2).toDouble();
    protoviewWidth      = s.value("protoviewWidth",3.0).toDouble();
    motifViewWidth      = s.value("motifViewWidth",3.0).toDouble();
    genCycle            = static_cast<eCycleMode>(s.value("genCycle",CYCLE_SAVE_MOSAIC_BMPS).toInt());
    viewCycle           = static_cast<eCycleMode>(s.value("viewCycle",CYCLE_MOSAICS).toInt());
    fileFilter          = static_cast<eLoadType>(s.value("fileFilter",ALL_MOSAICS).toInt());
    versionFilter       = static_cast<eLoadType>(s.value("versionFilter",ALL_MOSAICS).toInt());
    lastCompareName     = s.value("lastCompareName","").toString();

    worklist.load(s);

    QStringList qsl;
    qsl << "#ff1496d2"   // map
        << "#ff006c00"   // tile
        << "#ffd60000"   // motif
        << "#ffffff00"   // del tile
        << "#ff0000ff"   // del motif
        << "#80ffd9d9";  // tile brush
    protoViewColors     = s.value("protoViewColors",qsl).toStringList();

    // ensures indices are in range
    if (viewerType > VIEW_MAX)          viewerType      = VIEW_MAX;
    if (mapEditorMode > MAPED_MODE_MAX) mapEditorMode   = MAPED_MODE_MAP;
    if (repeatMode > REPEAT_MAX)        repeatMode      = REPEAT_MAX;

    // defaults (volatile)
    appInstance     = 0;
    primaryDisplay  = false;
    circleX         = false;
    hideCircles     = false;
    showCenterMouse = false;
    dontReplicate   = false;
    highlightUnit   = false;
    debugMapEnable  = false;
    dontTrapLog     = false;
    measure         = false;
    localCycle      = false;

    updatePanel     = true;
    motifPropagate  = true;
    enableDetachedPages = true;

    configurePaths();
}

Configuration::~Configuration()
{
    save();
}

void Configuration::save()
{
    QSettings s;
    s.setValue("design2",lastLoadedDesignId);
    s.setValue("protoViewMode2",protoViewMode);
    s.setValue("polySides",polySides);
    s.setValue("cycleInterval",cycleInterval);
    s.setValue("lastLoadedTileName",lastLoadedTileName);
    s.setValue("lastLoadedXML",lastLoadedXML);
    s.setValue("image0",image0);
    s.setValue("image1",image1);
    s.setValue("viewerType",viewerType);
    s.setValue("mapEditorMode",mapEditorMode);
    s.setValue("repeat",repeatMode);
    s.setValue("panelName", panelName);
    s.setValue("darkTheme",darkTheme);
    s.setValue("autoLoadStyles",autoLoadStyles);
    s.setValue("verifyMaps",verifyMaps);
    s.setValue("verifyProtos",verifyProtos);
    s.setValue("verifyPopup",verifyPopup);
    s.setValue("verifyDump",verifyDump);
    s.setValue("verifyVerbose",verifyVerbose);
    s.setValue("baseLogName",baseLogName);
    s.setValue("logToStderr",logToStderr);
    s.setValue("logToDisk",logToDisk);
    s.setValue("logToAppDir",logToAppDir);
    s.setValue("logToPanel",logToPanel);
    s.setValue("logDebug",logDebug);
    s.setValue("logNumberLines",logNumberLines);
    s.setValue("logTimer",logTime);
    s.setValue("mapedStatusBox",mapedStatusBox);
    s.setValue("autoLoadTiling",autoLoadTiling);
    s.setValue("autoLoadDesigns",autoLoadDesigns);
    s.setValue("scaleToView",scaleToView);
    s.setValue("stopIfDiff",stopIfDiff);
    s.setValue("designFilterCheck",mosaicFilterCheck);
    s.setValue("mosaicWorklistCheck",mosaicWorklistCheck);
    s.setValue("mosaicOrigCheck",mosaicOrigCheck);
    s.setValue("mosaicNewCheck",mosaicNewCheck);
    s.setValue("mosaicTestCheck",mosaicTestCheck);
    s.setValue("tilingOrigCheck",tilingOrigCheck);
    s.setValue("tilingNewCheck",tilingNewCheck);
    s.setValue("tilingTestCheck",tilingTestCheck);
    s.setValue("tileFilterCheck",tileFilterCheck);
    s.setValue("tilingWorklistCheck",tilingWorklistCheck);
    s.setValue("designFilter",mosaicFilter);
    s.setValue("tileFilter",tileFilter);
    s.setValue("tm_showAllTiles",tm_showAllTiles);
    s.setValue("tm_hideTable",tm_hideTable);
    s.setValue("tm_showDebug",tm_showDebug);
    s.setValue("tm_autofill",tm_autofill);
    s.setValue("tm_showOverlaps",tm_showOverlaps);
    s.setValue("lockView",lockView);
    s.setValue("screenIsSplit",splitScreen);
    s.setValue("showTileBoundary",showTileBoundary);
    s.setValue("showMotifBoundary",showMotifBoundary);
    s.setValue("showMotif",showMotif);
    s.setValue("showExtendedBoundary",showExtendedBoundary);
    s.setValue("showMotifCenter",showMotifCenter);
    s.setValue("showTileCenter",showTileCenter);

    s.setValue("compare_transparent",compare_transparent);
    s.setValue("compare_popup",compare_popup);
    s.setValue("view_popup",view_popup);
    s.setValue("compare_transparent",compare_transparent);
    s.setValue("view_transparent",view_transparent);
    s.setValue("filter_transparent",filter_transparent);
    s.setValue("compare_differences",display_differences);
    s.setValue("use_workListForCompare",use_workListForCompare);
    s.setValue("generate_workList",generate_workList);
    s.setValue("showBackgroundImage",showBackgroundImage);
    s.setValue("motifMultiView",motifMultiView);
    s.setValue("motifBkgdWhite",motifBkgdWhite);
    s.setValue("motifEnlarge",motifEnlarge);
    s.setValue("xmlTool",xmlTool);
    s.setValue("diffTool",diffTool);
    s.setValue("insightMode",insightMode);
    s.setValue("cs_showBkgds",cs_showBkgds);
    s.setValue("cs_showFrameSettings",cs_showFrameSettings);
    s.setValue("defaultImageRoot",defaultImageRoot);
    s.setValue("defaultMediaRoot",defaultMediaRoot);
    s.setValue("rootImageDir",rootImageDir);
    s.setValue("rootMediaDir",rootMediaDir);
    s.setValue("saveMosaicTest",saveMosaicTest);
    s.setValue("saveTilingTest",saveTilingTest);
    s.setValue("vCompLock",vCompLock);
    s.setValue("vCompXML",vCompXML);
    s.setValue("vCompTile",vCompTile);

    s.setValue("protoViewColors",protoViewColors);

    s.setValue("showCenterDebug",showCenterDebug);
    s.setValue("showGrid",showGrid);
    s.setValue("showGridLayerCenter",showGridLayerCenter);
    s.setValue("showGridModelCenter",showGridModelCenter);
    s.setValue("showGridScreenCenter",showGridScreenCenter);
    s.setValue("gridUnits",gridUnits);
    s.setValue("gridTilingAlgo",gridTilingAlgo);
    s.setValue("gridType2",gridType);
    s.setValue("gridModelWidth",gridModelWidth);
    s.setValue("gridModelCenter",gridModelCenter);
    s.setValue("gridModelSpacing",gridModelSpacing);
    s.setValue("gridScreenWidth",gridScreenWidth);
    s.setValue("gridScreenCenter",gridScreenCenter);
    s.setValue("gridTilingWidth",gridTilingWidth);
    s.setValue("snapToGrid",snapToGrid);
    s.setValue("gridScreenSpacing",gridScreenSpacing);
    s.setValue("gridZLevel",gridZLevel);
    s.setValue("gridAngle",gridAngle);
    s.setValue("mapedAngle",mapedAngle);
    s.setValue("mapedRadius",mapedRadius);
    s.setValue("mapedLen",mapedLen);
    s.setValue("mapedMergeSensitivity",mapedMergeSensitivity);
    s.setValue("protoviewWidth",protoviewWidth);
    s.setValue("motifViewWidth",motifViewWidth);
    s.setValue("transparentColor",transparentColor.name(QColor::HexRgb));
    s.setValue("gridColorTiling2",gridColorTiling);
    s.setValue("gridColorModel2",gridColorModel);
    s.setValue("gridColorScreen2",gridColorScreen);
    s.setValue("genCycle",genCycle);
    s.setValue("viewCycle",viewCycle);
    s.setValue("fileFilter",fileFilter);
    s.setValue("versionFilter",versionFilter);
    s.setValue("lastCompareName",lastCompareName);

    worklist.save(s);
}

void Configuration::setViewerType(eViewType  viewerType)
{
    qDebug().noquote() << "Configuration::setViewerType()" << sViewerType[viewerType];
    this->viewerType = viewerType;
}

QString Configuration::getMediaRoot()
{
    QString root;

    root = getMediaRootLocal();
    if (!root.isEmpty())
    {
        return root;
    }

    root = getMediaRootAppdata();

    return root;
}

QString Configuration::getMediaRootLocal()
{
    QString root = qApp->applicationDirPath();
    bool isQmake = false;
#ifdef __APPLE__
    int i = root.indexOf("debug");
    if (i == -1)
    {
        i = root.indexOf("release");
        if (i == -1)
        {
            QString app = qApp->applicationName();
            i = root.indexOf((app));
        }
    }
    if (i != -1)
    {
        int len = root.length() -i;
        root.remove(i,len);
    }
    isQmake = true;
#else
    if (root.indexOf("debug") != -1)
    {
        root.replace("debug","");
        isQmake = true;
    }
    else if (root.indexOf("release") != -1)
    {
        root.replace("release","");
        isQmake = true;
    }
#endif
    if (isQmake)
    {
        root += "../media";
    }
    else
    {
        // could be cmake
        QStringList qsl = root.split("/");
        qsl.removeLast();
        root = qsl.join("/");
        root += "/media";
    }
    root = QDir::cleanPath(root);

    QDir adir(root);
    if (adir.exists())
    {
        return root;
    }
    else
    {
        QString empty;
        return empty;
    }
}

QString Configuration::getMediaRootAppdata()
{
    QString root = QStandardPaths::locate(QStandardPaths::AppLocalDataLocation,"media",QStandardPaths::LocateDirectory);
    return root;
}

QString Configuration::getImageRoot()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/TiledPatternMakerImages/";
    return path;
}

void Configuration::configurePaths()
{
    QString root;
    if (defaultMediaRoot)
    {
        root = getMediaRoot();
    }
    else
    {
        root = rootMediaDir;    // fetched
    }
    root = root.trimmed();
    if (!root.endsWith("/"))
    {
        root += "/";
    }

    rootMediaDir        = root;
    rootTileDir         = root + "tilings/";
    originalTileDir     = root + "tilings/original/";
    newTileDir          = root + "tilings/new_tilings/";
    testTileDir         = root + "tilings/tests/";
    rootMosaicDir       = root + "designs/";
    originalMosaicDir   = root + "designs/original/";
    newMosaicDir        = root + "designs/new_designs/";
    testMosiacDir       = root + "designs/tests/";
    templateDir         = root + "designs/templates/";
    examplesDir         = root + "history/examples/";
    mapsDir             = root + "maps/";
    worklistsDir        = root + "worklists/";

    if (defaultImageRoot)
        rootImageDir  = getImageRoot();
    // else fetched
    rootImageDir = rootImageDir.trimmed();
    if (!rootImageDir.endsWith("/"))
    {
        rootImageDir += "/";
    }
}

void Worklist::set(QStringList & list)
{
    this->list     = list;
    emit theApp->sig_workListChanged();
}

void Worklist::setName(QString listName)
{
    this->listname = listName;
    emit theApp->sig_workListChanged();
}

void Worklist::add(QString designName)
{
    list << designName;
    emit theApp->sig_workListChanged();
}

void Worklist::clear()
{
    listname.clear();
    list.clear();
    emit theApp->sig_workListChanged();
}

void Worklist::load(QSettings & s)
{
    list     = s.value("workList","").toStringList();
    listname = s.value("workListName3","").toString();
}

void Worklist::save(QSettings & s)
{
    s.setValue("workList",list);
    s.setValue("workListName3",listname);
}
