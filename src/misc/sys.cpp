#include <QDebug>
#include "misc/sys.h"
#include "motifs/motif.h"
#include "geometry/dcel.h"
#include "geometry/edge.h"
#include "geometry/vertex.h"
#include "mosaic/mosaic.h"
#include "mosaic/design_element.h"
#include "makers/prototype_maker/prototype.h"
#include "style/style.h"
#include "tile/tile.h"
#include "tile/tiling.h"
#include "viewers/view_controller.h"

View            * Sys::view             = nullptr;
ViewController  * Sys::viewController   = nullptr;
Layer           * Sys::selectedLayer    = nullptr;
ImageEngine     * Sys::imageEngine      = nullptr;

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
bool Sys::debugMapEnable    = false;
bool Sys::dontTrapLog       = false;
bool Sys::measure           = false;
bool Sys::flagA             = false;
bool Sys::flagB             = false;
bool Sys::updatePanel       = true;
bool Sys::motifPropagate    = true;
bool Sys::enableDetachedPages = true;

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

void Sys::appDontPaint(bool stop)
{
    view->setAppPaint(!stop);
    view->setPaintEnable(!stop);
}

QMultiMap<eMotifType,QString>  Sys::motifRepresentation = init();

QMultiMap<eMotifType,QString> Sys::init()
{
    QMultiMap<eMotifType,QString> mmap;

    // Note: this table is ordered, so that the most recent (current) value is inserted later (after earlier)
    mmap.insert(MOTIF_TYPE_ROSETTE             , "app.Rosette");
    mmap.insert(MOTIF_TYPE_ROSETTE             , "Rosette");

    mmap.insert(MOTIF_TYPE_ROSETTE2            , "Rosette2");

    mmap.insert(MOTIF_TYPE_STAR                , "app.Star");
    mmap.insert(MOTIF_TYPE_STAR                , "Star");

    mmap.insert(MOTIF_TYPE_STAR2               , "Star2");

    mmap.insert(MOTIF_TYPE_CONNECT_STAR        , "ConnectStar");

    mmap.insert(MOTIF_TYPE_CONNECT_ROSETTE     , "app.ConnectFigure");
    mmap.insert(MOTIF_TYPE_CONNECT_ROSETTE     , "ConnectRosette");

    mmap.insert(MOTIF_TYPE_EXTENDED_ROSETTE    , "ExtendedRosette");

    mmap.insert(MOTIF_TYPE_EXTENDED_STAR       , "ExtendedStar");

    mmap.insert(MOTIF_TYPE_EXTENDED_STAR2      , "ExtendedStar2");

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

QString Sys::getMotifName(eMotifType type)
{
    auto list = motifRepresentation.values(type);
    return list.first();
}

eMotifType Sys::getMotifType(QString name)
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

QStringList Sys::getMotifNames(eMotifType type)
{
    return motifRepresentation.values(type);
}

void Sys::dumpRefs()
{
    qDebug() << "Mosaics:"  << Mosaic::refs
             << "Styles:"   << Style::refs
             << "Protos:"   << Prototype::refs
             << "Maps:"     << Map::refs
             << "DCELs"     << DCEL::refs
             << "faces"     << Face::refs
             << "DELs:"     << DesignElement::refs
             << "Motifs:"   << Motif::refs
             << "Tilings:"  << Tiling::refs
             << "Tiles:"    << Tile::refs
             << "Edges:"    << Edge::refs
             << "Vertices:" << Vertex::refs;
}
