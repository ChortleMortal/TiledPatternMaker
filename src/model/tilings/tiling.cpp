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

#include <QtWidgets>
#include "model/tilings/tiling.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/fill_region.h"
#include "sys/geometry/map.h"
#include "sys/geometry/transform.h"
#include "sys/geometry/vertex.h"
#include "gui/viewers/gui_modes.h"
#include "model/prototypes/prototype.h"
#include "model/makers/tiling_maker.h"
#include "gui/viewers/geo_graphics.h"
#include "sys/sys.h"
#include "model/prototypes/design_element.h"
#include "model/mosaics/mosaic.h"
#include "model/mosaics/mosaic_writer.h"
#include "model/motifs/tile_motif.h"
#include "model/settings/configuration.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "gui/top/view_controller.h"

using std::make_shared;

int Tiling::refs = 0;

Tiling::Tiling() : LayerController("Tiling",true)
{
    name.set(Sys::defaultTilingName);
    version     = -1;
    refs++;
}

Tiling::~Tiling()
{
#ifdef EXPLICIT_DESTRUCTOR
#endif
    refs--;
}

bool Tiling::isEmpty()
{
    if (name.get() == Sys::defaultTilingName && db.isEmpty())
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
    db      = tp->db.copy();
    bip     = tp->bip;
}

