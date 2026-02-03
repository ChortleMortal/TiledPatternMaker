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

#include "gui/top/system_view_controller.h"
#include "gui/viewers/geo_graphics.h"
#include "model/makers/tiling_maker.h"
#include "model/mosaics/mosaic.h"
#include "model/mosaics/mosaic_writer.h"
#include "model/motifs/tile_motif.h"
#include "model/prototypes/design_element.h"
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

Tiling::Tiling() : LayerController(VIEW_TILING,PRIMARY,"Tiling"), _tilingUnit(this)
{
    name.set(Sys::defaultTilingName);
    version           = -1;
    _saveStatus        = new SaveStatus(this);   // has never been  saved
    _view             = true;     // default (and why not)
    _tilingViewChange = false;
    _legacyCenterConverted = false;
    refs++;
}

Tiling::Tiling(Tiling * other) : LayerController(VIEW_TILING,PRIMARY,"Tiling"), _tilingUnit(this)
{
    name    = other->name;
    desc    = other->desc;
    author  = other->author;
    version = other->version;
    bip     = other->bip;
    _header = other->hdr();
    auto ohdr = other->unit().uniqueCopy();
    _tilingUnit.replaceUnitData(ohdr);
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
    bip     = tp->bip;
}

bool Tiling::hasIntrinsicOverlaps()
{
    if (intrinsicOverlaps.get() == Tristate::Unknown)
    {
        intrinsicOverlaps.set(Tristate::False);
        PlacedTiles tilingUnit;
        if (Sys::config->tm_showExcludes)
            tilingUnit = unit().getAll();
        else
            tilingUnit = unit().getIncluded();
        for (const auto & tile1 : std::as_const(tilingUnit))
        {
            QPolygonF poly = tile1->getPlacedPoints();

            for (const auto & tile2 : std::as_const(tilingUnit))
            {
                if (tile2 == tile1) continue;

                QPolygonF poly2 = tile2->getPlacedPoints();
                if (poly2.intersects(poly))
                {
                    QPolygonF p3 = poly2.intersected(poly);
                    if (!p3.isEmpty())
                    {
                        //qDebug() << "overlapping";
                        intrinsicOverlaps.set(Tristate::True);
                    }
                }
            }
        }
    }
    return (intrinsicOverlaps.get() == Tristate::True);
}

bool Tiling::hasTiledOverlaps()
{
    if (tiledOverlaps.get() == Tristate::Unknown)
    {
        MapPtr map = createMapFull();
        auto state = (map->hasIntersectingEdges() ?  Tristate::True : Tristate::False);
        tiledOverlaps.set(state);
    }
    return (tiledOverlaps.get() == Tristate::True);
}

