#ifndef INTERSECT_H
#define INTERSECT_H

#include <QPointF>
#include <QLineF>

// Some routines for testing line segment intersections.

class Intersect
{
public:
    // Get the position of the intersection by interpolating.
    // Returns null if parallel or if it ends up outside of the segments.
    static  bool getIntersection(QLineF line1, QLineF line2, QPointF & intersect );
    static  bool getIntersection(QPointF p1, QPointF q1, QPointF p2, QPointF q2 , QPointF &intersect);

    // Don't return the intersection if it is at the enpoints of both segments.
    static  bool getTrueIntersection(QLineF  l1, QLineF  l2, QPointF &intersect);
    static  bool getTrueIntersection(QPointF p1, QPointF q1, QPointF p2, QPointF q2 , QPointF &intersect);

protected:
    // Return a point (s,t), where s is the fraction of the from p1 to
    // q1 where an intersection occurs.  t is defined similarly for p2 and q2.
    // If there's no intersection, return null.
    static bool getIntersectionParams(QPointF p1, QPointF q1, QPointF p2, QPointF q2, QPointF & intersect);

    // Coerce the point to be null if not on both segments.
    static bool stayOnSegments(QPointF ip );
};

#endif

