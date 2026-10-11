#include <QDebug>
#include <QTransform>
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

TilingUnitController::TilingUnitController(Tiling * parent) : tiling(parent)
{}


bool TilingUnitController::operator ==(const TilingUnitController &other) const
{
    int numthis = tunit.count();
    if (numthis != other.tunit.count())
        return false;

    for (int i=0; i < numthis; i++)
    {
        const TileUnit & thisTuleUnit  = tunit[i];
        const TileUnit & otherTileUnit = other.tunit[i];
        if (*thisTuleUnit.first.get() != *otherTileUnit.first.get())
            return false;

        const PlacedTileSet &  myp = thisTuleUnit.second;
        const PlacedTileSet & othp = otherTileUnit.second;
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

TilingUnitController  TilingUnitController::uniqueCopy()
{
    TilingUnitController acopy(tiling);
    for (TileUnit & tileUnit : tunit)
    {
        TilePtr newTile = make_shared<Tile>(tileUnit.first);
        PlacedTileSet newPs;
        PlacedTileSet & tps = tileUnit.second;
        for (PlacedTilePtr & placed : tps)
        {
            PlacedTilePtr ptp = placed->copy();
            ptp->setTile(newTile);
            newPs.push_back(ptp);
        }
        QPair<TilePtr,PlacedTileSet> newpair(newTile,newPs);
        acopy.tunit.push_back(newpair);
    }

    if (tiling) tiling->setTilingViewChanged();
    return acopy;
}

bool TilingUnitController::containsTile(TilePtr fp)
{
    for (TileUnit & tileUnit : tunit)
    {
        TilePtr f = tileUnit.first;
        if (f == fp)
        {
            return true;
        }
    }
    return false;
}

PlacedTileSet & TilingUnitController::getPlacedTiles(TilePtr fp)
{
    Q_ASSERT(containsTile(fp));

    for (TileUnit & tileUnit : tunit)
    {
        TilePtr f = tileUnit.first;
        if (f == fp)
        {
            return tileUnit.second;
        }
    }

    qWarning("should never reach here");
    static PlacedTileSet tpls;
    return tpls;
}

int TilingUnitController::numIncluded() const
{
    int num = 0;
    for (const TileUnit & tileUnit : tunit)
    {
        const PlacedTileSet & tps = tileUnit.second;
        for (const PlacedTilePtr & placed : tps)
        {
            if (placed->isIncluded())
            {
                num++;
            }
        }
    }
    return num;
}

int TilingUnitController::numExcluded() const
{
    int num = 0;
    for (const TileUnit & tileUnit : tunit)
    {
        const PlacedTileSet & tps = tileUnit.second;
        for (const PlacedTilePtr & placed : tps)
        {
            if (!placed->isIncluded())
            {
                num++;
            }
        }
    }
    return num;
}

int TilingUnitController::numAll() const
{
    int num = 0;
    for (const TileUnit & tileUnit : tunit)
    {
        const PlacedTileSet & tps = tileUnit.second;
        num += tps.count();
    }
    return num;
}

void TilingUnitController::addPlacedTile(const PlacedTilePtr placedTile)
{
    // adds to TilingUnit are always included
    // they move from Undefined to False and can always be excluded (True) later
    placedTile->getState().excluded.set(Tristate::eTristate::False);

    TilePtr tile = placedTile->getTile();
    if (containsTile(tile))
    {
        PlacedTileSet & v = getPlacedTiles(tile);
        if (!v.contains(placedTile))
        {
            // not a duplicate
            v.push_back(placedTile);
        }
    }
    else
    {
        PlacedTileSet tps;
        tps.push_back(placedTile);
        QPair<TilePtr,PlacedTileSet> apair(tile,tps);
        tunit.push_back(apair);
    }
    if (tiling) tiling->setTilingViewChanged();
}

void TilingUnitController::replaceUnitData(TilingUnitController & other)
{
    tunit = other.tunit;
    if (tiling) tiling->setTilingViewChanged();
}

void TilingUnitController::addUnitData(TilingUnitController & other)
{
    tunit += other.tunit;
    if (tiling) tiling->setTilingViewChanged();
}

void TilingUnitController::removeExcludeds()
{
    PlacedTileSet excludes;
    for (TileUnit & tileUnit : tunit)
    {
        const PlacedTileSet & tps = tileUnit.second;

        for (const PlacedTilePtr & placed : tps)
        {
            if (placed->isExcluded())
            {
                excludes.push_back(placed);
            }
        }
    }

    for (const PlacedTilePtr & ptp : std::as_const(excludes))
    {
        removePlacedTile(ptp);
    }

    if (tiling) tiling->setTilingViewChanged();
}

void TilingUnitController::removePlacedTile(PlacedTilePtr placedTile)
{
    TilePtr tile = placedTile->getTile();
    for (TileUnit & tileUnit : tunit)
    {
        if (tileUnit.first == tile)
        {
            PlacedTileSet & tps = tileUnit.second;
            if (tps.contains(placedTile))
            {
                tps.removeOne(placedTile);
                if (tps.isEmpty())
                {
                    tunit.removeOne(tileUnit);
                }
            }
            if (tiling) tiling->setTilingViewChanged();
            return;
        }
    }
}

void TilingUnitController::clearPlacedTiles()
{
    tunit.clear();
    if (tiling) tiling->setTilingViewChanged();
}

TileSet TilingUnitController::getUniqueTiles() const
{
    TileSet tiles;

    for (const TileUnit & tileUnit : tunit)
    {
        tiles.push_back(tileUnit.first);
    }

    return tiles;
}

PlacedTileSet TilingUnitController::getUniquePlacedTiles() const
{
    PlacedTileSet tiles;

    for (const TileUnit & tileUnit : tunit)
    {
        tiles.push_back(tileUnit.second.first());
    }

    return tiles;
}

int TilingUnitController::numPlacements(TilePtr tile)
{
    for (TileUnit & tileUnit : tunit)
    {
        if (tileUnit.first == tile)
        {
            return tileUnit.second.count();
        }
    }
    return 0;
}

Placements TilingUnitController::getPlacements(TilePtr tile, const QTransform & distortion)
{
    Placements placements;

    for (TileUnit & tileUnit : tunit)
    {
        if (tileUnit.first == tile)
        {
            const PlacedTileSet & tps = tileUnit.second;
            for (const PlacedTilePtr & placed : tps)
            {
                if (placed->isIncluded())
                {
                    placements.push_back(placed->getWorldTransform(distortion));
                }
            }
        }
    }
    return placements;
}

Placements TilingUnitController::getPlacementsWithExcludes(Tile * tile)
{
    Placements placements;

    for (TileUnit & tileUnit : tunit)
    {
        if (tileUnit.first.get() == tile)
        {
            const PlacedTileSet & tps = tileUnit.second;
            for (const PlacedTilePtr & placed : tps)
            {
                placements.push_back(placed->getPlacement());
            }
        }
    }
    return placements;
}

QTransform TilingUnitController::getFirstPlacement(TilePtr tile, const QTransform & distortion)
{
    QTransform placement;
    PlacedTileSet tileSet = getIncluded();
    for (const PlacedTilePtr & placedTile : std::as_const(tileSet))
    {
        TilePtr atile = placedTile->getTile();
        if (atile == tile)
        {
            return placedTile->getWorldTransform(distortion);
        }
    }
    // none found
    return placement;
}

PlacedTileSet TilingUnitController::getIncluded() const
{
    PlacedTileSet placements;
    for (const TileUnit & tileUnit : tunit)
    {
        const PlacedTileSet & tps = tileUnit.second;

        for (const PlacedTilePtr & placed : tps)
        {
            if (placed->isIncluded())
            {
                placements.push_back(placed);
            }
        }
    }
    return placements;
}

PlacedTileSet  TilingUnitController::getWithExcluded() const
{
    PlacedTileSet placements;
    for (const TileUnit & tileUnit : tunit)
    {
        const PlacedTileSet & tps = tileUnit.second;

        for (const PlacedTilePtr & placed : tps)
        {
            placements.push_back(placed);
        }
    }
    return placements;
}

QString TilingUnitController::info() const
{
    QString astring;
    QDebug  deb(&astring);

    deb.noquote() << "num tiles=" << numAll();
    return astring;
}

void TilingUnitController::dump()
{
    for (TileUnit & tileUnit: tunit)
    {
        TilePtr tp = tileUnit.first;
        qDebug() << "*** "  << tp->info() << tp.get();

        PlacedTileSet set = tileUnit.second;
        for (PlacedTilePtr & placedTile : set)
        {
            qDebug().noquote() << placedTile->info()  << placedTile.get();
        }
    }
}