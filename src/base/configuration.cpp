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
#include "base/qtapplog.h"
#include "designs/design.h"
#include <cstddef>

const QString Configuration::sCanvasMode[]  = {
    E2STR(KBD_MODE_XFORM_VIEW),
    E2STR(KBD_MODE_XFORM_BKGD),
    E2STR(KBD_MODE_XFORM_MODEL),
    E2STR(KBD_MODE_XFORM_OBJECT),
    E2STR(KBD_MODE_LAYER),
    E2STR(KBD_MODE_ZLEVEL),
    E2STR(KBD_MODE_STEP),
    E2STR(KBD_MODE_SEPARATION),
    E2STR(KBD_MODE_ORIGIN),
    E2STR(KBD_MODE_OFFSET),
    E2STR(KBD_MODE_CENTER)
};

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
    gridWidth           = s.value("gridWidth",3).toInt();

    lastLoadedTileName  = s.value("lastLoadedTileName","").toString();
    lastLoadedXML       = s.value("lastLoadedXML","").toString();
    rootMediaDir        = s.value("rootMediaDir","").toString();
    rootTileDir         = s.value("rootTileDir3","").toString();
    rootImageDir        = s.value("rootImageDir","").toString();
    examplesDir         = s.value("examplesDir","").toString();
    newTileDir          = s.value("newTileDir3","").toString();
    rootDesignDir       = s.value("rootDesignDir3","").toString();
    newDesignDir        = s.value("newDesignDir3","").toString();
    templateDir         = s.value("templateDir","").toString();
    image0              = s.value("image0","").toString();
    image1              = s.value("image1","").toString();
    compareDir0         = s.value("compareDir0","").toString();
    compareDir1         = s.value("compareDir1","").toString();
    panelName           = s.value("panelName","Control").toString();

    designFilter        = s.value("designFilter","").toString();
    tileFilter          = s.value("tileFilter","").toString();
    xmlTool             = s.value("xmlTool","").toString();

    firstBirthday       = s.value("firstBirthday7",false).toBool();
    autoLoadStyles      = s.value("autoLoadStyles",false).toBool();
    autoLoadTiling      = s.value("autoLoadTiling",false).toBool();
    autoLoadDesigns     = s.value("autoLoadDesigns",false).toBool();
    scaleSceneToView    = s.value("scaleToView",true).toBool();
    autoCycle           = s.value("autoCycle",false).toBool();
    stopIfDiff          = s.value("stopIfDiff",true).toBool();
    verifyMaps          = s.value("verifyMaps",false).toBool();
    verifyDump          = s.value("verifyDump",false).toBool();
    verifyVerbose       = s.value("verifyVerbose",false).toBool();
    logToStderr         = s.value("logToStderr",true).toBool();
    logToDisk           = s.value("logToDisk",true).toBool();
    logToPanel          = s.value("logToPanel",true).toBool();
    logNumberLines      = s.value("logNumberLines",true).toBool();
    logWarningsOnly     = s.value("logWarningsOnly",false).toBool();
    logElapsedTime      = s.value("logElapsedTime",false).toBool();
    wsStatusBox         = s.value("wsStatusBox",false).toBool();
    mapedStatusBox      = s.value("mapedStatusBox",false).toBool();
    designFilterCheck   = s.value("designFilterCheck",false).toBool();
    tileFilterCheck     = s.value("tileFilterCheck",false).toBool();
    tm_showAllFeatures  = s.value("tm_showAllFeatures",false).toBool();
    tm_hideTable        = s.value("tm_hideTable",true).toBool();
    tm_showDebug        = s.value("tm_showDebug",false).toBool();
    tm_autofill         = s.value("tm_autofill",false).toBool();
    tm_showOverlaps     = s.value("tm_showOverlaps",true).toBool();
    lockView            = s.value("lockView",false).toBool();
    screenIsSplit       = s.value("screenIsSplit",false).toBool();
    compare_transparent = s.value("compare_transparent",false).toBool();
    display_differences = s.value("compare_differences",true).toBool();
    compare_ping_pong   = s.value("compare_ping_pong",false).toBool();
    compare_side_by_side= s.value("compare_side_by_side",false).toBool();
    showCenter          = s.value("showCenter",false).toBool();
    gridCenter          = s.value("gridCenter",false).toBool();
    hideBackgroundImage = s.value("hideBackgroundImage",false).toBool();
    highlightUnit       = s.value("highlightUnit",false).toBool();

    viewerType          = static_cast<eViewType>(s.value("viewerType",VIEW_MOSAIC).toUInt());
    mapEditorMode       = static_cast<eMapEditorMode>(s.value("mapEditorMode",MAP_MODE_FIGURE).toUInt());
    repeatMode          = static_cast<eRepeatType>(s.value("repeat",REPEAT_DEFINED).toUInt());
    gridModel           = static_cast<eGridModel>(s.value("gridModel2",GRID_SCREEN).toUInt());

    gridStepScreen   = s.value("fgdGridStep",100).toInt();
    gridStepModel    = s.value("fgdGridStepModel2",1.0).toDouble();

    // ensures indices are in range
    if (viewerType > VIEW_MAX)          viewerType      = VIEW_MAX;
    if (mapEditorMode > MAP_MODE_MAX)   mapEditorMode   = MAP_MODE_MAX;
    if (repeatMode > REPEAT_MAX)        repeatMode      = REPEAT_MAX;
    if (cycleMode > CYCLE_SAVE_TILING_BMPS) cycleMode   = CYCLE_SAVE_TILING_BMPS;

    // defaults (volatile)
    circleX         = false;
    sceneGrid       = false;
    hideCircles     = false;
    updatePanel     = true;
    enableDetachedPages = true;

    figureViewBkgdColor = QColor(Qt::black);
    kbdMode         = KBD_MODE_XFORM_VIEW;
    debugReplicate  = false;
    debugMapEnable  = false;

    selectedDesignElementFeature = nullptr;
    faceSet = nullptr;

    qtAppLog * log = qtAppLog::getInstance();
    log->logToStdErr(logToStderr);
    log->logToDisk(logToDisk);
    log->logToPanel(logToPanel);
    log->logLines(logNumberLines);
    log->logElapsed(logElapsedTime);
    log->logWarningsOnly(logWarningsOnly);

    if (!firstBirthday)
    {
        reconfigurePaths();
    }
}

