////////////////////////////////////////////////////////////////////////////
//
// Tiling.java
//
// The representation of a tiling, which will serve as the skeleton for
// Islamic designs.  A Tiling has two translation vectors and a set of
// placedTiles that make up a translational unit.  The idea is that
// the whole tiling can be replicated across the plane by placing
// a copy of the translational unit at every integer linear combination
// of the translation vectors.  In practice, we only draw at those
// linear combinations within some viewport.

#include "gui/top/view_controller.h"
#include "gui/viewers/geo_graphics.h"
#include "model/makers/tiling_maker.h"
#include "model/mosaics/mosaic.h"
#include "model/mosaics/mosaic_writer.h"
#include "model/motifs/adaptive_tile.h"
#include "model/prototypes/tile_motif.h"
#include "model/prototypes/prototype.h"
#include "model/settings/configuration.h"
#include "model/tilings/backgroundimage.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "model/tilings/tiling.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/fill_region.h"
#include "sys/geometry/map.h"
#include "sys/geometry/map_cleanser.h"
#include "sys/geometry/transform.h"
#include "sys/geometry/vertex.h"
#include "sys/sys.h"

using std::make_shared;

int Tiling::refs = 0;

Tiling::Tiling() : LayerController(Type::Tiling,Model::Primary,ZLevel::Standard_Z0,"Tiling"), _tilingUnit(this)
{
    name.set(Sys::defaultTilingName);
    version           = -1;
    _saveStatus       = new SaveStatus(this);   // has never been  saved
    _view             = true;     // default (and why not)
    _tilingViewChange = false;
    _legacyCenterConverted = false;
    refs++;
}

Tiling::Tiling(Tiling * other) : LayerController(Type::Tiling,Model::Primary,ZLevel::Standard_Z0,"Tiling"), _tilingUnit(this)
{
    name    = other->name;
    desc    = other->desc;
    author  = other->author;
    version = other->version;
    bkgdRecord = other->bkgdRecord;
    _header = other->hdr();
    auto ohdr = other->unit().uniqueCopy();
    _tilingUnit.replaceUnitData(ohdr);
    _constructionLines = other->_constructionLines;
    _saveStatus = new SaveStatus(this);   // has never been  saved
}

Tiling::~Tiling()
{
#ifdef EXPLICIT_DESTRUCTOR
#endif

    if(_saveStatus)
        delete _saveStatus;
    refs--;
}

bool Tiling::isEmpty()
{
    if (name.get() == Sys::defaultTilingName && unit().isEmpty())
        return true;
    else
        return false;
}

void Tiling::copy(TilingPtr tp)
{
    name    = tp->name;
    desc    = tp->desc;
    author  = tp->author;
    version = tp->version;
    _header  = tp->hdr().uniqueCopy();
    _tilingUnit = tp->unit().uniqueCopy();
    bkgdRecord = tp->bkgdRecord;
    _constructionLines = tp->_constructionLines;
}

QTransform Tiling::getDistortion()
{
    return hdr().getTilingCanvas()->getDistorionTransform();
}

bool Tiling::hasIntrinsicOverlaps()
{
    if (intrinsicOverlaps.get() == Tristate::eTristate::Undefined)
    {
        intrinsicOverlaps.set(Tristate::eTristate::False);
        PlacedTileSet tileSet = unit().getIncluded();
        for (const PlacedTilePtr & tile1 : std::as_const(tileSet))
        {
            QPolygonF poly = tile1->getWorldPoints(getDistortion());

            for (const PlacedTilePtr & tile2 : std::as_const(tileSet))
            {
                if (tile2 == tile1) continue;

                QPolygonF poly2 = tile2->getWorldPoints(getDistortion());
                if (poly2.intersects(poly))
                {
                    QPolygonF p3 = poly2.intersected(poly);
                    if (!p3.isEmpty())
                    {
                        //qDebug() << "overlapping";
                        intrinsicOverlaps.set(Tristate::eTristate::True);
                    }
                }
            }
        }
    }
    return (intrinsicOverlaps.get() == Tristate::eTristate::True);
}

