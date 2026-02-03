#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QApplication>
#include "model/settings/configuration.h"
#include "sys/enums/eviewtype.h"
#include "sys/sys.h"
#include "sys/tiledpatternmaker.h"

extern TiledPatternMaker * theApp;

Configuration::Configuration()
{
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
    qRegisterMetaType<QList<int>>();
#endif

    firstBirthday = false;

    // peersist
    QSettings s;

    lastLoadedTiling    = s.value("lastLoadedTiling","").toString();
    lastLoadedMosaic    = s.value("lastLoadedMosaic","").toString();
    lastLoadedLegacy    = s.value("lastLoadedLegacy","").toString();
    lastLoadedType      = (eLoadUnitType)s.value("lastLoadedType2",LT_UNDEFINED).toUInt();

    cycleInterval       = s.value("cycleInterval",4).toInt();
    polySides           = s.value("polySides",8).toInt();
    protoViewMode       = s.value("protoViewMode2",SHOW_TILES | SHOW_MOTIFS).toInt();
    rootMediaDir        = s.value("rootMediaDir",getMediaRoot()).toString();
    rootImageDir        = s.value("rootImageDir",getImageRoot()).toString();
    BMPCompare0         = s.value("image0","").toString();
    BMPCompare1         = s.value("image1","").toString();
    pageName            = s.value("panelName","Load").toString();
    mosaicFilter        = s.value("designFilter","").toString();
    tileFilter          = s.value("tileFilter","").toString();
    xmlTool             = s.value("xmlTool","").toString();
    diffTool            = s.value("diffTool","").toString();
    transparentColor    = s.value("transparentColor","black").toString();
    gridColorTiling     = s.value("gridColorTiling2", QColor(Qt::red).name(QColor::HexArgb)).toString();
    gridColorModel      = s.value("gridColorModel2", QColor(Qt::green).name(QColor::HexArgb)).toString();
    gridColorScreen     = s.value("gridColorScreen2", QColor(Qt::blue).name(QColor::HexArgb)).toString();
    colorTheme          = static_cast<eColorTheme>(s.value("colorTheme",AUTO_THEME).toUInt());
    autoLoadLast        = s.value("autoLoadLast",false).toBool();
    scaleToView         = s.value("scaleToView",true).toBool();
    verifyMaps          = s.value("verifyMaps",false).toBool();
    forceVerifyProtos   = s.value("verifyProtos",false).toBool();
    verifyPopup         = s.value("verifyPopup",false).toBool();
    verifyDump          = s.value("verifyDump",false).toBool();
    verifyVerbose       = s.value("verifyVerbose",false).toBool();
    slowCleanseMapMerges = s.value("unDuplicateMerge",false).toBool();
    buildEmptyNmaps     = s.value("buildEmptyNmaps",false).toBool();
    baseLogName         = s.value("baseLogName","tiledPatternMakerLog").toString();
    logToStderr         = s.value("logToStderr",true).toBool();
    logToDisk           = s.value("logToDisk",true).toBool();
    logToAppDir         = s.value("logToAppDir",false).toBool();
    logToPanel          = s.value("logToPanel",true).toBool();
    logNumberLines      = s.value("logNumberLines",true).toBool();
    logDebug            = s.value("logDebug",false).toBool();
    enableLog2          = s.value("enableLog2",false).toBool();
    logTime             = static_cast<eLogTimer>(s.value("logTimer",LOGT_NONE).toUInt());
    mapedStatusBox      = s.value("mapedStatusBox",false).toBool();
    mapedLoadCopies     = s.value("mapedLoadCopies",false).toBool();
    mapedOldTemplates   = s.value("mapedOldTemplates",false).toBool();
    mosaicFilterCheck   = s.value("designFilterCheck",false).toBool();
    mosaicWorklistCheck = s.value("mosaicWorklistCheck",false).toBool();
    mosaicWorklistXCheck = s.value("mosaicWorklistXCheck",false).toBool();
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
    tilingWorklistXCheck = s.value("tilingWorklistXCheck",false).toBool();
    tm_showExcludes     = s.value("tm_showAllTiles",false).toBool();
    tm_hideTranslations = s.value("tm_hideTranslations",true).toBool();
    tm_showDebug        = s.value("tm_showDebug",false).toBool();
    tm_loadFill         = s.value("tm_autofill",false).toBool();
    tm_tileColorMode    = static_cast<eTileColorModes>(s.value("tm_tileColorMode",TILE_COLOR_TOUCHING).toUInt());
    lockView            = s.value("lockView",false).toBool();
    multiView           = s.value("multiView",false).toBool();
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
    compare_filterColor = s.value("filter_transparent",false).toBool();

    use_workListForCompare= s.value("use_workListForCompare",false).toBool();
    skipExisting        = s.value("skipExisting",false).toBool();
    motifMakerView      = static_cast<eMotifView>(s.value("motifMakerView",MOTIF_VIEW_SOLO).toInt());
    limitViewSize       = s.value("limitViewSize",true).toBool();
    disableSplash       = s.value("disableSplash",false).toBool();
    disableResizeNotify = s.value("disableResizeNotify",false).toBool();
    debugMotifView      = s.value("debugMotifView",false).toBool();
    insightMode         = s.value("insightMode",false).toBool();
    cs_showBkgds        = s.value("cs_showBkgds",false).toBool();
    defaultImageRoot    = s.value("defaultImageRoot",true).toBool();
    defaultMediaRoot    = s.value("defaultMediaRoot",true).toBool();
    saveMosaicTest      = s.value("saveMosaicTest",false).toBool();
    saveTilingTest      = s.value("saveTilingTest",false).toBool();
    vCompLock           = s.value("vCompLock",true).toBool();
    vCompXML            = s.value("vCompXML",true).toBool();
    sysinfo_all         = s.value("sysinfo_all",false).toBool();

    mapEditorMode       = static_cast<eMapEditorMode>(s.value("mapEditorMode",MAPED_MODE_MAP).toUInt());
    repeatMode          = static_cast<eRepeatType>(s.value("repeat",REPEAT_DEFINED).toUInt());

    debugTabIndex       = s.value("debugTabIndex",0).toUInt();
    showCenterDebug     = s.value("showCenterDebug",false).toBool();
    showGridModelCenter = s.value("showGridModelCenter",false).toBool();
    showGridViewCenter= s.value("showGridScreenCenter",false).toBool();
    gridUnits           = static_cast<eGridUnits>(s.value("gridUnits",GRID_UNITS_SCREEN).toUInt());
    if (gridUnits > GRID_UNITS_MAX) gridUnits = GRID_UNITS_MAX;
    gridType            = static_cast<eGridType>(s.value("gridType2",GRID_ORTHOGONAL).toUInt());
    if (gridType > GRID_TYPE_MAX) gridType = GRID_TYPE_MAX;
    gridTilingAlgo      = static_cast<eGridTilingAlgo>(s.value("gridTilingAlgo",REGION).toUInt());
    gridModelWidth      = s.value("gridModelWidth",3).toInt();
    gridModelCenter     = s.value("gridModelCenter",false).toBool();
    gridModelSpacing    = s.value("gridModelSpacing",1.0).toDouble();
    gridScreenWidth     = s.value("gridScreenWidth",3).toInt();
    gridScreenSpacing   = s.value("gridScreenSpacing",100).toInt();
    gridScreenCenter    = s.value("gridScreenCenter",false).toBool();
    gridTilingWidth     = s.value("gridTilingWidth",3).toInt();
    gridZLevel          = (eZLevel)s.value("gridZLevel",GRID_ZLEVEL).toInt();
    debugViewConfig     = s.value("debugViewConfig2",0xffff).toUInt();
    mapedCleanseLevel   = s.value("mapedCleanseLevel",44).toUInt();
    tm_snapToGrid       = s.value("snapToGrid",true).toBool();
    tm_snapOnly         = s.value("tm_snapOnly",false).toBool();
    gridAngle           = s.value("gridAngle",30.0).toDouble();
    mapedAngle          = s.value("mapedAngle",30.0).toDouble();
    mapedRadius         = s.value("mapedRadius",2.0).toDouble();
    mapedLen            = s.value("mapedLen",1.0).toDouble();
    mapedMergeSensitivity = s.value("mapedMergeSensitivity",1e-2).toDouble();
    protoviewWidth      = s.value("protoviewWidth",3.0).toDouble();
    motifViewWidth      = s.value("motifViewWidth",3.0).toDouble();
    multithreadedGeneration = s.value("multithreadedGeneration",true).toBool();
    includeBkgdGeneration   = s.value("includeBkgdGeneration",false).toBool();
    
    imageType           = static_cast<eImageType>(s.value("imageType",IMAGE_MOSAICS).toInt());
    imageFileFilter     = static_cast<eLoadType>(s.value("fileFilter",ALL_MOSAICS).toInt());

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
    viewColors          = s.value("viewColors3",qsl2).toStringList();
    if (viewColors.size() != NUM_VIEW_TYPES)
    {
        int diff = viewColors.size() - NUM_VIEW_TYPES;
        if (diff > 0)
        {
            viewColors.resize(NUM_VIEW_TYPES);
        }
        else
        {
            for (int i=0; i < abs(diff); i++)
            {
                viewColors << "#ffffff";
            }
        }
    }

    // ensures indices are in range
    if (mapEditorMode > MAPED_MODE_MAX) mapEditorMode   = MAPED_MODE_MAP;
    if (repeatMode > REPEAT_MAX)        repeatMode      = REPEAT_MAX;

    configurePaths();
}

