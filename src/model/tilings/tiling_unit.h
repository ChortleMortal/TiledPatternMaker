#pragma once

#include <QtGlobal>
#include <QObject>

#include "model/tilings/placed_tile.h"
#include "sys/geometry/fill_region.h"

typedef std::shared_ptr<class Tile>         TilePtr;
typedef QVector<TilePtr>                    TileSet;
typedef std::shared_ptr<class PlacedTile>   PlacedTilePtr;
typedef QPair<TilePtr,PlacedTileSet>        TileUnit;
typedef QVector<TileUnit>                   TilingUnit;

/*
 * The Tiling Unit contains both excluded=True and included=False Placed Tiles.
 * Upstream from the Tiling maker, filled/flooded placed tiles are Undefine
 */

class TilingUnitController
{
public:
    TilingUnitController(Tiling * tiling);

    bool        operator == (const TilingUnitController &other) const;
    bool        operator != (const TilingUnitController & other) const { return !(*this == other); }

    // getters

    const TilingUnit  & get()  { return tunit; }
    PlacedTileSet       getIncluded() const;
    PlacedTileSet       getWithExcluded()      const;

    Placements          getPlacements(TilePtr tile, const QTransform & distortion = QTransform());                // no excludes
    Placements          getPlacementsWithExcludes(Tile * tile);     // used only by system info tree
    QTransform          getFirstPlacement(TilePtr tile, const QTransform & distortion = QTransform());
    PlacedTileSet       getUniquePlacedTiles() const;
    TileSet             getUniqueTiles() const;

    TilingUnitController uniqueCopy();

    // setters

    void        clearPlacedTiles();
    void        addPlacedTile(const PlacedTilePtr pfm);
    void        removePlacedTile(PlacedTilePtr placedTile);

    void        addUnitData(TilingUnitController & other);
    void        replaceUnitData(TilingUnitController & other);
    void        removeExcludeds();

    // info

    bool        isEmpty() { return tunit.isEmpty(); }
    int         numIncluded() const;
    int         numExcluded() const;
    int         numAll()      const;
    int         numUnique()   const { return getUniqueTiles().count(); }

    int         numPlacements(TilePtr tile);
    QString     info() const;

    void        dump();

private:
    bool            containsTile(TilePtr fp);
    PlacedTileSet & getPlacedTiles(TilePtr fp);

    TilingUnit      tunit;      // this is the tiling unit
    Tiling *        tiling;     // parent
};

