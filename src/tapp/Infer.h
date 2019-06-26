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

#ifndef INFER_H
#define INFER_H

#include <QtCore>
#include "geometry/Transform.h"
#include "geometry/Point.h"
#include "geometry/Map.h"
#include "tile/Tiling.h"
#include "base/tile.h"

// Transformed mid-points of a feature.
class placed_points
{
public:
    placed_points( FeaturePtr feature, Transform T, QPolygonF mids );

    FeaturePtr     feature;
    Transform       T;

    // The transformed midpoints of the feature's edges.
    // Since the tilings are edge to edge, a tile edge can
    // be uniquely identified by its midpoint.  So we can
    // compare two edges by just comparing their midpoints
    // instead of comparing the endpoints pairwise.
    QPolygonF     mids;
};

// Information about what feature and edge on that feature is adjacent
// to a given edge on a given feature.
class adjacency_info
{
public:
    adjacency_info( FeaturePtr feature, int edge, Transform T, qreal tolerance );

    FeaturePtr  feature;
    int         edge;
    Transform   T;
    qreal tolerance;

};

// Information about intersection of a side left or right in-going edge
// with a given side right or left edge.
class intersection_info
{
public:
    intersection_info();
    intersection_info(int side, int otherSide, bool otherIsLeft, qreal dist2, QPointF i );

    bool equals( intersection_info & other );
    int compareTo( intersection_info & other );

    int     side;           // Which side of the feature this describes.
    int     otherSide;      // Which other side it meets.
    bool    otherIsLeft;    // True if the other side is the left edge.
    qreal   dist2;          // The square of the distance (square to avoid a square root op.)
    QPointF intersection;   // The intersection point.
};

// the intersection point.

class edges_length_info
{
public:
    edges_length_info( int side1, bool isLeft1, int side2, bool isLeft2, int ic, qreal dist2, QPointF i );

    bool equals  ( edges_length_info & other );
    int compareTo( edges_length_info & other );

    static bool lessThan(const edges_length_info *&e1, const edges_length_info *&e2);

    int     side1;          // Which side of the feature this describes.
    bool    isLeft1;        // True if first side is left edge.
    int     side2;          // Which side of the feature this describes.
    bool    isLeft2;        // True if second side is left edge.
    int     intersection_count;
    qreal   dist2;          // The square of the distance (square to avoid a square root op.)
    QPointF intersection;   // The intersection point.
};

// The information about one point of contact on the boundary of the
// region being inferred.

class contact
{
#define COLINEAR_NONE   0
#define COLINEAR_MASTER 1
#define COLINEAR_SLAVE  2

public:
    contact( QPointF position, QPointF other );

    QString toString();

    QPointF     position;
    QPointF		other;
    QPointF 	end;

    QPointF 	isect;
    int			isect_idx;

    int 		colinear;

    bool        taken;
};



#define TIP_POINT 0
#define QE_POINT  1
#define F_POINT   2

// The different kinds of connections that can be made between
// contacts, in increasing order of badness.  This is used to
// compare two possible connections.

class Infer
{
public:

    Infer( PrototypePtr proto );   // Creation

    MapPtr infer( FeaturePtr feature );                                 // "Normal" magic inferring
    MapPtr inferRosette( FeaturePtr feature, qreal q, int s, qreal r ); // Rosette inferring
    MapPtr inferHourglass( FeaturePtr feature, qreal d, int s );        // Hourglass inferring
    MapPtr inferIntersectProgressive( FeaturePtr feature, int starSides, qreal starSkip, int s ); // Progressive intersect inferring
    MapPtr inferIntersect( FeaturePtr feature, int starSides, qreal starSkip, int s ); // Intersect inferring
    MapPtr inferGirih( FeaturePtr feature, int starSides, qreal starSkip ); // Girih inferring
    MapPtr inferStar( FeaturePtr feature, qreal d, int s );             // Star inferring
    MapPtr inferFeature(FeaturePtr feature);                            // make a figure from the tiling itself

private:

#define INSIDE_EVEN 	0
#define INSIDE_COLINEAR 1
#define INSIDE_UNEVEN 	2
#define OUTSIDE_EVEN 	3
#define OUTSIDE_UNEVEN 	4
#define INFER_NONE 		5

