#pragma once
#ifndef TILING_UNIT
#define TILING_UNIT

#include <QtGlobal>
#include <QObject>

#include "sys/geometry/fill_region.h"

typedef std::shared_ptr<class Tile>         TilePtr;
typedef std::shared_ptr<class PlacedTile>   PlacedTilePtr;
typedef QVector<PlacedTilePtr>              PlacedTiles;
typedef QPair<TilePtr,PlacedTiles>          UnitPlacedTiles;

class TilingUnit
{
public:
    TilingUnit(Tiling * tiling);

    bool        operator == (const TilingUnit &other) const;
    bool        operator != (const TilingUnit & other) const { return !(*this == other); }

    TilingUnit  uniqueCopy();
    bool        isEmpty() { return tunit.isEmpty(); }

    void        replaceUnitData(TilingUnit & other);
    void        addUnitData(TilingUnit & other);

    void        group(PlacedTiles & ptiles);

    void        clearPlacedTiles();
    void        removePlacedTile(PlacedTilePtr placedTile);
    void        addPlacedTile(const PlacedTilePtr pfm);

    int         numIncluded() const;
    int         numExcluded() const;
    int         numAll()      const;
    int         numUnique()   const { return getUniqueTiles().count(); }

    PlacedTiles getIncluded() const;
    PlacedTiles getExcluded() const;
    PlacedTiles getAll()      const;

    void        removeExcludeds();

    const QVector<UnitPlacedTiles> & getUnitPlacedTiles() { return tunit; }

    int                 numPlacements(TilePtr tile);
    Placements          getPlacements(TilePtr tile);
    QTransform          getFirstPlacement(TilePtr tile);
    QVector<TilePtr>    getUniqueTiles() const;

    QString             info() const;

protected:

private:
    bool                containsTile(TilePtr fp);
    PlacedTiles &       getPlacedTiles(TilePtr fp);

    QVector<UnitPlacedTiles> tunit;      // this is the tiling unit
    Tiling *                 tiling;     // parent
};

#endif

