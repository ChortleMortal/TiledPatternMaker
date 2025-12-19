#include "gui/map_editor/map_editor.h"
#include "gui/map_editor/map_editor_db.h"
#include "gui/top/controlpanel.h"
#include "gui/top/splash_screen.h"
#include "gui/top/split_screen.h"
#include "gui/top/system_view.h"
#include "gui/top/system_view_controller.h"
#include "gui/viewers/crop_viewer.h"
#include "gui/viewers/debug_view.h"
#include "gui/viewers/grid_view.h"
#include "gui/viewers/gui_modes.h"
#include "gui/viewers/image_view.h"
#include "gui/viewers/map_editor_view.h"
#include "gui/viewers/motif_maker_view.h"
#include "gui/viewers/prototype_view.h"
#include "legacy/design_maker.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/prototype_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/mosaics/mosaic.h"
#include "model/motifs/motif.h"
#include "model/prototypes/design_element.h"
#include "model/prototypes/prototype.h"
#include "model/settings/configuration.h"
#include "model/styles/style.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "sys/geometry/dcel.h"
#include "sys/geometry/debug_map.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/vertex.h"
#include "sys/debugflags.h"
#include "sys/sys.h"
#include "sys/version.h"

#ifdef QT_DEBUG
#ifdef Q_OS_WINDOWS
#include <Windows.h>
#else
#include <signal.h>
#endif
#endif

using std::make_shared;

// system
qtAppLog        * Sys::log              = nullptr;
Configuration   * Sys::config           = nullptr;
ControlPanel    * Sys::controlPanel     = nullptr;
SplitScreen     * Sys::splitter         = nullptr;
SplashScreen    * Sys::splash           = nullptr;
GuiModes        * Sys::guiModes         = nullptr;
DebugMap        * Sys::debugMapCreate   = nullptr;
DebugMap        * Sys::debugMapPaint    = nullptr;
DebugFlags      * Sys::flags            = nullptr;

// makers
DesignMaker    * Sys::designMaker       = nullptr;
MapEditor      * Sys::mapEditor         = nullptr;
MosaicMaker    * Sys::mosaicMaker       = nullptr;
PrototypeMaker * Sys::prototypeMaker    = nullptr;
TilingMaker    * Sys::tilingMaker       = nullptr;
ImageEngine    * Sys::imageEngine       = nullptr;

// viewers
SystemView           * Sys::sysview           = nullptr;
SystemViewController * Sys::viewController    = nullptr;

DebugViewPtr     Sys::debugView;
CropViewPtr      Sys::cropViewer;
GridViewPtr      Sys::gridViewer;
MapedViewPtr     Sys::mapEditorView;
MMViewiewPtr     Sys::motifMakerView;
TMViewPtr        Sys::tilingMakerView;
ProtoViewPtr     Sys::prototypeView;
ImageViewPtr     Sys::imageViewer;

Layer          * Sys::selectedLayer     = nullptr;

wBkgdImagePtr    Sys::definedBkImage;

int  Sys::appInstance       = 0;
uint Sys::rx_sigid          = 1;
bool Sys::isDarkTheme       = false;
bool Sys::imgGeneratorInUse = false;
bool Sys::localCycle        = false;
bool Sys::primaryDisplay    = false;
bool Sys::hideCircles       = false;
bool Sys::showCenterMouse   = false;
bool Sys::dontReplicate     = false;
bool Sys::highlightUnit     = false;
bool Sys::dontTrapLog       = false;

bool Sys::updatePanel       = true;
bool Sys::tm_fill           = false;
bool Sys::enableDetachedPages = true;

QString Sys::gitBranch;
QString Sys::gitSha;
QString Sys::gitRoot;

QString Sys::sysBMPDir;
QString Sys::workingBMPDir;
QString Sys::examplesDir;
QString Sys::mapsDir;
QString Sys::newMosaicDir;
QString Sys::newTileDir;
QString Sys::originalMosaicDir;
QString Sys::originalTileDir;
QString Sys::rootMosaicDir;
QString Sys::rootTileDir;
QString Sys::templateDir;
QString Sys::testMosiacDir;
QString Sys::testTileDir;
QString Sys::worklistsDir;

