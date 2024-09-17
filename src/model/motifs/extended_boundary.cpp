#include <QTransform>
#include "model/motifs/extended_boundary.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/transform.h"
#include "sys/geometry/loose.h"
#include "model/motifs/motif.h"
#include "model/tilings/tile.h"

///////////////////////////////////////////////////////////
///
/// ExtendedBoundary
///
///////////////////////////////////////////////////////////

ExtendedBoundary::ExtendedBoundary(uint nsides)
{
    radial   = true;
    sides    = nsides;
    scale    = 1.0;
    rotate   = 0.0;
}

ExtendedBoundary::ExtendedBoundary(const ExtendedBoundary & other)
{
    radial   = other.radial;
    sides    = other.sides;
    scale    = other.scale;
    rotate   = other.rotate;
    boundary = other.boundary;
}

void ExtendedBoundary::set(const QPolygonF & p)
{
    Q_ASSERT(p.isClosed());
    boundary = p;
}

const QPolygonF ExtendedBoundary::getPoly() const
{
    if (!boundary.isEmpty())
    {
        Q_ASSERT(boundary.isClosed());
    }

    if (radial)
    {
        return boundary;
    }
    else
    {
        auto pt = Geo::irregularCenter(boundary);
        QTransform t;
        if (!Loose::equals(scale,1.0))
        {
            t *= Transform::scaleAroundPoint(pt,scale);
        }
        if (!Loose::zero(rotate))
        {
            t *= Transform::rotateDegreesAroundPoint(pt,rotate);
        }

        if (!t.isIdentity())
        {
            return t.map(boundary);
        }
        else
        {
            return boundary;
        }
    }
}

bool ExtendedBoundary::equals(const ExtendedBoundary & other)
{
    return (   sides      == other.sides
            && scale      == other.scale
            && rotate     == other.rotate
            && boundary   == other.boundary);
}

bool ExtendedBoundary::isUnity(MotifPtr motif)
{
    int tile_sides = motif->getTile()->numSides();
    if ( (sides == 1 || sides == tile_sides)
        && Loose::equals(scale,1.0)
        && Loose::zero(rotate))
        return true;
    else
        return false;
}

void ExtendedBoundary::buildRadial()
{
    radial = true;
    if (sides >= 3)
    {
        Tile f2(sides,rotate,scale);
        set(f2.getPolygon());
    }
    else
    {
        boundary.clear();
    }
}

void ExtendedBoundary::buildExplicit(TilePtr tile)
{
    radial    = false;
    sides      = tile->numSides();
    set(tile->getPolygon());
}
