#include <QtMath>
#include "motifs/intersect_motif.h"
#include "tile/tile.h"
#include "geometry/geo.h"
#include "geometry/map.h"
#include "geometry/intersect.h"

IntersectMotif::IntersectMotif() : IrregularGirihBranches()
{
    setMotifType(MOTIF_TYPE_INTERSECT);
}

void IntersectMotif::init(qreal starSkip, int s, bool progressive)
{
    this->skip = starSkip;
    this->s    = s;
    this->progressive = progressive;
}

IntersectMotif::IntersectMotif(const Motif &other) : IrregularGirihBranches(other)
{
    setMotifType(MOTIF_TYPE_INTERSECT);
}

void IntersectMotif::buildMotifMaps()
{
    Q_ASSERT(getTile());
    motifMap = std::make_shared<Map>("Intersect Motifmap");
    if (progressive)
        inferIntersectProgressive();
    else
        inferIntersect();
    scaleAndRotate();
    completeMap();
    buildMotifBoundary();
    buildExtendedBoundary();
}

void IntersectMotif::inferIntersect()
{
    qDebug() << "Infer::inferIntersect";

#if 1
    debugMap = std::make_shared<DebugMap>("Intersect debug map");
#endif

    // We use the number of side of the star and how many side it
    // hops over from branch to branch (where 1 would mean drawing
    // a polygon) and deduce the inner angle of the star branches.
    // We support fractional side skipping.
    qreal qstarSides           = static_cast<qreal>(getN());
    qreal polygonInnerAngle    = M_PI * (qstarSides-2.0) / qstarSides;
    qreal starBranchInnerAngle = (skip * polygonInnerAngle) - (skip - 1) * M_PI;
    qreal requiredRotation     = (M_PI - starBranchInnerAngle) / 2;

#if 0
    // Get the index of a good transform for this tile.
    int cur       = findPrimaryTile(tile);
    MidsPtr pmain = allMotifMids[cur];
    mids          = pmain->getTileMidPoints();
    corners       = pmain->getTransformedPoints();
#else
    corners = getTile()->getPoints();
    mids    = getTile()->getMids();
#endif
    // Accumulate all edge intersections and their length.
    QList<EdgesLenPtr> infos;

    int sides = mids.size();
    for ( int side = 0; side < sides; ++side )
    {
        QPointF sideHalf = buildGirihHalfBranch(side, true, requiredRotation);
        if (debugMap && side==2)
        {
            debugMap->insertDebugMark(sideHalf,QString("%1t").arg(side));
            debugMap->insertDebugLine(QLineF(mids[side],sideHalf));
        }
        infos.append(buildIntersectEdgesLengthInfos(side, sideHalf, true, requiredRotation));

        sideHalf = buildGirihHalfBranch(side, false, requiredRotation);
        if (debugMap && side==2)
        {
            debugMap->insertDebugMark(sideHalf,QString("%1f").arg(side));
            debugMap->insertDebugLine(QLineF(mids[side],sideHalf));
        }
        infos.append(buildIntersectEdgesLengthInfos(side, sideHalf, false, requiredRotation));
    }

    for (auto & info : std::as_const(infos))
    {
        if (debugMap && info->side1 == 0)
        {
            debugMap->insertDebugMark(info->intersection,"isect");
        }
    }

    // Sort edge-to-edge intersection by their total length.
    QMultiMap<qreal,EdgesLenPtr> sortMap;
    for (auto & info : std::as_const(infos))
    {
        sortMap.insert(info->dist2,info);   // insert is sorted on key
    }
    infos = sortMap.values();

    // Record the starting point of each edge. As we grow the edge,
    // when we want more than one intersection (s > 1), we will update
    // these starting points.
    QVector<QVector<int>> counts(sides);
    for (int i = 0; i < sides; ++i)
    {
        counts[i].push_back(0);
        counts[i].push_back(0);
    }

    // Build the map using the shortest edges first.
    for (auto & info : std::as_const(infos))
    {
        info->dump();
        int side1   = info->side1;
        int isLeft1 = info->isLeft1 ? 0 : 1;
        int side2   = info->side2;
        int isLeft2 = info->isLeft2 ? 0 : 1;
        if (counts[side1][isLeft1] < s && counts[side2][isLeft2] < s)
        {
            QPointF from1 = mids[side1];
            QPointF from2 = mids[side2];
            if ( !info->intersection.isNull() )
            {
                motifMap->insertEdge(from1, info->intersection);
                motifMap->insertEdge(info->intersection, from2);
            }
            else
            {
                motifMap->insertEdge(from1, from2);
            }
            counts[side1][isLeft1]++;
            counts[side2][isLeft2]++;
        }
    }

#if 0
     motifMap->transformMap(pmain->getTransform().inverted() );
    if (debug) debugMap->transformMap(pmain->getTransform().inverted() );
#endif
    qDebug().noquote() << motifMap->summary();
}

