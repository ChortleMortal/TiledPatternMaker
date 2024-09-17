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
#include "sys/geometry/edgepoly.h"

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

class Tile : public QObject
{
    Q_OBJECT

public:
    Tile(int n, qreal rotate = 0.0, qreal scale = 1.0);           // Create an n-sided regular polygon with a vertex at (1,0).
    Tile(EdgePoly epoly, qreal rotate = 0.0, qreal scale = 1.0);
    Tile(const TilePtr other);
    Tile(const Tile & other);
    ~Tile();

    bool operator == (const Tile & other) const;

    void        compose();      // makes epoly from base
    void        decompose();    // makes base from epoly

    TilePtr     copy();
    TilePtr     recreate();

    void        setRegular(bool enb);
    void        flipRegularity();
    void        setN(int n);
    void        setRotation(qreal rot);
    void        setScale(qreal rot);

    void        deltaRotation(qreal delta);
    void        deltaScale(qreal delta);

    qreal       getRotation()       { return rotation; }
    qreal       getScale()          { return scale; }

    bool        equals(const TilePtr other);
    bool        isSimilar(const TilePtr other);
    bool        isRegular()         { return regular; }
    bool        isClockwise()       { return  epoly.isClockwise(); }

    const EdgePoly& getEdgePoly()   { return epoly;}
    EdgePoly&       getEdgePolyRW() { return epoly;}
    const EdgePoly& getBase()       { return base; }

    QVector<EdgePtr> getEdges()     { return epoly; }
    QVector<QLineF>  getLines()     { return epoly.getLines(); }        // ignores curves

    QPolygonF   getPolygon()        { return epoly.getPoly(); }         // closed
    QPolygonF   getPoints()         { return epoly.getPoints(); }       // not closed
    QPolygonF   getMids()           { return epoly.getMids(); }         // not closed
    int         numPoints()         { return epoly.size(); }
    int         numSides()          { return epoly.size(); }

    QPointF     getCenter();
    QLineF      getEdge(int side);
    qreal       edgeLen(int side = 0);

    QString     toString() const;
    QString     toBaseString() const;
    QString     info();
    QString     summary();

    QTransform  getTransform()      { return QTransform::fromScale(scale,scale).rotate(rotation); }
    void        legacyDecompose();

    static int  refs;

signals:
    void        sig_tileChanged();

private:
    void        createRegularBase();

    int         n;              // sides
    bool        regular;
    qreal       rotation;
    qreal       scale;

    EdgePoly    base;           // calculated
    EdgePoly    epoly;          // calculated

    PreConvert  conversion;     // stored when going from regular to irregular and vice versa
};

#endif
