#pragma once
#ifndef TILE_H
#define TILE_H

////////////////////////////////////////////////////////////////////////////
//
// Feature.java
//
// A Tile is an element of a tiling, i.e. a tile.  It's really
// just an array of points.  I could just use a geometry.Polygon,
// but I keep adding (and removing) per-point information, which makes it
// useful to store the array explicitly -- sometimes I switch Point
// for a FancyPoint of some kind.  Also, we don't expect the number
// of points to change once the feature is created, so the array
// is fine.
//
// We also store whether the Tile was created as a regular polygon.
// This helps later when deciding what Tiles can have Rosettes
// in them.

#include <QObject>
#include <QPolygonF>
#include "sys/geometry/edge_poly.h"

typedef std::shared_ptr<class Tile>         TilePtr;

class PreConvert
{
public:
    PreConvert() { converted = false; }
    bool     converted;
    bool     wasRegular;
    EdgePoly ep;
    qreal    rotate;
    qreal    scale;
};

class Tile : public EdgePoly
{
public:
    Tile(int n, qreal rotate = 0.0, qreal scale = 1.0); // Create an n-sided regular polygon with a vertex at (1,0).
    Tile(EdgePoly epoly);                               // Creates an irregular tile
    Tile(const TilePtr other);
    Tile(const Tile & other);
    ~Tile();

    bool operator == (const Tile & other) const;
    bool operator != (const Tile & other) const { return !(*this == other); }

    TilePtr     copy();
    TilePtr     recreate();

    void        setRegular(bool enb);
    void        flipRegularity();
    void        setN(uint n);

    void        deltaRotation(qreal delta);
    void        deltaScale(qreal delta);

    bool        isSimilar(const TilePtr other);
    bool        isRegular()         { return regular; }

    const EdgePoly& getEdgePoly() const { return *this;}
    EdgePoly&       getEdgePolyRW()     { return *this;}

    QPointF     getCenter();
    QLineF      getEdge(uint side);
    qreal       edgeLen(uint side = 0);

    QString     toString() const;
    QString     toBaseString() const;
    QString     info();
    QString     summary();

    static int  refs;

private:
    void        createRegularBase();

    uint        n;              // sides
    bool        regular;

    PreConvert  preconversion;     // stored when going from regular to irregular and vice versa
};

#endif
