#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QApplication>

#include "model/settings/configuration.h"
#include "sys/sys.h"
#include "sys/tiledpatternmaker.h"
#include "sys/sys/pugixml.hpp"

extern TiledPatternMaker * theApp;

#ifdef Q_OS_LINUX
#include "sys/version.h"

static int filesWritten = 0;

// Copies any file from the system-installed seed media tree into the user's Documents media tree.

static void writeMediaFile(const QFileInfo &srcFile, const QString &dstDir)
{
    const QString dstPath = dstDir + "/" + srcFile.fileName();

    QDir().mkpath(dstDir);

    if (QFile::exists(dstPath))
        QFile::remove(dstPath);

    if (!QFile::copy(srcFile.filePath(), dstPath))
    {
        qWarning() << "Failed to write media file:" << srcFile.filePath() << "->" << dstPath;
    }
    else
    {
        qInfo() << "Wrote media file:" << srcFile.fileName();
        filesWritten++;
    }
}

static void writeMediaTree(const QString &srcDir, const QString &dstDir)
{
    QDir dir(srcDir);
    if (!dir.exists())
        return;

    // Files at this level
    for (const QFileInfo &entry : dir.entryInfoList(QDir::Files))
    {
        writeMediaFile(entry, dstDir);
    }

    // Recurse into subdirectories (e.g. designs/, backgrounds/)
    for (const QFileInfo &entry : dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        writeMediaTree(entry.filePath(), dstDir + "/" + entry.fileName());
    }
}

static void installMediaTree(const QString &srcDir, const QString &dstDir)
{
    qInfo() << "installing from" << srcDir << "to"  << dstDir;
    writeMediaTree(srcDir,dstDir);
    qInfo() << "files installed =" << filesWritten;

    QString manifest = srcDir + "/manifest.xml";

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(manifest.toStdString().c_str());
    if (result == false)
    {
        qWarning() << "Error: could not load" << manifest;
        return;
    }

    QList<QString> deletedFiles;
    for (auto file : doc.child("manifest").child("deleted").children("file"))
        deletedFiles << file.attribute("path").value();

    for (auto & file : deletedFiles)
    {
        QString filename = dstDir + "/" + file;
        bool rv =  QFile::remove(filename);
        if (rv)
            qDebug() << "removed" << filename;
        else
            qWarning() << "did not remove" << filename;
    }
}
#endif

// stored as translateX, translateY, rotation (radians), scale
static Xform readXform(QSettings & s, const QString & key)
{
    Xform xf;
    QVariantList vl = s.value(key).toList();
    if (vl.size() == 4)
    {
        xf.setTranslateX(vl[0].toDouble());
        xf.setTranslateY(vl[1].toDouble());
        xf.setRotateRadians(vl[2].toDouble());
        xf.setScale(vl[3].toDouble());
    }
    return xf;
}

static void writeXform(QSettings & s, const QString & key, const Xform & xf)
{
    s.setValue(key,QVariantList{xf.getTranslateX(),xf.getTranslateY(),xf.getRotateRadians(),xf.getScale()});
}

