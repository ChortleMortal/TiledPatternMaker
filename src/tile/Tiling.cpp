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
#include "tile/tiling.h"
#include "geometry/edge.h"
#include "geometry/fill_region.h"
#include "geometry/map.h"
#include "geometry/transform.h"
#include "geometry/vertex.h"
#include "makers/prototype_maker/prototype.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "makers/tiling_maker/tiling_monitor.h"
#include "misc/geo_graphics.h"
#include "mosaic/design_element.h"
#include "mosaic/mosaic_writer.h"
#include "motifs/tile_motif.h"
#include "settings/configuration.h"
#include "tile/placed_tile.h"
#include "tile/tile.h"
#include "tile/tiling.h"
#include "viewers/view_controller.h"

using std::make_shared;

const QString Tiling::defaultName = "The Unnamed";

int Tiling::refs = 0;

Tiling::Tiling() : LayerController("Tiling",true)
{
    title       = defaultName;
    version     = -1;
    refs++;

    connect(&db,  &TilingData::sig_tilingChanged, Sys::tilingMaker->getTilingMonitor(), &TilingMonitor::slot_tilingChanged);
}

Tiling::~Tiling()
{
#ifdef EXPLICIT_DESTRUCTOR
    placed_tiloes.clear();
#endif
    refs--;
}

bool Tiling::isEmpty()
{
    if (title == "The Unnamed" && db.isEmpty())
        return true;
    else
        return false;
}

bool Tiling::hasIntrinsicOverlaps()
{
    if (intrinsicOverlaps.get() == Tristate::Unknown)
    {
        intrinsicOverlaps.set(Tristate::False);
        for (const auto & tile1 : std::as_const(getInTiling()))
        {
            QPolygonF poly = tile1->getPlacedPoints();

            for (const auto & tile2 : std::as_const(getInTiling()))
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
    for (const auto & placedTile : std::as_const(getInTiling()))
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
    auto proto = make_shared<Prototype>(shared_from_this());

    QVector<TilePtr> uniqueTiles = getUniqueTiles();

    for (const auto & tile : std::as_const(uniqueTiles))
    {
        auto motif = make_shared<TileMotif>();
        motif->setN(tile->numSides());
        motif->setTile(tile);
        motif->buildMotifMaps();
        DesignElementPtr  dep = make_shared<DesignElement>(tile, motif);
        proto->addElement(dep);
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

        Placements tilePlacements;
        for (const auto & placedTile : std::as_const(getInTiling()))
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

    MapPtr tilingMap = make_shared<Map>("tiling map");
    for (const auto & pfp : std::as_const(getInTiling()))
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
    // Thios builds a prototype using  Tile Motif and generates its map
    Prototype proto(shared_from_this());

    for (const auto & tile : getUniqueTiles())
    {
        auto motif = make_shared<TileMotif>();
        motif->setN(tile->numSides());
        motif->setTile(tile);
        motif->buildMotifMaps();
        DesignElementPtr  dep = make_shared<DesignElement>(tile, motif);
        proto.addElement(dep);
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

        Placements tilePlacements;
        for (const auto & placedTile : std::as_const(getInTiling()))
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

void Tiling::add(const PlacedTilePtr pfm)
{
    db.add(pfm);
}

void Tiling::replace(PlacedTiles & tiles)
{
    db.clear();
    db.add(tiles);
}

void Tiling::remove(PlacedTilePtr pf)
{
    db.remove(pf);
}

void Tiling::clear()
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
    UniqueQVector<TilePtr> tiles;

    for (const auto & placedTile : std::as_const(getInTiling()))
    {
        TilePtr tile = placedTile->getTile();
        tiles.push_back(tile);
    }

    return static_cast<QVector<TilePtr>>(tiles);
}

int Tiling::numPlacements(TilePtr tile)
{
    int count = 0;
    for (const auto & placedTile : std::as_const(getInTiling()))
    {
        TilePtr fp2 = placedTile->getTile();
        if (fp2 == tile)
        {
            count++;
        }
    }
    return count;
}

Placements Tiling::getPlacements(TilePtr tile)
{
    Placements placements;
    for (const auto & placedTile : std::as_const(getInTiling()))
    {
        if (placedTile->getTile() == tile)
        {
            placements.push_back(placedTile->getTransform());
        }
    }
    return placements;
}

QTransform Tiling::getFirstPlacement(TilePtr tile)
{
    QTransform placement;
    for (const auto & placedTile : std::as_const(getInTiling()))
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

// Regroup tiles by their translation so that we write each tile only once.
TileGroup Tiling::regroupTiles()
{
    TileGroup tileGroup;
    for (const auto & placedTile : std::as_const(getInTiling()))
    {
        TilePtr tile = placedTile->getTile();
        if (tileGroup.containsTile(tile))
        {
            PlacedTiles  & v = tileGroup.getTilePlacements(tile);
            v.push_back(placedTile);
        }
        else
        {
            PlacedTiles v;
            v.push_back(placedTile);
            tileGroup.push_back(qMakePair(tile,v));
        }
    }
    return tileGroup;
}

QString Tiling::info() const
{
    QString astring;
    QDebug  deb(&astring);

    deb << "tiling=" << title  << db.info();
    return astring;
}

///////////////////////////////////////////////////////////////////////
//
//  Viewer functions
//
////////////////////////////////////////////////////////////////////////

void Tiling::paint(QPainter *painter)
{
    //qDebug() << "Tiling::paint";

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

    for (auto T : placements)
    {
        gg->pushAndCompose(T);

        for (const auto & tile : std::as_const(getInTiling()))
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
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        QTransform T = getCanvasTransform();
        qreal scale = Transform::scalex(T);
        QPointF mpt = spt/scale;
        QTransform tt = QTransform::fromTranslate(mpt.x(),mpt.y());
        for (const auto & pfp :  std::as_const(getInTiling()))
        {
            QTransform t = pfp->getTransform();
            t *= tt;
            pfp->setTransform(t);
        }
        forceRedraw();
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getModelXform();
        xf.setTranslateX(xf.getTranslateX() + spt.x());
        xf.setTranslateY(xf.getTranslateY() + spt.y());
        setModelXform(xf,true);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_SELECTED))
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
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        qreal sc = 1.0 + delta;
        QTransform ts;
        ts.scale(sc,sc);

        for (const auto & pfp : std::as_const(getInTiling()))
        {
            QTransform t = pfp->getTransform();
            t *= ts;
            pfp->setTransform(t);
        }
        forceRedraw();
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_VIEW) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getModelXform();
        xf.setScale(xf.getScale() * (1.0 + delta));
        setModelXform(xf,true);
    }
}

void Tiling::slot_wheel_rotate(qreal delta)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        QTransform tr;
        tr.rotate(delta);
        Xform xf(tr);
        xf.setModelCenter(getCenterModelUnits());
        QTransform tr2 = xf.toQTransform(QTransform());

        for (const auto & pfp : std::as_const(getInTiling()))
        {
            QTransform t = pfp->getTransform();
            t *= tr2;
            pfp->setTransform(t);
        }
        forceRedraw();
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_VIEW) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getModelXform();
        xf.setRotateDegrees(xf.getRotateDegrees() + delta);
        setModelXform(xf,true);
    }
}