bool Tiling::hasTiledOverlaps()
{
    if (tiledOverlaps.get() == Tristate::eTristate::Undefined)
    {
        MapPtr map = createMapFull();
        auto state = (map->hasIntersectingEdges() ?  Tristate::eTristate::True : Tristate::eTristate::False);
        tiledOverlaps.set(state);
    }
    return (tiledOverlaps.get() == Tristate::eTristate::True);
}

MapPtr Tiling::createMapSingle()
{
    MapPtr map = make_shared<Map>("tiling map single");
    PlacedTileSet tileSet = unit().getIncluded();
    for (const PlacedTilePtr & placedTile : std::as_const(tileSet))
    {
        EdgePoly poly = placedTile->getWorldEdgePoly(getDistortion());
        MapPtr emap = make_shared<Map>("tile",poly);
        map->mergeMap(emap);
    }

    MapCleanser mc(map);
    mc.deDuplicateEdgesUsingNeighbours();      // conceals an unknonwn problem

    return map;
}

MapPtr Tiling::createMapFullSimple()
{
    MapPtr map = make_shared<Map>("tiling map full simple");

    Placements fillPlacements = hdr().getFillPlacements();

    MapPtr tilingMap = createMapSingle();

    for (const QTransform & placement : std::as_const(fillPlacements))
    {
        MapPtr m  = tilingMap->recreate();
        m->transform(placement);
        map->mergeMap(m);
    }

    return map;
}

MapPtr Tiling::createMapFull()
{
    // This builds a prototype using explicit tile figures and generates its map
    MosaicPtr mosaic = std::make_shared<Mosaic>();
    FillData fd = hdr().getTilingCanvas()->getFillData();

    Canvas * cs = mosaic->getMosaicCanvas();
    cs->setFillData(fd);

    TilingPtr ptr = std::make_shared<Tiling>(this);
    ProtoPtr proto = make_shared<Prototype>(ptr,mosaic);

    TileSet uniqueTiles = unit().getUniqueTiles();

    for (const TilePtr & tile : std::as_const(uniqueTiles))
    {
        auto motif = make_shared<AdaptiveTile>();
        motif->setN(tile->numEdges());
        motif->setTile(tile);
        motif->build();
        TileMotifPtr  timo = make_shared<TileMotif>(ptr,tile, motif);
        proto->addTileMotif(timo);
    }

    // Now, for each different tile, build a submap corresponding
    // to all translations of that tile.

    Placements fillPlacements = hdr().getFillPlacements();

    MapPtr testmap = make_shared<Map>("testmap");
    for (auto & dep : proto->getTileMotifs())
    {
        TilePtr tile    = dep->getTile();
        MotifPtr motif  = dep->getMotif();
        MapPtr motifmap = motif->getMotifMap();

        PlacedTileSet tileSet = unit().getIncluded();
        Placements tilePlacements;
        for (const auto & placedTile : std::as_const(tileSet))
        {
            TilePtr f  = placedTile->getTile();
            if (f == tile)
            {
                QTransform t = placedTile->getWorldTransform(getDistortion());
                tilePlacements.push_back(t);
            }
        }

        // Within a single translational unit, assemble the different
        // transformed figures corresponding to the given tile into a map.
        MapPtr transmap = make_shared<Map>("tile transmap");
        transmap->mergeMany(motifmap, tilePlacements);

        // Just add these, so we get oversslaps
        MapPtr tilemap = make_shared<Map>("single tile map");
        for (const auto & T : std::as_const(fillPlacements))
        {
            for (const auto & edge : std::as_const(transmap->getEdges()))
            {
                EdgePtr nedge = tilemap->makeCopy(edge,T);
                if (!tilemap->edgeExists(nedge))
                {
                    tilemap->private_insertEdge(nedge);
                }
            }
        }

        // And do a quicl add (not a merge) to add this map to the finished design.
        testmap->addMap(tilemap);
    }
    return testmap;
}