QString Sys::lastPanelTitle;
QString Sys::lastViewTitle;

VersionFileList  Sys::tilingsList;
VersionFileList  Sys::mosaicsList;
TilingUses       Sys::tilingUses;

eBkgdImgSource  Sys::currentBkgImage    = BKGD_IMAGE_NONE;

const QChar Sys::MathSymbolSquareRoot   = QChar(0x221A);
const QChar Sys::MathSymbolPi           = QChar(0x03A0);
const QChar Sys::MathSymbolDelta        = QChar(0x0394);
const QChar Sys::MathSymbolSigma        = QChar(0x03A3);

const QPointF Sys::ORIGIN               = QPointF(0,0);

const qreal Sys::NEAR_TOL               = 1e-4;
const qreal Sys::TOL                    = 1e-7;
const qreal Sys::TOL2                   = 1e-10;
const qreal Sys::TOLSQ                  = TOL * TOL;

const qreal Sys::GOLDEN_RATIO           = 1.61803398874989484820;

const int   Sys::DEFAULT_WIDTH          = 1500;
const int   Sys::DEFAULT_HEIGHT         = 927;
const int   Sys::MAX_WIDTH              = 4096;
const int   Sys::MAX_HEIGHT             = 2160;

const QString Sys::defaultMosaicName    = "The Formless";
const QString Sys::defaultTilingName    = "The Unnamed";

///////////////////////////////////////////////////////////////
///
/// Sys
///
///////////////////////////////////////////////////////////////


Sys::Sys()
{
    // Configuration and log have already been instantiated
    // log has been initialised too

    QString astring     = QString("Tiled Pattern Maker (Version %1) Loading...........").arg(tpmVersion);
    splash              = new SplashScreen();
    splash->disable(config->disableSplash);
    splash->display(astring);

    mosaicMaker         = new MosaicMaker;
    prototypeMaker      = new PrototypeMaker;
    tilingMaker         = new TilingMaker;
    mapEditor           = new MapEditor;
    designMaker         = new DesignMaker;

    controlPanel        = new ControlPanel;
    guiModes            = new GuiModes;

    sysview             = new SystemView;
    viewController      = new SystemViewController;

    tilingMakerView     = make_shared<TilingMakerView>();
    prototypeView       = make_shared<PrototypeView>();
    motifMakerView      = make_shared<MotifMakerView>();
    mapEditorView       = make_shared<MapEditorView>();
    debugView           = make_shared<DebugView>();
    cropViewer          = make_shared<CropViewer>();
    gridViewer          = make_shared<GridView>();
    imageViewer         = make_shared<ImageViewer>();

    debugMapCreate      = new DebugMap;
    debugMapPaint       = new DebugMap;
    flags               = new DebugFlags;

    sysBMPDir           = createBMPDirectory();

    // init makers
    mosaicMaker->init();
    prototypeMaker->init();
    tilingMaker->init();        // creates an empty tiling which is propagated
    designMaker->init();
    mapEditor->init();

    // init views
    sysview->init(viewController);
    viewController->attach(sysview);

    // init control panel
    controlPanel->init();

    if (config->splitScreen)
    {
        splitter = new SplitScreen();
        splitter->show();
    }
}

Sys::~Sys()
{
    flags->persist();

    gridViewer.reset();
    imageViewer.reset();
    cropViewer.reset();
    debugView.reset();
    mapEditorView.reset();
    motifMakerView.reset();
    prototypeView.reset();
    tilingMakerView.reset();

    delete viewController;
    viewController = nullptr;
    delete sysview;
    sysview = nullptr;

    delete controlPanel;
    controlPanel = nullptr;
    delete guiModes;
    guiModes = nullptr;

    delete designMaker;
    designMaker = nullptr;
    delete mapEditor;
    mapEditor = nullptr;
    delete tilingMaker;
    tilingMaker = nullptr;
    delete prototypeMaker;
    prototypeMaker = nullptr;
    delete mosaicMaker;
    mosaicMaker = nullptr;

    dumpRefs();
}