Configuration::Configuration()
{
    qRegisterMetaType<QList<int>>();

    firstBirthday    = false;
    developmentBuild = false;

    // persist
    QSettings s = Sys::settings();

    lastLoadedMosaic    = s.value("lastLoadedMosaic","").toString();
    lastLoadedTiling    = s.value("lastLoadedTiling","").toString();
    lastLoadedLegacy    = s.value("lastLoadedLegacy","").toString();
    lastLoadedType      = (eLoadUnitType)s.value("lastLoadedType2",LT_UNDEFINED).toUInt();

    cycleInterval       = s.value("cycleInterval",4).toInt();
    protoViewMode       = s.value("protoViewMode2",SHOW_TILES | SHOW_MOTIFS).toInt();

    _alternateRootMediaDir  = s.value("alt_rootMediaDir","").toString();
    _alternateRootImageDir  = s.value("alt_rootImageDir","").toString();
    _alternateRootLogDir    = s.value("alt_rootLogDir","").toString();

    BMPCompare0         = s.value("image0","").toString();
    BMPCompare1         = s.value("image1","").toString();
    pageName            = s.value("panelName","Load").toString();
    mosaicFilter        = s.value("designFilter","").toString();
    tileFilter          = s.value("tileFilter","").toString();
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
    logToStderr         = s.value("logToStderr",true).toBool();
    logToDisk           = s.value("logToDisk",true).toBool();
    logToPanel          = s.value("logToPanel",true).toBool();
    logNumberLines      = s.value("logNumberLines",true).toBool();
    logDebug            = s.value("logDebug",false).toBool();
    enableLog2          = s.value("enableLog2",false).toBool();
    logTime             = static_cast<eLogTimer>(s.value("logTimer",LOGT_NONE).toUInt());
    mapedStatusBox      = s.value("mapedStatusBox",false).toBool();
    mapedLoadCopies     = s.value("mapedLoadCopies",false).toBool();
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
    tm_excludesInTable  = s.value("tm_excludesInTable",false).toBool();
    tm_fillOnLoad       = s.value("tm_fillOnLoad",false).toBool();
    tm_poly_side_count  = s.value("polySides",8).toInt();
    tm_poly_rotation    = s.value("polyRotation",0.0).toDouble();

    lockView            = s.value("lockView",false).toBool();
    multiView           = s.value("multiView",false).toBool();
    integratedScreen    = s.value("integratedScreen",false).toBool();
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
    highContrastDiffs   = s.value("highContrastDiffs",false).toBool();

    use_workListForCompare= s.value("use_workListForCompare",false).toBool();
    skipExisting        = s.value("skipExisting",false).toBool();
    limitViewSize       = s.value("limitViewSize",true).toBool();
    disableSplash       = s.value("disableSplash",false).toBool();
    disableResizeNotify = s.value("disableResizeNotify",false).toBool();
    insightMode         = s.value("insightMode",false).toBool();
    pendingInsightMode  = insightMode;
    cs_showBkgds        = s.value("cs_showBkgds",false).toBool();
    defaultImageRoot    = s.value("defaultImageRoot",true).toBool();
    defaultMediaRoot    = s.value("defaultMediaRoot",true).toBool();
    defaultLogRoot      = s.value("defaultLogRoot",true).toBool();
    saveMosaicTest      = s.value("saveMosaicTest",false).toBool();
    saveTilingTest      = s.value("saveTilingTest",false).toBool();
    vCompLock           = s.value("vCompLock",true).toBool();
    vCompXML            = s.value("vCompXML",true).toBool();
    motifMakerViewType  = static_cast<MotifMakerView::ViewType>(s.value("motifMakerView",(int)MotifMakerView::ViewType::Single).toInt());

    mapEditorMode       = static_cast<MapEditor::EditorMode>(s.value("mapEditorMode2",(int)MapEditor::EditorMode::Map).toInt());
    repeatMode          = static_cast<eRepeatType>(s.value("repeat",REPEAT_DEFINED).toUInt());

    debugTabIndex       = s.value("debugTabIndex",0).toUInt();
    showGridModelCenter = s.value("showGridModelCenter",false).toBool();
    showGridViewCenter  = s.value("showGridScreenCenter",false).toBool();
    gridUnits           = static_cast<eGridUnits>(s.value("gridUnits",GRID_UNITS_SCREEN).toUInt());
    if (gridUnits > GRID_UNITS_MAX) gridUnits = GRID_UNITS_MAX;
    gridType            = static_cast<eGridType>(s.value("gridType2",GRID_ORTHOGONAL).toUInt());
    if (gridType > GRID_TYPE_MAX) gridType = GRID_TYPE_MAX;
    gridTilingAlgo      = static_cast<eGridTilingAlgo>(s.value("gridTilingAlgo",REGION).toUInt());
    gridModelWidth      = s.value("gridModelWidth",3).toInt();
    gridModelPos        = static_cast<eGridPosition>(s.value("gridModelPos",GRID_POS_CENTERED).toUInt());
    if (gridModelPos > GRID_POS_MAX) gridModelPos = GRID_POS_MAX;
    gridModelXform      = readXform(s,"gridModelXform");
    gridModelSpacing    = s.value("gridModelSpacing",1.0).toDouble();
    gridScreenWidth     = s.value("gridScreenWidth",3).toInt();
    gridScreenSpacing   = s.value("gridScreenSpacing",100).toInt();
    gridScreenPos       = static_cast<eGridPosition>(s.value("gridScreenPos",GRID_POS_CENTERED).toUInt());
    if (gridScreenPos > GRID_POS_MAX) gridScreenPos = GRID_POS_MAX;
    gridScreenXform     = readXform(s,"gridScreenXform");
    gridTilingWidth     = s.value("gridTilingWidth",3).toInt();
    gridZLevel          = static_cast<Layer::ZLevel>(s.value("gridZLevel",(int)Layer::ZLevel::Grid_Z5).toInt());
    debugViewConfig     = s.value("debugViewConfig2",0xffff).toUInt();
    mapedCleanseLevel   = s.value("mapedCleanseLevel",44).toUInt();
    tm_snapToGrid       = s.value("snapToGrid",true).toBool();
    tm_snapOnly         = s.value("tm_snapOnly",false).toBool();
    tm_tileAnchorMode   = s.value("tm_tileAnchorMode",false).toBool();
    tm_vectorAnchorMode = s.value("tm_vectorAnchorMode",false).toBool();
    tm_tileOpacity      = s.value("tm_tileOpacity",100).toInt();
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

    protoViewColors     = s.value("protoViewColors",defaultProtoViewColors()).toStringList();

    // ensures indices are in range
    if (repeatMode > REPEAT_MAX)        repeatMode      = REPEAT_MAX;

    if (!insightMode)
    {
        // designer mode cannot change these
        Sys::highlightUnit      = false;
        Sys::dontReplicate      = false;
        showTileBoundary        = true;
        showMotifBoundary       = true;
        showMotif               = true;
        showExtendedBoundary    = true;
        showTileCenter          = false;
        showMotifCenter         = false;

        mosaicWorklistCheck     = false;
        mosaicWorklistXCheck    = false;
        tilingWorklistCheck     = false;
        tilingWorklistXCheck    = false;

        verifyMaps              = false;
        forceVerifyProtos       = false;
        verifyPopup             = false;
        verifyDump              = false;
        verifyVerbose           = false;
        buildEmptyNmaps         = false;
        slowCleanseMapMerges    = false;
        enableLog2              = false;

        mapedMergeSensitivity   = 1e-2;

        protoViewMode           = SHOW_TILES | SHOW_MOTIFS;
        protoViewColors         = defaultProtoViewColors();
        protoviewWidth          = 3.0;
    }

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
    QSettings s = Sys::settings();
    s.setValue("lastLoadedTiling",lastLoadedTiling.get());
    s.setValue("lastLoadedMosaic",lastLoadedMosaic.get());
    s.setValue("lastLoadedLegacy",lastLoadedLegacy.get());
    s.setValue("lastLoadedType2",lastLoadedType);

    s.setValue("protoViewMode2",protoViewMode);
    s.setValue("cycleInterval",cycleInterval);
    s.setValue("image0",BMPCompare0);
    s.setValue("image1",BMPCompare1);
    s.setValue("mapEditorMode2",(int)mapEditorMode);
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
    s.setValue("logToStderr",logToStderr);
    s.setValue("logToDisk",logToDisk);
    s.setValue("logToPanel",logToPanel);
    s.setValue("logDebug",logDebug);
    s.setValue("enableLog2",enableLog2);
    s.setValue("logNumberLines",logNumberLines);
    s.setValue("logTimer",logTime);
    s.setValue("mapedStatusBox",mapedStatusBox);
    s.setValue("mapedLoadCopies",mapedLoadCopies);
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
    s.setValue("tm_excludesInTable",tm_excludesInTable);
    s.setValue("tm_fillOnLoad",tm_fillOnLoad);
    s.setValue("polySides",tm_poly_side_count);
    s.setValue("polyRotation",tm_poly_rotation);

    s.setValue("lockView",lockView);
    s.setValue("multiView",multiView);
    s.setValue("integratedScreen",integratedScreen);
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
    s.setValue("highContrastDiffs",highContrastDiffs);

    s.setValue("use_workListForCompare",use_workListForCompare);
    s.setValue("motifMakerView",(int)motifMakerViewType);
    s.setValue("limitViewSize",limitViewSize);
    s.setValue("disableSplash",disableSplash);
    s.setValue("disableResizeNotify",disableResizeNotify);
    s.setValue("insightMode",pendingInsightMode);
    s.setValue("cs_showBkgds",cs_showBkgds);

    s.setValue("saveMosaicTest",saveMosaicTest);
    s.setValue("saveTilingTest",saveTilingTest);
    s.setValue("vCompLock",vCompLock);
    s.setValue("vCompXML",vCompXML);

    s.setValue("protoViewColors",protoViewColors);

    s.setValue("showGridModelCenter",showGridModelCenter);
    s.setValue("showGridScreenCenter",showGridViewCenter);
    s.setValue("gridUnits",gridUnits);
    s.setValue("gridTilingAlgo",gridTilingAlgo);
    s.setValue("gridType2",gridType);
    s.setValue("gridModelWidth",gridModelWidth);
    s.setValue("gridModelPos",gridModelPos);
    writeXform(s,"gridModelXform",gridModelXform);
    s.setValue("gridModelSpacing",gridModelSpacing);
    s.setValue("gridScreenWidth",gridScreenWidth);
    s.setValue("gridScreenPos",gridScreenPos);
    writeXform(s,"gridScreenXform",gridScreenXform);
    s.setValue("gridTilingWidth",gridTilingWidth);
    s.setValue("snapToGrid",tm_snapToGrid);
    s.setValue("snapOnly",tm_snapOnly);
    s.setValue("tm_tileAnchorMode",tm_tileAnchorMode);
    s.setValue("tm_vectorAnchorMode",tm_vectorAnchorMode);
    s.setValue("tm_tileOpacity",tm_tileOpacity);
    s.setValue("gridScreenSpacing",gridScreenSpacing);
    s.setValue("gridZLevel",(int)gridZLevel);
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

    s.setValue("defaultImageRoot",defaultImageRoot);
    s.setValue("defaultMediaRoot",defaultMediaRoot);
    s.setValue("defaultLogRoot",defaultLogRoot);

    s.setValue("alt_rootImageDir",_alternateRootImageDir);
    s.setValue("alt_rootMediaDir",_alternateRootMediaDir);
    s.setValue("alt_rootLogDir",_alternateRootLogDir);

    worklist.save(s);
    saveBackgroundColors();
}

