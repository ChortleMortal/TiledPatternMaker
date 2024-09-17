#include <QDebug>
#include "gui/map_editor/map_editor.h"
#include "gui/top/controlpanel.h"
#include "gui/top/splash_screen.h"
#include "gui/top/view_controller.h"
#include "gui/viewers/backgroundimageview.h"
#include "gui/viewers/crop_view.h"
#include "gui/viewers/debug_view.h"
#include "gui/viewers/grid_view.h"
#include "gui/viewers/gui_modes.h"
#include "gui/viewers/map_editor_view.h"
#include "gui/viewers/measure_view.h"
#include "gui/viewers/motif_view.h"
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
#include "sys/geometry/edge.h"
#include "sys/geometry/vertex.h"
#include "sys/sys.h"
#include "sys/sys/load_unit.h"
#include "sys/version.h"

// system
qtAppLog        * Sys::log              = nullptr;
Configuration   * Sys::config           = nullptr;
ControlPanel    * Sys::controlPanel     = nullptr;
SplashScreen    * Sys::splash           = nullptr;
GuiModes        * Sys::guiModes         = nullptr;
LoadUnit        * Sys::loadUnit         = nullptr;

// makers
DesignMaker    * Sys::designMaker       = nullptr;
MapEditor      * Sys::mapEditor         = nullptr;
MosaicMaker    * Sys::mosaicMaker       = nullptr;
PrototypeMaker * Sys::prototypeMaker    = nullptr;
TilingMaker    * Sys::tilingMaker       = nullptr;

// viewers
View           * Sys::view              = nullptr;
ViewController * Sys::viewController    = nullptr;
DebugView      * Sys::debugView         = nullptr;
CropViewer     * Sys::cropViewer        = nullptr;
GridView       * Sys::gridViewer        = nullptr;
MapEditorView  * Sys::mapEditorView     = nullptr;
MeasureView    * Sys::measureView       = nullptr;
MotifView      * Sys::motifView         = nullptr;
TilingMakerView* Sys::tilingMakerView   = nullptr;
PrototypeView  * Sys::prototypeView     = nullptr;
ImageEngine    * Sys::imageEngine       = nullptr;
BackgroundImageView * Sys::backgroundImageView = nullptr;

Layer          * Sys::selectedLayer     = nullptr;

int  Sys::appInstance       = 0;

bool Sys::isDarkTheme       = false;
bool Sys::usingImgGenerator = false;
bool Sys::localCycle        = false;
bool Sys::primaryDisplay    = false;
bool Sys::circleX           = false;
bool Sys::hideCircles       = false;
bool Sys::showCenterMouse   = false;
bool Sys::dontReplicate     = false;
bool Sys::highlightUnit     = false;
bool Sys::dontTrapLog       = false;
bool Sys::measure           = false;
bool Sys::flagA             = false;
bool Sys::flagB             = false;
bool Sys::updatePanel       = true;
bool Sys::motifPropagate    = true;
bool Sys::enableDetachedPages = true;
bool Sys::tm_fill           = false;

bool Sys::_tilingViewChange = false;

QString Sys::gitBranch;
QString Sys::gitSha;
QString Sys::rootTileDir;
QString Sys::originalTileDir;
QString Sys::newTileDir;
QString Sys::testTileDir;
QString Sys::rootMosaicDir;
QString Sys::originalMosaicDir;
QString Sys::newMosaicDir;
QString Sys::testMosiacDir;
QString Sys::templateDir;
QString Sys::examplesDir;
QString Sys::mapsDir;
QString Sys::worklistsDir;
QString Sys::lastPanelTitle;
QString Sys::lastViewTitle;

VersionFileList  Sys::tilingsList;
VersionFileList  Sys::mosaicsList;
TilingUses       Sys::tilingUses;

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

const QString Sys::defaultMosaicName =  "The Formless";
const QString Sys::defaultTilingName = "The Unnamed";

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
    splash->display(astring);

    mosaicMaker         = new MosaicMaker;
    prototypeMaker      = new PrototypeMaker;
    tilingMaker         = new TilingMaker;
    mapEditor           = new MapEditor;
    designMaker         = new DesignMaker;

    controlPanel        = new ControlPanel;
    guiModes            = new GuiModes;
    loadUnit            = new LoadUnit;

    view                = new View;
    viewController      = new ViewController;

    tilingMakerView     = new TilingMakerView;
    prototypeView       = new PrototypeView;
    motifView           = new MotifView;
    mapEditorView       = new MapEditorView;
    debugView           = new DebugView;
    cropViewer          = new CropViewer;
    backgroundImageView = new BackgroundImageView;
    ProtoPtr nullProto;
    measureView         = new MeasureView(nullProto);
    gridViewer          = new GridView;

    // init makers
    mosaicMaker->init();
    prototypeMaker->init();
    tilingMaker->init();        // creates an empty tiling which is propagated
    designMaker->init();
    mapEditor->init();

    // init views
    view->init(viewController);
    viewController->init(view);

    // init control panel
    controlPanel->init();
}

Sys::~Sys()
{
    delete gridViewer;
    gridViewer = nullptr;
    delete measureView;
    measureView = nullptr;
    delete backgroundImageView;
    backgroundImageView = nullptr;
    delete cropViewer;
    cropViewer = nullptr;
    delete debugView;
    debugView = nullptr;
    delete mapEditorView;
    mapEditorView = nullptr;
    delete motifView;
    motifView = nullptr;
    delete prototypeView;
    prototypeView = nullptr;
    delete tilingMakerView;
    tilingMakerView = nullptr;

    delete viewController;
    viewController = nullptr;
    delete view;
    view = nullptr;
    delete loadUnit;
    loadUnit = nullptr;

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
             << "Vertices:" << Vertex::refs;
}
