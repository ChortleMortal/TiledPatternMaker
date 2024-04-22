#pragma once
#ifndef IRREGULAR_TOOLS_H
#define IRREGULAR_TOOLS_H

#include <QMap>
#include <QTransform>
#include <QDebug>
#include "motifs/motif.h"
#include "tile/placed_tile.h"
#include "tile/tile.h"

typedef std::shared_ptr<class IntersectionInfo> IntersectionPtr;
typedef std::shared_ptr<class EdgesLengthInfo>  EdgesLenPtr;
typedef std::weak_ptr<class Contact>            wContactPtr;

// The transformed midpoints of the tile's edges.
// Since the tilings are edge to edge, a tile edge can
// be uniquely identified by its midpoint.  So we can
// compare two edges by just comparing their midpoints
// instead of comparing the endpoints pairwise.
class TileMidPoints
{
public:
    TileMidPoints(PlacedTilePtr placedTile, Points &mids, EdgePoly & ep);

    Points          getTileMidPoints()      { return midPoints; }
    QPolygonF       getTileMidPoly()        { QPolygonF p = midPoints; p << p[0]; return p;}

    Points          getTransformedPoints()  { return placedTile->getPlacedPoints(); }
    QTransform      getTransform()          { return placedTile->getTransform(); }
    TilePtr         getTile()               { return placedTile->getTile(); }
    PlacedTilePtr   getPlacedTile()         { return placedTile; }
    EdgePoly &      getEdgePoly()           { return ep; }

private:
    PlacedTilePtr   placedTile;
    Points          midPoints;              // these are placed, i.e they are transformed
    EdgePoly        ep;
};

// Information about what tile and edge on that tile is adjacent
// to a given edge on a given tile.
class AdjacenctTile
{
public:
    AdjacenctTile(PlacedTilePtr placedTile, qreal tolerance);

    PlacedTilePtr  placedTile;
    qreal          tolerance;
};

// Information about intersection of a side left or right in-going edge
// with a given side right or left edge.
class IntersectionInfo
{
public:
    IntersectionInfo();
    IntersectionInfo(int side, int otherSide, bool otherIsLeft, qreal dist2, QPointF i );

    bool    equals(   IntersectionInfo & other );
    int     compareTo(IntersectionInfo & other );

    int     side;           // Which side of the tile this describes.
    int     otherSide;      // Which other side it meets.
    bool    otherIsLeft;    // True if the other side is the left edge.
    qreal   dist2;          // The square of the distance (square to avoid a square root op.)
    QPointF intersection;   // The intersection point.
};

// The information about one point of contact on the boundary of the region being inferred.
class Contact
{
public:

    enum eColinear
    {
        COLINEAR_NONE   = 0,
        COLINEAR_MASTER = 1,
        COLINEAR_SLAVE  = 2
    };

    Contact(QPointF position, QPointF other);

    void dump(int idx, QString info) { qDebug().noquote() << info << idx << "pos" << position << "other:" << other << "end:" << end << "isect:" << isect << "colinear:"  << colinear << "taken:" << taken; }

    QPointF     position;
    QPointF		other;

    QPointF 	end;
    QPointF 	isect;
    wContactPtr isect_contact;
    eColinear   colinear;
    bool        taken;
};

#endif
