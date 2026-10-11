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
    tileScale= 1.0;
}

ExtendedBoundary::ExtendedBoundary(const ExtendedBoundary & other)
{
    radial   = other.radial;
    sides    = other.sides;
    scale    = other.scale;
    rotate   = other.rotate;
    tileScale= other.tileScale;
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

bool ExtendedBoundary::isUnity(Motif * motif)
{
    if (sides != motif->getN())
        return false;
    auto tile = motif->getTile();
    if (tile && sides != tile->numPoints())
        return false;
    if (!Loose::equals(scale,1.0))
        return false;
    if (!Loose::zero(rotate))
        return false;

    return true;
}

void ExtendedBoundary::buildRadial(TilePtr tile)
{
    radial    = true;
    tileScale = tile->getScale();
    if (sides >= 3)
    {
        Tile f2(sides,rotate + tile->getRotation(),scale * tileScale);
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
    tileScale = 1.0;
    set(tile->getPolygon());
}