MapPtr Tiling::debug_createFilledMap()
{
    MapPtr map = make_shared<Map>("tiling map");

    Placements placements  = hdr().getFillPlacements();
    PlacedTileSet tileSet = unit().getIncluded();

    MapPtr tilingMap = make_shared<Map>("tiling map");
    for (const auto & pfp : std::as_const(tileSet))
    {
        EdgePoly poly = pfp->getWorldEdgePoly();
        MapPtr emap = make_shared<Map>("tile",poly);
        map->mergeMap(emap);
    }

    for (const auto & transform : std::as_const(placements))
    {
        MapPtr m  = tilingMap->recreate();
        m->transform(transform);
        map->mergeMap(m);
    }
    return map;
}

MapPtr Tiling::debug_createProtoMap()
{
    // This builds a prototype using  Tile Motif and generates its map
    MosaicPtr mosaic = std::make_shared<Mosaic>();
    FillData fd = hdr().getTilingCanvas()->getFillData();
    Canvas * cs = mosaic->getMosaicCanvas();
    cs->setFillData(fd);

    TilingPtr ptr = std::make_shared<Tiling>(this);
    Prototype proto(ptr,mosaic);

    for (auto & tile : unit().getUniqueTiles())
    {
        auto motif = make_shared<AdaptiveTile>();
        motif->setN(tile->numEdges());
        motif->setTile(tile);
        motif->build();
        TileMotifPtr  dep = make_shared<TileMotif>(ptr,tile, motif);
        proto.addTileMotif(dep);
    }

    // Now, for each different feature, build a submap corresponding
    // to all translations of that feature.

    Placements fillPlacements = hdr().getFillPlacements();

    MapPtr testmap = make_shared<Map>("testmap");

    for (const auto & del : std::as_const(proto.getTileMotifs()))
    {
        TilePtr tile         = del->getTile();
        MotifPtr motif       = del->getMotif();
        constMapPtr motifmap = motif->getMotifMap();

        PlacedTileSet tileSet = unit().getIncluded();
        Placements tilePlacements;
        for (const auto & placedTile : std::as_const(tileSet))
        {
            TilePtr f  = placedTile->getTile();
            if (f == tile)
            {
                QTransform t = placedTile->getPlacement();
                tilePlacements.push_back(t);
            }
        }

        // Within a single translational unit, assemble the different
        // transformed figures corresponding to the given feature into a map.
        MapPtr transmap = make_shared<Map>("proto transmap");
        if (hasIntrinsicOverlaps())
            transmap->mergeMany(motifmap, tilePlacements);
        else
            transmap->mergeSimpleMany(motifmap, tilePlacements);

        constMapPtr cTransmap = transmap;
        // Now put all the translations together into a single map for this feature.
        MapPtr featuremap = make_shared<Map>("proto featuremap");
        featuremap->mergeSimpleMany(cTransmap, fillPlacements);

        // And do a slow merge to add this map to the finished design.
        testmap->mergeMap(featuremap);
    }
    return testmap;
}


int Tiling::getVersion()
{
    return version;
}

void Tiling::setVersion(int ver)
{
    version = ver;
}

QString Tiling::info()
{
    QString astring;
    QDebug  deb(&astring);

    deb.noquote() << "tiling=" << name.get() << hdr().info() << unit().info();
    return astring;
}

void Tiling::correctBackgroundAlignment(BkgdRecord & rec)
{
    // correction for legacy XML files
    const Xform & xf_til = getModelXform();
    QPointF til_trans    = xf_til.getTranslate();

    if (!til_trans.isNull())
    {
        QPointF bkgd_trans = rec.xf.getTranslate();
        bkgd_trans        -= til_trans;
        rec.xf.setTranslate(bkgd_trans);
    }
}

///////////////////////////////////////////////////////////////////////
//
//  Viewer functions
//
////////////////////////////////////////////////////////////////////////

void Tiling::paint(QPainter *painter)
{
    //qDebug() << "Tiling::paint" << name.get();

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QTransform tr = getLayerTransform();
    GeoGraphics gg(painter,tr);
    draw(&gg);

    drawLayerModelCenter(painter);
}

void Tiling::draw(GeoGraphics *gg)
{
    FillRegion flood(this,hdr().getTilingCanvas()->getFillData());
    Placements placements = flood.getPlacements(Sys::config->repeatMode);

    PlacedTileSet tileSet = unit().getIncluded();
    for (auto & T : placements)
    {
        gg->pushAndCompose(T);

        for (const auto & tile : std::as_const(tileSet))
        {
            drawPlacedTile(gg, tile);
        }

        gg->pop();
    }
}