    // Building a 3x3 tiling of the prototype.
    // The next two routines create placed_points instances for all
    // the tiles in the nine translational units generated above.
    int add(int t1, int t2, int count);
    int add2(Transform T, FeaturePtr feature, int count);

    // Choose an appropriate transform of the feature to infer, i.e.
    // one that is surrounded by other features.  That means that we
    // should just find an instance of that feature in the (0,0) unit.

    int findPrimaryFeature( FeaturePtr feature );

    // For this edge of the feature being inferred, find the edges of
    // neighbouring features and store.

    adjacency_info *getAdjacency( QPointF main_point, int main_idx );
    QVector<adjacency_info *> getAdjacencies(placed_points *pp, int main_idx );

    // Take the adjacencies and build a list of contacts by looking at
    // vertices of the maps for the adjacent features that lie on the
    // boundary with the inference region.
    QVector<contact *> buildContacts(placed_points *pp, QVector<adjacency_info *> & adjs );

    // Pseudo points around a circle inscribed in the figure, like those for
    // regular radial figures. Of course, the figure being ierrgular, we
    // instead interpolate betwwen mid-points.
    //
    // XTODO: use bezier interpolation instead of linear.
    static QPointF getArc( qreal frac, QPolygonF pts );

    // Star inferring.
    static QPolygonF buildStarBranchPoints(qreal d, int s, qreal side_frac, qreal sign, QPolygonF mid_points);
    static MapPtr    buildStarHalfBranch ( qreal d, int s, qreal side_frac, qreal sign, QPolygonF mid_points );

    // Girih inferring.
    static QPointF buildGirihHalfBranch(int side, bool leftBranch, qreal requiredRotation, QPolygonF points, QPolygonF midPoints );
    static intersection_info FindClosestIntersection(int side, QPointF sideHalf, bool isLeftHalf, qreal requiredRotation, QPolygonF points, QPolygonF midPoints );
    static MapPtr buildGirihBranch(int side, qreal requiredRotation, QPolygonF points, QPolygonF midPoints );

    static int getIntersectionRank(int side, bool isLeft, QList<intersection_info *> infos);
    static QList<edges_length_info *> buildIntersectEdgesLengthInfos(
            int side, QPointF sideHalf, bool isLeftHalf, qreal requiredRotation, QPolygonF points, QPolygonF midPoints );


    // Progressive intersect inferring.
    static QList<intersection_info *> buildIntersectionInfos(
        int side, QPointF sideHalf, bool isLeftHalf, qreal requiredRotation,
        QPolygonF points, QPolygonF midPoints );



    // Rosette inferring.
    static QPolygonF buildRosetteBranchPoints( qreal q, int s, qreal r,
                                                     qreal side_frac, qreal sign,
                                                     QPolygonF mid_points,
                                                     QPolygonF corner_points );
    static QPolygonF buildRosetteIntersections( qreal q, int s, qreal r,
                                                      qreal side_frac, qreal sign,
                                                      QPolygonF mid_points,
                                                      QPolygonF corner_points,
                                                      QPolygonF points );

    static MapPtr buildRosetteHalfBranch( qreal q, int s, qreal r,
                                               qreal side_frac, qreal sign,
                                               QPolygonF mid_points,
                                               QPolygonF corner_points );

    // "Normal" magic inferring.
    static bool isColinear( QPointF p, QPointF q, QPointF a );

    static int lexCompareDistances(int kind1, qreal dist1, int kind2, qreal dist2 );

private:
    TilingPtr                 tiling;
    QMap<FeaturePtr,MapPtr>   maps;
    QVector<placed_points *>  placed;
};


#endif // INFER_H
