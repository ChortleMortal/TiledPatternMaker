#include <QDebug>
#include <QtMath>
#include "model/motifs/irregular_star_branches.h"
#include "model/motifs/motif.h"
#include "sys/geometry/intersect.h"
#include "sys/geometry/map.h"
#include "sys/geometry/geo.h"
#include "model/prototypes/prototype.h"
#include "model/tilings/tiling.h"

using std::make_shared;
using std::max;
using std::min;

IrregularStarBranches::IrregularStarBranches() : IrregularMotif()
{
}

IrregularStarBranches::IrregularStarBranches(const Motif & other) : IrregularMotif(other)
{
}

IrregularStarBranches::IrregularStarBranches(const IrregularStarBranches & other) : IrregularMotif(other)
{
}


MapPtr IrregularStarBranches::buildStarHalfBranchV1(qreal d, int s, qreal side_frac, int sign)
{
    MapPtr map = make_shared<Map>("star half branch map");

    QPolygonF points = buildStarBranchPointsV1(d, s, side_frac, sign);

    VertexPtr vt   = map->insertVertex(points[0]);
    VertexPtr prev = vt;

    for (int idx = 1; idx < points.size(); ++idx)
    {
        VertexPtr next = map->insertVertex(points[idx]);
        map->insertEdge(prev,next);
        prev = next;
    }

    int side_count    = mids.size();
    qreal circle_frac = 1.0 / static_cast<qreal>(side_count);
    qreal clamp_d     = max( 1.0, min( d, 0.5 * side_count - 0.01));
    int d_i           = static_cast<int>(floor(clamp_d + 0.01));
    qreal d_frac      = clamp_d - d_i;
    s                 = min( s, d_i );

    if( s == d_i && sign > 0 )
    {
        QPolygonF next_branch_points = buildStarBranchPointsV1( d, s, side_frac + sign * circle_frac, sign);
        QPointF midr = next_branch_points[next_branch_points.size() - 1];

        if ( d_frac == 0.0 )
        {
            VertexPtr v4 = map->insertVertex(midr);
            map->insertEdge(prev,v4);
        }
        else
        {
            QPointF ar   = getArc(side_frac + sign * d_i    * circle_frac, mids);
            QPointF br   = getArc(side_frac - sign * d_frac * circle_frac, mids);
            QPointF c    = getArc(side_frac + sign * d * circle_frac, mids);
            QPointF cent;
            if (Intersect::getIntersection( ar, br, points[0], c, cent))
            {
                VertexPtr v4    = map->insertVertex(midr);
                VertexPtr vcent = map->insertVertex(cent);
                map->insertEdge(prev,vcent);
                map->insertEdge(vcent,v4);
            }
        }
    }

    return map;
}

// Star inferring.
QPolygonF  IrregularStarBranches::buildStarBranchPointsV1(qreal d, int s, qreal side_frac, int sign)
{
    int side_count    = mids.size();
    qreal circle_frac = 1.0 / static_cast<qreal>(side_count);
    qreal clamp_d     = max( 1.0, min( d, 0.5 * side_count - 0.01 ) );
    int d_i           = static_cast<int>(floor(clamp_d + 0.01));
    s = min( s, d_i );
    int outer_s       = min( s, d_i - 1 );

#if 0
    qreal d_frac      = clamp_d - d_i;
    if( d_frac < Sys::TOL )
    {
        d_frac = 0.0;
    }
    else if( (1.0 - d_frac) < Sys::TOL )
    {
        d_frac = 0.0;
        d_i += 1;
    }
#endif

    QPointF a = getArc(side_frac, mids );
    QPointF b = getArc(side_frac + sign * clamp_d * circle_frac, mids);

    QPolygonF points;
    points << a;

    for( int idx = 1; idx <= outer_s; ++idx )
    {
        QPointF ar = getArc(side_frac + sign *  idx * circle_frac, mids);
        QPointF br = getArc(side_frac + sign * (idx - clamp_d) * circle_frac, mids);
        // FIXMECSK: we should handle the concave case by extending the intersection.
        //        (After all, two lines will intersect if not parallel and two
        //         consecutive edges can hardly be parallel.)
        QPointF inter;
        if (Intersect::getIntersection(a, b, ar, br, inter))
        {
            points << inter;
        }
    }

    return points;
}


MapPtr IrregularStarBranches::buildStarHalfBranchV2(qreal d, int s, int side, int sign)
{
    MapPtr map = make_shared<Map>("star half branch map");

    QVector<QPointF> points = buildStarBranchPointsV2(d, s, side, sign);

    VertexPtr vt   = map->insertVertex(points[0]);
    VertexPtr prev = vt;

    qDebug() << "n:" << getN() << "side:" << side << "points"  << points.size();
    for( int idx = 1; idx < points.size(); ++idx )
    {
        VertexPtr next = map->insertVertex(points[idx] );
        map->insertEdge(prev,next);
        prev = next;
    }
    if (points.size() == 0)
    {
        qCritical("I thought so");
    }

#if 0
    qreal clamp_d    = max( 1.0, min( d, 0.5 * getN() - 0.01));
    int di           = static_cast<int>(floor(clamp_d + 0.01));
    qreal dfrac      = clamp_d - di;
    s                = min(s, di);
#else
    int  di          = qFloor(d);          // di is equivalent to the mids index, equivalent to the edge index;
    qreal dfrac      = d - qreal(di);
    if ((1.0 - dfrac) < Sys::TOL)
    {
        di++;
    }
#endif


    if (s == di && sign == 1)
    {
        QVector<QPointF> next_branch_points = buildStarBranchPointsV2(d, s, side + 1, 1);
        QPointF endpt = next_branch_points.last();

        if ( dfrac == 0.0 )
        {
            VertexPtr endv = map->insertVertex(endpt);
            map->insertEdge(prev,endv);
        }
        else
        {
            QPointF ar   = getArc( side + sign * di, mids );
            QPointF br   = getArc( side - sign * dfrac, mids );
            QLineF arbr(ar,br);
            QPointF c    = getArc( side + sign * d, mids );
            QPointF isect;
            QLineF mc(points[0],c);
            if (Intersect::getIntersection(arbr, mc, isect))
            {
                VertexPtr endv  = map->insertVertex(endpt);
                VertexPtr vcent = map->insertVertex(isect);
                //map->insertEdge(prev, vcent);
                //map->insertEdge(vcent, endv);
            }
        }
    }
    return map;
}