void Tiling::drawPlacedTile(GeoGraphics * g2d, PlacedTilePtr ptp)
{
    QPen pen(Qt::red,3);

    EdgePoly ep   = ptp->getWorldEdgePoly(viewControl()->getCanvas()->getDistorionTransform());
    Q_ASSERT(ep.isCorrect());
    for (auto & edge : ep.get())
    {
        g2d->drawEdge(edge,pen);
    }
}

void Tiling::slot_mousePressed(QPointF spt, Qt::MouseButton btn)
{ Q_UNUSED(spt); Q_UNUSED(btn); }

void Tiling::slot_mouseDragged(QPointF spt)
{ Q_UNUSED(spt); }

void Tiling::slot_mouseTranslate(uint sigid, QPointF spt)
{
    if (!validateSignal()) return;

    if (Sys::tilingMaker->isKeybdMode(TilingMaker::KeybdMode::XformTiling))
    {
        QTransform T              = getCanvasTransform();
        qreal scale               = Transform::scalex(T);
        QPointF mpt               = spt/scale;
        QTransform tt             = QTransform::fromTranslate(mpt.x(),mpt.y());
        PlacedTileSet tileSet     = unit().getIncluded();
        for (const auto & pfp :  std::as_const(tileSet))
        {
            QTransform t = pfp->getPlacement();
            t *= tt;
            pfp->setPlacement(t);
        }
        forceRedraw();
    }
    else
    {
        Xform xf = getModelXform();
        spt /= xf.getScale();
        xf.setTranslate(xf.getTranslate() + spt);
        setModelXform(xf,true,sigid);
    }
}

void Tiling::slot_mouseMoved(QPointF spt)
{ Q_UNUSED(spt); }
void Tiling::slot_mouseReleased(QPointF spt)
{ Q_UNUSED(spt); }
void Tiling::slot_mouseDoublePressed(QPointF spt)
{ Q_UNUSED(spt); }

void Tiling::slot_wheel_scale(uint sigid, qreal delta)
{
    if (!validateSignal()) return;

    if (Sys::tilingMaker->isKeybdMode(TilingMaker::KeybdMode::XformTiling))
    {
        qreal sc = 1.0 + delta;
        QTransform ts;
        ts.scale(sc,sc);

        PlacedTileSet tileSet = unit().getIncluded();
        for (const auto & pfp : std::as_const(tileSet))
        {
            QTransform t = pfp->getPlacement();
            t *= ts;
            pfp->setPlacement(t);
        }
        forceRedraw();
    }
    else
    {
        Xform xf = getModelXform();
        xf.setScale(xf.getScale() * (1.0 + delta));
        setModelXform(xf,true,sigid);
    }
}

void Tiling::slot_wheel_rotate(uint sigid, qreal delta)
{
    if (!validateSignal()) return;

    if (Sys::tilingMaker->isKeybdMode(TilingMaker::KeybdMode::XformTiling))
    {
        qWarning() << "FIXME" << "TM_MODE_XFORM_TILING" << "Tiling::slot_wheel_rotate";
#if 0 // FIXME Tiling::slot_wheel_rotate
        QTransform tr;
        tr.rotate(delta);
        Xform xf(tr);
        xf.setModelCenter(getCenterModelUnits());
        QTransform tr2 = xf.getTransform();

        PlacedTiles tilingUnit = unit().getIncluded();
        for (const auto & pfp : std::as_const(tilingUnit))
        {
            QTransform t = pfp->getPlacement();
            t *= tr2;
            pfp->setPlacement(t);
        }
        forceRedraw();
#endif
    }
    else
    {
        Xform xf = getModelXform();
        xf.setRotateDegrees(xf.getRotateDegrees() + delta);
        setModelXform(xf,true,sigid);
    }
}