bool Tiling::hasIntrinsicOverlaps()
{
    if (intrinsicOverlaps.get() == Tristate::Unknown)
    {
        intrinsicOverlaps.set(Tristate::False);
        TilingPlacements tilingUnit = getTilingUnitPlacements(config->tm_showExcludes);
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
    TilingPlacements tilingUnit = getTilingUnitPlacements();
    for (const auto & placedTile : std::as_const(tilingUnit))
    {
        EdgePoly poly = placedTile->getPlacedEdgePoly();
        MapPtr emap = make_shared<Map>("tile",poly);
        map->mergeMap(emap);
    }

    map->deDuplicateEdgesUsingNeighbours();      // conceals an unknonwn problem

    return map;
}

MapPtr Tiling::createMapFullSimple()
{
    MapPtr map = make_shared<Map>("tiling map full simple");

    Placements fillPlacements = db.getFillPlacements();

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
    FillData fd = getCanvasSettings().getFillData();
    mosaic->getCanvasSettings().setFillData(fd);

    auto proto = make_shared<Prototype>(shared_from_this(),mosaic);

    QVector<TilePtr> uniqueTiles = getUniqueTiles();

    for (const auto & tile : std::as_const(uniqueTiles))
    {
        auto motif = make_shared<TileMotif>();
        motif->setN(tile->numSides());
        motif->setTile(tile);
        motif->buildMotifMap();
        DesignElementPtr  dep = make_shared<DesignElement>(tile, motif);
        proto->addDesignElement(dep);
    }

    // Now, for each different tile, build a submap corresponding
    // to all translations of that tile.

    Placements fillPlacements = db.getFillPlacements();

    MapPtr testmap = make_shared<Map>("testmap");
    for (const auto & dep : proto->getDesignElements())
    {
        TilePtr tile    = dep->getTile();
        MotifPtr motif  = dep->getMotif();
        MapPtr motifmap = motif->getMotifMap();

        TilingPlacements tilingUnit = getTilingUnitPlacements();
        Placements tilePlacements;
        for (const auto & placedTile : std::as_const(tilingUnit))
        {
            TilePtr f  = placedTile->getTile();
            if (f == tile)
            {
                QTransform t = placedTile->getTransform();
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

    Placements placements = db.getFillPlacements();
    TilingPlacements tilingUnit = getTilingUnitPlacements();

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
    FillData fd = getCanvasSettings().getFillData();
    mosaic->getCanvasSettings().setFillData(fd);

    Prototype proto(shared_from_this(),mosaic);

    for (const auto & tile : getUniqueTiles())
    {
        auto motif = make_shared<TileMotif>();
        motif->setN(tile->numSides());
        motif->setTile(tile);
        motif->buildMotifMap();
        DesignElementPtr  dep = make_shared<DesignElement>(tile, motif);
        proto.addDesignElement(dep);
    }

    // Now, for each different feature, build a submap corresponding
    // to all translations of that feature.

    Placements fillPlacements = db.getFillPlacements();

    MapPtr testmap = make_shared<Map>("testmap");

    for (const auto & del : std::as_const(proto.getDesignElements()))
    {
        TilePtr tile         = del->getTile();
        MotifPtr motif       = del->getMotif();
        constMapPtr motifmap = motif->getMotifMap();

        TilingPlacements tilingUnit = getTilingUnitPlacements();
        Placements tilePlacements;
        for (const auto & placedTile : std::as_const(tilingUnit))
        {
            TilePtr f  = placedTile->getTile();
            if (f == tile)
            {
                QTransform t = placedTile->getTransform();
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


// tile management.
// Added tiles are embedded into a placedTile.

void Tiling::addPlacedTile(const PlacedTilePtr pfm)
{
    db.addPlacedTile(pfm);
}

void Tiling::replaceTilingUnit(TilingUnit & tilingUnit)
{
    db.getTilingUnit2().set(tilingUnit);
}

void Tiling::removeExcludeds()
{
    const TilingPlacements excludes = db.getExcluded();
    for (const auto & ptp : std::as_const(excludes))
    {
        removePlacedTile(ptp);
    }
}

void Tiling::removePlacedTile(PlacedTilePtr pf)
{
    db.removePlacedTile(pf);
}

void Tiling::clearPlacedTiles()
{
    db.clear();
}

int Tiling::getVersion()
{
    return version;
}

void Tiling::setVersion(int ver)
{
    version = ver;
}

QVector<TilePtr> Tiling::getUniqueTiles()
{
    QVector<TilePtr> tiles;

    const TilingUnit & tilingUnit = db.getTilingUnit2();
    for (auto & pair : tilingUnit)
    {
        tiles.push_back(pair.first);
    }

    return tiles;
}

int Tiling::numPlacements(TilePtr tile)
{
    const TilingUnit & tilingUnit = db.getTilingUnit2();
    for (auto & pair : tilingUnit)
    {
        if (pair.first == tile)
        {
            return pair.second.count();
        }
    }
    return 0;
}

Placements Tiling::getPlacements(TilePtr tile)
{
    Placements placements;

    const TilingUnit & tilingUnit = db.getTilingUnit2();
    for (auto & pair : tilingUnit)
    {
        if (pair.first == tile)
        {
            const TilingPlacements & tps = pair.second;
            for (auto & placed : tps)
            {
                placements.push_back(placed->getTransform());
            }
        }
    }
    return placements;
}

QTransform Tiling::getFirstPlacement(TilePtr tile)
{
    QTransform placement;
    TilingPlacements tilingUnit = getTilingUnitPlacements();
    for (const auto & placedTile : std::as_const(tilingUnit))
    {
        TilePtr atile = placedTile->getTile();
        if (atile == tile)
        {
            placement = placedTile->getTransform();
            return placement;
        }
    }
    // none found
    return placement;
}

QString Tiling::info() const
{
    QString astring;
    QDebug  deb(&astring);

    deb.noquote() << "tiling=" << name.get()<< db.info();
    return astring;
}

//////////////////////////////////////////////////////////////////////////////////
///
/// Viewable
///
//////////////////////////////////////////////////////////////////////////////////

TilingPlacements Tiling::getTilingUnitPlacements(bool all) const
{
    if (all)
        return db.getAll();
    else
        return db.getIncluded();
}

// called on demand by paint
void Tiling::creaateViewablePlacedTiles()
{
    if (!Sys::tm_fill)
    {
         _viewable = getTilingUnitPlacements(config->tm_showExcludes);
        return;
    }
    else
    {
        TilingPlacements placements  = getTilingUnitPlacements();  // does not show excluded
        _viewable = placements;

        QPointF t1    = db.getTrans1();
        QPointF t2    = db.getTrans2();
        if (t1.isNull() || t2.isNull())
        {
            return;
        }

        int minX, maxX, minY, maxY;
        bool singleton;
        const FillData fd = getCanvasSettings().getFillData();
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
            QTransform T = placed->getTransform();

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
                    _viewable.push_back(make_shared<PlacedTile>(f, placement));
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////
//
//  Viewer functions
//
////////////////////////////////////////////////////////////////////////

void Tiling::paint(QPainter *painter)
{
    qDebug() << "Tiling::paint" << name.get();

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QTransform tr = getLayerTransform();
    GeoGraphics gg(painter,tr);
    draw(&gg);

    drawLayerModelCenter(painter);
}

void Tiling::draw(GeoGraphics *gg)
{
    FillRegion flood(this,getCanvasSettings().getFillData());
    Placements placements = flood.getPlacements(config->repeatMode);

    TilingPlacements tilingUnit = getTilingUnitPlacements();
    for (auto T : placements)
    {
        gg->pushAndCompose(T);

        for (const auto & tile : std::as_const(tilingUnit))
        {
            drawPlacedTile(gg, tile);
        }

        gg->pop();
    }
}

void Tiling::drawPlacedTile(GeoGraphics * g2d, PlacedTilePtr pf)
{
    //qDebug().noquote() << "PlacedFeat:" << pf->getTile().get() <<  "transform:" << Transform::toInfoString(t);

    QPen pen(Qt::red,3);

    TilePtr f  = pf->getTile();
    EdgePoly ep   = pf->getPlacedEdgePoly();

    for (auto & edge : std::as_const(ep))
    {
        if (edge->getType() == EDGETYPE_LINE)
        {
            g2d->drawLine(edge->getLine(),pen);
        }
        else if (edge->getType() == EDGETYPE_CURVE)
        {
            g2d->drawArc(edge->v1->pt,edge->v2->pt,edge->getArcCenter(),edge->isConvex(),pen);
        }
        else if (edge->getType() == EDGETYPE_CHORD)
        {
            g2d->drawChord(edge->v1->pt,edge->v2->pt,edge->getArcCenter(),edge->isConvex(),pen);
        }
    }
}

void Tiling::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{ Q_UNUSED(spt); Q_UNUSED(btn); }

void Tiling::slot_mouseDragged(QPointF spt)
{ Q_UNUSED(spt); }

void Tiling::slot_mouseTranslate(QPointF spt)
{
    if (!Sys::view->isActiveLayer(VIEW_TILING)) return;

    if (Sys::guiModes->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        QTransform T          = getCanvasTransform();
        qreal scale           = Transform::scalex(T);
        QPointF mpt           = spt/scale;
        QTransform tt         = QTransform::fromTranslate(mpt.x(),mpt.y());
        TilingPlacements tilingUnit = getTilingUnitPlacements();
        for (const auto & pfp :  std::as_const(tilingUnit))
        {
            QTransform t = pfp->getTransform();
            t *= tt;
            pfp->setTransform(t);
        }
        forceRedraw();
    }
    else if (Sys::guiModes->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getModelXform();
        xf.setTranslateX(xf.getTranslateX() + spt.x());
        xf.setTranslateY(xf.getTranslateY() + spt.y());
        setModelXform(xf,true);
    }
    else if (Sys::guiModes->getKbdMode(KBD_MODE_XFORM_SELECTED))
    {
        if (isSelected())
        {
            Xform xf = getModelXform();
            xf.setTranslateX(xf.getTranslateX() + spt.x());
            xf.setTranslateY(xf.getTranslateY() + spt.y());
            setModelXform(xf,true);
        }
    }
}

void Tiling::slot_mouseMoved(QPointF spt)
{ Q_UNUSED(spt); }
void Tiling::slot_mouseReleased(QPointF spt)
{ Q_UNUSED(spt); }
void Tiling::slot_mouseDoublePressed(QPointF spt)
{ Q_UNUSED(spt); }

void Tiling::slot_wheel_scale(qreal delta)
{
    if (!Sys::view->isActiveLayer(VIEW_TILING)) return;

    if (Sys::guiModes->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        qreal sc = 1.0 + delta;
        QTransform ts;
        ts.scale(sc,sc);

        TilingPlacements tilingUnit = getTilingUnitPlacements();
        for (const auto & pfp : std::as_const(tilingUnit))
        {
            QTransform t = pfp->getTransform();
            t *= ts;
            pfp->setTransform(t);
        }
        forceRedraw();
    }
    else if (Sys::guiModes->getKbdMode(KBD_MODE_XFORM_VIEW) || (Sys::guiModes->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getModelXform();
        xf.setScale(xf.getScale() * (1.0 + delta));
        setModelXform(xf,true);
    }
}

void Tiling::slot_wheel_rotate(qreal delta)
{
    if (!Sys::view->isActiveLayer(VIEW_TILING)) return;

    if (Sys::guiModes->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        QTransform tr;
        tr.rotate(delta);
        Xform xf(tr);
        xf.setModelCenter(getCenterModelUnits());
        QTransform tr2 = xf.toQTransform(QTransform());

        TilingPlacements tilingUnit = getTilingUnitPlacements();
        for (const auto & pfp : std::as_const(tilingUnit))
        {
            QTransform t = pfp->getTransform();
            t *= tr2;
            pfp->setTransform(t);
        }
        forceRedraw();
    }
    else if (Sys::guiModes->getKbdMode(KBD_MODE_XFORM_VIEW) || (Sys::guiModes->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getModelXform();
        xf.setRotateDegrees(xf.getRotateDegrees() + delta);
        setModelXform(xf,true);
    }
}

void Tiling::slot_scale(int amount)
{
    if (!Sys::view->isActiveLayer(VIEW_TILING)) return;

    if (Sys::guiModes->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        qreal scale = 1.0 + (0.01 * amount);
        TilingPlacements tilingUnit = getTilingUnitPlacements();
        for (const auto & pfp : std::as_const(tilingUnit))
        {
            QTransform t = pfp->getTransform();
            qDebug() << "t0" << Transform::info(t);
            QTransform t1 = t.scale(scale,scale);

            t = pfp->getTransform();
            QTransform t2 = t *QTransform::fromScale(scale,scale);

            qDebug() << "t1" << Transform::info(t1);
            qDebug() << "t2" << Transform::info(t2);

            t = pfp->getTransform();
            // scales position too
            t *= QTransform::fromScale(scale,scale);
            pfp->setTransform(t);
        }
        forceRedraw();
    }
    else if (Sys::guiModes->getKbdMode(KBD_MODE_XFORM_VIEW) || (Sys::guiModes->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getModelXform();
        xf.setScale(xf.getScale() * (1 + static_cast<qreal>(amount)/100.0));
        setModelXform(xf,true);
    }
}

void Tiling::slot_rotate(int amount)
{
    if (!Sys::view->isActiveLayer(VIEW_TILING)) return;

    qDebug() << "TilingMaker::slot_rotate" << amount;
    if (Sys::guiModes->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        qreal qdelta = 0.01 * amount;
        TilingPlacements tilingUnit = getTilingUnitPlacements();
        for (const auto & pfp : std::as_const(tilingUnit))
        {
            QTransform t = pfp->getTransform();
            t *= QTransform().rotateRadians(qdelta);
            pfp->setTransform(t);
        }
        forceRedraw();
    }
    else if (Sys::guiModes->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getModelXform();
        xf.setRotateRadians(xf.getRotateRadians() + qDegreesToRadians(static_cast<qreal>(amount)));
        setModelXform(xf,true);
    }
    else if (Sys::guiModes->getKbdMode(KBD_MODE_XFORM_SELECTED))
    {
        if (isSelected())
        {
            Xform xf = getModelXform();
            xf.setRotateRadians(xf.getRotateRadians() + qDegreesToRadians(static_cast<qreal>(amount)));
            setModelXform(xf,true);
        }
    }
}

void Tiling:: slot_moveX(qreal amount)
{
    if (!Sys::view->isActiveLayer(VIEW_TILING)) return;

    if (Sys::guiModes->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        qreal qdelta = 0.01 * amount;
        TilingPlacements tilingUnit = getTilingUnitPlacements();
        for (const auto & pfp : std::as_const(tilingUnit))
        {
            QTransform t = pfp->getTransform();
            t *= QTransform::fromTranslate(qdelta,0.0);
            pfp->setTransform(t);
        }
        forceRedraw();
    }
    else if (Sys::guiModes->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getModelXform();
        xf.setTranslateX(xf.getTranslateX() + amount);
        setModelXform(xf,true);
    }
    else if (Sys::guiModes->getKbdMode(KBD_MODE_XFORM_SELECTED))
    {
        if (isSelected())
        {
            Xform xf = getModelXform();
            xf.setTranslateX(xf.getTranslateX() + amount);
            setModelXform(xf,true);
        }
    }
}

void Tiling::slot_moveY(qreal amount)
{
    if (!Sys::view->isActiveLayer(VIEW_TILING)) return;

    if (Sys::guiModes->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        qreal qdelta = 0.01 * amount;
        TilingPlacements tilingUnit = getTilingUnitPlacements();
        for (const auto & pfp : std::as_const(tilingUnit))
        {
            QTransform t = pfp->getTransform();
            t *= QTransform::fromTranslate(0.0,qdelta);
            pfp->setTransform(t);
        }
        forceRedraw();
    }
    else if (Sys::guiModes->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getModelXform();
        xf.setTranslateY(xf.getTranslateY() + amount);
        setModelXform(xf,true);
    }
    else if (Sys::guiModes->getKbdMode(KBD_MODE_XFORM_SELECTED))
    {
        if (isSelected())
        {
            Xform xf = getModelXform();
            xf.setTranslateY(xf.getTranslateY() + amount);
            setModelXform(xf,true);
        }
    }
}

void Tiling::setModelXform(const Xform & xf, bool update)
{
    Q_ASSERT(_unique);
    if (debug & DEBUG_XFORM) qInfo().noquote() << "SET" << getLayerName() << xf.info() << (isUnique() ? "unique" : "common");
    xf_model = xf;
    forceLayerRecalc(update);
}

const Xform & Tiling::getModelXform()
{
    Q_ASSERT(_unique);
    if (debug & DEBUG_XFORM) qInfo().noquote() << "GET" << getLayerName() << xf_model.info() << (isUnique() ? "unique" : "common");
    return xf_model;
}


