/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

////////////////////////////////////////////////////////////////////////////
//
// Infer.java
//
// Infer is the black magic part of the Islamic design system.  Given
// an empty, irregular feature, it infers a map for that feature by
// extending line segments from neighbouring features.  For many common
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

#include "tapp/Infer.h"
#include "tapp/Prototype.h"
#include "geometry/Loose.h"
#include "geometry/Point.h"
#include "geometry/Intersect.h"
#include "tapp/Star.h"
#include <algorithm>

using std::max;
using std::min;

// Transformed mid-points of a feature.
placed_points::placed_points(FeaturePtr feature, QTransform T, QPolygonF mids )
{
    this->feature = feature;
    this->mids    = mids;
    this->T       = T;
}


// Information about what feature and edge on that feature is adjacent to a given edge on a given feature.
adjacency_info::adjacency_info( FeaturePtr feature, int edge, QTransform T, qreal tolerance )
{
    this->feature   = feature;
    this->edge      = edge;
    this->T         = T;
    this->tolerance = tolerance;
}

// Information about intersection of a side left or right in-going edge with a given side right or left edge.
intersection_info::intersection_info()
{
    side = -1;
    dist2 = 1e100;
    intersection = QPointF();
    Q_ASSERT(intersection.isNull());
}

intersection_info::intersection_info( int side, int otherSide, bool otherIsLeft, qreal dist2, QPointF i )
{
    this->side         = side;
    this->otherSide    = otherSide;
    this->otherIsLeft  = otherIsLeft;
    this->dist2        = dist2;
    this->intersection = i;
}

bool  intersection_info::equals( intersection_info & other )
{
    qreal diff = dist2 - other.dist2;
    return diff > -Point::TOLERANCE2 && diff < Point::TOLERANCE2;
}

int  intersection_info::compareTo(  intersection_info & other )
{
    qreal diff = dist2 - other.dist2;
    return diff < -Point::TOLERANCE2 ? -1  : diff >  Point::TOLERANCE2 ?  1 : 0;
}

// Information about the length of edges connecting two sides and the intersection point.
edges_length_info::edges_length_info( int side1, bool isLeft1, int side2, bool isLeft2, int ic, qreal dist2, QPointF i )
{
    this->side1   = side1;
    this->isLeft1 = isLeft1;
    this->side2   = side2;
    this->isLeft2 = isLeft2;
    this->intersection_count = ic;
    this->dist2   = dist2;
    this->intersection = i;
}

bool edges_length_info::equals( edges_length_info & other )
{
    int ic_diff = intersection_count - other.intersection_count;
    if ( ic_diff != 0 )
        return false;

    qreal diff = dist2 - other.dist2;
    return diff > -Point::TOLERANCE2 && diff < Point::TOLERANCE2;
}

int edges_length_info::compareTo( edges_length_info & other )
{
    qreal diff = dist2 - other.dist2;
    if ( diff < -Point::TOLERANCE2 )
        return -1;
    if ( diff >  Point::TOLERANCE2 )
        return 1;

    int ic_diff = intersection_count - other.intersection_count;
    if ( ic_diff < 0 )
        return -1;
    else if ( ic_diff > 0 )
        return 1;

    return 0;
}

bool edges_length_info::lessThan(const edges_length_info* & e1, const edges_length_info*  & e2)
{
    return e1->dist2 < e2->dist2;
}


// The information about one point of contact on the boundary of the region being inferred.
contact::contact( QPointF position, QPointF other )
{
    this->position = position;
    this->other = other;

    QPointF pt = position - other;
    pt = Point::normalize(pt);
    pt *= 100.0;
    this->end = position + pt;
    this->isect = QPointF();
    this->taken = false;
    this->isect_idx = -1;
    this->colinear = COLINEAR_NONE;
}

Infer::Infer( PrototypePtr proto )
{
    tiling = proto->getTiling();
    if (!tiling)
    {
        qDebug() << "Infer::Infer = tiling is null";
        return;
    }

    qDebug() << "Infer::Infer proto=" << proto.get()  << "tiling=" << tiling.get();

    // Get a map for each feature in the prototype.
    QList<FeaturePtr> fpl = proto->getFeatures();
    for (int i=0; i < fpl.size(); i++)
    {
        FeaturePtr feature = fpl[i];
        FigurePtr  figure  = proto->getFigure(feature);
        Q_ASSERT(figure);
        qDebug() << "     figure=" <<  figure->getFigureDesc() << figure.get();
        maps.insert(feature, figure->getFigureMap());
    }

    // I'm going to generate all the tiles in the translational units
    // (x,y) where -1 <= x, y <= 1.  This is guaranteed to surround
    // every feature in the (0,0) unit by tiles.  You can then get
    // a sense of what other tiles surround a tile on every edge.

    int count = 0;
    for( int y = -1; y <= 1; ++y )
    {
        for( int x = -1; x <= 1; ++x )
        {
            count = add(x, y, count);     // add uses tiling
            qDebug() << x << y << count;
        }
    }
}

////////////////////////////////////////////////////////////////////////////
//
// Building a 3x3 tiling of the prototype.
//
// The next two routines create placed_points instances for all
// the tiles in the nine translational units generated above.

int Infer::add(int t1, int t2, int count)
{
    QPointF pt   = (tiling->getTrans1() * static_cast<qreal>(t1)) + (tiling->getTrans2() * static_cast<qreal>(t2));
    QTransform T = QTransform::fromTranslate(pt.x(),pt.y());

    QList<PlacedFeaturePtr> & pflist = tiling->getPlacedFeatures();
    for(auto it = pflist.begin(); it != pflist.end(); it++)
    {
        PlacedFeaturePtr pf = *it;
        QTransform Tf       = pf->getTransform();
        FeaturePtr feature  = pf->getFeature();
        count = add2(Tf * T, feature, count);
    }
    return count;
}

int Infer::add2(QTransform T, FeaturePtr feature, int count)
{
    QPolygonF fpts = T.map(feature->getPolygon());

    QPolygonF mids;
    int sz = feature->numPoints();
    for( int idx = 0; idx < sz; ++idx )
    {
        mids << Point::convexSum(fpts[idx], fpts[(idx+1)%sz], 0.5 );
    }

    placed << new placed_points(feature, T, mids);

    return count++;
}

