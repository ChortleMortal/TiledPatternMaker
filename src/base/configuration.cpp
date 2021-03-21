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

#include "base/configuration.h"

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
    cycleMode           = static_cast<eCycleMode>(s.value("cycleMode",0).toInt());
    cycleInterval       = s.value("cycleInterval",4).toInt();
    polySides           = s.value("polySides",8).toInt();

    lastLoadedTileName  = s.value("lastLoadedTileName","").toString();
    lastLoadedXML       = s.value("lastLoadedXML","").toString();

    rootMediaDir        = s.value("rootMediaDir",getMediaRoot()).toString();
    rootImageDir        = s.value("rootImageDir",getImageRoot()).toString();
    image0              = s.value("image0","").toString();
    image1              = s.value("image1","").toString();
    compareDir0         = s.value("compareDir0","").toString();
    compareDir1         = s.value("compareDir1","").toString();
    panelName           = s.value("panelName","Control").toString();

    mosaicFilter        = s.value("designFilter","").toString();
    tileFilter          = s.value("tileFilter","").toString();
    xmlTool             = s.value("xmlTool","").toString();

    workList            = s.value("workList","").toStringList();

    autoLoadStyles      = s.value("autoLoadStyles",false).toBool();
    autoLoadTiling      = s.value("autoLoadTiling",false).toBool();
    autoLoadDesigns     = s.value("autoLoadDesigns",false).toBool();
    loadTilingMulti     = s.value("loadReplaceTiling",true).toBool();
    scaleToView         = s.value("scaleToView",true).toBool();
    stopIfDiff          = s.value("stopIfDiff",true).toBool();
    verifyMaps          = s.value("verifyMaps",false).toBool();
    verifyDump          = s.value("verifyDump",false).toBool();
    verifyVerbose       = s.value("verifyVerbose",false).toBool();
    logToStderr         = s.value("logToStderr",true).toBool();
    logToDisk           = s.value("logToDisk",true).toBool();
    logToPanel          = s.value("logToPanel",true).toBool();
    logNumberLines      = s.value("logNumberLines",true).toBool();
    logWarningsOnly     = s.value("logWarningsOnly",false).toBool();
    logTime             = static_cast<eLogTimer>(s.value("logTimer",LOGT_NONE).toUInt());
    mapedStatusBox      = s.value("mapedStatusBox",false).toBool();
    mosaicFilterCheck   = s.value("designFilterCheck",false).toBool();
    tileFilterCheck     = s.value("tileFilterCheck",false).toBool();
    tm_showAllFeatures  = s.value("tm_showAllFeatures",false).toBool();
    tm_hideTable        = s.value("tm_hideTable",true).toBool();
    tm_showDebug        = s.value("tm_showDebug",false).toBool();
    tm_autofill         = s.value("tm_autofill",false).toBool();
    tm_showOverlaps     = s.value("tm_showOverlaps",true).toBool();
    lockView            = s.value("lockView",false).toBool();
    splitScreen         = s.value("screenIsSplit",false).toBool();
    compare_transparent = s.value("compare_transparent",false).toBool();
    display_differences = s.value("compare_differences",true).toBool();
    use_workListForCompare  = s.value("use_workListForCompare",false).toBool();
    use_workListForGenerate = s.value("use_workListForGenerate",false).toBool();
    generate_workList   = s.value("generate_workList",false).toBool();
    skipExisting        = s.value("skipExisting",false).toBool();
    showBackgroundImage = s.value("showBackgroundImage",true).toBool();
    highlightUnit       = s.value("highlightUnit",false).toBool();
    insightMode         = s.value("insightMode",false).toBool();
    cs_showBkgds        = s.value("cs_showBkgds",false).toBool();
    cs_showBorders      = s.value("cs_showBorders",false).toBool();
    cs_showFrameSettings = s.value("cs_showFrameSettings",false).toBool();
    defaultImageRoot    = s.value("defaultImageRoot",true).toBool();
    defaultMediaRoot    = s.value("defaultMediaRoot",true).toBool();

    viewerType          = static_cast<eViewType>(s.value("viewerType",VIEW_MOSAIC).toUInt());
    mapEditorMode       = static_cast<eMapEditorMode>(s.value("mapEditorMode",MAP_MODE_FIGURE).toUInt());
    repeatMode          = static_cast<eRepeatType>(s.value("repeat",REPEAT_DEFINED).toUInt());

    showGrid            = s.value("showGrid",false).toBool();
    gridUnits           = static_cast<eGridUnits>(s.value("gridUnits",GRID_UNITS_SCREEN).toUInt());
    gridType            = static_cast<eGridType>(s.value("gridType2",GRID_ORTHOGONAL).toUInt());
    gridModelWidth      = s.value("gridModelWidth",3).toInt();
    gridModelCenter     = s.value("gridModelCenter",false).toBool();
    gridModelSpacing    = s.value("gridModelSpacing",1.0).toDouble();
    gridScreenWidth     = s.value("gridScreenWidth",3).toInt();
    gridScreenSpacing   = s.value("gridScreenSpacing",100).toInt();
    gridScreenCenter    = s.value("gridScreenCenter",false).toBool();
    gridAngle           = s.value("gridAngle",30.0).toDouble();

    // ensures indices are in range
    if (viewerType > VIEW_MAX)          viewerType      = VIEW_MAX;
    if (mapEditorMode > MAP_MODE_MAX)   mapEditorMode   = MAP_MODE_MAX;
    if (repeatMode > REPEAT_MAX)        repeatMode      = REPEAT_MAX;
    if (cycleMode > CYCLE_SAVE_TILING_BMPS) cycleMode   = CYCLE_SAVE_TILING_BMPS;

    // defaults (volatile)
    circleX         = false;
    hideCircles     = false;
    updatePanel     = true;
    showCenterDebug = false;
    showCenterMouse = false;
    enableDetachedPages = true;

    figureViewBkgdColor = QColor(Qt::black);
    kbdMode         = KBD_MODE_UNDEFINED;
    debugReplicate  = false;
    debugMapEnable  = false;

    configurePaths();

    qtAppLog * log = qtAppLog::getInstance();
    if (insightMode)
    {
        log->logToStdErr(logToStderr);
        log->logToDisk(logToDisk);
        log->logToPanel(logToPanel);
        log->logLines(logNumberLines);
        log->logWarningsOnly(logWarningsOnly);
        log->logTimer(logTime);     // last
    }
    else
    {
        log->logToStdErr(true);
        log->logToDisk(true);
        log->logToPanel(false);
        log->logLines(false);
        log->logWarningsOnly(false);
        log->logTimer(LOGT_NONE);   // last
    }
}

