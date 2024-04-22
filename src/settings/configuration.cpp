#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QApplication>
#include "enums/eviewtype.h"
#include "settings/configuration.h"
#include "tiledpatternmaker.h"
#include "misc/sys.h"

extern TiledPatternMaker * theApp;

Configuration::Configuration()
{
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
    qRegisterMetaType<QList<int>>();
    qRegisterMetaTypeStreamOperators<QList<int> >("QList<int>");
#endif

    // peersist
    QSettings s;

    lastLoadedLegacyDes = s.value("design3","").toString();
    lastLoadedTiling    = s.value("lastLoadedTileName","").toString();
    lastLoadedMosaic    = s.value("lastLoadedXML","").toString();
    cycleInterval       = s.value("cycleInterval",4).toInt();
    polySides           = s.value("polySides",8).toInt();
    protoViewMode       = s.value("protoViewMode2",PROTO_ALL_TILES | PROTO_ALL_MOTIFS).toInt();
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
    colorTheme          = static_cast<eColorTheme>(s.value("colorTheme",AUTO_THEME).toUInt());
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
    unDuplicateMerge    = s.value("unDuplicateMerge",false).toBool();
    buildEmptyNmaps     = s.value("buildEmptyNmaps",false).toBool();
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
    mosaicSortCheck     = s.value("mosaicSortCheck",false).toBool();
    showWithBkgds       = s.value("showWithBkgds",false).toBool();
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
    bigScreen           = s.value("bigScreen",false).toBool();
    showTileBoundary    = s.value("showTileBoundary",true).toBool();
    showMotifBoundary   = s.value("showMotifBoundary",true).toBool();
    showMotif           = s.value("showMotif",true).toBool();
    showExtendedBoundary= s.value("showExtendedBoundary",true).toBool();;
    showMotifCenter     = s.value("showMotifCenter",false).toBool();;
    showTileCenter      = s.value("showTileCenter",false).toBool();;

    compare_transparent = s.value("compare_transparent",false).toBool();
    compare_popup       = s.value("compare_popup",true).toBool();
    filterColor         = s.value("filter_transparent",false).toBool();
    display_differences = s.value("compare_differences",true).toBool();
    use_workListForCompare= s.value("use_workListForCompare",false).toBool();
    skipExisting        = s.value("skipExisting",false).toBool();
    showBackgroundImage = s.value("showBackgroundImage",true).toBool();
    motifMultiView      = s.value("motifMultiView",false).toBool();
    motifEnlarge        = s.value("motifEnlarge",true).toBool();
    limitViewSize       = s.value("limitViewSize",true).toBool();
    insightMode         = s.value("insightMode",false).toBool();
    cs_showBkgds        = s.value("cs_showBkgds",false).toBool();
    defaultImageRoot    = s.value("defaultImageRoot",true).toBool();
    defaultMediaRoot    = s.value("defaultMediaRoot",true).toBool();
    saveMosaicTest      = s.value("saveMosaicTest",false).toBool();
    saveTilingTest      = s.value("saveTilingTest",false).toBool();
    vCompLock           = s.value("vCompLock",true).toBool();
    vCompXML            = s.value("vCompXML",true).toBool();
    vCompTile           = s.value("vCompTile",false).toBool();

    mapEditorMode       = static_cast<eMapEditorMode>(s.value("mapEditorMode",MAPED_MODE_MAP).toUInt());
    repeatMode          = static_cast<eRepeatType>(s.value("repeat",REPEAT_DEFINED).toUInt());

    showCenterDebug     = s.value("showCenterDebug",false).toBool();
    showGrid            = s.value("showGrid",false).toBool();
    lockGridToView      = s.value("lockGridToView",true).toBool();
    showGridLayerCenter= s.value("showGridLayerCenter",false).toBool();
    showGridModelCenter = s.value("showGridModelCenter",false).toBool();
    showGridViewCenter= s.value("showGridScreenCenter",false).toBool();
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
    debugViewConfig     = s.value("debugViewConfig",0).toUInt();
    snapToGrid          = s.value("snapToGrid",true).toBool();
    gridAngle           = s.value("gridAngle",30.0).toDouble();
    mapedAngle          = s.value("mapedAngle",30.0).toDouble();
    mapedRadius         = s.value("mapedRadius",0.25).toDouble();
    mapedLen            = s.value("mapedLen",1.0).toDouble();
    mapedMergeSensitivity = s.value("mapedMergeSensitivity",1e-2).toDouble();
    protoviewWidth      = s.value("protoviewWidth",3.0).toDouble();
    motifViewWidth      = s.value("motifViewWidth",3.0).toDouble();
    genCycleMosaic      = s.value("genCycleMosaic",true).toBool();
    multithreadedGeneration = s.value("multithreadedGeneration",true).toBool();
    
    viewCycle2          = static_cast<eCycleMode>(s.value("viewCycle2",CYCLE_VIEW_MOSAICS).toInt());

    genFileFilter       = static_cast<eLoadType>(s.value("fileFilter",ALL_MOSAICS).toInt());
    viewFileFilter      = static_cast<eLoadType>(s.value("viewFilter",ALL_MOSAICS).toInt());
    versionFilter       = static_cast<eLoadType>(s.value("versionFilter",ALL_MOSAICS).toInt());

    lastCompareName     = s.value("lastCompareName","").toString();

    worklist.load(s);

    QStringList qsl;
    qsl << "#ff1496d2"   // map
        << "#ff006c00"   // tile
        << "#ffffff00"   // motif
        << "#ffd60000"   // deltile
        << "#ff0000ff"   // del motif
        << "#80ffd9d9";  // tile brush
    protoViewColors     = s.value("protoViewColors",qsl).toStringList();

    QStringList qsl2;
    for (int i = 0; i < NUM_VIEW_TYPES; i++)
    {
        qsl2 << "#ffffff";
    }
    viewColors          = s.value("viewColors2",qsl2).toStringList();

    // ensures indices are in range
    if (mapEditorMode > MAPED_MODE_MAX) mapEditorMode   = MAPED_MODE_MAP;
    if (repeatMode > REPEAT_MAX)        repeatMode      = REPEAT_MAX;

    configurePaths();
}