////////////////////////////////////////////////////////////////////////////
//
// Choose an appropriate transform of the feature to infer, i.e.
// one that is surrounded by other features.  That means that we
// should just find an instance of that feature in the (0,0) unit.

int Infer::findPrimaryFeature( FeaturePtr feature )
{
    // The start and end of the tiles in the (0,0) unit.
    int start = tiling->countPlacedFeatures() * 4;
    int end   = tiling->countPlacedFeatures() * 5;
    int cur_reg_count = -1;
    int cur           = -1;

    for( int idx = start; idx < end; ++idx )
    {
        placed_points * pp = placed[ idx ];

        if (pp->feature == feature)
        {
            // Count the number of regular features surrounding this one,
            // in the case a feature is not always surrounded by the same
            // features, we want to select the one with teh most regulars.
            QVector<adjacency_info*> adjs = getAdjacencies( pp, idx );
            if ( adjs.isEmpty() )
            {
                continue;
            }

            int new_reg_count = 0;
            for ( int i = 0; i < adjs.size(); i++ )
            {
                if ( adjs[i] != nullptr )
                {
                    if ( adjs[i]->feature->isRegular() )
                    {
                        new_reg_count++;
                    }
                }
            }
            if ( new_reg_count > cur_reg_count )
            {
                cur_reg_count = new_reg_count;
                cur = idx;
            }
        }
    }

    if( cur == -1 )
    {
        qFatal("Couldn't find feature in (0,0) unit!");
    }
    return cur;
}

// For this edge of the feature being inferred, find the edges of neighbouring features and store.

adjacency_info * Infer::getAdjacency( QPointF main_point, int main_idx )
{
    static bool debug = false;

    if (debug) qDebug() << "Searching for adjacency for " << main_point;

    for (qreal tolerance = 1e-12; tolerance < 5.0; tolerance *= 2 )
    {
        for( int idx = 0; idx < placed.size(); ++idx )
        {
            if( idx == main_idx )
            {
                continue;
            }

            placed_points * pcur = placed[ idx ];
            for( int ov = 0; ov < pcur->mids.size(); ++ov )
            {
                if( Loose::Near( pcur->mids[ ov ], main_point, tolerance ) )
                {
                    if (debug) qWarning() << "Found with tolerance " << tolerance ;
                    return new adjacency_info( pcur->feature, ov, pcur->T, tolerance );
                }
            }
        }
    }

    return nullptr;
}

QVector<adjacency_info *> Infer::getAdjacencies( placed_points * pp, int main_idx )
{
    QVector<adjacency_info *> ret;

    for( int v = 0; v < pp->mids.size(); ++v )
    {
        adjacency_info * ai = getAdjacency( pp->mids[ v ], main_idx );
        if (ai != nullptr)
        {
            ret.push_back(ai);
        }
    }
    return ret;
}


// Take the adjacencies and build a list of contacts by looking at
// vertices of the maps for the adjacent features that lie on the
// boundary with the inference region.

QVector<contact *> Infer::buildContacts( placed_points * pp, QVector<adjacency_info *> & adjs )
{
    QVector<contact*> ret;

    QPolygonF fpts = pp->T.map( pp->feature->getPolygon() );

    // Get the transformed map for each adjacent feature.  I'm surprised
    // at how fast this ends up being!
    QVector<MapPtr> amaps;
    for( int idx = 0; idx < adjs.size(); ++idx )
    {
        if (adjs[ idx ] == nullptr)
        {
            amaps.push_back(make_shared<Map>());
        }
        else
        {
            MapPtr fig = maps.value( adjs[ idx ]->feature);
            if (fig)
            {
                MapPtr fig2 = fig->recreate();
                fig2->transformMap( adjs[ idx ]->T );
                amaps.push_back(fig2);
            }
            else
            {
                amaps.push_back(make_shared<Map>());    // DAC bugfix
            }
        }
    }
    Q_ASSERT(amaps.size());

    // Now, for each edge in the transformed feature, find a (hopefully
    // _the_) vertex in the adjacent map that lies on the edge.  When
    // a vertex is found, all (hopefully _both_) the edges incident
    // on that vertex are added as contacts.

    for( int idx2 = 0; idx2 < fpts.size(); ++idx2 )
    {
        QPointF a = fpts[idx2];
#if 1
        int  idx3 = (idx2+1) % fpts.size();
        QPointF b = fpts[idx3];
#else
        int  idx3 = (idx2+1);
        if(idx3 == fpts.size())
        {
            idx3=0; // DAC wraparound
        }
        QPointF b = fpts[idx3];
#endif
        if (a == b)
        {
            continue;   // DAC
        }

        MapPtr map = amaps[idx2];
        adjacency_info  * adj = adjs[idx2];

        const QVector<VertexPtr> * vertices = map->getVertices();
        foreach (VertexPtr v, *vertices)
        {
            QPointF pos = v->getPosition();
            qreal dist2 = Point::dist2ToLine(pos, a, b );
            if(         Loose::Near( dist2, adj->tolerance )
                  &&  ! Loose::Near( pos, a, adj->tolerance )
                  &&  ! Loose::Near( pos, b, adj->tolerance ) )
            {
                // This vertex lies on the edge.  Add all its edges to the contact list.
                QVector<EdgePtr> & qvep = v->getEdges();
                for (auto it = qvep.begin(); it != qvep.end(); it++)
                {
                    EdgePtr edge    = *it;
                    QPointF opos    = edge->getOtherP(v);
                    ret.push_back(new contact(pos, opos));
                }
            }
        }
    }
    return ret;
}

// Pseudo points around a circle inscribed in the figure, like those for
// regular radial figures. Of course, the figure being ierrgular, we
// instead interpolate betwwen mid-points.
//
// XTODO: use bezier interpolation instead of linear.

