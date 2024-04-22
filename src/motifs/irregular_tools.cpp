////////////////////////////////////////////////////////////////////////////
//
// Infer.java
//
// Infer is the black magic part of the Islamic design system.  Given
// an empty, irregular tile, it infers a map for that tile by
// extending line segments from neighbouring tiles.  For many common
// Islamic designs, this extension is natural and easy to carry out.
// Of course, because this design tool lets you explore designs that
// were never drawn in the Islamic tradition, there will be cases where
// it's not clear what to do.  There will, in fact, be cases where
// natural extensions of line segments don't exist.  So Infer has to
// have black magic built-in.  It has to use heuristics to construct
// plausible inferred maps, or at least maps that don't break the rest
// of the system.
//
// If you're using Taprats and experiencing weird behaviour, it's
// probably coming from in here.  But you knew what you were infer
// when you started using Taprats (sorry -- couldn't resist the pun).

#include <QDebug>

#include "motifs/irregular_tools.h"
#include "motifs/motif.h"
#include "geometry/geo.h"
#include "geometry/edgepoly.h"
#include "tile/placed_tile.h"
#include "tile/tiling.h"

// Transformed mid-points of a tile.
TileMidPoints::TileMidPoints(PlacedTilePtr placedTile, Points & mids, EdgePoly &ep)
{
    this->placedTile = placedTile;
    this->midPoints  = mids;
    this->ep         = ep;
}

// Information about what tile and edge on that tile is adjacent to a given edge on a given tile.
AdjacenctTile::AdjacenctTile(PlacedTilePtr placedTile, qreal tolerance)
{
    this->placedTile = placedTile;
    this->tolerance = tolerance;
}

// Information about intersection of a side left or right in-going edge with a given side right or left edge.
IntersectionInfo::IntersectionInfo()
{
    side  = -1;
    dist2 = 1e100;
    intersection = QPointF();
    Q_ASSERT(intersection.isNull());

    otherSide   = -1;
    otherIsLeft = false;
}

IntersectionInfo::IntersectionInfo( int side, int otherSide, bool otherIsLeft, qreal dist2, QPointF i )
{
    this->side         = side;
    this->otherSide    = otherSide;
    this->otherIsLeft  = otherIsLeft;
    this->dist2        = dist2;
    this->intersection = i;
}

bool  IntersectionInfo::equals( IntersectionInfo & other )
{
    qreal diff = dist2 - other.dist2;
    return diff > -Sys::TOLSQ && diff < Sys::TOLSQ;
}

int  IntersectionInfo::compareTo(  IntersectionInfo & other )
{
    qreal diff = dist2 - other.dist2;
    return diff < -Sys::TOLSQ ? -1  : diff >  Sys::TOLSQ ?  1 : 0;
}

// The information about one point of contact on the boundary of the region being inferred.
Contact::Contact(QPointF position, QPointF other)
{
    this->position = position;
    this->other    = other;

    QPointF pt     = position - other;
    pt             = Geo::normalize(pt);
    pt            *= 100.0;
    end            = position + pt;

    taken          = false;
    colinear       = COLINEAR_NONE;
}
