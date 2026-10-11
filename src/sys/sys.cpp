#include <QApplication>
#include <QThread>
#include <QStandardPaths>
#include "gui/map_editor/map_editor.h"
#include "gui/map_editor/map_editor_db.h"
#include "gui/top/controlpanel.h"
#include "gui/top/splash_screen.h"
#include "gui/top/split_screen.h"
#include "gui/top/system_view.h"
#include "gui/top/view_controller.h"
#include "gui/viewers/construction_lines_view.h"
#include "gui/viewers/crop_maker_view.h"
#include "gui/viewers/debug_view.h"
#include "gui/viewers/grid_view.h"
#include "gui/viewers/image_view.h"
#include "model/tilings/backgroundimage.h"
#include "gui/viewers/map_editor_view.h"
#include "gui/viewers/motif_maker_view.h"
#include "gui/viewers/prototype_view.h"
#include "legacy/design_maker.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/motif_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/mosaics/mosaic.h"
#include "model/motifs/motif.h"
#include "model/prototypes/tile_motif.h"
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
SplashScreen    * Sys::splash           = nullptr;
qtAppLog        * Sys::log              = nullptr;
Configuration   * Sys::config           = nullptr;
ControlPanel    * Sys::controlPanel     = nullptr;
SplitScreen     * Sys::splitter         = nullptr;
DebugMap        * Sys::debugMapCreate   = nullptr;
DebugMap        * Sys::debugMapPaint    = nullptr;
DebugMap        * Sys::debugMapMotif    = nullptr;
DebugFlags      * Sys::flags            = nullptr;

// makers
DesignMaker    * Sys::designMaker       = nullptr;
MapEditor      * Sys::mapEditor         = nullptr;
MosaicMaker    * Sys::mosaicMaker       = nullptr;
MotifMaker *     Sys::motifMaker        = nullptr;
TilingMaker    * Sys::tilingMaker       = nullptr;
ImageEngine    * Sys::imageEngine       = nullptr;

// system view controller
ViewController * Sys::systemViewController    = nullptr;

DebugViewPtr     Sys::debugView;
CropViewPtr      Sys::cropMakerView;
GridViewPtr      Sys::gridViewer;
CLinesViewPtr    Sys::constructionLinesView;
MapedViewPtr     Sys::mapEditorView;
MMViewiewPtr     Sys::motifMakerView;
TMViewPtr        Sys::tilingMakerView;
ProtoViewPtr     Sys::prototypeView;
ImageViewPtr     Sys::imageViewer;
BkgdImagePtr     Sys::backgroundImage;

Layer          * Sys::selectedLayer     = nullptr;


int  Sys::appInstance       = 0;
uint Sys::rx_sigid          = 1;
int  Sys::iMouseMode        = 0;

qreal Sys::aspectRatio         = 0.0;
bool  Sys::lockViewAspectRatio = false;

bool Sys::isDarkTheme       = false;
bool Sys::imgGeneratorInUse = false;
bool Sys::localCycle        = false;
bool Sys::primaryDisplay    = false;
bool Sys::startupAutoloading = false;
bool Sys::tm_hideTiles      = false;
bool Sys::hideCircles       = false;
bool Sys::bm_editBorder     = false;
bool Sys::showCenterDebug   = false;
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

QString Sys::_rootImageDir;
QString Sys::_rootMediaDir;
QString Sys::_logDir;
QString Sys::logName;

QString Sys::bkgdPhotosDir;
QString Sys::examplesDir;
QString Sys::girihDir;
QString Sys::mapsDir;
QString Sys::newMosaicDir;
QString Sys::newTileDir;
QString Sys::originalMosaicDir;
QString Sys::originalTileDir;
QString Sys::renderedDir;        // svg files
QString Sys::rootMosaicDir;
QString Sys::rootTileDir;
QString Sys::snapsDir;
QString Sys::templateDir;
QString Sys::testMosiacDir;
QString Sys::testTileDir;
QString Sys::worklistsDir;

QString Sys::lastPanelTitle;
QString Sys::lastViewTitle;

QString Sys::xmlTool;
QString Sys::diffTool;

