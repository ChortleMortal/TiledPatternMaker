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
#include "motifs/tile_motif.h"
#include "geometry/map.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "misc/defaults.h"
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
    db.init(this,QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT));
    name        = defaultName;
    version     = -1;
    state       = EMPTY;
    refs++;
}

Tiling::Tiling(QString name, QPointF t1, QPointF t2)
{
    db.init(this,QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT),t1,t2);

    if (!name.isEmpty())
    {
        this->name = name;
    }
    else
    {
        name    = defaultName;
    }

    version      = -1;
    state        = EMPTY;
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
        db = undoStack.pop();
        return true;
    }
    return false;
}


void Tiling::pushStack()
{
    state = MODIFIED;
    undoStack.push(db.copy());
}

bool Tiling::hasIntrinsicOverlaps()
{
    if (intrinsicOverlaps.get() == Tristate::Unknown)
    {
        auto placedTiles = db.getPlacedTiles();
        for (auto pf : qAsConst(placedTiles))
        {
            QPolygonF poly = pf->getPlacedPoints();

            for (auto pf2 : qAsConst(placedTiles))
            {
                if (pf2 == pf) continue;

                QPolygonF poly2 = pf2->getPlacedPoints();
                if (poly2.intersects(poly))
                {
                    QPolygonF p3 = poly2.intersected(poly);
                    if (!p3.isEmpty())
                    {
                        //qDebug() << "overlapping";
                        intrinsicOverlaps.set(true);
                    }
                }
            }
        }
        intrinsicOverlaps.set(false);
    }
    return (intrinsicOverlaps.get() == Tristate::True);
}

bool Tiling::hasTiledOverlaps()
{
    if (tiledOveraps.get() == Tristate::Unknown)
    {
        MapPtr map = createMapFull();
        tiledOveraps.set(map->hasIntersectingEdges());
    }
    return (tiledOveraps.get() == Tristate::True);
}

MapPtr Tiling::createMapSingle()
{
    MapPtr map = make_shared<Map>("tiling map single");
    for (auto placedTiles : db.getPlacedTiles())
    {
        EdgePoly poly = placedTiles->getPlacedEdgePoly();
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

    for (TilePtr tile : qAsConst(uniqueTiles))
    {
        auto motif = make_shared<TileMotif>();
        motif->setN(tile->numSides());
        motif->setup(tile);
        motif->buildMotifMaps();
        DesignElementPtr  dep = make_shared<DesignElement>(tile, motif);
        proto->addElement(dep);
    }

    // Now, for each different tile, build a submap corresponding
    // to all translations of that tile.

    Placements fillPlacements = db.getFillPlacemenets();

    MapPtr testmap = make_shared<Map>("testmap");

    for (auto & dep : qAsConst(proto->getDesignElements()))
    {
        TilePtr tile    = dep->getTile();
        MotifPtr motif  = dep->getMotif();
        MapPtr motifmap = motif->getMotifMap();

        Placements tilePlacements;
        for (auto placedTiles : db.getPlacedTiles())
        {
            TilePtr f  = placedTiles->getTile();
            if (f == tile)
            {
                QTransform t = placedTiles->getTransform();
                tilePlacements.push_back(t);
            }
        }

        // Within a single translational unit, assemble the different
        // transformed figures corresponding to the given tile into a map.
        MapPtr transmap = make_shared<Map>("tile transmap");
        transmap->mergeMany(motifmap, tilePlacements);

        // Now put all the translations together into a single map for this tile.
        MapPtr tilemap = make_shared<Map>("single tile map");
        tilemap->mergeMany(transmap, fillPlacements);

        // And do a slow merge to add this map to the finished design.
        testmap->mergeMap(tilemap);
    }
    return testmap;
}

// tile management.
// Added tiles are embedded into a placedTile.

void Tiling::add(const PlacedTilePtr pf )
{
    auto & placed = getDataAccess(true).getPlacedTileAccess();
    placed.push_back(pf);
}

void Tiling::add(TilePtr f, QTransform  T)
{
    add(make_shared<PlacedTile>(f, T));
}

void Tiling::remove(PlacedTilePtr pf)
{
    auto & placed = getDataAccess(true).getPlacedTileAccess();
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

    for (auto placedTiles : qAsConst(getData().getPlacedTiles()))
    {
        TilePtr tile = placedTiles->getTile();
        tiles.push_back(tile);
    }

    return static_cast<QVector<TilePtr>>(tiles);
}

int Tiling::numPlacements(TilePtr tile)
{
    int count = 0;
    for (auto placedTiles : qAsConst(getData().getPlacedTiles()))
    {
        TilePtr fp2 = placedTiles->getTile();
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
    for (auto placedTile : qAsConst(getData().getPlacedTiles()))
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
    const PlacedTiles & placedTiles = db.getPlacedTiles();
    for (auto placedTile : placedTiles)
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
    for(auto placedTile : qAsConst(getData().getPlacedTiles()))
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
