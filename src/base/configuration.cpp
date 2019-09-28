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
#include "designs/design.h"
#include <cstddef>

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
    cycleMode           = static_cast<eCycleMode>(s.value("cycleMode",0).toInt());
    cycleInterval       = s.value("cycleInterval",4).toInt();

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
    image0              = s.value("imageDir0","").toString();
    image1              = s.value("imageDir1","").toString();
    compareDir0         = s.value("compareDir0","").toString();
    compareDir1         = s.value("compareDir1","").toString();
    panelName           = s.value("panelName","Control").toString();
    designFilter        = s.value("designFilter","").toString();
    tileFilter          = s.value("tileFilter","").toString();

    firstBirthday       = s.value("firstBirthday5",false).toBool();
    autoLoadStyles      = s.value("autoLoadStyles",false).toBool();
    autoLoadTiling      = s.value("autoLoadTiling",false).toBool();
    autoLoadDesigns     = s.value("autoLoadDesigns",false).toBool();
    autoCycle           = s.value("autoCycle",false).toBool();
    stopIfDiff          = s.value("stopIfDiff",true).toBool();
    autoClear           = s.value("autoClear",true).toBool();
    verifyMaps          = s.value("verifyMaps",false).toBool();
    wsStatusBox         = s.value("wsStatusBox",true).toBool();
    designFilterCheck   = s.value("designFilterCheck",false).toBool();
    tileFilterCheck     = s.value("tileFilterCheck",false).toBool();
    showAllFeatures     = s.value("tiling_feature_chk",false).toBool();
    lockView            = s.value("lockView",false).toBool();
    screenIsSplit       = s.value("screenIsSplit",false).toBool();
    transparentCompare  = s.value("transparentCompare",false).toBool();

    viewerType          = static_cast<eViewType>(s.value("viewerType",VIEW_DESIGN).toUInt());
    designViewer        = static_cast<eDesignViewer>(s.value("designViewer",DV_LOADED_STYLE).toUInt());
    protoViewer         = static_cast<eProtoViewer>(s.value("protoViewer2",PV_STYLE).toUInt());
    protoFeatureViewer  = static_cast<eProtoFeatureViewer>(s.value("protoFeatureViewer",PVF_STYLE).toUInt());
    tilingViewer        = static_cast<eTilingViewer>(s.value("tilingViewer2",TV_STYLE).toUInt());
    tilingMakerViewer   = static_cast<eTilingMakerView>(s.value("makerSource4",TD_STYLE).toUInt());
    figureViewer        = static_cast<eFigureViewer>(s.value("figureViewer3",FV_STYLE).toUInt());
    delViewer           = static_cast<eDELViewer>(s.value("delViewer",DEL_STYLES).toUInt());
    mapEditorView       = static_cast<eMapEditorView>(s.value("mapEditorView",ME_FIGURE_MAP).toUInt());
    canvasSettings      = static_cast<eCSSelect>(s.value("canvasSettings",CS_STYLE).toUInt());

    pushTarget          = static_cast<ePushTarget>(s.value("pushTarget",TARGET_LOADED_STYLES).toUInt());
    repeatMode          = static_cast<eRepeatType>(s.value("repeat",REPEAT_DEFINED).toUInt());

    fgdGridModel        = s.value("fgdGridModel",false).toBool();
    fgdGridStepScreen         = s.value("fgdGridStep",100).toInt();
    fgdGridStepModel    = s.value("fgdGridStepModel2",1.0).toDouble();

    // ensures indices are in range
    if (viewerType > VIEW_MAX)        viewerType      = VIEW_MAX;
    if (designViewer > DV_MAX)          designViewer    = DV_MAX;
    if (protoViewer > PV_MAX)           protoViewer     = PV_MAX;
    if (protoFeatureViewer > PVF_MAX)   protoFeatureViewer = PVF_MAX;
    if (tilingViewer > TV_MAX)          tilingViewer    = TV_MAX;
    if (tilingMakerViewer > TD_MAX)     tilingMakerViewer = TD_MAX;
    if (figureViewer > FV_MAX)          figureViewer    = FV_MAX;
    if (delViewer > DEL_MAX)            delViewer       = DEL_MAX;
    if (mapEditorView > ME_MAX)         mapEditorView   = ME_MAX;
    if (repeatMode > REPEAT_MAX)        repeatMode      = REPEAT_MAX;
    if (pushTarget > TARGET_MAX)        pushTarget      = TARGET_MAX;

    // defaults (volatile)
    circleX         = false;
    sceneGrid       = false;
    boundingRects   = false;
    hideCircles     = false;

    updatePanel     = true;

    figureViewBkgdColor = QColor(Qt::black);

    debugReplicate  = false;
    debugMapEnable  = false;

    selectedDesignElementFeature = nullptr;
    faceSet = nullptr;

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
    s.setValue("imageDir0",image0);
    s.setValue("imageDir1",image1);
    s.setValue("compareDir0",compareDir0);
    s.setValue("compareDir1",compareDir1);
    s.setValue("viewerType",viewerType);
    s.setValue("designViewer",designViewer);
    s.setValue("protoViewer2",protoViewer);
    s.setValue("protoFeatureViewer",protoFeatureViewer);
    s.setValue("tilingViewer2",tilingViewer);
    s.setValue("tilingViewer2",tilingViewer);
    s.setValue("figureViewer3",figureViewer);
    s.setValue("delViewer",delViewer);
    s.setValue("mapEditorView",mapEditorView);
    s.setValue("repeat",repeatMode);
    s.setValue("makerSource4",tilingMakerViewer);
    s.setValue("panelName", panelName);
    s.setValue("autoLoadStyles",autoLoadStyles);
    s.setValue("verifyMaps",verifyMaps);
    s.setValue("wsStatusBox",wsStatusBox);
    s.setValue("autoLoadTiling",autoLoadTiling);
    s.setValue("autoLoadDesigns",autoLoadDesigns);
    s.setValue("autoCycle",autoCycle);
    s.setValue("stopIfDiff",stopIfDiff);
    s.setValue("autoClear",autoClear);
    s.setValue("fgdGridModel",fgdGridModel);
    s.setValue("fgdGridStep",fgdGridStepScreen);
    s.setValue("fgdGridStepModel2",fgdGridStepModel);
    s.setValue("designFilterCheck",designFilterCheck);
    s.setValue("tileFilterCheck",tileFilterCheck);
    s.setValue("designFilter",designFilter);
    s.setValue("tileFilter",tileFilter);
    s.setValue("tiling_feature_chk",showAllFeatures);
    s.setValue("lockView",lockView);
    s.setValue("screenIsSplit",screenIsSplit);
    s.setValue("transparentCompare",transparentCompare);
    s.setValue("pushTarget",pushTarget);
    s.setValue("canvasSettings",canvasSettings);
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
    root += "/media";

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
    QSettings s;
    s.setValue("firstBirthday5",firstBirthday);
}
