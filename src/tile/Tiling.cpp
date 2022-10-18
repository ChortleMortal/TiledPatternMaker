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
#include "motifs/explicit_motif.h"
#include "geometry/map.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "misc/defaults.h"
#include "mosaic/design_element.h"
#include "mosaic/mosaic_writer.h"
#include "mosaic/prototype.h"
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
    undoStack.push(db.copy());
}

bool Tiling::hasIntrinsicOverlaps()
{
    if (intrinsicOverlaps.get() == Tristate::Unknown)
    {
        auto placed = db.getPlacedTiles();
        for (auto pf : qAsConst(placed))
        {
            QPolygonF poly = pf->getPlacedPolygon();

            for (auto pf2 : qAsConst(placed))
            {
                if (pf2 == pf) continue;

                QPolygonF poly2 = pf2->getPlacedPolygon();
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
    for (auto pfp : db.getPlacedTiles())
    {
        EdgePoly poly = pfp->getPlacedEdgePoly();
        MapPtr emap = make_shared<Map>("tile",poly);
        map->mergeMap(emap);
    }

    return map;
}

MapPtr Tiling::createMapFullSimple()
{
    MapPtr map = make_shared<Map>("tiling map full simple");

    QVector<QTransform> translations = db.getFillTranslations();

    MapPtr tilingMap = createMapSingle();

    for (auto transform : translations)
    {
        MapPtr m  = tilingMap->recreate();
        m->transformMap(transform);
        map->mergeMap(m);
    }
    return map;
}

MapPtr Tiling::createMapFull()
{
    // This builds a prototype using explicit tile figures and generates its map
    Prototype proto(shared_from_this());

    QVector<TilePtr> uniqueTiles = getUniqueTiles();

    for (auto  & tile : qAsConst(uniqueTiles))
    {
        EdgePoly & ep = tile->getEdgePoly();
        MapPtr     fm = make_shared<Map>("tile map",ep);
        MotifPtr fig = make_shared<ExplicitMotif>(fm,MOTIF_TYPE_EXPLICIT_TILE,tile->numSides());
        DesignElementPtr  dep = make_shared<DesignElement>(tile, fig);
        proto.addElement(dep);
    }

    // Now, for each different tile, build a submap corresponding
    // to all translations of that tile.

    QVector<QTransform> translations = db.getFillTranslations();

    MapPtr testmap = make_shared<Map>("testmap");

    for (auto & dep : qAsConst(proto.getDesignElements()))
    {
        TilePtr tile    = dep->getTile();
        MotifPtr motif  = dep->getMotif();
        MapPtr motifmap = motif->getMap();

        QVector<QTransform> subT;
        for (auto placed_tile : db.getPlacedTiles())
        {
            TilePtr f  = placed_tile->getTile();
            if (f == tile)
            {
                QTransform t = placed_tile->getTransform();
                subT.push_back(t);
            }
        }

        // Within a single translational unit, assemble the different
        // transformed figures corresponding to the given tile into a map.
        MapPtr transmap = make_shared<Map>("proto transmap");
        transmap->mergeMany(motifmap, subT);

        // Now put all the translations together into a single map for this tile.
        MapPtr tilemap = make_shared<Map>("proto tile map");
        tilemap->mergeMany(transmap, translations);

        // And do a slow merge to add this map to the finished design.
        testmap->mergeMap(tilemap);
    }
    return testmap;
}


// tile management.
// Added tile are embedded into a placedTile.

void Tiling::add(const PlacedTilePtr pf )
{
    auto & placed = getDataAccess().getPlacedTileAccess();
    placed.push_back(pf);
}

void Tiling::add(TilePtr f, QTransform  T)
{
    add(make_shared<PlacedTile>(this,f, T));
}

void Tiling::remove(PlacedTilePtr pf)
{
    auto & placed = getDataAccess().getPlacedTileAccess();
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
    UniqueQVector<TilePtr> fs;

    for (auto pfp : qAsConst(getData().getPlacedTiles()))
    {
        TilePtr fp = pfp->getTile();
        fs.push_back(fp);
    }

    return static_cast<QVector<TilePtr>>(fs);
}

int Tiling::numPlacements(TilePtr fp)
{
    int count = 0;
    for (auto pfp : qAsConst(getData().getPlacedTiles()))
    {
        TilePtr fp2 = pfp->getTile();
        if (fp2 == fp)
        {
            count++;
        }
    }
    return count;
}

QVector<QTransform> Tiling::getPlacements(TilePtr fp)
{
    QVector<QTransform> placements;
    for (auto pfp : qAsConst(getData().getPlacedTiles()))
    {
        TilePtr fp2 = pfp->getTile();
        if (fp2 == fp)
        {
            placements.push_back(pfp->getTransform());
        }
    }
    return placements;
}

QTransform Tiling::getPlacement(TilePtr fp, int index)
{
    QTransform placement;
    int i = 0;
    for (auto pfp : qAsConst(getData().getPlacedTiles()))
    {
        TilePtr fp2 = pfp->getTile();
        if (fp2 == fp)
        {
            placement = pfp->getTransform();
            if (i == index)
            {
                return placement;
            }
            i++;
        }
    }
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
            QVector<PlacedTilePtr>  & v = tileGroup.getPlacements(tile);
            v.push_back(placedTile);
        }
        else
        {
            QVector<PlacedTilePtr> v;
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

QVector<PlacedTilePtr> & TileGroup::getPlacements(TilePtr fp)
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