void IntersectMotif::inferIntersectProgressive()
{
    qDebug() << "Infer::inferIntersectProgressive";

    // We use the number of side of the star and how many side it
    // hops over from branch to branch (where 1 would mean drawing
    // a polygon) and deduce the inner angle of the star branches.
    // We support fractional side skipping.
    qreal qstarSides           = static_cast<qreal>(getN());
    qreal polygonInnerAngle    = M_PI * (qstarSides-2.0) / qstarSides;
    qreal starBranchInnerAngle = (skip * polygonInnerAngle) - (skip - 1) * M_PI;
    qreal requiredRotation     = (M_PI - starBranchInnerAngle) / 2;

#if 0
    // Get the index of a good transform for this tile.
    int cur           = findPrimaryTile(tile);
    MidsPtr pmain     = allMotifMids[cur];
    QPolygonF mids    = pmain->getTileMidPoints();
    QPolygonF corners = pmain->getTransformedPoints();
#else
    corners = getTile()->getPoints();
    mids    = getTile()->getMids();
#endif

    int side_count = mids.size();
    for ( int side = 0; side < side_count; ++side )
    {
        for ( int i_halves = 0; i_halves < 2; ++i_halves )
        {
            bool isLeftHalf = (i_halves == 0);
            QPointF sideHalf = buildGirihHalfBranch( side, isLeftHalf, requiredRotation);
            QList<IntersectionPtr> infos = buildIntersectionInfos(side, sideHalf, isLeftHalf, requiredRotation);
            if ( !infos.isEmpty() )
            {
                // Record the starting point of the edge. As we grow the edge,
                // when we want more than one intersection (s > 1), we will update
                // this starting point.
                QPointF from = mids[side];

                // Build the map using the shortest edges first.
                int count = 0;
                for ( int i = 0; i < infos.size() && count < s; ++i )
                {
                    IntersectionPtr info = infos[i];
                    if ( !info->intersection.isNull())
                    {
                        VertexPtr p1  = motifMap->insertVertex( from );
                        VertexPtr p2  = motifMap->insertVertex( info->intersection );
                        motifMap->insertEdge( p1, p2 );
                        from = info->intersection;
                        count++;
                    }
                }
            }
        }
    }

#if 0
    motifMap->transformMap(pmain->getTransform().inverted());
#endif
    qDebug().noquote() << motifMap->summary();
}

// Intersect inferring.
int IntersectMotif::getIntersectionRank(int side, bool isLeft, QList<IntersectionPtr> infos)
{
    for ( int i = 0; i < infos.size(); ++i )
    {
        if ( infos[i]->otherSide == side && infos[i]->otherIsLeft == isLeft )
        {
            return i;
        }
    }
    return 100;
}

QList<EdgesLenPtr> IntersectMotif::buildIntersectEdgesLengthInfos(int side, QPointF sideHalf, bool isLeftHalf, qreal requiredRotation)
{
    // First, get a list of intersections for this edge so that we can sort the
    // edge pairs by the fewest number of intersections.
    QList<IntersectionPtr> inter_infos = buildIntersectionInfos( side, sideHalf, isLeftHalf, requiredRotation);

    QList<EdgesLenPtr> infos;

    QPointF sideMidPoint = mids[side];

    int side_count = corners.size();
    for ( int i_side = side + 1; i_side < side_count; ++i_side )
    {
        for ( int i_halves = 0; i_halves < 2; ++i_halves )
        {
            bool otherIsLeft      = (i_halves == 0);
            QPointF otherMidPoint = mids[i_side];
            QPointF otherSide     = buildGirihHalfBranch( i_side, otherIsLeft, requiredRotation);
            QList<IntersectionPtr> other_inter_infos = buildIntersectionInfos( i_side, otherSide, otherIsLeft, requiredRotation);

            QPointF intersection;
            if (!Intersect::getIntersection( otherMidPoint, otherSide, sideMidPoint, sideHalf,intersection))
            {
                // Lines are parallel, see if they actually point at each other.
                if ( Loose::zero( Geo::dist2ToLine(otherMidPoint, sideMidPoint, sideHalf ) ) )
                {
                    // Edge meets directly the other mid-points, so the distance is the middle in-between.
                    intersection = Geo::convexSum( otherMidPoint, sideMidPoint, 0.5 );
                }
            }

            if (corners.containsPoint(intersection,Qt::OddEvenFill))
            {
                int inter_rank = getIntersectionRank( i_side, otherIsLeft, inter_infos );
                int other_rank = getIntersectionRank( side, isLeftHalf, other_inter_infos );
                qreal dist2    = Geo::dist2( intersection, sideMidPoint ) + Geo::dist2( intersection, otherMidPoint );
                infos << std::make_shared<EdgesLengthInfo>(side, isLeftHalf, i_side, otherIsLeft, inter_rank + other_rank, dist2, intersection);
            }
        }
    }

    return infos;
}


