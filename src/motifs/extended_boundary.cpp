#include <QTransform>
#include "motifs/extended_boundary.h"
#include "geometry/geo.h"
#include "geometry/transform.h"
#include "geometry/loose.h"
#include "tile/tile.h"

///////////////////////////////////////////////////////////
///
/// ExtendedBoundary
///
///////////////////////////////////////////////////////////

ExtendedBoundary::ExtendedBoundary()
{
    radial   = true;
    sides    = 1;   // defaults to a circle
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