void Configuration::save()
{
    QSettings s;
    s.setValue("design2",lastLoadedDesignId);
    s.setValue("cycleMode",cycleMode);
    s.setValue("polySides",polySides);
    s.setValue("cycleInterval",cycleInterval);
    s.setValue("lastLoadedTileName",lastLoadedTileName);
    s.setValue("lastLoadedXML",lastLoadedXML);
    s.setValue("image0",image0);
    s.setValue("image1",image1);
    s.setValue("compareDir0",compareDir0);
    s.setValue("compareDir1",compareDir1);
    s.setValue("viewerType",viewerType);
    s.setValue("mapEditorMode",mapEditorMode);
    s.setValue("repeat",repeatMode);
    s.setValue("panelName", panelName);
    s.setValue("autoLoadStyles",autoLoadStyles);
    s.setValue("verifyMaps",verifyMaps);
    s.setValue("verifyDump",verifyDump);
    s.setValue("verifyVerbose",verifyVerbose);
    s.setValue("logToStderr",logToStderr);
    s.setValue("logToDisk",logToDisk);
    s.setValue("logToPanel",logToPanel);
    s.setValue("logWarningsOnly",logWarningsOnly);
    s.setValue("logNumberLines",logNumberLines);
    s.setValue("logTimer",logTime);
    s.setValue("mapedStatusBox",mapedStatusBox);
    s.setValue("autoLoadTiling",autoLoadTiling);
    s.setValue("loadReplaceTiling",loadTilingMulti);
    s.setValue("autoLoadDesigns",autoLoadDesigns);
    s.setValue("scaleToView",scaleToView);
    s.setValue("stopIfDiff",stopIfDiff);
    s.setValue("designFilterCheck",mosaicFilterCheck);
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
    s.setValue("compare_transparent",compare_transparent);
    s.setValue("compare_differences",display_differences);
    s.setValue("use_workListForCompare",use_workListForCompare);
    s.setValue("use_workListForGenerate",use_workListForGenerate);
    s.setValue("skipExisting",skipExisting);
    s.setValue("generate_workList",generate_workList);
    s.setValue("showBackgroundImage",showBackgroundImage);
    s.setValue("highlightUnit",highlightUnit);
    s.setValue("xmlTool",xmlTool);
    s.setValue("insightMode",insightMode);
    s.setValue("cs_showBkgds",cs_showBkgds);
    s.setValue("cs_showBorders",cs_showBorders);
    s.setValue("cs_showFrameSettings",cs_showFrameSettings);
    s.setValue("defaultImageRoot",defaultImageRoot);
    s.setValue("defaultMediaRoot",defaultMediaRoot);
    s.setValue("rootImageDir",rootImageDir);
    s.setValue("rootMediaDir",rootMediaDir);

    s.setValue("workList",workList);

    s.setValue("showGrid",showGrid);
    s.setValue("gridUnits",gridUnits);
    s.setValue("gridType2",gridType);
    s.setValue("gridModelWidth",gridModelWidth);
    s.setValue("gridModelCenter",gridModelCenter);
    s.setValue("gridModelSpacing",gridModelSpacing);
    s.setValue("gridScreenWidth",gridScreenWidth);
    s.setValue("gridScreenCenter",gridScreenCenter);
    s.setValue("gridScreenSpacing",gridScreenSpacing);
    s.setValue("gridAngle",gridAngle);
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
    qInfo().noquote() << "App path:"  << root;
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
#else
    root.replace("debug","");
    root.replace("release","");
#endif
    root += "../media";

    root = QDir::cleanPath(root);

    QDir adir(root);
    if (adir.exists())
    {
        qInfo().noquote() << "Local Media root:"  << root;
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
    qInfo().noquote() << "App Media root:"  << root;
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
    rootDesignDir       = root + "designs/";
    originalDesignDir   = root + "designs/original/";
    newDesignDir        = root + "designs/new_designs/";
    templateDir         = root + "designs/templates/";
    examplesDir         = root + "history/examples/";

    if (defaultImageRoot)
        rootImageDir  = getImageRoot();
    // else fetched
    rootImageDir = rootImageDir.trimmed();
    if (!rootImageDir.endsWith("/"))
    {
        rootImageDir += "/";
    }
}