void Sys::appDebugBreak()
{
#ifdef QT_DEBUG
    qWarning() << "Sys::appDebugBreak";
#ifdef Q_OS_WINDOWS
    if (IsDebuggerPresent())
    {
        DebugBreak();
    }
#else
    raise(SIGTRAP);
#endif
#endif
}

void Sys::render(eRenderType rtype)
{
    switch (rtype)
    {
    case RENDER_RESET_MOTIFS:
        prototypeMaker->sm_resetMotifMaps();
        prototypeMaker->sm_resetProtoMaps();
        mosaicMaker->sm_resetStyles();
        break;

    case RENDER_RESET_PROTOTYPES:
        prototypeMaker->sm_resetProtoMaps();
        mosaicMaker->sm_resetStyles();
        break;

    case RENDER_RESET_STYLES:
        mosaicMaker->sm_resetStyles();
        break;
    }

    viewController->slot_reconstructView();
}

bool Sys::isGuiThread()
{
    return QThread::currentThread() == QApplication::instance()->thread();
}

QMultiMap<eMotifType,QString>  Sys::motifRepresentation = initMotifAssociations();

QMultiMap<eMotifType,QString> Sys::initMotifAssociations()
{
    QMultiMap<eMotifType,QString> mmap;

    // Note: this table is ordered, so that the most recent (current) value is inserted later (after earlier)
    // Note: never delete entries - only add or modify entries
    mmap.insert(MOTIF_TYPE_ROSETTE             , "app.Rosette");
    mmap.insert(MOTIF_TYPE_ROSETTE             , "ConnectRosette");
    mmap.insert(MOTIF_TYPE_ROSETTE             , "app.ConnectFigure");  // deprecated
    mmap.insert(MOTIF_TYPE_ROSETTE             , "ExtendedRosette");    // deprecated
    mmap.insert(MOTIF_TYPE_ROSETTE             , "Rosette");

    mmap.insert(MOTIF_TYPE_ROSETTE2            , "ExtendedRosette2");
    mmap.insert(MOTIF_TYPE_ROSETTE2            , "Rosette2");

    mmap.insert(MOTIF_TYPE_STAR                , "app.Star");
    mmap.insert(MOTIF_TYPE_STAR                , "ExtendedStar");
    mmap.insert(MOTIF_TYPE_STAR                , "ConnectStar");
    mmap.insert(MOTIF_TYPE_STAR                , "Star");

    mmap.insert(MOTIF_TYPE_STAR2               , "ExtendedStar2");
    mmap.insert(MOTIF_TYPE_STAR2               , "Star2");

    mmap.insert(MOTIF_TYPE_EXPLICIT_MAP        , "app.ExplicitFigure");
    mmap.insert(MOTIF_TYPE_EXPLICIT_MAP        , "ExplicitMap");

    mmap.insert(MOTIF_TYPE_INFERRED            , "app.Infer");
    mmap.insert(MOTIF_TYPE_INFERRED            , "IrregularInfer");

    mmap.insert(MOTIF_TYPE_IRREGULAR_ROSETTE   , "app.ExplicitRosette");
    mmap.insert(MOTIF_TYPE_IRREGULAR_ROSETTE   , "IrregularRosette");

    mmap.insert(MOTIF_TYPE_HOURGLASS           , "app.ExplicitHourglass");
    mmap.insert(MOTIF_TYPE_HOURGLASS           , "IrregularHourglass");

    mmap.insert(MOTIF_TYPE_INTERSECT           , "app.ExplicitIntersect");
    mmap.insert(MOTIF_TYPE_INTERSECT           , "IrregularIntersect");

    mmap.insert(MOTIF_TYPE_GIRIH               , "app.ExplicitGirih");
    mmap.insert(MOTIF_TYPE_GIRIH               , "IrregularGirih");

    mmap.insert(MOTIF_TYPE_IRREGULAR_STAR      , "app.ExplicitStar");
    mmap.insert(MOTIF_TYPE_IRREGULAR_STAR      , "IrregularStar");

    mmap.insert(MOTIF_TYPE_EXPLCIT_TILE        , "app.ExplicitFeature");
    mmap.insert(MOTIF_TYPE_EXPLCIT_TILE        , "ExplicitTile");

    mmap.insert(MOTIF_TYPE_IRREGULAR_NO_MAP    , "IrregularNoMotif");
    mmap.insert(MOTIF_TYPE_IRREGULAR_NO_MAP    , "NoMotif");

    return mmap;
}