VersionFileList  Sys::tilingsList;
VersionFileList  Sys::mosaicsList;
TilingUses       Sys::tilingUses;
eActiveCanvas    Sys::_activeCanvas     = ACTIVE_TILING;

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

    QString astring     = QString("Tiled Pattern Maker (Version %1) Loading...........").arg(TpmVersion::getVersion());
    splash              = new SplashScreen();
    splash->disable(Sys::config->disableSplash);
    splash->display(astring);

    mosaicMaker         = new MosaicMaker;
    motifMaker          = new MotifMaker;
    tilingMaker         = new TilingMaker;
    mapEditor           = new MapEditor;
    designMaker         = new DesignMaker;

    controlPanel        = new ControlPanel;

    auto sysview         = new SystemView;
    systemViewController = new ViewController;
    systemViewController->attach(sysview);

    tilingMakerView     = make_shared<TilingMakerView>();
    prototypeView       = make_shared<PrototypeView>();
    motifMakerView      = make_shared<MotifMakerView>();
    mapEditorView       = make_shared<MapEditorView>();
    debugView           = make_shared<DebugView>();
    cropMakerView       = make_shared<CropMakerView>();
    gridViewer          = make_shared<GridView>();
    constructionLinesView = make_shared<ConstructionLinesView>();
    imageViewer         = make_shared<ImageViewer>();
    backgroundImage     = make_shared<BackgroundImage>();

    debugMapCreate      = new DebugMap;
    debugMapPaint       = new DebugMap;
    debugMapMotif       = new DebugMap;
    flags               = new DebugFlags;

    // init makers
    mosaicMaker->init();
    motifMaker->init();
    tilingMaker->init();        // creates an empty tiling which is propagated
    designMaker->init();
    mapEditor->init();

    // init system view
    sysview->init(systemViewController);

    // init control panel
    controlPanel->init();

    if (config->integratedScreen)
    {
        splitter = new SplitScreen();
        splitter->show();
    }
}

Sys::~Sys()
{
    flags->persist();

    gridViewer.reset();
    constructionLinesView.reset();
    imageViewer.reset();
    cropMakerView.reset();
    debugView.reset();
    mapEditorView.reset();
    motifMakerView.reset();
    prototypeView.reset();
    tilingMakerView.reset();

    delete systemViewController;
    systemViewController = nullptr;

    delete controlPanel;
    controlPanel = nullptr;

    delete designMaker;
    designMaker = nullptr;
    delete mapEditor;
    mapEditor = nullptr;
    delete tilingMaker;
    tilingMaker = nullptr;
    delete motifMaker;
    motifMaker = nullptr;
    delete mosaicMaker;
    mosaicMaker = nullptr;

    dumpRefs();
}

void Sys::setActiveCanvas(eActiveCanvas owner)
{
    _activeCanvas = owner;

    Canvas * canvas;
    if (owner == ACTIVE_TILING)
    {
        TilingPtr tiling = tilingMaker->getSelectedTiling();
        Q_ASSERT(tiling);
        canvas = tiling->hdr().getTilingCanvas();
    }
    else
    {
        canvas = mosaicMaker->getMosaic()->getMosaicCanvas();
    }

    systemViewController->setCanvas(canvas);
}

Canvas * Sys::getActiveCanvas()
{
    Q_ASSERT(isGuiThread());
    return systemViewController->getCanvas();
}

QTransform Sys::getActiveDistortion()
{
    return getActiveCanvas()->getDistorionTransform();
}

QSettings Sys::settings()
{
#if defined(Q_OS_WIN)
    return QSettings("HKEY_CURRENT_USER\\Software\\TiledPatternMaker", QSettings::NativeFormat);
#else
    // IniFormat gives you an explicit, predictable file path on Linux/Mac,
    // independent of organizationName/applicationName too.
    return QSettings(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/TiledPatternMaker/settings.ini", QSettings::IniFormat);
#endif
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
    {
        motifMaker->sm_resetMotifMaps();
        motifMaker->sm_resetProtoMaps();
        mosaicMaker->sm_resetStyles();

        ProtoPtr pp = Sys::motifMaker->motifData.getActivePrototype();
        pp->buildMotifMaps();
    }   break;

    case RENDER_RESET_PROTOTYPES:
    {
        motifMaker->sm_resetProtoMaps();
        mosaicMaker->sm_resetStyles();

        auto vec = Sys::motifMaker->protoData.getPrototypes();
        for (auto & pp : vec)
        {
            pp->createProtoMap();
        }
    }   break;

    case RENDER_RESET_STYLES:
        mosaicMaker->sm_resetStyles();

        mosaicMaker->getMosaic()->build();
        break;
    }

    systemViewController->slot_reconstructView();
}

bool Sys::isGuiThread()
{
    return QThread::currentThread() == QApplication::instance()->thread();
}

QMultiMap<Motif::Type,QString>  Sys::motifRepresentation = initMotifAssociations();

