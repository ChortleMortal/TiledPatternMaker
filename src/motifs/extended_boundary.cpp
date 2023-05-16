#include "motifs/extended_boundary.h"
#include "tile/tile.h"

///////////////////////////////////////////////////////////
///
/// ExtendedBoundary
///
///////////////////////////////////////////////////////////

ExtendedBoundary::ExtendedBoundary()
{
    sides    = 1;   // defaults to a circle
    scale    = 1.0;
    rotate   = 0;
}

ExtendedBoundary::ExtendedBoundary(const ExtendedBoundary & other)
{
    sides    = other.sides;
    scale    = other.scale;
    rotate   = other.rotate;
    boundary2 = other.boundary2;
}

void ExtendedBoundary::set(const QPolygonF & p)
{
    Q_ASSERT(p.isClosed());
    boundary2 = p;
    //qDebug() << "set ext b" << boundary2;
}

const QPolygonF &ExtendedBoundary::get() const
{
    if (!boundary2.isEmpty())
    {
        Q_ASSERT(boundary2.isClosed());
    }
    //qDebug() << "get ext b" << boundary2;
    return boundary2;
}

bool ExtendedBoundary::equals(const ExtendedBoundary & other)
{
    return (sides == other.sides
            && scale == other.scale
            && rotate == other.rotate
            && boundary2 == other.boundary2);
}

void ExtendedBoundary::buildRadial()
{
    if (sides >= 3)
    {
        Tile f2(sides,rotate,scale);
        set(f2.getPolygon());
    }
    else
    {
        boundary2.clear();
    }
}

void ExtendedBoundary::buildExplicit(TilePtr tile)
{
    set(tile->getPolygon());
}