QString Sys::XMLgetMotifName(eMotifType type)
{
    auto list = motifRepresentation.values(type);
    return list.first();
}

eMotifType Sys::XMLgetMotifType(QString name)
{
    auto i = motifRepresentation.cbegin();
    while (i != motifRepresentation.cend())
    {
        if (i.value() == name)
            return i.key();
        ++i;
    }

    return MOTIF_TYPE_UNDEFINED;
}

QStringList Sys::XMLgetMotifNames(eMotifType type)
{
    return motifRepresentation.values(type);
}

QString Sys::createBMPDirectory()
{
    QString dirname;

    QDateTime d = QDateTime::currentDateTime();

    dirname = d.toString("yyyy-MM-dd");
    if (!Sys::gitBranch.isEmpty())
    {
        dirname = dirname + "-" + Sys::gitBranch;
    }
    else if (!Sys::gitSha.isEmpty())
    {
        dirname = dirname + "-" + Sys::gitSha;
    }
    return dirname;
}

QString Sys::getBMPPath(eActionType generatorType)
{
    QString subdir;
    switch (config->repeatMode)
    {
    case REPEAT_SINGLE:
        subdir = "single/";
        break;
    case REPEAT_PACK:
        subdir = "pack/";
        break;
    case REPEAT_DEFINED:
        subdir = "defined/";
        break;
    }

    QString dir = workingBMPDir;

    QString path = config->rootImageDir;
    if (generatorType == ACT_GEN_TILING_BMP)
        path += "tilings/" + subdir + dir;
    else
        path += subdir + dir;

    QDir adir(path);
    if (!adir.exists())
    {
        if (!adir.mkpath(path))
        {
            qFatal("could not make path");
        }
    }
    return path;
}

BkgdImagePtr Sys::getBackgroundImageFromSource()
{
    BkgdImagePtr bip;
    switch (currentBkgImage)
    {
    case BKGD_IMAGE_NONE:
        break;

    case BKGD_IMAGE_MOSAIC:
    {
        MosaicPtr mosaic = Sys::mosaicMaker->getMosaic();
        if (mosaic)
        {
            bip = mosaic->getBkgdImage();
        }
    }   break;

    case BKGD_IMAGE_TILING:
    {
        TilingPtr tiling = Sys::tilingMaker->getSelected();
        if (tiling)
        {
            bip = tiling->getBkgdImage();
        }
    }   break;

    case BKGD_IMAGE_MAPED:
        bip = Sys::mapEditor->getDb()->getBackgroundImage();
        break;

    case BKGD_IMAGE_DEFINED:
        bip =  definedBkImage.lock();
        break;
    }

    return bip;
}

void Sys::dumpRefs()
{
    qDebug() << "Mosaics:"  << Mosaic::refs
             << "Styles:"   << Style::refs
             << "Protos:"   << Prototype::refs
             << "Maps:"     << Map::refs
             << "DCELs:"    << DCEL::refs
             << "Faces:"    << Face::refs
             << "DELs:"     << DesignElement::refs
             << "Motifs:"   << Motif::refs
             << "Tilings:"  << Tiling::refs
             << "Tiles:"    << Tile::refs
             << "Edges:"    << Edge::refs
             << "Vertices:" << Vertex::refs
             << "Neighbours:" << Neighbours::refs;
}
