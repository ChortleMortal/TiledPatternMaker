#pragma once

#include <QMap>
#include <QTransform>
#include <QDebug>
#include "model/motifs/motif.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"

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

    Points          getTransformedPoints()  { return placedTile->getWorldPoints(); }
    QTransform      getTransform()          { return placedTile->getPlacement(); }
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
class AdjacentTile
{
public:
    AdjacentTile(PlacedTilePtr placedTile, qreal tolerance);

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

    void dump(int idx, QString info) { qDebug().noquote() << info << idx << "edge" << edgeIdx << "pos" << position << "other:" << other << "end:" << end << "isect:" << isect << "colinear:"  << colinear << "taken:" << taken; }

    int         edgeIdx = -1;   // which tile edge (0..n-1) this contact's mid-point came from

    QPointF     position;       // point on the boundary of the tile being inferred (where an adjacent tile's already-built motif line touches it)
    QPointF		other;          // the far end, in the adjacent tile, of that line - i.e. where it's coming from

    QPointF 	end;            // position projected 100 units away from other - a long ray heading INTO this tile, used to find where it should cross a partner's ray
    QPointF 	isect;          // once matched: where this contact's ray crosses its partner's (or, for a COLINEAR match, the partner's own position)
    wContactPtr isect_contact;  // the contact this one was matched with, once paired
    eColinear   colinear;       // set on both contacts of a COLINEAR match: which one "owns" drawing the joining edge (MASTER) vs which is just along for the ride (SLAVE)
    bool        taken;          // true once this contact has been matched (or claimed as someone else's match)
};