QPointF Infer::getArc(qreal frac, QPolygonF pts )
{
    while ( frac > 1.0 )
        frac -= 1.0;
    while ( frac < 0.0 )
        frac += 1.0;
    int indexPrev = (static_cast<int>(floor(pts.size() * frac + 0.01))) % pts.size();
    int indexNext = (static_cast<int>(ceil( pts.size() * frac - 0.01))) % pts.size();
    QPointF prev = pts[ indexPrev ];
    QPointF next = pts[ indexNext ];

    qreal pts_frac = pts.size() * frac - indexPrev;
    return Point::convexSum( prev,next, pts_frac );
}

// Star inferring.

QPolygonF  Infer::buildStarBranchPoints( qreal d, int s, qreal side_frac, qreal sign, QPolygonF mid_points)
{
    int side_count = mid_points.size();
    qreal circle_frac = 1.0 / static_cast<qreal>(side_count);

    qreal clamp_d = max( 1.0, min( d, 0.5 * side_count - 0.01 ) );

    int d_i = static_cast<int>(floor( clamp_d + 0.01));
    qreal d_frac = clamp_d - d_i;

    s = min( s, d_i );
    int outer_s = min( s, d_i - 1 );

    if( d_frac < Loose::TOL )
    {
        d_frac = 0.0;
    }
    else if( (1.0 - d_frac) < Loose::TOL )
    {
        d_frac = 0.0;
        d_i += 1;
    }

    QPointF a = getArc( side_frac, mid_points );
    QPointF b = getArc( side_frac + sign * clamp_d * circle_frac, mid_points );

    QPolygonF points;
    points << a;

    for( int idx = 1; idx <= outer_s; ++idx )
    {
        QPointF ar = getArc( side_frac + sign *  idx * circle_frac, mid_points );
        QPointF br = getArc( side_frac + sign * (idx - clamp_d) * circle_frac, mid_points );
        QPointF inter = Intersect::getIntersection( a, b, ar, br );
        // FIXMECSK: we should handle the concave case by extending the intersection.
        //        (After all, two lines will intersect if not parallel and two
        //         consecutive edges can hardly be parallel.)
        if ( !inter.isNull() )
        {
            points << inter;
        }
    }

    return points;
}

MapPtr Infer::buildStarHalfBranch( qreal d, int s, qreal side_frac, qreal sign, QPolygonF mid_points )
{
    MapPtr map = make_shared<Map>();
    QPolygonF points = buildStarBranchPoints( d, s, side_frac, sign, mid_points );

    VertexPtr vt   = map->insertVertex( points[0] );
    VertexPtr prev = vt;

    for( int idx = 1; idx < points.size(); ++idx )
    {
        VertexPtr next = map->insertVertex( points[ idx ] );
        map->insertEdge( prev, next );
        prev = next;
    }

    int side_count    = mid_points.size();
    qreal circle_frac = 1.0 / static_cast<qreal>(side_count);
    qreal clamp_d     = max( 1.0, min( d, 0.5 * side_count - 0.01));
    int d_i           = static_cast<int>(floor(clamp_d + 0.01));
    qreal d_frac      = clamp_d - d_i;
    s                 = min( s, d_i );

    if( s == d_i && sign > 0 )
    {
        QPolygonF next_branch_points = buildStarBranchPoints( d, s, side_frac + sign * circle_frac, sign, mid_points );
        QPointF midr = next_branch_points[ next_branch_points.size() - 1 ];

        if ( d_frac == 0.0 )
        {
            VertexPtr v4 = map->insertVertex( midr );
            map->insertEdge( prev, v4 );
        }
        else
        {
            QPointF ar   = getArc( side_frac + sign * d_i    * circle_frac, mid_points );
            QPointF br   = getArc( side_frac - sign * d_frac * circle_frac, mid_points );
            QPointF c    = getArc( side_frac + sign * d * circle_frac, mid_points );
            QPointF cent = Intersect::getIntersection( ar, br, points[0], c );
            if ( !cent.isNull() )
            {
                VertexPtr v4    = map->insertVertex( midr );
                VertexPtr vcent = map->insertVertex( cent );
                map->insertEdge( prev, vcent );
                map->insertEdge( vcent, v4 );
            }
        }
    }

    return map;
}

MapPtr Infer::inferStar( FeaturePtr feature, qreal d, int s )
{
    // Get the index of a good transform for this feature.
    int cur = findPrimaryFeature( feature );
    placed_points * pmain = placed[ cur ];
    QPolygonF mid_points  = pmain->mids;

    MapPtr map = make_shared<Map>();

    int side_count = mid_points.size();
    for ( int side = 0; side < side_count; ++side )
    {
        qreal side_frac = static_cast<qreal>(side) / static_cast<qreal>(side_count);

        map->mergeMap( buildStarHalfBranch( d, s, side_frac,  1.0, mid_points ));
        map->mergeMap( buildStarHalfBranch( d, s, side_frac, -1.0, mid_points ));
    }

    map->transformMap( pmain->T.inverted() );

    return map;
}

// Girih inferring.

QPointF Infer::buildGirihHalfBranch(int side, bool leftBranch, qreal requiredRotation, QPolygonF points, QPolygonF midPoints )
{
    int side_count = points.size();
    int next = (side + 1) % side_count;
    QTransform rot = Transform::rotateAroundPoint(
                midPoints[side],
                leftBranch ? -requiredRotation : requiredRotation );

    QPointF halfBranch = rot.map( points[ leftBranch ? side : next ] );
    halfBranch -= midPoints[side];
    halfBranch *= 32;
    halfBranch += midPoints[side];

    return halfBranch;
}

