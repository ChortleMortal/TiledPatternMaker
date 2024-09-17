#pragma once
#include "sys/sys/versioning.h"
#ifndef PLACED_TILE
#define PLACED_TILE

////////////////////////////////////////////////////////////////////////////
//
// placedTile.java
//
// A placedTile is a Tile together with a transform matrix.
// It allows us to share an underlying feature while putting several
// copies together into a tiling.  A tiling is represented as a
// collection of placedTiles (that may share Tiles) that together
// make up a translational unit.

#include <QObject>
#include <QString>
#include <QTransform>
#include <QTextStream>
#include "sys/sys/pugixml.hpp"
#include "sys/geometry/edgepoly.h"

typedef std::shared_ptr<class PlacedTile>   PlacedTilePtr;
typedef std::shared_ptr<class Tile>         TilePtr;

class Tiling;

class PlacedTile
{
    enum eTileState
    {
        UNDEFINED   = 0x00,
        TOUCHING    = 0x01,
        OVERLAPPING = 0x02,
    };

public:
    PlacedTile();
    PlacedTile(TilePtr tile, QTransform T);
    ~PlacedTile() {}

    bool operator == (const PlacedTile other);

    PlacedTilePtr   copy();

    void            setTransform(QTransform newT);
    void            setTile(TilePtr tile);
    void            setShow(bool show);
    bool            loadFromGirihShape(VersionedName vname);

    TilePtr         getTile();
    QTransform      getTransform();
    const EdgePoly& getTileEdgePoly();
    EdgePoly&       getTileEdgePolyRW();
    QPolygonF       getTilePoints();
    EdgePoly        getPlacedEdgePoly();
    QPolygonF       getPlacedPoints();

    bool            isIncluded() { return _included; }
    void            include()    { _included = true; }
    void            exclude()    { _included = false; }

    bool            saveAsGirihShape(VersionedName vname);
    bool            isGirihShape()      { return !girihShapeName.isEmpty(); }
    VersionedName   getGirihShapeName() { return girihShapeName; }

    bool show()             { return _show; }

    void clearViewState()   { _viewState  = UNDEFINED; }
    void setOverlapping()   { _viewState |= OVERLAPPING; }
    void setTouching()      { _viewState |= TOUCHING; }
    bool isOverlapping()    { return (_viewState & OVERLAPPING); }
    bool isTouching()       { return (_viewState & TOUCHING); }

    void dump();

protected:
    void saveGirihShape(QTextStream & out, VersionedName vname);
    void loadGirihShape(pugi::xml_node & poly_node);
    void loadGirihShape(int sides, pugi::xml_node & poly_node);
    void loadGirihShapeOld(pugi::xml_node & poly_node);

private:
    VersionedName   girihShapeName;
    TilePtr         tile;
    QTransform      T;
    bool            _show;  // used by tiling maker
    uint            _viewState;
    bool            _included;
};
#endif