QVector<QPointF> IrregularStarBranches::buildStarBranchPointsV2(qreal d, int s, int side, int sign)
{
    side = modulo(side, getN());

    qreal clamp_d = max( 1.0, min( d, 0.5 * getN() - 0.01 ) );

    int d_i = static_cast<int>(floor(clamp_d + 0.01));

    s = min( s, d_i );
    //int outer_s = min( s, d_i - 1 );

    QLineF branchRay = getRay(side,clamp_d,sign);
    qDebug() << branchRay.p1()  << branchRay.p2();
    if (motifDebug)
    {
        qreal angle = branchRay.angle();
        Sys::debugMapCreate->insertDebugMark(branchRay.p1(),QString("a%1").arg(side),Qt::red);
        Sys::debugMapCreate->insertDebugMark(branchRay.p2(),QString::number(angle),Qt::red);
        Sys::debugMapCreate->insertDebugLine(branchRay,Qt::blue);
    }
    QVector<QPointF> points;
    //points << mids[side];
    points << branchRay.p1();

    //for( int nside = 1; nside <= outer_s; ++nside )
    for( int nside = 1; nside <= s; ++nside )
    {
        int theside;
        if (sign == 1)
            theside = modulo(side+nside,getN());
        else
            theside = modulo(side-nside,getN());

        QLineF otherRay = getRay(theside,d,-sign);
        if (motifDebug)
        {
            qreal angle = otherRay.angle();
            Sys::debugMapCreate->insertDebugMark(otherRay.p1(),QString("a%1").arg(side),Qt::red);
            Sys::debugMapCreate->insertDebugMark(otherRay.p2(),QString::number(angle),Qt::red);
            Sys::debugMapCreate->insertDebugLine(otherRay,Qt::blue);
        }
        // FIXMECSK: we should handle the concave case by extending the intersection.
        //        (After all, two lines will intersect if not parallel and two
        //         consecutive edges can hardly be parallel.)
        QPointF isect;
        if (Intersect::getIntersection(branchRay, otherRay, isect))
        {
            points << isect;
            if (motifDebug)
            {
                Sys::debugMapCreate->insertDebugMark(isect,QString("isect%1").arg(side),Qt::red);
            }
        }
    }
    if (points.size() == 1)
    {
        // an edge case
        points << branchRay.p2();
    }
    return points;
}

// Pseudo points around a circle inscribed in the figure, like those for
// regular radial figures. Of course, the figure being ierrgular, we
// instead interpolate betwwen mid-points.
//
// XTODO: use bezier interpolation instead of linear.
QPointF IrregularStarBranches::getArc(qreal frac, const QPolygonF & pts )
{
    while ( frac > 1.0 )
        frac -= 1.0;
    while ( frac < 0.0 )
        frac += 1.0;
    int indexPrev = (static_cast<int>(floor(pts.size() * frac + 0.01))) % pts.size();
    int indexNext = (static_cast<int>(ceil( pts.size() * frac - 0.01))) % pts.size();
    QPointF prev = pts[indexPrev];
    QPointF next = pts[indexNext];

    qreal pts_frac = pts.size() * frac - indexPrev;
    auto pt = Geo::convexSum(prev,next, pts_frac);
    //qDebug() << "getArc" << "prev:" << indexPrev << "next:" << indexNext << "frac:" << pts_frac << "pt" << pt;
    return pt;
}

QLineF IrregularStarBranches::getRay(int side, qreal d, int sign)
{
    //qDebug() << "IrregularStar" << this << getN();

    side = modulo(side,getN());

    qreal d2;
    int di;
    qreal dfrac;
    if (sign == 1)
    {
        d2    = d  + side + 0.5;
        di    = qFloor(d2);          // di is equivalent to the mids index, equivalent to the edge index;
        dfrac = d2 - qreal(di);
    }
    else
    {
        d2    = getN() - d  + side + 0.5;
        di    = qFloor(d2);          // di is equivalent to the mids index, equivalent to the edge index;
        dfrac = d2 - qreal(di);
    }

    if (dfrac < Sys::TOL)
    {
        dfrac = 0.0;
    }
    else if ((1.0 - dfrac) < Sys::TOL)
    {
        dfrac    = 0.0;
        di++;
    }

    di = modulo(di,getN());
    int di2 = modulo(di+1,getN());

    QPointF   a =  mids[side];
    QLineF edge(corners[di],corners[di2]);
    QPointF   b =  edge.pointAt(dfrac);
    QLineF branchRay(a,b);

    //qDebug() << "Ray:" << "n" << getN() << "side:" << side << "sign:" << sign << "d" << d << "di" << di  << "dfrac"  << dfrac;

    return branchRay;
}
