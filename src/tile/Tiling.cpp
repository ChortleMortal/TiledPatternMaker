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
#include "makers/map_editor/map_editor.h"
#include "tile/tiling.h"
#include "motifs/tile_motif.h"
#include "geometry/map.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "mosaic/design_element.h"
#include "mosaic/mosaic_writer.h"
#include "makers/prototype_maker/prototype.h"
#include "tile/tile.h"
#include "tile/placed_tile.h"

using std::make_shared;

const QString Tiling::defaultName = "The Unnamed";

int Tiling::refs = 0;

Tiling::Tiling()
{
    name        = defaultName;
    version     = -1;
    state       = EMPTY;
    refs++;
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
    if (name == "The Unnamed" && db.isEmpty())
        return true;
    else
        return false;
}

bool Tiling::popStack()
{
    if (!undoStack.isEmpty())
    {
        resetOverlaps();
        db = undoStack.pop();
        return true;
    }
    return false;
}


void Tiling::pushStack()
{
    state = MODIFIED;
    resetOverlaps();
    undoStack.push(db.copy());
}

bool Tiling::hasIntrinsicOverlaps()
{
    if (intrinsicOverlaps.get() == Tristate::Unknown)
    {
        intrinsicOverlaps.set(Tristate::False);
        for (const auto & pf : getInTiling())
        {
            QPolygonF poly = pf->getPlacedPoints();

            for (const auto & pf2 : getInTiling())
            {
                if (pf2 == pf) continue;

                QPolygonF poly2 = pf2->getPlacedPoints();
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
    for (const auto & placedTile : getInTiling())
    {
        EdgePoly poly = placedTile->getPlacedEdgePoly();
        MapPtr emap = make_shared<Map>("tile",poly);
        map->mergeMap(emap);
    }

    return map;
}

MapPtr Tiling::createMapFullSimple()
{
    MapPtr map = make_shared<Map>("tiling map full simple");

    Placements fillPlacements = db.getFillPlacemenets();

    MapPtr tilingMap = createMapSingle();

    for (auto placement : fillPlacements)
    {
        MapPtr m  = tilingMap->recreate();
        m->transformMap(placement);
        map->mergeMap(m);
    }
    return map;
}

MapPtr Tiling::createMapFull()
{
    // This builds a prototype using explicit tile figures and generates its map
    auto proto = make_shared<Prototype>(shared_from_this());

    QVector<TilePtr> uniqueTiles = getUniqueTiles();

    for (const auto & tile : uniqueTiles)
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

    Placements fillPlacements = db.getFillPlacemenets();

    MapPtr testmap = make_shared<Map>("testmap");
    for (const auto & dep : proto->getDesignElements())
    {
        TilePtr tile    = dep->getTile();
        MotifPtr motif  = dep->getMotif();
        MapPtr motifmap = motif->getMotifMap();

        Placements tilePlacements;
        for (const auto & placedTile : getInTiling())
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
        for (const auto & T : fillPlacements)
        {
            for (const auto & edge : transmap->getEdges())
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

    Placements placements = db.getFillPlacemenets();

    MapPtr tilingMap = make_shared<Map>("tiling map");
    for (const auto & pfp : getInTiling())
    {
        EdgePoly poly = pfp->getPlacedEdgePoly();
        MapPtr emap = make_shared<Map>("tile",poly);
        map->mergeMap(emap);
    }

    for (const auto & transform : placements)
    {
        MapPtr m  = tilingMap->recreate();
        m->transformMap(transform);
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

    Placements fillPlacements = db.getFillPlacemenets();

    MapPtr testmap = make_shared<Map>("testmap");

    for (const auto & del : proto.getDesignElements())
    {
        TilePtr tile         = del->getTile();
        MotifPtr motif       = del->getMotif();
        constMapPtr motifmap = motif->getMotifMap();

        Placements tilePlacements;
        for (const auto & placedTile : getInTiling())
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

void Tiling::add(const PlacedTilePtr pf )
{
    auto & placed = getRWInTiling(true);
    placed.push_back(pf);
}

void Tiling::add(TilePtr f, QTransform  T)
{
    add(make_shared<PlacedTile>(f, T));
}

void Tiling::remove(PlacedTilePtr pf)
{
    auto & placed = getRWInTiling(true);
    placed.removeOne(pf);
}

int Tiling::getVersion()
{
    return version;
}

void Tiling::setVersion(int ver)
{
    version = ver;
}

Tiling::eTilingState Tiling::getState()
{
    return state;
}

void Tiling::setState(eTilingState state)
{
    this->state = state;
}

QVector<TilePtr> Tiling::getUniqueTiles()
{
    UniqueQVector<TilePtr> tiles;

    for (const auto & placedTile : getInTiling())
    {
        TilePtr tile = placedTile->getTile();
        tiles.push_back(tile);
    }

    return static_cast<QVector<TilePtr>>(tiles);
}

int Tiling::numPlacements(TilePtr tile)
{
    int count = 0;
    for (const auto & placedTile : getInTiling())
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
    for (const auto & placedTile : getInTiling())
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
    for (const auto & placedTile : getInTiling())
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
    for (const auto & placedTile : getInTiling())
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

QString Tiling::dump() const
{
    QString astring;
    QDebug  deb(&astring);

    deb << "tiling=" << name  << db.dump();
    return astring;
}


///
/// class Feature Group
///

bool TileGroup::containsTile(TilePtr fp)
{
    for (auto& apair : *this)
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

    for (auto& apair : *this)
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
