#include <QDebug>
#include "motifs/irregular_girih_branches.h"
#include "motifs/motif.h"
#include "geometry/intersect.h"
#include "geometry/map.h"
#include "makers/prototype_maker/prototype.h"
#include "tile/tiling.h"
#include "geometry/transform.h"

IrregularGirihBranches::IrregularGirihBranches() : IrregularMotif()
{
}

IrregularGirihBranches::IrregularGirihBranches(const Motif & other) : IrregularMotif(other)
{
}

IrregularGirihBranches::IrregularGirihBranches(const IrregularGirihBranches & other) : IrregularMotif(other)
{
}

MapPtr IrregularGirihBranches::buildGirihBranch(int side, qreal requiredRotation)
{
    MapPtr map = std::make_shared<Map>("buildGirihBranch map");

    // Mid-point of this side is always included in the map.
    VertexPtr midVertex  = map->insertVertex( mids[side] );

    // Find which other edge will intersect this one first.
    for ( int i_halves = 0; i_halves < 2; ++i_halves )
    {
        bool isLeftHalf = (i_halves == 0);

        // Find an intersection, if any.
        QPointF sideHalf       = buildGirihHalfBranch( side, isLeftHalf, requiredRotation);
        IntersectionInfo info = findClosestIntersection( side, sideHalf, isLeftHalf, requiredRotation);

        if ( info.intersection.isNull() )
            continue;

        VertexPtr interVertex = map->insertVertex( info.intersection );
        map->insertEdge( midVertex, interVertex );
    }

    return map;
}

// Girih inferring.
QPointF IrregularGirihBranches::buildGirihHalfBranch(int side, bool leftBranch, qreal requiredRotation)
{
    int side_count = corners.size();
    int next = (side + 1) % side_count;
    QTransform rot = Transform::rotateRadiansAroundPoint(mids[side], leftBranch ? -requiredRotation : requiredRotation);

    QPointF halfBranch = rot.map(corners[ leftBranch ? side : next ]);
    halfBranch -= mids[side];
    halfBranch *= 3;  // 32;       // I think this just makes a long line
    halfBranch += mids[side];

    return halfBranch;
}

IntersectionInfo IrregularGirihBranches::findClosestIntersection(int side, QPointF sideHalf, bool isLeftHalf, qreal requiredRotation)
{
    IntersectionInfo info;

    QPointF sideMidPoint = mids[side];

    bool otherIsLeft = !isLeftHalf;

    int side_count = corners.size();
    for ( int i_side = 0; i_side < side_count; ++i_side )
    {
        if ( i_side == side )
            continue;

        QPointF otherMidPoint = mids[i_side];

        QPointF intersection;
        if ( Loose::zero(Point::dist2ToLine( otherMidPoint, sideMidPoint, sideHalf ) ) )
        {
            // Edge meets directly the other mid-points, so the distance is the shortest possible.
            intersection = Point::convexSum(otherMidPoint, sideMidPoint, 0.5 );
        }
        else
        {
            QPointF otherSide = buildGirihHalfBranch( i_side, otherIsLeft, requiredRotation);
            if (!Intersect::getIntersection( otherMidPoint, otherSide, sideMidPoint, sideHalf, intersection))
            {
                continue;
            }
        }

        qreal dist2 = Point::dist2( intersection, sideMidPoint ) + Point::dist2(intersection, otherMidPoint );
        if ( Loose::equals( dist2, info.dist2 ) )
        {
            // In case of absolute equality, we use the nearest side.
            if ( abs(side - i_side) < abs(side - info.side) )
            {
                info.side = i_side;
                info.dist2 = dist2;
                info.intersection = intersection;
            }
        }
        if ( dist2 < info.dist2 )
        {
            info.side = i_side;
            info.dist2 = dist2;
            info.intersection = intersection;
        }
    }

    return info;
}