void Tiling::slot_scale(uint sigid, int amount)
{
    if (!validateSignal()) return;

    if (Sys::tilingMaker->isKeybdMode(TilingMaker::KeybdMode::XformTiling))
    {
        qreal scale = 1.0 + (0.01 * amount);
        PlacedTileSet tileSet = unit().getIncluded();
        for (const auto & pfp : std::as_const(tileSet))
        {
            QTransform t = pfp->getPlacement();
            qDebug() << "t0" << Transform::info(t);
            QTransform t1 = t.scale(scale,scale);

            t = pfp->getPlacement();
            QTransform t2 = t *QTransform::fromScale(scale,scale);

            qDebug() << "t1" << Transform::info(t1);
            qDebug() << "t2" << Transform::info(t2);

            t = pfp->getPlacement();
            // scales position too
            t *= QTransform::fromScale(scale,scale);
            pfp->setPlacement(t);
        }
        forceRedraw();
    }
    else
    {
        Xform xf = getModelXform();
        xf.setScale(xf.getScale() * (1 + static_cast<qreal>(amount)/100.0));
        setModelXform(xf,true,sigid);
    }
}

void Tiling::slot_rotate(uint sigid, int amount)
{
    if (!validateSignal()) return;

    qDebug() << "TilingMaker::slot_rotate" << amount;
    if (Sys::tilingMaker->isKeybdMode(TilingMaker::KeybdMode::XformTiling))
    {
        qreal qdelta = 0.01 * amount;
        PlacedTileSet tileSet = unit().getIncluded();
        for (const auto & pfp : std::as_const(tileSet))
        {
            QTransform t = pfp->getPlacement();
            t *= QTransform().rotateRadians(qdelta);
            pfp->setPlacement(t);
        }
        forceRedraw();
    }
    else
    {
        Xform xf = getModelXform();
        xf.setRotateRadians(xf.getRotateRadians() + qDegreesToRadians(static_cast<qreal>(amount)));
        setModelXform(xf,true,sigid);
    }
}

void Tiling:: slot_moveX(uint sigid, qreal amount)
{
    if (!validateSignal()) return;

    if (Sys::tilingMaker->isKeybdMode(TilingMaker::KeybdMode::XformTiling))
    {
        qreal qdelta = 0.01 * amount;
        PlacedTileSet tileSet = unit().getIncluded();
        for (const auto & pfp : std::as_const(tileSet))
        {
            QTransform t = pfp->getPlacement();
            t *= QTransform::fromTranslate(qdelta,0.0);
            pfp->setPlacement(t);
        }
        forceRedraw();
    }
    else
    {
        Xform xf = getModelXform();
        amount /= xf.getScale();
        xf.setTranslateX(xf.getTranslateX() + amount);
        setModelXform(xf,true,sigid);
    }
}

void Tiling::slot_moveY(uint sigid, qreal amount)
{
    if (!validateSignal()) return;

    if (Sys::tilingMaker->isKeybdMode(TilingMaker::KeybdMode::XformTiling))
    {
        qreal qdelta = 0.01 * amount;
        PlacedTileSet tileSet = unit().getIncluded();
        for (const auto & pfp : std::as_const(tileSet))
        {
            QTransform t = pfp->getPlacement();
            t *= QTransform::fromTranslate(0.0,qdelta);
            pfp->setPlacement(t);
        }
        forceRedraw();
    }
    else
    {
        Xform xf = getModelXform();
        amount /= xf.getScale();
        xf.setTranslateY(xf.getTranslateY() + amount);
        setModelXform(xf,true,sigid);
    }
}

bool Tiling::requiresSaving()
{
    return _saveStatus->needsSaving();
}

///////////////////////////////////////////////////////////////////////////////////
///
///     Status
///
///////////////////////////////////////////////////////////////////////////////////

SaveStatus::SaveStatus(Tiling * parent) : unit(parent)
{
    this->parent = parent;
    // starts off requiring saving
    // on load - init is called
    // on save - init is called again
}

void SaveStatus::init()
{
    header      = parent->hdr();
    auto ohdr   = parent->unit().uniqueCopy();
    unit.replaceUnitData(ohdr);
    constructionLines = parent->getConstructionLines();
}

bool SaveStatus::needsSaving()
{
    if (header != parent->hdr())
        return  true;

    TilingUnitController tunit(parent->unit());
    if (unit != tunit)
        return true;

    if (constructionLines != parent->getConstructionLines())
        return true;

    return false;
}