Configuration::~Configuration()
{
    save();
}

void Configuration::save()
{
    QSettings s;
    s.setValue("design3",lastLoadedLegacyDes);
    s.setValue("lastLoadedTileName",lastLoadedTiling);
    s.setValue("lastLoadedXML",lastLoadedMosaic);
    s.setValue("protoViewMode2",protoViewMode);
    s.setValue("polySides",polySides);
    s.setValue("cycleInterval",cycleInterval);
    s.setValue("image0",image0);
    s.setValue("image1",image1);
    s.setValue("mapEditorMode",mapEditorMode);
    s.setValue("repeat",repeatMode);
    s.setValue("panelName", panelName);
    s.setValue("colorTheme",colorTheme);
    s.setValue("autoLoadStyles",autoLoadStyles);
    s.setValue("verifyMaps",verifyMaps);
    s.setValue("verifyProtos",verifyProtos);
    s.setValue("verifyPopup",verifyPopup);
    s.setValue("verifyDump",verifyDump);
    s.setValue("verifyVerbose",verifyVerbose);
    s.setValue("unDuplicateMerge",unDuplicateMerge);
    s.setValue("buildEmptyNmaps",buildEmptyNmaps);
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
    s.setValue("mosaicSortCheck",mosaicSortCheck);
    s.setValue("showWithBkgds",showWithBkgds);
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
    s.setValue("bigScreen",bigScreen);
    s.setValue("showTileBoundary",showTileBoundary);
    s.setValue("showMotifBoundary",showMotifBoundary);
    s.setValue("showMotif",showMotif);
    s.setValue("showExtendedBoundary",showExtendedBoundary);
    s.setValue("showMotifCenter",showMotifCenter);
    s.setValue("showTileCenter",showTileCenter);

    s.setValue("compare_transparent",compare_transparent);
    s.setValue("compare_popup",compare_popup);
    s.setValue("compare_transparent",compare_transparent);
    s.setValue("filter_transparent",filterColor);
    s.setValue("compare_differences",display_differences);
    s.setValue("use_workListForCompare",use_workListForCompare);
    s.setValue("showBackgroundImage",showBackgroundImage);
    s.setValue("motifMultiView",motifMultiView);
    s.setValue("limitViewSize",limitViewSize);
    s.setValue("motifEnlarge",motifEnlarge);
    s.setValue("xmlTool",xmlTool);
    s.setValue("diffTool",diffTool);
    s.setValue("insightMode",insightMode);
    s.setValue("cs_showBkgds",cs_showBkgds);
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
    s.setValue("viewColors2",viewColors);

    s.setValue("showCenterDebug",showCenterDebug);
    s.setValue("showGrid",showGrid);
    s.setValue("lockGridToView",lockGridToView);
    s.setValue("showGridLayerCenter",showGridLayerCenter);
    s.setValue("showGridModelCenter",showGridModelCenter);
    s.setValue("showGridScreenCenter",showGridViewCenter);
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
    s.setValue("debugViewConfig",debugViewConfig);
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
    s.setValue("genCycleMosaic",genCycleMosaic);
    s.setValue("multithreadedGeneration",multithreadedGeneration);
    s.setValue("viewCycle2",viewCycle2);
    s.setValue("fileFilter",genFileFilter);
    s.setValue("viewFilter",viewFileFilter);
    s.setValue("versionFilter",versionFilter);
    s.setValue("lastCompareName",lastCompareName);

    worklist.save(s);
}

