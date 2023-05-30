#include "motifs/extended_boundary.h"
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
}

ExtendedBoundary::ExtendedBoundary(const ExtendedBoundary & other)
{
    radial   = other.radial;
    sides    = other.sides;
    scale    = other.scale;
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
        QTransform t;
        t.scale(scale,scale);
        return t.map(boundary);
    }
}

bool ExtendedBoundary::equals(const ExtendedBoundary & other)
{
    return (   sides      == other.sides
            && scale      == other.scale
            && boundary   == other.boundary);
}

void ExtendedBoundary::buildRadial()
{
    radial = true;
    if (sides >= 3)
    {
        Tile f2(sides,0.0,scale);
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
    scale      = 1.0;
    set(tile->getPolygon());
}