// Progressive intersect inferring.
QList<IntersectionPtr> IntersectMotif::buildIntersectionInfos(int side, QPointF sideHalf, bool isLeftHalf, qreal requiredRotation)
{
    Q_UNUSED(isLeftHalf)
    QList<IntersectionPtr> infos;

    QPointF sideMidPoint = mids[side];

    int side_count = corners.size();
    for ( int i_side = 0; i_side < side_count; ++i_side )
    {
        if ( i_side == side )
            continue;

        for ( int i_halves = 0; i_halves < 2; ++i_halves )
        {
            bool otherIsLeft = (i_halves == 0);
            QPointF otherMidPoint = mids[i_side];

            QPointF otherSide = buildGirihHalfBranch(i_side, otherIsLeft, requiredRotation);
            QPointF intersection;
            if (!Intersect::getIntersection( otherMidPoint, otherSide, sideMidPoint, sideHalf, intersection))
            {
                if ( Loose::zero( Geo::dist2ToLine( otherMidPoint, sideMidPoint, sideHalf ) ) )
                {
                    // Edge meets directly the other mid-points, so the distance is the middle in-between.
                    intersection = Geo::convexSum( otherMidPoint, sideMidPoint, 0.5 );
                }
            }
            qreal dist2 = Geo::dist2(intersection, sideMidPoint );
            infos << std::make_shared<IntersectionInfo>(side, i_side, otherIsLeft, dist2, intersection);
        }
    }

    // Sort edge-to-edge intersection by their total length.
    QMultiMap<qreal,IntersectionPtr> sortMap;
    for (int i= 0; i < infos.size(); i++)
    {
        sortMap.insert(infos[i]->dist2,infos[i]);   // insert is sorted on key
    }
    infos = sortMap.values();

    return infos;
}

/////////////////////////////////////////////////////////////////////
///
/// Information about the length of edges connecting two sides and the intersection point.
///
/////////////////////////////////////////////////////////////////////

EdgesLengthInfo::EdgesLengthInfo( int side1, bool isLeft1, int side2, bool isLeft2, int ic, qreal dist2, QPointF i )
{
    this->side1   = side1;
    this->isLeft1 = isLeft1;
    this->side2   = side2;
    this->isLeft2 = isLeft2;
    this->intersection_count = ic;
    this->dist2   = dist2;
    this->intersection = i;
}

void EdgesLengthInfo::dump()
{
    qDebug() << side1 << isLeft1 << side2 << isLeft2 << intersection_count << dist2 << intersection;
}

bool EdgesLengthInfo::equals( EdgesLengthInfo & other )
{
    int ic_diff = intersection_count - other.intersection_count;
    if ( ic_diff != 0 )
        return false;

    qreal diff = dist2 - other.dist2;
    return diff > -Sys::TOLSQ && diff < Sys::TOLSQ;
}

int EdgesLengthInfo::compareTo( EdgesLengthInfo & other )
{
    qreal diff = dist2 - other.dist2;
    if ( diff < -Sys::TOLSQ )
        return -1;
    if ( diff >  Sys::TOLSQ )
        return 1;

    int ic_diff = intersection_count - other.intersection_count;
    if ( ic_diff < 0 )
        return -1;
    else if ( ic_diff > 0 )
        return 1;

    return 0;
}

bool EdgesLengthInfo::lessThan(const EdgesLengthInfo* & e1, const EdgesLengthInfo*  & e2)
{
    return e1->dist2 < e2->dist2;
}