#if 0
void Configuration::setViewerType(eViewType  viewerType)
{
    qDebug().noquote() << "Configuration::setViewerType()" << sViewerType[viewerType];
    this->viewerType = viewerType;
}
#endif

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
        // could be old qmake
        QString qroot =  root + "../media";
        QDir adir(qroot);
        if (adir.exists())
        {
            return qroot;
        }

        // could be new qmake
        QStringList qsl2 = root.split("/");
        qsl2.removeLast();
        qsl2.removeLast();
        qsl2.removeLast();
        qsl2.removeLast();
        QString c2root = qsl2.join("/");
        c2root += "/media";
        c2root = QDir::cleanPath(c2root);
        QDir cdir(c2root);
        if (cdir.exists())
        {
            return c2root;
        }
    }

    // could be old cmake
    QStringList qsl1 = root.split("/");
    qsl1.removeLast();
    QString c1root = qsl1.join("/");
    c1root += "/media";
    c1root = QDir::cleanPath(c1root);
    QDir bdir(c1root);
    if (bdir.exists())
    {
        return c1root;
    }

    // could be new cmake
    QStringList qsl2 = root.split("/");
    qsl2.removeLast();
    qsl2.removeLast();
    qsl2.removeLast();
    QString c2root = qsl2.join("/");
    c2root += "/media";
    c2root = QDir::cleanPath(c2root);
    QDir cdir(c2root);
    if (cdir.exists())
    {
        return c2root;
    }

    // not found
    return QString();
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

    rootMediaDir             = root;
    Sys::rootTileDir         = root + "tilings/";
    Sys::originalTileDir     = root + "tilings/original/";
    Sys::newTileDir          = root + "tilings/new_tilings/";
    Sys::testTileDir         = root + "tilings/tests/";
    Sys::rootMosaicDir       = root + "designs/";
    Sys::originalMosaicDir   = root + "designs/original/";
    Sys::newMosaicDir        = root + "designs/new_designs/";
    Sys::testMosiacDir       = root + "designs/tests/";
    Sys::templateDir         = root + "designs/templates/";
    Sys::examplesDir         = root + "history/examples/";
    Sys::mapsDir             = root + "maps/";
    Sys::worklistsDir        = root + "worklists/";

    if (defaultImageRoot)
        rootImageDir  = getImageRoot();
    // else fetched
    rootImageDir = rootImageDir.trimmed();
    if (!rootImageDir.endsWith("/"))
    {
        rootImageDir += "/";
    }
}

void Worklist::set(QString name, QStringList & list)
{
    listname   = name;
    this->list = list;
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