intersection_info Infer::FindClosestIntersection(int side, QPointF sideHalf, bool isLeftHalf, qreal requiredRotation, QPolygonF points, QPolygonF midPoints )
{
    intersection_info info;

    QPointF sideMidPoint = midPoints[side];

    bool otherIsLeft = !isLeftHalf;

    int side_count = points.size();
    for ( int i_side = 0; i_side < side_count; ++i_side )
    {
        if ( i_side == side )
            continue;

        QPointF otherMidPoint = midPoints[i_side];

        QPointF intersection;
        if ( Loose::zero(Point::dist2ToLine( otherMidPoint, sideMidPoint, sideHalf ) ) )
        {
            // Edge meets directly the other mid-points, so the distance is the shortest possible.
            intersection = Point::convexSum(otherMidPoint, sideMidPoint, 0.5 );
        }
        else
        {
            QPointF otherSide = buildGirihHalfBranch( i_side, otherIsLeft, requiredRotation, points, midPoints );
            intersection = Intersect::getIntersection( otherMidPoint, otherSide, sideMidPoint, sideHalf );
        }

        if (intersection.isNull())
        {
            continue;
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

MapPtr Infer::buildGirihBranch(int side, qreal requiredRotation, QPolygonF points, QPolygonF midPoints )
{
    MapPtr map = make_shared<Map>();

    // Mid-point of this side is always included in the map.
    VertexPtr midVertex  = map->insertVertex( midPoints[side] );

    // Find which other edge will intersect this one first.
    for ( int i_halves = 0; i_halves < 2; ++i_halves )
    {
        bool isLeftHalf = (i_halves == 0);

        // Find an intersection, if any.
        QPointF sideHalf       = buildGirihHalfBranch( side, isLeftHalf, requiredRotation, points, midPoints );
        intersection_info info = FindClosestIntersection( side, sideHalf, isLeftHalf, requiredRotation, points, midPoints );

        if ( info.intersection.isNull() )
            continue;

        VertexPtr interVertex = map->insertVertex( info.intersection );
        map->insertEdge( midVertex, interVertex );
    }

    return map;
}

MapPtr Infer::inferGirih( FeaturePtr feature, int starSides, qreal starSkip )
{
    // We use the number of side of the star and how many side it
    // hops over from branch to branch (where 1 would mean drawing
    // a polygon) and deduce the inner angle of the star branches.
    // We support fractional side skipping.
    qreal polygonInnerAngle    = M_PI * (starSides-2) / starSides;
    qreal starBranchInnerAngle = (starSkip * polygonInnerAngle) - ((starSkip - 1) * M_PI);
    qreal requiredRotation     = (M_PI - starBranchInnerAngle) / 2;

    // Get the index of a good transform for this feature.
    int cur               = findPrimaryFeature( feature );
    placed_points * pmain = placed[ cur ];
    QPolygonF mid_points  = pmain->mids;

    QPolygonF points;
    for ( int i = 0; i < pmain->mids.size(); ++i )
    {
        points <<  pmain->T.map( pmain->feature->getPoints()[i] );
    }

    MapPtr map = make_shared<Map>();

    int side_count = mid_points.size();
    for ( int side = 0; side < side_count; ++side )
    {
        MapPtr branchMap = buildGirihBranch(side, requiredRotation, points, mid_points);
        map->mergeMap(branchMap);     // merge even if empty (null) map
    }

    map->transformMap( pmain->T.inverted() );

    return map;
}

// Intersect inferring.

int Infer::getIntersectionRank(int side, bool isLeft,QList<intersection_info *> infos)
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

QList<edges_length_info *> Infer::buildIntersectEdgesLengthInfos(
        int side, QPointF sideHalf, bool isLeftHalf, qreal requiredRotation, QPolygonF points, QPolygonF midPoints )

{
    // First, get a list of intersections for this edge so that we can sort the
    // edge pairs by the fewest number of intersections.
    QList<intersection_info*> inter_infos = buildIntersectionInfos( side, sideHalf, isLeftHalf,
                                                              requiredRotation, points, midPoints );

    QList<edges_length_info*> infos;

    QPointF sideMidPoint = midPoints[side];

    int side_count = points.size();
    for ( int i_side = side + 1; i_side < side_count; ++i_side )
    {
        for ( int i_halves = 0; i_halves < 2; ++i_halves )
        {
            bool otherIsLeft      = (i_halves == 0);
            QPointF otherMidPoint = midPoints[i_side];
            QPointF otherSide     = buildGirihHalfBranch( i_side, otherIsLeft, requiredRotation, points, midPoints );
            QList<intersection_info*> other_inter_infos = buildIntersectionInfos( i_side, otherSide, otherIsLeft,
                                                          requiredRotation, points, midPoints );

            QPointF intersection = Intersect::getIntersection( otherMidPoint, otherSide, sideMidPoint, sideHalf );
            if ( intersection.isNull() )
            {
                // Lines are parallel, see if they actually point at each other.
                if ( Loose::zero( Point::dist2ToLine(otherMidPoint, sideMidPoint, sideHalf ) ) )
                {
                    // Edge meets directly the other mid-points, so the distance is the middle in-between.
                    intersection = Point::convexSum( otherMidPoint, sideMidPoint, 0.5 );
                }
            }
            if ( !intersection.isNull() )
            {
                if (points.containsPoint(intersection,Qt::OddEvenFill))
                {
                    int inter_rank = getIntersectionRank( i_side, otherIsLeft, inter_infos );
                    int other_rank = getIntersectionRank( side, isLeftHalf, other_inter_infos );
                    qreal dist2    = Point::dist2( intersection, sideMidPoint ) + Point::dist2( intersection, otherMidPoint );
                    infos << new edges_length_info( side, isLeftHalf, i_side, otherIsLeft, inter_rank + other_rank, dist2, intersection );
                }
            }
        }
    }

    return infos;
}

MapPtr Infer::inferIntersect( FeaturePtr feature, int starSides, qreal starSkip, int s )
{
    // We use the number of side of the star and how many side it
    // hops over from branch to branch (where 1 would mean drawing
    // a polygon) and deduce the inner angle of the star branches.
    // We support fractional side skipping.
    qreal polygonInnerAngle = M_PI * (starSides-2) / starSides;
    qreal starBranchInnerAngle = (starSkip * polygonInnerAngle) - (starSkip - 1) * M_PI;
    qreal requiredRotation = (M_PI - starBranchInnerAngle) / 2;

    // Get the index of a good transform for this feature.
    int cur = findPrimaryFeature( feature );
    placed_points * pmain = placed[ cur ];

    // Get the mid-points of each side of the feature.
    QPolygonF mid_points = pmain->mids;

    // Get the corners of the features.
    QPolygonF points;
    for ( int i = 0; i < pmain->mids.size(); ++i )
    {
        points << pmain->T.map( pmain->feature->getPoints()[i] );
    }

    // Accumulate all edge intersections and their length.
    QList<edges_length_info*> infos;

    int side_count = mid_points.size();
    for ( int side = 0; side < side_count; ++side )
    {
        for ( int i_halves = 0; i_halves < 2; ++i_halves )
        {
            bool isLeftHalf  = (i_halves == 0);
            QPointF sideHalf = buildGirihHalfBranch( side, isLeftHalf, requiredRotation, points, mid_points );
            QList<edges_length_info*> side_infos;
            side_infos = buildIntersectEdgesLengthInfos( side, sideHalf, isLeftHalf, requiredRotation, points, mid_points );
            infos += side_infos;
        }
    }

    // Sort edge-to-edge intersection by their total length.
#if 0
    Arrays.sort( infos, new Comparator<edges_length_info>()
    {
        public int compare( edges_length_info a, edges_length_info b )
                     { return a.compareTo( b ); } } );
#endif
    QMultiMap<qreal,edges_length_info*> sortMap;
    for (int i= 0; i < infos.size(); i++)
    {
        sortMap.insert(infos[i]->dist2,infos[i]);   // insert is sorted on key
    }
    infos = sortMap.values();

    MapPtr map = make_shared<Map>();

    // Record the starting point of each edge. As we grow the edge,
    // when we want more than one intersection (s > 1), we will update
    // these starting points.
    QPointF ** froms;
    froms = new QPointF*[side_count];
    for (int i=0; i<side_count; i++)
    {
        froms[i] = new QPointF[2];
    }

    for ( int i = 0; i < side_count; ++i )
    {
        froms[i][0] = mid_points[i];
        froms[i][1] = mid_points[i];
    }

    // Build the map using the shortest edges first.
    QVector<QVector<int>> counts(side_count,QVector<int>(2));    // thank you stack overflow
    for ( int i = 0; i < infos.size(); ++i )
    {
        edges_length_info * info = infos[i];
        int side1   = info->side1;
        int isLeft1 = info->isLeft1 ? 0 : 1;
        int side2   = info->side2;
        int isLeft2 = info->isLeft2 ? 0 : 1;
        if ( counts[side1][isLeft1] < s && counts[side2][isLeft2] < s )
        {
            QPointF from1 = froms[side1][isLeft1];
            QPointF from2 = froms[side2][isLeft2];
            VertexPtr m1  = map->insertVertex( from1 );
            VertexPtr m2  = map->insertVertex( from2 );
            if ( !info->intersection.isNull() )
            {
                VertexPtr inter = map->insertVertex( info->intersection );
                map->insertEdge( m1, inter );
                map->insertEdge( inter, m2 );
                froms[side1][isLeft1] = froms[side2][isLeft2] = info->intersection;
            }
            else
            {
                map->insertEdge( m1, m2 );
                froms[side1][isLeft1] = from2;
                froms[side2][isLeft2] = from1;
            }
            counts[side1][isLeft1] += 1;
            counts[side2][isLeft2] += 1;
        }
    }

    map->transformMap( pmain->T.inverted() );

    return map;
}

// Progressive intersect inferring.
QList<intersection_info*> Infer::buildIntersectionInfos(
    int side, QPointF sideHalf, bool isLeftHalf, qreal requiredRotation,
    QPolygonF points, QPolygonF midPoints )
{
    Q_UNUSED(isLeftHalf);
    QList<intersection_info*> infos;

    QPointF sideMidPoint = midPoints[side];

    int side_count = points.size();
    for ( int i_side = 0; i_side < side_count; ++i_side )
    {
        if ( i_side == side )
            continue;

        for ( int i_halves = 0; i_halves < 2; ++i_halves )
        {
            bool otherIsLeft = (i_halves == 0);
            QPointF otherMidPoint = midPoints[i_side];

            QPointF otherSide = buildGirihHalfBranch( i_side, otherIsLeft, requiredRotation, points, midPoints );
            QPointF intersection = Intersect::getIntersection( otherMidPoint, otherSide, sideMidPoint, sideHalf );
            if ( intersection.isNull() )
            {
                if ( Loose::zero( Point::dist2ToLine( otherMidPoint, sideMidPoint, sideHalf ) ) )
                {
                    // Edge meets directly the other mid-points, so the distance is the middle in-between.
                    intersection = Point::convexSum( otherMidPoint, sideMidPoint, 0.5 );
                }
            }
            if ( !intersection.isNull() )
            {
                qreal dist2 = Point::dist2(intersection, sideMidPoint );
                infos <<  new intersection_info( side, i_side, otherIsLeft, dist2, intersection );
            }
        }
    }

    // Sort edge-to-edge intersection by their total length.
    QMap<qreal,intersection_info*> sortMap;
    for (int i= 0; i < infos.size(); i++)
    {
        sortMap.insert(infos[i]->dist2,infos[i]);   // insert is sorted on key
    }
    infos = sortMap.values();

    return infos;
}

MapPtr Infer::inferIntersectProgressive( FeaturePtr feature, int starSides, qreal starSkip, int s )
{
    // We use the number of side of the star and how many side it
    // hops over from branch to branch (where 1 would mean drawing
    // a polygon) and deduce the inner angle of the star branches.
    // We support fractional side skipping.
     qreal polygonInnerAngle = M_PI * (starSides-2) / starSides;
     qreal starBranchInnerAngle = (starSkip * polygonInnerAngle) - (starSkip - 1) * M_PI;
     qreal requiredRotation = (M_PI - starBranchInnerAngle) / 2;

    // Get the index of a good transform for this feature.
    int cur = findPrimaryFeature( feature );
    placed_points * pmain = placed[ cur ];

    // Get the mid-points of each side of the feature.
    QPolygonF mid_points = pmain->mids;

    // Get the corners of the features.
    QPolygonF points;
    for ( int i = 0; i < pmain->mids.size(); ++i )
    {
        points << pmain->T.map( pmain->feature->getPoints()[i] );
    }

    MapPtr map = make_shared<Map>();

    int side_count = mid_points.size();
    for ( int side = 0; side < side_count; ++side )
    {
        for ( int i_halves = 0; i_halves < 2; ++i_halves )
        {
            bool isLeftHalf = (i_halves == 0);
            QPointF sideHalf = buildGirihHalfBranch( side, isLeftHalf, requiredRotation, points, mid_points );
            QList<intersection_info*> infos = buildIntersectionInfos( side, sideHalf, isLeftHalf, requiredRotation, points, mid_points );
            if ( !infos.isEmpty() )
            {
                // Record the starting point of the edge. As we grow the edge,
                // when we want more than one intersection (s > 1), we will update
                // this starting point.
                QPointF from = mid_points[side];

                // Build the map using the shortest edges first.
                int count = 0;
                for ( int i = 0; i < infos.size() && count < s; ++i )
                {
                    intersection_info * info = infos[i];
                    if ( !info->intersection.isNull())
                    {
                        VertexPtr p1  = map->insertVertex( from );
                        VertexPtr p2  = map->insertVertex( info->intersection );
                        map->insertEdge( p1, p2 );
                        from = info->intersection;
                        count++;
                    }
                }

            }
        }
    }

    map->transformMap( pmain->T.inverted() );

    return map;
}

// Hourglass inferring.

MapPtr Infer::inferHourglass( FeaturePtr feature, qreal d, int s )
{
    // Get the index of a good transform for this feature.
    int cur = findPrimaryFeature( feature );
    placed_points * pmain = placed[ cur ];
    QPolygonF mid_points = pmain->mids;

    MapPtr map = make_shared<Map>();

    int side_count = mid_points.size();

    // Fix the s value to be between [0, side_count / 2 - 1]
    // instead of [1, side_count / 2].
    int side_modulo;
    if ( (side_count & 1) != 0 )
    {
        side_modulo = side_count;
    }
    else
    {
        side_modulo = side_count / 2;
    }
    s = s % side_modulo;

    for ( int side = 0; side < side_count; ++side )
    {
        qreal side_frac = static_cast<qreal>(side) / static_cast<qreal>(side_count);
        qreal hour_d_pos = (side                 ) % side_modulo != s ? d : 1.0;
        qreal hour_d_neg = (side + side_count - 1) % side_modulo != s ? d : 1.0;
        map->mergeMap( buildStarHalfBranch( hour_d_pos, 1, side_frac,  1.0, mid_points ));
        map->mergeMap( buildStarHalfBranch( hour_d_neg, 1, side_frac, -1.0, mid_points ));
    }

    map->transformMap( pmain->T.inverted() );

    return map;
}

// Rosette inferring.

QPolygonF Infer::buildRosetteBranchPoints( qreal q, int s, qreal r,
                                                 qreal side_frac, qreal sign,
                                                 QPolygonF mid_points,
                                                 QPolygonF corner_points )
{
    Q_UNUSED(s);    // DAC
    int side_count = mid_points.size();
    qreal circle_frac = 1.0 / static_cast<qreal>(side_count);

    QPointF center = Point::center( mid_points );   // DAC - this algiorithm seemd daffy
    mid_points     = Point::recenter( mid_points, center );
    corner_points  = Point::recenter( corner_points, Point::center( corner_points ) );

    QPointF tip = getArc( side_frac, mid_points );                // The point to build from.
    QPointF rtip = getArc( side_frac + sign * circle_frac, mid_points ); // The next point over.

    qreal q_clamp = min(max( q, -0.99 ), 0.99 );
    //DAC int s_clamp   = min( s, side_count / 2 );

    // Consider an equilateral triangle formed by the origin,
    // up_outer and a vertical edge extending down from up_outer.
    // The center of the bottom edge of that triangle defines the
    // bisector of the angle leaving up_outer that we care about.
    QPointF up_outer = getArc( side_frac + circle_frac, corner_points );
    QPointF down_outer = getArc( side_frac, corner_points );
    if ( sign < 0.0 )
    {
        QPointF temp = up_outer;
        up_outer = down_outer;
        down_outer = temp;
    }

    QPointF bisector = down_outer * 0.5;
    bisector -= up_outer ;
    bisector *= 10.0;
    bisector += up_outer;

    QPointF e = Intersect::getIntersection( up_outer, bisector, tip, rtip );
    if ( e.isNull() )
        e = Point::convexSum(up_outer, QPointF(0,0), 0.5 );
    QPointF ad = Intersect::getIntersection( up_outer, bisector, tip, QPointF(0,0) );
    if ( ad.isNull())
        ad = QPointF(0,0);
    QPointF qe = q_clamp >= 0
            ? Point::convexSum(e, up_outer, q )
            : Point::convexSum(e, ad, -q );

    QPointF f = up_outer * r;

    QPolygonF points(3);
    points[TIP_POINT] = tip;
    points[QE_POINT]  = qe;
    points[F_POINT]   = f;

    return Point::recenter( points, center *  -1.0);
}

QPolygonF Infer::buildRosetteIntersections( qreal q, int s, qreal r,
                                                  qreal side_frac, qreal sign,
                                                  QPolygonF mid_points,
                                                  QPolygonF corner_points,
                                                  QPolygonF points )
{
    int side_count = mid_points.size();
    qreal circle_frac = 1.0 / static_cast<qreal>(side_count);

    QPolygonF intersections;

    QPointF qe_f = points[F_POINT] - points[QE_POINT];
    //for ( qreal other_sign = -1.0; other_sign <= 1.0 + Loose.TOL; other_sign += 2.0 ) {
    qreal other_sign = -1.0;
    {
        for ( int is = 1; is <= s + 1 && intersections.size() < s; ++is )
        {
            QPolygonF other_points = buildRosetteBranchPoints( q, s, r, side_frac + sign * circle_frac * is, other_sign * sign, mid_points, corner_points );
            QPointF other_qe_f = other_points[F_POINT] - other_points[QE_POINT];
            QPointF meet_f = Intersect::getIntersection(
                        points[QE_POINT], points[QE_POINT] + ( qe_f * 10.0),
                        other_points[QE_POINT], other_points[QE_POINT] + other_qe_f * 10.0 );
            if ( meet_f.isNull())
                continue;
            intersections << meet_f;
        }
    }

    QMap<qreal,QPointF> sortMap;
    for (int i=0; i < intersections.size(); i++)
    {
        sortMap.insert(Point::mag2(intersections[i]),intersections[i]);
    };
    QList<QPointF> sorted = sortMap.values();
    QPolygonF ret;
    for (int i=0; i < sorted.size(); i++)
    {
        ret << sorted[i];
    }
    return ret;
}


MapPtr Infer::buildRosetteHalfBranch( qreal q, int s, qreal r,
                                           qreal side_frac, qreal sign,
                                           QPolygonF mid_points,
                                           QPolygonF corner_points )
{
    //qDebug() << "Infer::buildRosetteHalfBranch";
    QPolygonF points        = buildRosetteBranchPoints(  q, s, r, side_frac, sign, mid_points, corner_points );
    //qDebug() << "points=" << points.size();
    //qDebug() << points;

    QPolygonF intersections = buildRosetteIntersections( q, s, r, side_frac, sign, mid_points, corner_points, points );
    //qDebug() << "intersections=" << intersections.size();

    MapPtr from = make_shared<Map>();

    VertexPtr prev;
    for( int idx = 0; idx < points.size() - 1; ++idx )      // DAC why -1 ?
    {
        VertexPtr next = from->insertVertex( points[ idx ] );
        if ( prev )
        {
            from->insertEdge( prev, next );
            //from->verify("from",true,true,true);
        }
        prev = next;
    }

    for ( int is = 0; is < s && is < intersections.size(); ++is )
    {
        VertexPtr next = from->insertVertex( intersections[is] );
        if ( prev )
            from->insertEdge( prev, next );
        prev = next;
    }

    //from->verify("half",true,true,true);
    return from;
}

MapPtr Infer::inferRosette( FeaturePtr feature, qreal q, int s, qreal r )
{
    // Get the index of a good transform for this feature.
    int cur = findPrimaryFeature( feature );
    placed_points * pmain = placed[ cur ];
    QPolygonF mid_points = pmain->mids;
    //qDebug() << "mid" << mid_points;
    QPolygonF corner_points = pmain->T.map( feature->getPolygon() );
    //qDebug() << "corner" << corner_points;

    MapPtr map = make_shared<Map>();

    int side_count = mid_points.size();
    for ( int side = 0; side < side_count; ++side )
    {
        qreal side_frac = static_cast<qreal>(side) / static_cast<qreal>(side_count);

        map->mergeMap( buildRosetteHalfBranch( q, s, r, side_frac,  1.0, mid_points, corner_points ));
        //map->verify("one",true);
        map->mergeMap( buildRosetteHalfBranch( q, s, r, side_frac, -1.0, mid_points, corner_points ));
        //map->verify("two",true);
    }

    map->transformMap( pmain->T.inverted() );
    //map->verify("three",true);
    return map;
}

// "Normal" magic inferring.
bool Infer::isColinear( QPointF p, QPointF q, QPointF a )
{
    qreal px = p.x();
    qreal py = p.y();

    qreal qx = q.x();
    qreal qy = q.y();

    qreal x = a.x();
    qreal y = a.y();

    qreal left = (qx-px)*(y-py);
    qreal right = (qy-py)*(x-px);

    return Loose::equals( left, right );
}

MapPtr Infer::infer( FeaturePtr feature )
{
    // Get the index of a good transform for this feature.
    int cur               = findPrimaryFeature( feature );
    placed_points * pmain = placed[ cur ];
    QPolygonF fpts        = pmain->T.map( pmain->feature->getPolygon() );

    QVector<adjacency_info *>  adjs = getAdjacencies( pmain, cur );
    QVector<contact *> cons         = buildContacts( pmain, adjs );

    // For every contact, if it hasn't found an extension,
    // Look at all other contacts for likely candidates.
    for( int idx = 0; idx < cons.size(); ++idx )
    {
        contact * con = cons.at( idx );
        if( con->taken )
        {
            continue;
        }

        int jbest = -1;

        QPointF bestisect;
        qreal bestdist = 0.0;
        int bestkind = INFER_NONE;

        for( int jdx = 0; jdx < cons.size(); ++jdx )
        {
            QPointF isect;

            if( jdx == idx )
            {
                continue;
            }

            contact * ocon =  cons.at(jdx);

            if( ocon->taken )
            {
                continue;
            }

            // Don't try on two contacts that involve the same vertex.
            if( Loose::equals( con->position, ocon->position ) )
            {
                continue;
            }

            qreal mydist = 0.0;
            int mykind = INFER_NONE;

            // First check if everybody's colinear.
            if( isColinear( con->other, con->position, ocon->position ) &&
                isColinear( con->position, ocon->position, ocon->other ))
            {

                // The two segments have to point at each other.
                QPointF d1 = con->position  - con->other;
                QPointF d2 = ocon->position - ocon->other;
                QPointF dc = con->position  - ocon->position;

                if( QPointF::dotProduct(d1,d2)> 0.0 )
                {
                    // They point in the same direction.
                    continue;
                }

                // The first segment must point at the second point.
                if( QPointF::dotProduct(d1,dc ) > 0.0 )
                {
                    continue;
                }
                if( QPointF::dotProduct(d2,dc ) < 0.0 )
                {
                    continue;
                }

                mykind = INSIDE_COLINEAR;
                mydist = Point::dist(con->position,ocon->position);
            }
            else
            {
                isect = Intersect::getTrueIntersection( con->position, con->end, ocon->position, ocon->end );
                if( !isect.isNull())
                {
                    // We don't want the case where the intersection
                    // lies too close to either vertex.  Note that
                    // I think these checks are subsumed by
                    // getTrueIntersection.
                    if( Loose::equals( isect, con->position ) )
                    {
                        continue;
                    }
                    if( Loose::equals( isect, ocon->position ) )
                    {
                        continue;
                    }

                    qreal dist = Point::dist(con->position,isect );
                    qreal odist = Point::dist(ocon->position,isect );

                    bool inside = fpts.containsPoint(isect,Qt::OddEvenFill);

                    if( !Loose::equals( dist, odist ) )
                    {
                        if( inside )
                        {
                            mykind = INSIDE_UNEVEN;
                        }
                        else
                        {
                            mykind = OUTSIDE_UNEVEN;
                        }
                        mydist = fabs( dist - odist );
                    }
                    else
                    {
                        if( inside )
                        {
                            mykind = INSIDE_EVEN;
                        }
                        else
                        {
                            mykind = OUTSIDE_EVEN;
                        }
                        mydist = dist;
                    }
                }
                else
                {
                    continue;
                }
            }

            if (lexCompareDistances( mykind, mydist, bestkind, bestdist ) < 0)
            {
                qDebug() <<  "New best:"  << con  << ocon;
                jbest = jdx;
                bestkind = mykind;
                bestdist = mydist;
                bestisect = isect;
            }
        }

        if( jbest == -1 )
        {
            qDebug() << "Couldn't find a match";
            continue;
        }

        contact * ocon = cons.at(jbest);
        con->taken = true;
        ocon->taken = true;

        if( bestkind == INSIDE_COLINEAR )
        {
            qDebug() << "best is colinear";
            con->colinear  = COLINEAR_MASTER;
            ocon->colinear = COLINEAR_SLAVE;
        }
        else
        {
            qDebug() << "isect: " << bestisect << con << ocon;
            con->isect  = bestisect;
            ocon->isect = bestisect;
        }

        con->isect_idx = jbest;
        ocon->isect_idx = idx;
    }

    // Using the stored intersections in the contacts,
    // build a  inferred map.

    MapPtr ret = make_shared<Map>();

    for( int idx = 0; idx < cons.size(); ++idx )
    {
        contact * con = cons.at(idx);
        if ( con->isect.isNull() )
        {
            if ( con->colinear == COLINEAR_MASTER )
            {
                MapPtr tmap = make_shared<Map>();
                VertexPtr v1 = tmap->insertVertex( con->position );
                VertexPtr v2 = tmap->insertVertex( cons.at( con->isect_idx)->position );
                tmap->insertEdge( v1, v2 );
                ret->mergeMap( tmap );
            }
        }
        else
        {
            MapPtr tmap = make_shared<Map>();
            VertexPtr v1 = tmap->insertVertex( con->position );
            VertexPtr v2 = tmap->insertVertex( con->isect );
            tmap->insertEdge( v1, v2 );
            ret->mergeMap( tmap );
        }

    }

    // Try to link up unlinked edges.
    qreal minlen = Point::dist(fpts[0], fpts[fpts.size()-1] );
    for( int idx = 1; idx < fpts.size(); ++idx )
    {
        minlen = min( minlen, Point::dist(fpts[idx-1], fpts[ idx ] ) );
    }

    for( int idx = 0; idx < cons.size(); ++idx )
    {
        contact * con = cons.at( idx );
        if ( con->isect_idx == -1 )
        {
            for( int jdx = 0; jdx < cons.size(); ++jdx )
            {
                if( jdx == idx )
                {
                    continue;
                }
                contact * ocon = cons.at( jdx );
                if( ocon->isect_idx != -1 )
                {
                    continue;
                }

                // Two unmatched edges.  match them up.
                MapPtr t = make_shared<Map>();

                QPointF tmp  = con->position - con->other;
                tmp = Point::normalize(tmp);
                tmp *= (minlen*0.5);
                QPointF ex1 = con->position + tmp;  // DAC hard to decipher the jave precedence rules used here

                tmp  = ocon->position - ocon->other;
                tmp = Point::normalize(tmp);
                tmp *= (minlen*0.5);
                QPointF ex2 = ocon->position + tmp; // ditto

                VertexPtr vx1 = t->insertVertex( ex1 );
                VertexPtr vx2 = t->insertVertex( ex2 );

                VertexPtr c = t->insertVertex( con->position );
                VertexPtr oc = t->insertVertex( ocon->position );

                t->insertEdge( c, vx1 );
                t->insertEdge( vx1, vx2 );
                t->insertEdge( vx2, oc );

                ret->mergeMap( t );

                con->isect_idx = jdx;
                ocon->isect_idx = idx;
            }
        }
    }

    ret->transformMap( pmain->T.inverted() );
    ret->verify("infer",false,true);
    return ret;
}

int Infer::lexCompareDistances( int kind1, qreal dist1, int kind2, qreal dist2 )
{
    if( kind1 < kind2 )
    {
        return -1;
    }
    else if( kind1 > kind2 )
    {
        return 1;
    }
    else
    {
        if( Loose::equals( dist1, dist2 ) )
        {
            return 0;
        }
        else if( dist1 < dist2 )
        {
            return -1;
        }
        else
        {
            return 1;
        }
    }
}

MapPtr Infer::inferFeature(FeaturePtr feature)
{
    MapPtr map = make_shared<Map>();

    EdgePoly epoly = feature->getEdgePoly();

    for (auto it = epoly.begin(); it != epoly.end(); it++)
    {
        // this makes new eges and vertices since they can get altered in the map
        EdgePtr edge = *it;
        VertexPtr v1 = map->insertVertex(edge->getV1()->getPosition());
        VertexPtr v2 = map->insertVertex(edge->getV2()->getPosition());
        EdgePtr newEdge = map->insertEdge(v1,v2);
        if (edge->getType() == EDGE_CURVE)
        {
            bool convex = edge->isConvex();
            QPointF ac  = edge->getArcCenter();
            newEdge->setArcCenter(ac,convex);
        }
    }
    map->verify("Feature-figure",true,true,true);

    return map;
}

#if 0
// debug_contacts is a quick helper class for viewing the transformed
// feature along with the contacts along its boundary.

class debug_contacts extends toolkit.GeoView
{
    QPolygonF pts;
    Vector<contact> contacts;

    debug_contacts( QPolygonF pts, Vector<contact> contacts )
    {
        super( -2.0, 2.0, 4.0 );
        setPreferredSize( new Dimension( 400, 400 ) );
        this.pts = pts;
        this.contacts = contacts;
    }

    public void redraw( toolkit.GeoGraphics gg )
    {
        gg.setColor( java.awt.Color.black );
        for( int idx = 0; idx < pts.length; ++idx ) {
            gg.drawLine( pts[ idx ], pts[ (idx+1) % pts.length ] );
        }
        gg.setColor( java.awt.Color.blue );
        for( int idx = 0; idx < contacts.size(); ++idx ) {
            contact c = (contact)( contacts.elementAt( idx ) );
            gg.drawLine( c.other, c.position );
        }
    }
}
#endif


