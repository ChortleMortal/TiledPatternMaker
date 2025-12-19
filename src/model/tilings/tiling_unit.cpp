#include <QtWidgets>
#include "model/tilings/tiling.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling_unit.h"
#include "model/tilings/placed_tile.h"

using std::make_shared;

///////////////////////////////////////////////////////////////////////////
///
/// Tiling Unit
///
///////////////////////////////////////////////////////////////////////////

TilingUnit::TilingUnit(Tiling * parent) : tiling(parent)
{}

void TilingUnit::group(PlacedTiles & ptiles)
{
    for (const PlacedTilePtr & placedTile : std::as_const(ptiles))
    {
        addPlacedTile(placedTile);
    }
    if (tiling) tiling->setTilingViewChanged();
}

bool TilingUnit::operator ==(const TilingUnit &other) const
{
    int numthis = tunit.count();
    if (numthis != other.tunit.count())
        return false;

    for (int i=0; i < numthis; i++)
    {
        const QPair<TilePtr,PlacedTiles> & thisPair  = tunit[i];
        const QPair<TilePtr,PlacedTiles> & otherPair = other.tunit[i];
        if (*thisPair.first.get() != *otherPair.first.get())
            return false;

        const PlacedTiles &  myp = thisPair.second;
        const PlacedTiles & othp = otherPair.second;
        int numPl = myp.count();
        if (numPl != othp.count())
            return false;

        for (int j=0; j < numPl; j++)
        {
            if (*myp[j].get() != *othp[j].get())
                return false;
        }
    }
    return true;
}

TilingUnit  TilingUnit::uniqueCopy()
{
    TilingUnit acopy(tiling);
    for (auto & apair : tunit)
    {
        TilePtr newTile = make_shared<Tile>(apair.first);
        PlacedTiles newPs;
        PlacedTiles & tps = apair.second;
        for (auto  & placed : tps)
        {
            PlacedTilePtr ptp = placed->copy();
            ptp->setTile(newTile);
            newPs.push_back(ptp);
        }
        QPair<TilePtr,PlacedTiles> newpair(newTile,newPs);
        acopy.tunit.push_back(newpair);
    }

    if (tiling) tiling->setTilingViewChanged();
    return acopy;
}

bool TilingUnit::containsTile(TilePtr fp)
{
    for (auto & apair : tunit)
    {
        TilePtr f = apair.first;
        if (f == fp)
        {
            return true;
        }
    }
    return false;
}

PlacedTiles & TilingUnit::getPlacedTiles(TilePtr fp)
{
    Q_ASSERT(containsTile(fp));

    for (auto & apair : tunit)
    {
        TilePtr f = apair.first;
        if (f == fp)
        {
            return apair.second;
        }
    }

    qWarning("should never reach here");
    static PlacedTiles tpls;
    return tpls;
}

int TilingUnit::numIncluded() const
{
    int num = 0;
    for (auto & apair : tunit)
    {
        const PlacedTiles & tps = apair.second;
        for (auto & placed : tps)
        {
            if (placed->isIncluded())
            {
                num++;
            }
        }
    }
    return num;
}

int TilingUnit::numExcluded() const
{
    int num = 0;
    for (auto & apair : tunit)
    {
        const PlacedTiles & tps = apair.second;
        for (auto & placed : tps)
        {
            if (!placed->isIncluded())
            {
                num++;
            }
        }
    }
    return num;
}

int TilingUnit::numAll() const
{
    int num = 0;
    for (auto & apair : tunit)
    {
        const PlacedTiles & tps = apair.second;
        num += tps.count();
    }
    return num;
}

void TilingUnit::addPlacedTile(const PlacedTilePtr placedTile)
{
    TilePtr tile = placedTile->getTile();
    if (containsTile(tile))
    {
        PlacedTiles & v = getPlacedTiles(tile);
        v.push_back(placedTile);
    }
    else
    {
        PlacedTiles tps;
        tps.push_back(placedTile);
        QPair<TilePtr,PlacedTiles> apair(tile,tps);
        tunit.push_back(apair);
    }
    if (tiling) tiling->setTilingViewChanged();
}

void TilingUnit::replaceUnitData(TilingUnit & other)
{
    tunit = other.tunit;
    if (tiling) tiling->setTilingViewChanged();
}

void TilingUnit::addUnitData(TilingUnit & other)
{
    tunit += other.tunit;
    if (tiling) tiling->setTilingViewChanged();
}

void TilingUnit::removeExcludeds()
{
    const PlacedTiles excludes = getExcluded();
    for (const auto & ptp : std::as_const(excludes))
    {
        removePlacedTile(ptp);
    }
    if (tiling) tiling->setTilingViewChanged();
}

void TilingUnit::removePlacedTile(PlacedTilePtr placedTile)
{
    TilePtr tile = placedTile->getTile();
    for (UnitPlacedTiles & apair : tunit)
    {
        if (apair.first == tile)
        {
            PlacedTiles & tps = apair.second;
            if (tps.contains(placedTile))
            {
                tps.removeOne(placedTile);
                if (tps.isEmpty())
                {
                    tunit.removeOne(apair);
                }
            }
            if (tiling) tiling->setTilingViewChanged();
            return;
        }
    }
}

void TilingUnit::clearPlacedTiles()
{
    tunit.clear();
    if (tiling) tiling->setTilingViewChanged();
}

QVector<TilePtr> TilingUnit::getUniqueTiles() const
{
    QVector<TilePtr> tiles;

    for (auto & pair : tunit)
    {
        tiles.push_back(pair.first);
    }

    return tiles;
}

int TilingUnit::numPlacements(TilePtr tile)
{
    for (auto & pair : tunit)
    {
        if (pair.first == tile)
        {
            return pair.second.count();
        }
    }
    return 0;
}

Placements TilingUnit::getPlacements(TilePtr tile)
{
    Placements placements;

    for (auto & pair : tunit)
    {
        if (pair.first == tile)
        {
            const PlacedTiles & tps = pair.second;
            for (auto & placed : tps)
            {
                placements.push_back(placed->getPlacement());
            }
        }
    }
    return placements;
}

QTransform TilingUnit::getFirstPlacement(TilePtr tile)
{
    QTransform placement;
    PlacedTiles tilingUnit = getIncluded();
    for (const auto & placedTile : std::as_const(tilingUnit))
    {
        TilePtr atile = placedTile->getTile();
        if (atile == tile)
        {
            placement = placedTile->getPlacement();
            return placement;
        }
    }
    // none found
    return placement;
}

PlacedTiles TilingUnit::getIncluded() const
{
    PlacedTiles placements;
    for (auto & apair : tunit)
    {
        const PlacedTiles & tps = apair.second;

        for (auto & placed : tps)
        {
            if (placed->isIncluded())
            {
                placements.push_back(placed);
            }
        }
    }
    return placements;
}

PlacedTiles TilingUnit::getExcluded() const
{
    PlacedTiles placements;
    for (auto & apair : tunit)
    {
        const PlacedTiles & tps = apair.second;

        for (auto & placed : tps)
        {
            if (!placed->isIncluded())
            {
                placements.push_back(placed);
            }
        }
    }
    return placements;
}

PlacedTiles  TilingUnit::getAll() const
{
    PlacedTiles placements;
    for (auto & apair : tunit)
    {
        const PlacedTiles & tps = apair.second;

        for (auto & placed : tps)
        {
            placements.push_back(placed);
        }
    }
    return placements;
}

QString TilingUnit::info() const
{
    QString astring;
    QDebug  deb(&astring);

    deb.noquote() << "num tiles=" << numAll();
    return astring;
}
