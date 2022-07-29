#include <QSettings>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QApplication>
#include "settings/configuration.h"

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
    viewImages          = s.value("viewImages","").toStringList();
    compareDir0         = s.value("compareDir0","").toString();
    compareDir1         = s.value("compareDir1","").toString();
    panelName           = s.value("panelName","Control").toString();
    mosaicFilter        = s.value("designFilter","").toString();
    tileFilter          = s.value("tileFilter","").toString();
    xmlTool             = s.value("xmlTool","").toString();
    transparentColor    = s.value("transparentColor","black").toString();
    workList            = s.value("workList","").toStringList();
    darkTheme           = s.value("darkTheme",false).toBool();
    autoLoadStyles      = s.value("autoLoadStyles",false).toBool();
    autoLoadTiling      = s.value("autoLoadTiling",false).toBool();
    autoLoadDesigns     = s.value("autoLoadDesigns",false).toBool();
    loadTilingMulti     = s.value("loadTilingMulti",false).toBool();
    loadTilingModify    = s.value("loadTilingModify",false).toBool();
    scaleToView         = s.value("scaleToView",true).toBool();
    stopIfDiff          = s.value("stopIfDiff",true).toBool();
    verifyMaps          = s.value("verifyMaps",false).toBool();
    verifyProtos        = s.value("verifyProtos",false).toBool();
    verifyPopup         = s.value("verifyPopup",false).toBool();
    verifyDump          = s.value("verifyDump",false).toBool();
    verifyVerbose       = s.value("verifyVerbose",false).toBool();
    logToStderr         = s.value("logToStderr",true).toBool();
    logToDisk           = s.value("logToDisk",true).toBool();
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
    tileFilterCheck     = s.value("tileFilterCheck",false).toBool();
    tm_showAllFeatures  = s.value("tm_showAllFeatures",false).toBool();
    tm_hideTable        = s.value("tm_hideTable",true).toBool();
    tm_showDebug        = s.value("tm_showDebug",false).toBool();
    tm_autofill         = s.value("tm_autofill",false).toBool();
    tm_showOverlaps     = s.value("tm_showOverlaps",true).toBool();
    lockView            = s.value("lockView",false).toBool();
    splitScreen         = s.value("screenIsSplit",false).toBool();
    showFeatureBoundary = s.value("showFeatureBoundary",true).toBool();
    showFigureBoundary  = s.value("showFigureBoundary",true).toBool();
    showExtendedBoundary= s.value("showExtendedBoundary",true).toBool();;

    compare_popup       = s.value("compare_popup",true).toBool();
    view_popup          = s.value("view_popup",true).toBool();
    compare_transparent = s.value("compare_transparent",false).toBool();
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

    viewerType          = static_cast<eViewType>(s.value("viewerType",VIEW_MOSAIC).toUInt());
    mapEditorMode       = static_cast<eMapEditorMode>(s.value("mapEditorMode",MAPED_MODE_MAP).toUInt());
    repeatMode          = static_cast<eRepeatType>(s.value("repeat",REPEAT_DEFINED).toUInt());

    showCenterDebug     = s.value("showCenterDebug",false).toBool();
    showGrid            = s.value("showGrid",false).toBool();
    gridUnits           = static_cast<eGridUnits>(s.value("gridUnits",GRID_UNITS_SCREEN).toUInt());
    gridType            = static_cast<eGridType>(s.value("gridType2",GRID_ORTHOGONAL).toUInt());
    gridModelWidth      = s.value("gridModelWidth",3).toInt();
    gridModelCenter     = s.value("gridModelCenter",false).toBool();
    gridModelSpacing    = s.value("gridModelSpacing",1.0).toDouble();
    gridScreenWidth     = s.value("gridScreenWidth",3).toInt();
    gridScreenSpacing   = s.value("gridScreenSpacing",100).toInt();
    gridScreenCenter    = s.value("gridScreenCenter",false).toBool();
    snapToGrid          = s.value("snapToGrid",true).toBool();
    gridAngle           = s.value("gridAngle",30.0).toDouble();
    mapedAngle          = s.value("mapedAngle",30.0).toDouble();
    mapedRadius         = s.value("mapedRadius",0.25).toDouble();
    mapedLen            = s.value("mapedLen",1.0).toDouble();
    mapedMergeSensitivity = s.value("mapedMergeSensitivity",1e-2).toDouble();
    genCycle            = static_cast<eCycleMode>(s.value("genCycle",CYCLE_SAVE_STYLE_BMPS).toInt());
    viewCycle           = static_cast<eCycleMode>(s.value("viewCycle",CYCLE_STYLES).toInt());
    fileFilter          = static_cast<eLoadType>(s.value("fileFilter",LOAD_ALL).toInt());

    QStringList qsl;
    qsl << "#ff1496d2"   // map
        << "#ff006c00"   // feature
        << "#ffd60000"   // figure
        << "#ffffff00"   // del feature
        << "#ff0000ff"   // del figure
        << "#ffffd9d9";  // feature brush
    protoViewColors     = s.value("protoViewColors",qsl).toStringList();

    // ensures indices are in range
    if (viewerType > VIEW_MAX)          viewerType      = VIEW_MAX;
    if (mapEditorMode > MAPED_MODE_MAX) mapEditorMode   = MAPED_MODE_MAP;
    if (repeatMode > REPEAT_MAX)        repeatMode      = REPEAT_MAX;

    // defaults (volatile)
    primaryDisplay  = false;
    circleX         = false;
    hideCircles     = false;
    updatePanel     = true;
    showCenterMouse = false;
    enableDetachedPages = true;
    dontReplicate   = false;
    highlightUnit   = false;
    debugMapEnable  = false;
    dontTrapLog     = false;
    measure         = false;

    configurePaths();


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
    s.setValue("viewImages",viewImages);
    s.setValue("compareDir0",compareDir0);
    s.setValue("compareDir1",compareDir1);
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
    s.setValue("logToStderr",logToStderr);
    s.setValue("logToDisk",logToDisk);
    s.setValue("logToPanel",logToPanel);
    s.setValue("logDebug",logDebug);
    s.setValue("logNumberLines",logNumberLines);
    s.setValue("logTimer",logTime);
    s.setValue("mapedStatusBox",mapedStatusBox);
    s.setValue("autoLoadTiling",autoLoadTiling);
    s.setValue("loadTilingMulti",loadTilingMulti);
    s.setValue("loadTilingModify",loadTilingModify);
    s.setValue("autoLoadDesigns",autoLoadDesigns);
    s.setValue("scaleToView",scaleToView);
    s.setValue("stopIfDiff",stopIfDiff);
    s.setValue("designFilterCheck",mosaicFilterCheck);
    s.setValue("mosaicWorklistCheck",mosaicWorklistCheck);
    s.setValue("mosaicOrigCheck",mosaicOrigCheck);
    s.setValue("mosaicNewCheck",mosaicNewCheck);
    s.setValue("mosaicTestCheck",mosaicTestCheck);
    s.setValue("tileFilterCheck",tileFilterCheck);
    s.setValue("designFilter",mosaicFilter);
    s.setValue("tileFilter",tileFilter);
    s.setValue("tm_showAllFeatures",tm_showAllFeatures);
    s.setValue("tm_hideTable",tm_hideTable);
    s.setValue("tm_showDebug",tm_showDebug);
    s.setValue("tm_autofill",tm_autofill);
    s.setValue("tm_showOverlaps",tm_showOverlaps);
    s.setValue("lockView",lockView);
    s.setValue("screenIsSplit",splitScreen);
    s.setValue("showFeatureBoundary",showFeatureBoundary);
    s.setValue("showFigureBoundary",showFigureBoundary);
    s.setValue("showExtendedBoundary",showExtendedBoundary);
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
    s.setValue("insightMode",insightMode);
    s.setValue("cs_showBkgds",cs_showBkgds);
    s.setValue("cs_showFrameSettings",cs_showFrameSettings);
    s.setValue("defaultImageRoot",defaultImageRoot);
    s.setValue("defaultMediaRoot",defaultMediaRoot);
    s.setValue("rootImageDir",rootImageDir);
    s.setValue("rootMediaDir",rootMediaDir);

    s.setValue("workList",workList);
    s.setValue("protoViewColors",protoViewColors);

    s.setValue("showCenterDebug",showCenterDebug);
    s.setValue("showGrid",showGrid);
    s.setValue("gridUnits",gridUnits);
    s.setValue("gridType2",gridType);
    s.setValue("gridModelWidth",gridModelWidth);
    s.setValue("gridModelCenter",gridModelCenter);
    s.setValue("gridModelSpacing",gridModelSpacing);
    s.setValue("gridScreenWidth",gridScreenWidth);
    s.setValue("gridScreenCenter",gridScreenCenter);
    s.setValue("snapToGrid",snapToGrid);
    s.setValue("gridScreenSpacing",gridScreenSpacing);
    s.setValue("gridAngle",gridAngle);
    s.setValue("mapedAngle",mapedAngle);
    s.setValue("mapedRadius",mapedRadius);
    s.setValue("mapedLen",mapedLen);
    s.setValue("mapedMergeSensitivity",mapedMergeSensitivity);
    s.setValue("transparentColor",transparentColor.name(QColor::HexRgb));
    s.setValue("genCycle",genCycle);
    s.setValue("viewCycle",viewCycle);
    s.setValue("fileFilter",fileFilter);
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

    qInfo().noquote() << "App Media root:" << root;

    rootMediaDir        = root;
    rootTileDir         = root + "tilings/";
    originalTileDir     = root + "tilings/original/";
    newTileDir          = root + "tilings/new_tilings/";
    rootDesignDir       = root + "designs/";
    originalDesignDir   = root + "designs/original/";
    newDesignDir        = root + "designs/new_designs/";
    testDesignDir       = root + "designs/tests/";
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