void Configuration::loadBackgroundColors()
{
    QSettings s = Sys::settings();

    QStringList viewColors;

    if (Sys::isDarkTheme)
    {
        std::fill_n(bkgdColors, NUM_VIEW_TYPES, Qt::black);
        if (s.contains("darkLayerColors"))
            viewColors = s.value("darkLayerColors").toStringList();
        else
            viewColors = s.value("viewColors3").toStringList();
    }
    else
    {
        std::fill_n(bkgdColors, NUM_VIEW_TYPES, Qt::white);
        if (s.contains("lightLayerColors"))
            viewColors = s.value("lightLayerColors").toStringList();
        else
            viewColors = s.value("viewColors3").toStringList();
    }

    int i = 0;
    for (auto & str : viewColors)
    {
        bkgdColors[i++] = str;
    }

}
// order matches ProtoViewColors::getColors()
QStringList Configuration::defaultProtoViewColors()
{
    QStringList qsl;
    qsl << "#ff1496d2"   // map
        << "#ffd60000"   // tile
        << "#ff006c00"   // motif
        << "#ff00ff00"   // TU tile
        << "#ff0000ff"   // TU motif
        << "#80ffd9d9"   // TU tile brush
        << "#ffffff00";  // extended boundary
    return qsl;
}

void Configuration::saveBackgroundColors()
{
    QSettings s = Sys::settings();

    QStringList colors;
    for (auto & color : bkgdColors)
    {
        colors << color.name();
    }
    s.setValue("viewColors3",colors);   // for backwards compatability

    if (Sys::isDarkTheme)
        s.setValue("darkLayerColors",colors);
    else
        s.setValue("lightLayerColors",colors);
}