void Configuration::save()
{
    QSettings s;
    s.setValue("design2",lastLoadedDesignId);
    s.setValue("cycleMode",cycleMode);
    s.setValue("gridWidth",gridWidth);
    s.setValue("polySides",polySides);
    s.setValue("cycleInterval",cycleInterval);
    s.setValue("lastLoadedTileName",lastLoadedTileName);
    s.setValue("lastLoadedXML",lastLoadedXML);
    s.setValue("rootMediaDir",rootMediaDir);
    s.setValue("rootTileDir3",rootTileDir);
    s.setValue("rootImageDir",rootImageDir);
    s.setValue("examplesDir",examplesDir);
    s.setValue("newTileDir3",newTileDir);
    s.setValue("rootDesignDir3",rootDesignDir);
    s.setValue("newDesignDir3",newDesignDir);
    s.setValue("templateDir",templateDir);
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
    s.setValue("logElapsedTime",logElapsedTime);
    s.setValue("wsStatusBox",wsStatusBox);
    s.setValue("mapedStatusBox",mapedStatusBox);
    s.setValue("autoLoadTiling",autoLoadTiling);
    s.setValue("autoLoadDesigns",autoLoadDesigns);
    s.setValue("scaleToView",scaleSceneToView);
    s.setValue("autoCycle",autoCycle);
    s.setValue("stopIfDiff",stopIfDiff);
    s.setValue("gridModel2",gridModel);
    s.setValue("fgdGridStep",gridStepScreen);
    s.setValue("fgdGridStepModel2",gridStepModel);
    s.setValue("designFilterCheck",designFilterCheck);
    s.setValue("tileFilterCheck",tileFilterCheck);
    s.setValue("designFilter",designFilter);
    s.setValue("tileFilter",tileFilter);
    s.setValue("tm_showAllFeatures",tm_showAllFeatures);
    s.setValue("tm_hideTable",tm_hideTable);
    s.setValue("tm_showDebug",tm_showDebug);
    s.setValue("tm_autofill",tm_autofill);
    s.setValue("tm_showOverlaps",tm_showOverlaps);
    s.setValue("lockView",lockView);
    s.setValue("screenIsSplit",screenIsSplit);
    s.setValue("compare_transparent",compare_transparent);
    s.setValue("compare_differences",display_differences);
    s.setValue("compare_ping_pong",compare_ping_pong);
    s.setValue("compare_side_by_side",compare_side_by_side);
    s.setValue("showCenter",showCenter);
    s.setValue("gridCenter",gridCenter);
    s.setValue("hideBackgroundImage",hideBackgroundImage);
    s.setValue("highlightUnit",highlightUnit);
    s.setValue("xmlTool",xmlTool);
    s.setValue("firstBirthday7",firstBirthday);
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
    qDebug() << "root:"  << root;
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
        qDebug() << "Local Media root:"  << root;
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
    qDebug() << "App Media root:"  << root;
    return root;
}

QString Configuration::getImageRoot()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/TiledPatternMakerImages/";
    return path;
}

void Configuration::reconfigurePaths()
{
    qDebug() << "reconfigurePaths()";

    QString root = getMediaRoot();

    rootMediaDir  = root + "/";
    rootTileDir   = root + "/tilings/";
    newTileDir    = root + "/tilings/new_tilings/";
    rootDesignDir = root + "/designs/";
    newDesignDir  = root + "/designs/new_designs/";
    templateDir   = root + "/designs/templates/";
    examplesDir   = root + "/history/examples/";
    rootImageDir  = getImageRoot();

    firstBirthday = true;

    save();     //save everyting;

    // restart the application
    qApp->quit();
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
}