Configuration::~Configuration()
{
    if (!firstBirthday)
    {
        save();
    }
}

void Configuration::save()
{
    QSettings s;
    s.setValue("lastLoadedTiling",lastLoadedTiling.get());
    s.setValue("lastLoadedMosaic",lastLoadedMosaic.get());
    s.setValue("lastLoadedLegacy",lastLoadedLegacy.get());
    s.setValue("lastLoadedType2",lastLoadedType);

    s.setValue("protoViewMode2",protoViewMode);
    s.setValue("polySides",polySides);
    s.setValue("cycleInterval",cycleInterval);
    s.setValue("image0",BMPCompare0);
    s.setValue("image1",BMPCompare1);
    s.setValue("mapEditorMode",mapEditorMode);
    s.setValue("repeat",repeatMode);
    s.setValue("panelName", pageName);
    s.setValue("colorTheme",colorTheme);
    s.setValue("autoLoadLast",autoLoadLast);
    s.setValue("verifyMaps",verifyMaps);
    s.setValue("verifyProtos",forceVerifyProtos);
    s.setValue("verifyPopup",verifyPopup);
    s.setValue("verifyDump",verifyDump);
    s.setValue("verifyVerbose",verifyVerbose);
    s.setValue("unDuplicateMerge",slowCleanseMapMerges);
    s.setValue("buildEmptyNmaps",buildEmptyNmaps);
    s.setValue("baseLogName",baseLogName);
    s.setValue("logToStderr",logToStderr);
    s.setValue("logToDisk",logToDisk);
    s.setValue("logToAppDir",logToAppDir);
    s.setValue("logToPanel",logToPanel);
    s.setValue("logDebug",logDebug);
    s.setValue("enableLog2",enableLog2);
    s.setValue("logNumberLines",logNumberLines);
    s.setValue("logTimer",logTime);
    s.setValue("mapedStatusBox",mapedStatusBox);
    s.setValue("mapedLoadCopies",mapedLoadCopies);
    s.setValue("mapedOldTemplates",mapedOldTemplates);
    s.setValue("scaleToView",scaleToView);
    s.setValue("designFilterCheck",mosaicFilterCheck);
    s.setValue("mosaicWorklistCheck",mosaicWorklistCheck);
    s.setValue("mosaicWorklistXCheck",mosaicWorklistXCheck);
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
    s.setValue("tilingWorklistXCheck",tilingWorklistXCheck);
    s.setValue("designFilter",mosaicFilter);
    s.setValue("tileFilter",tileFilter);
    s.setValue("tm_showAllTiles",tm_showExcludes);
    s.setValue("tm_hideTranslations",tm_hideTranslations);
    s.setValue("tm_showDebug",tm_showDebug);
    s.setValue("tm_autofill",tm_loadFill);
    s.setValue("tm_tileColorMode",tm_tileColorMode);
    s.setValue("lockView",lockView);
    s.setValue("multiView",multiView);
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
    s.setValue("filter_transparent",compare_filterColor);

    s.setValue("use_workListForCompare",use_workListForCompare);
    s.setValue("motifMakerView",motifMakerView);
    s.setValue("limitViewSize",limitViewSize);
    s.setValue("disableSplash",disableSplash);
    s.setValue("disableResizeNotify",disableResizeNotify);
    s.setValue("debugMotifView",debugMotifView);
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
    s.setValue("sysinfo_all",sysinfo_all);

    s.setValue("protoViewColors",protoViewColors);
    s.setValue("viewColors3",viewColors);

    s.setValue("showCenterDebug",showCenterDebug);
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
    s.setValue("snapToGrid",tm_snapToGrid);
    s.setValue("snapOnly",tm_snapOnly);
    s.setValue("gridScreenSpacing",gridScreenSpacing);
    s.setValue("gridZLevel",gridZLevel);
    s.setValue("debugViewConfig2",debugViewConfig);
    s.setValue("mapedCleanseLevel",mapedCleanseLevel);
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
    s.setValue("imageType",imageType);
    s.setValue("multithreadedGeneration",multithreadedGeneration);
    s.setValue("includeBkgdGeneration",includeBkgdGeneration);
    s.setValue("fileFilter",imageFileFilter);
    s.setValue("lastCompareName",lastCompareName);
    s.setValue("debugTabIndex",debugTabIndex);

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
            return adir.canonicalPath();
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
    Sys::templateDir         = root + "templates/";
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