// return a path if a development build
QString Configuration::getMediaDirDevelopment()
{
    bool isQmake = false;
    QString root = qApp->applicationDirPath();
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
        QString c2root = climbAndFindMediaRoot(root, 4);
        if (!c2root.isEmpty())
        {
            return c2root;
        }
    }
    // could be old cmake
    QString c1root = climbAndFindMediaRoot(root, 1);
    if (!c1root.isEmpty())
    {
        return c1root;
    }
    // could be new cmake
    QString c2root = climbAndFindMediaRoot(root, 3);
    if (!c2root.isEmpty())
    {
        return c2root;
    }
    // not found
    return QString();
}

// Climbs 'levels' path components up from 'root', then checks for a
// sibling 'media' directory that also has a sibling 'src' directory
// (our repo layout guarantee). Returns empty if the climb runs past
// the filesystem root, or if the candidate doesn't look like a real
// repo checkout - this avoids false positives like an installed
// binary at /usr/bin coincidentally climbing to the unrelated
// system directory /media, which exists on every Linux install
// regardless of whether this is a dev build.

QString Configuration::climbAndFindMediaRoot(const QString & appDirPath, int levels)
{
    bool isAbsoluteUnix = appDirPath.startsWith("/");
    QStringList parts = appDirPath.split("/", Qt::SkipEmptyParts);

    if (parts.size() <= levels)
    {
        return QString();
    }

    for (int i = 0; i < levels; ++i)
    {
        parts.removeLast();
    }

    QString base = isAbsoluteUnix ? ("/" + parts.join("/")) : parts.join("/");
    QString mediaCandidate = QDir::cleanPath(base + "/media");
    QString srcCandidate   = QDir::cleanPath(base + "/src");

    QDir mediaDir(mediaCandidate);
    QDir srcDir(srcCandidate);

    if (mediaDir.exists() && srcDir.exists())
    {
        return mediaDir.canonicalPath();
    }

    return QString();
}