MapPtr Tiling::createMapSingle()
{
    MapPtr map = make_shared<Map>("tiling map single");
    PlacedTiles tilingUnit = unit().getIncluded();
    for (const auto & placedTile : std::as_const(tilingUnit))
    {
        EdgePoly poly = placedTile->getPlacedEdgePoly();
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

    for (const auto & placement : std::as_const(fillPlacements))
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
    FillData fd = hdr().getCanvasSettings().getFillData();
    mosaic->getCanvasSettings().setFillData(fd);

    TilingPtr ptr = std::make_shared<Tiling>(this);
    auto proto = make_shared<Prototype>(ptr,mosaic);

    QVector<TilePtr> uniqueTiles = unit().getUniqueTiles();

    for (const auto & tile : std::as_const(uniqueTiles))
    {
        auto motif = make_shared<TileMotif>();
        motif->setN(tile->numEdges());
        motif->setTile(tile);
        motif->buildMotifMap();
        DELPtr  dep = make_shared<DesignElement>(ptr,tile, motif);
        proto->addDesignElement(dep);
    }

    // Now, for each different tile, build a submap corresponding
    // to all translations of that tile.

    Placements fillPlacements = hdr().getFillPlacements();

    MapPtr testmap = make_shared<Map>("testmap");
    for (auto & dep : proto->getDesignElements())
    {
        TilePtr tile    = dep->getTile();
        MotifPtr motif  = dep->getMotif();
        MapPtr motifmap = motif->getMotifMap();

        PlacedTiles tilingUnit = unit().getIncluded();
        Placements tilePlacements;
        for (const auto & placedTile : std::as_const(tilingUnit))
        {
            TilePtr f  = placedTile->getTile();
            if (f == tile)
            {
                QTransform t = placedTile->getPlacement();
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
    PlacedTiles tilingUnit = unit().getIncluded();

    MapPtr tilingMap = make_shared<Map>("tiling map");
    for (const auto & pfp : std::as_const(tilingUnit))
    {
        EdgePoly poly = pfp->getPlacedEdgePoly();
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
    FillData fd = hdr().getCanvasSettings().getFillData();
    mosaic->getCanvasSettings().setFillData(fd);

    TilingPtr ptr = std::make_shared<Tiling>(this);
    Prototype proto(ptr,mosaic);

    for (auto & tile : unit().getUniqueTiles())
    {
        auto motif = make_shared<TileMotif>();
        motif->setN(tile->numEdges());
        motif->setTile(tile);
        motif->buildMotifMap();
        DELPtr  dep = make_shared<DesignElement>(ptr,tile, motif);
        proto.addDesignElement(dep);
    }

    // Now, for each different feature, build a submap corresponding
    // to all translations of that feature.

    Placements fillPlacements = hdr().getFillPlacements();

    MapPtr testmap = make_shared<Map>("testmap");

    for (const auto & del : std::as_const(proto.getDesignElements()))
    {
        TilePtr tile         = del->getTile();
        MotifPtr motif       = del->getMotif();
        constMapPtr motifmap = motif->getMotifMap();

        PlacedTiles tilingUnit = unit().getIncluded();
        Placements tilePlacements;
        for (const auto & placedTile : std::as_const(tilingUnit))
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

//////////////////////////////////////////////////////////////////////////////////
///
/// Viewable
///
//////////////////////////////////////////////////////////////////////////////////

// called on demand by paint
void Tiling::createViewablePlacedTiles()
{
    if (Sys::tm_fill == false)
    {
        // viewable consists of tiling unit
        if (Sys::config->tm_showExcludes)
            _viewable = unit().getAll();
        else
            _viewable = unit().getIncluded();
    }
    else
    {
        PlacedTiles placements  = unit().getIncluded();
        _viewable = placements;

        QPointF t1    = hdr().getTrans1();
        QPointF t2    = hdr().getTrans2();
        if (t1.isNull() || t2.isNull())
        {
            return;
        }

        int minX, maxX, minY, maxY;
        bool singleton;
        const FillData fd = hdr().getCanvasSettings().getFillData();
        fd.get(singleton,minX, maxX, minY, maxY);

        if (singleton)
        {
            return;
        }

        // Create copies of the tile to help visualise the result in the panel.
        for (const auto & placed : std::as_const(placements))
        {
            if (!placed->show())
            {
                continue;
            }

            TilePtr f    = placed->getTile();
            QTransform T = placed->getPlacement();

            for( int y = minY; y <= maxY; ++y )
            {
                for( int x = minX; x <= maxX; ++x )
                {
                    if ( y == 0 && x == 0 )
                    {
                        continue;
                    }
                    QPointF pt = (t1*x) + (t2 * y);
                    QTransform tt = QTransform::fromTranslate(pt.x(),pt.y());
                    QTransform placement= T * tt;
                    auto ptp = make_shared<PlacedTile>(f, placement);
                    ptp->exclude();
                    _viewable.push_back(ptp);
                }
            }
        }
    }
}

void Tiling::correctBackgroundAlignment(BkgdImagePtr bip)
{
    // correction for legacy XML files
    const Xform & xf_til = getModelXform();
    QPointF til_trans    = xf_til.getTranslate();

    if (!til_trans.isNull())
    {
        Xform xf_bkgd      = bip->getModelXform();
        QPointF bkgd_trans = xf_bkgd.getTranslate();
        bkgd_trans        -= til_trans;
        xf_bkgd.setTranslate(bkgd_trans);

        bip->setModelXform(xf_bkgd,false, Sys::nextSigid());
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
    FillRegion flood(this,hdr().getCanvasSettings().getFillData());
    Placements placements = flood.getPlacements(Sys::config->repeatMode);

    PlacedTiles tilingUnit = unit().getIncluded();
    for (auto & T : placements)
    {
        gg->pushAndCompose(T);

        for (const auto & tile : std::as_const(tilingUnit))
        {
            drawPlacedTile(gg, tile);
        }

        gg->pop();
    }
}

void Tiling::drawPlacedTile(GeoGraphics * g2d, PlacedTilePtr ptp)
{
    QPen pen(Qt::red,3);

    EdgePoly ep   = ptp->getPlacedEdgePoly();
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

    if (Sys::tilingMaker->isTMKbdMode(TM_MODE_XFORM_TILING))
    {
        QTransform T              = getCanvasTransform();
        qreal scale               = Transform::scalex(T);
        QPointF mpt               = spt/scale;
        QTransform tt             = QTransform::fromTranslate(mpt.x(),mpt.y());
        PlacedTiles tilingUnit    = unit().getIncluded();
        for (const auto & pfp :  std::as_const(tilingUnit))
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

    if (Sys::tilingMaker->isTMKbdMode(TM_MODE_XFORM_TILING))
    {
        qreal sc = 1.0 + delta;
        QTransform ts;
        ts.scale(sc,sc);

        PlacedTiles tilingUnit = unit().getIncluded();
        for (const auto & pfp : std::as_const(tilingUnit))
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

    if (Sys::tilingMaker->isTMKbdMode(TM_MODE_XFORM_TILING))
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

    if (Sys::tilingMaker->isTMKbdMode(TM_MODE_XFORM_TILING))
    {
        qreal scale = 1.0 + (0.01 * amount);
        PlacedTiles tilingUnit = unit().getIncluded();
        for (const auto & pfp : std::as_const(tilingUnit))
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
    if (Sys::tilingMaker->isTMKbdMode(TM_MODE_XFORM_TILING))
    {
        qreal qdelta = 0.01 * amount;
        PlacedTiles tilingUnit = unit().getIncluded();
        for (const auto & pfp : std::as_const(tilingUnit))
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

    if (Sys::tilingMaker->isTMKbdMode(TM_MODE_XFORM_TILING))
    {
        qreal qdelta = 0.01 * amount;
        PlacedTiles tilingUnit = unit().getIncluded();
        for (const auto & pfp : std::as_const(tilingUnit))
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

    if (Sys::tilingMaker->isTMKbdMode(TM_MODE_XFORM_TILING))
    {
        qreal qdelta = 0.01 * amount;
        PlacedTiles tilingUnit = unit().getIncluded();
        for (const auto & pfp : std::as_const(tilingUnit))
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
}

bool SaveStatus::needsSaving()
{
    if (header != parent->hdr())
        return  true;

    TilingUnit tunit(parent->unit());
    tunit.removeExcludeds();

    if (unit != tunit)
        return true;

    return false;
}