QMultiMap<Motif::Type,QString> Sys::initMotifAssociations()
{
    QMultiMap<Motif::Type,QString> mmap;

    // Note: this table is ordered, so that the most recent (current) value is inserted later (after earlier)
    // Note: never delete entries - only add or modify entries
    mmap.insert(Motif::Type::Rosette             , "app.Rosette");
    mmap.insert(Motif::Type::Rosette             , "ConnectRosette");
    mmap.insert(Motif::Type::Rosette             , "app.ConnectFigure");  // deprecated
    mmap.insert(Motif::Type::Rosette             , "ExtendedRosette");    // deprecated
    mmap.insert(Motif::Type::Rosette             , "Rosette");

    mmap.insert(Motif::Type::Rosette2            , "ExtendedRosette2");
    mmap.insert(Motif::Type::Rosette2            , "Rosette2");

    mmap.insert(Motif::Type::Star                , "app.Star");
    mmap.insert(Motif::Type::Star                , "ExtendedStar");
    mmap.insert(Motif::Type::Star                , "ConnectStar");
    mmap.insert(Motif::Type::Star                , "Star");

    mmap.insert(Motif::Type::Star2               , "ExtendedStar2");
    mmap.insert(Motif::Type::Star2               , "Star2");

    mmap.insert(Motif::Type::MapMotif            , "app.ExplicitFigure");
    mmap.insert(Motif::Type::MapMotif            , "ExplicitMap");

    mmap.insert(Motif::Type::Inferred            , "app.Infer");
    mmap.insert(Motif::Type::Inferred            , "IrregularInfer");

    mmap.insert(Motif::Type::AdaptiveRosette     , "app.ExplicitRosette");
    mmap.insert(Motif::Type::AdaptiveRosette     , "IrregularRosette");

    mmap.insert(Motif::Type::Hourglass           , "app.ExplicitHourglass");
    mmap.insert(Motif::Type::Hourglass           , "IrregularHourglass");

    mmap.insert(Motif::Type::Intersect           , "app.ExplicitIntersect");
    mmap.insert(Motif::Type::Intersect           , "IrregularIntersect");

    mmap.insert(Motif::Type::Girih               , "app.ExplicitGirih");
    mmap.insert(Motif::Type::Girih               , "IrregularGirih");

    mmap.insert(Motif::Type::AdaptiveStar        , "app.ExplicitStar");
    mmap.insert(Motif::Type::AdaptiveStar        , "IrregularStar");

    mmap.insert(Motif::Type::ExplicitTile        , "app.ExplicitFeature");
    mmap.insert(Motif::Type::ExplicitTile        , "ExplicitTile");

    mmap.insert(Motif::Type::NoMapMotif          , "IrregularNoMotif");
    mmap.insert(Motif::Type::NoMapMotif          , "NoMotif");

    return mmap;
}

bool Sys::getSysMouseMode(eMouseMode mode)
{
    if (mode == MOUSE_MODE_NONE)
        return iMouseMode == 0;
    else
        return (iMouseMode & mode) > 0;
}

void Sys::setSysMouseMode(eMouseMode newMode, bool set)
{
    qDebug() << "MouseMode:" << newMode;

    if (set)
    {
        switch (newMode)
        {
        case MOUSE_MODE_NONE:
            iMouseMode = 0;
            break;

        case MOUSE_MODE_TRANSLATE:
            iMouseMode |= newMode;
            break;

        case MOUSE_MODE_ROTATE:
            iMouseMode &= ~MOUSE_MODE_SCALE;
            iMouseMode |= newMode;
            break;

        case MOUSE_MODE_SCALE:
            iMouseMode &= ~MOUSE_MODE_ROTATE;
            iMouseMode |= newMode;
            break;
        }
    }
    else
    {
        switch (newMode)
        {
        case MOUSE_MODE_NONE:
            break;

        case MOUSE_MODE_TRANSLATE:
            iMouseMode &= ~newMode;
            break;

        case MOUSE_MODE_ROTATE:
            iMouseMode &= ~newMode;
            break;

        case MOUSE_MODE_SCALE:
            iMouseMode &= ~newMode;
            break;
        }
    }

    if (iMouseMode)
        showCenterMouse = true;
    else
        showCenterMouse = false;
}

QString Sys::XMLgetMotifName(Motif::Type type)
{
    auto list = motifRepresentation.values(type);
    return list.first();
}

Motif::Type Sys::XMLgetMotifType(QString name)
{
    auto i = motifRepresentation.cbegin();
    while (i != motifRepresentation.cend())
    {
        if (i.value() == name)
            return i.key();
        ++i;
    }

    return Motif::Type::Undefined;
}

QStringList Sys::XMLgetMotifNames(Motif::Type type)
{
    return motifRepresentation.values(type);
}

void Sys::dumpRefs()
{
    qDebug() << "Mosaics:"      << Mosaic::refs
             << "Styles:"       << Style::refs
             << "Protos:"       << Prototype::refs
             << "Maps:"         << Map::refs
             << "DCELs:"        << DCEL::refs
             << "Faces:"        << Face::refs
             << "TileMotifs:"   << TileMotif::refs
             << "Motifs:"       << Motif::refs
             << "Tilings:"      << Tiling::refs
             << "Tiles:"        << Tile::refs
             << "Edges:"        << Edge::refs
             << "Vertices:"     << Vertex::refs
             << "Neighbours:"   << Neighbours::refs;
}