QString Configuration::getDefaultMediaDir()
{
    QString root = getMediaDirDevelopment();

    if (root.isEmpty())
    {
        root = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/TiledPatternMaker/media/";
    }
    else
    {
        developmentBuild = true;
    }
    return root;
}

void Configuration::configurePaths()
{
    configureMediaPaths(false);
    configureImagePaths(false);
    configureLogPaths(false);
}

// Media
void Configuration::configureMediaPaths(bool doEmit)
{
    QString root;
    if (defaultMediaRoot)
    {
        root = getDefaultMediaDir();
    }
    else
    {
        if (_alternateRootMediaDir.isEmpty())
            _alternateRootMediaDir = getDefaultMediaDir();

        root = _alternateRootMediaDir;    // fetched
    }

    root = root.trimmed();
    if (!root.endsWith("/"))
        root += "/";

    Sys::setRootMediaDir(root);

#if defined(Q_OS_LINUX)
    // Windows/NSIS already writes new/changed files directly into
    // Documents at install time. On Linux, the .deb instead seeds
    // media into a fixed system location, so top it up here on
    // every launch - cheap once everything's already present, since
    // each per-file check is just an existence test.

    QSettings s = Sys::settings();
    int mediaFilesVersion   = s.value("mediaFilesVersion",0).toInt();

    if (mediaFilesVersion != LINUX_MEDIA_FILES_VERSION && !developmentBuild && defaultMediaRoot)
    {
        qInfo() << "TiledPatterMaker: installing media.......";

        const QString seedRoot = "/usr/share/tiledpatternmaker/media/";
        installMediaTree(seedRoot, root);

        s.setValue("mediaFilesVersion",LINUX_MEDIA_FILES_VERSION);
        s.sync();
    }
#if 0
    else
    {
        qInfo() << "Media: current =" << CURRENT_MEDIA_FILES_VERSION << "installed =" << mediaFilesVersion
                << "dev-build =" << developmentBuild << "default-medit-root =" << defaultMediaRoot;
    }
#endif
#endif

    Sys::rootTileDir         = root + "tilings/";
    Sys::originalTileDir     = root + "tilings/original/";
    Sys::newTileDir          = root + "tilings/new_tilings/";
    Sys::testTileDir         = root + "tilings/tests/";

    Sys::rootMosaicDir       = root + "designs/";
    Sys::originalMosaicDir   = root + "designs/original/";
    Sys::newMosaicDir        = root + "designs/new_designs/";
    Sys::testMosiacDir       = root + "designs/tests/";

    Sys::bkgdPhotosDir       = root + "bkgd_photos/";
    Sys::examplesDir         = root + "history/examples/";
    Sys::girihDir            = root + "girih_shapes/";
    Sys::mapsDir             = root + "maps/";
    Sys::templateDir         = root + "templates/";
    Sys::worklistsDir        = root + "worklists/";

    if (doEmit)
        sig_mediaPathsChanged();
}

// Images
void Configuration::configureImagePaths(bool doEmit)
{
    QString root = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/TiledPatternMaker/";
    if (!defaultImageRoot)
    {
        if (_alternateRootImageDir.isEmpty())
        {
            _alternateRootImageDir = root;
        }
        else
        {
            root = _alternateRootImageDir;
        }
    }

    root = root.trimmed();
    if (!root.endsWith("/"))
        root += "/";

    Sys::setRootImageDir(root);
    Sys::snapsDir       = root + "snaps/";
    Sys::renderedDir    = root + "rendered/";  // svg

    if (doEmit)
        sig_imagesPathChanged();
}

// Logs
void Configuration::configureLogPaths(bool doEmit)
{
    QString root = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/logs/";
    if (!defaultLogRoot)
    {
        if (_alternateRootLogDir.isEmpty())
        {
            _alternateRootLogDir = root;
        }
        else
        {
            root = _alternateRootLogDir;
        }
    }

    Sys::setLogDir(root);

    if (doEmit)
        sig_logPathChanged();
}

QColor Configuration::getBkgdColor(Layer::Type vtype)
{
    return bkgdColors[(int)vtype];
}

void Configuration::setBkgdColor(Layer::Type vtype, QColor color)
{
    bkgdColors[(int)vtype] = color;
}