void Tiling::slot_scale(int amount)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        qreal scale = 1.0 + (0.01 * amount);
        for (const auto & pfp : std::as_const(getInTiling()))
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
    else if (view->getKbdMode(KBD_MODE_XFORM_VIEW) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getModelXform();
        xf.setScale(xf.getScale() * (1 + static_cast<qreal>(amount)/100.0));
        setModelXform(xf,true);
    }
}

void Tiling::slot_rotate(int amount)
{
    if (!view->isActiveLayer(this)) return;

    qDebug() << "TilingMaker::slot_rotate" << amount;
    if (view->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        qreal qdelta = 0.01 * amount;
        for (const auto & pfp : std::as_const(getInTiling()))
        {
            QTransform t = pfp->getTransform();
            t *= QTransform().rotateRadians(qdelta);
            pfp->setTransform(t);
        }
        forceRedraw();
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getModelXform();
        xf.setRotateRadians(xf.getRotateRadians() + qDegreesToRadians(static_cast<qreal>(amount)));
        setModelXform(xf,true);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_SELECTED))
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
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        qreal qdelta = 0.01 * amount;
        for (const auto & pfp : std::as_const(getInTiling()))
        {
            QTransform t = pfp->getTransform();
            t *= QTransform::fromTranslate(qdelta,0.0);
            pfp->setTransform(t);
        }
        forceRedraw();
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getModelXform();
        xf.setTranslateX(xf.getTranslateX() + amount);
        setModelXform(xf,true);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_SELECTED))
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
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        qreal qdelta = 0.01 * amount;
        for (const auto & pfp : std::as_const(getInTiling()))
        {
            QTransform t = pfp->getTransform();
            t *= QTransform::fromTranslate(0.0,qdelta);
            pfp->setTransform(t);
        }
        forceRedraw();
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getModelXform();
        xf.setTranslateY(xf.getTranslateY() + amount);
        setModelXform(xf,true);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_SELECTED))
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


///
/// class Feature Group
///

bool TileGroup::containsTile(TilePtr fp)
{
    for (const auto & apair : std::as_const(*this))
    {
        TilePtr f = apair.first;
        if (f == fp)
        {
            return true;
        }
    }
    return false;
}

PlacedTiles & TileGroup::getTilePlacements(TilePtr fp)
{
    Q_ASSERT(containsTile(fp));

    for (auto & apair : *this)
    {
        TilePtr f = apair.first;
        if (f == fp)
        {
            return apair.second;
        }
    }

    qWarning("should never reach here");
    static QVector <PlacedTilePtr> v;
    return v;
}
