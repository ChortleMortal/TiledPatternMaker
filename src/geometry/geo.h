#pragma once
#ifndef GEO_H
#define GEO_H

#include <QPointF>
#include <QLineF>
#include <QPolygonF>

#include "misc/sys.h"
#include "geometry/circle.h"

class Layer;
class EdgePoly;
class QGraphicsItem;

class Geo
{
public:
    static bool     isValid(QPointF & pt);

    // Equality.
    static bool     isNear(const QPointF &pt, const QPointF &other);

    // Less-than compares x coordinates first, then y, using the default tolerance.
    static bool     lessThan(const QPointF & a, QPointF & other);

    // Useful maths on QPointFs.
    static qreal    mag2(const QPointF &pt);
    static qreal    mag(const QPointF & pt);
    static qreal    dist2(const QPointF &pt, const QPointF &other );
    static qreal    dist( QPointF pt, QPointF other );

    // some line operations
    static QLineF   reversed(QLineF & line);
    static QLineF   shiftParallel(QLineF bline, qreal offset);
    static QLineF   extendLine(QLineF line, qreal scale);
    static QLineF   extendAsRay(QLineF line, qreal scale);
    static QLineF   clipLine(QLineF line,QPolygonF bounds);
    static QLineF   createLine(QLineF line, QPointF mid, qreal angle, qreal length);

    static bool     intersectPoly(QLineF line, const QPolygonF &bounds, QPointF &intersect);
 
    static QPointF  normalize(QPointF & pt);
    static void     normalizeD(QPointF & pt);

    static qreal    dot(QPointF & pt, QPointF & other );

    static QPointF   reflectPoint(QPointF p, QLineF line);
    static QPolygonF reflectPolygon(QPolygonF &p, QLineF &line);

    // Return the absolute angle of the edge from this to other, in the range -PI to PI.
    static qreal    getAngleRadians(QPointF pt, QPointF other);
    static qreal    getAngleDegrees(QPointF pt, QPointF other);

    // Angle wrt the origin.
    static qreal    getAngle(QPointF pt);

    // Return a vector ccw-perpendicular to this.libglu1-mesa-dev
    static QPointF  perp(QPointF pt);
    static void     perpD(QPointF & pt);

    // Returns a point which is on a line between the two points at fraction t
    static QPointF  convexSum(const QPointF &pt, const QPointF &other, double t);

    static qreal    cross(QPointF & pt, QPointF & other);

    // Get the section of arc swept out between the edges this ==> from and this ==> to.
    static qreal    sweep(QPointF joint, QPointF from, QPointF to);

    static qreal    distToLine(QPointF pt, QLineF line);
    static qreal    distToLine(QPointF pt, QPointF p, QPointF q);
    static qreal    dist2ToLine(QPointF pt,  QPointF p, QPointF q);
    static bool     isOnLine(QPointF pt, QLineF line);

    static QPointF  center(const QPolygonF & pts);
    static QPointF  irregularCenter(const QPolygonF &poly);

    static QPointF  perpPt(QLineF line, QPointF C);
    static QPointF  perpPt(QPointF A, QPointF B, QPointF C);

    static QPointF  findNearestPoint(const QVector<QPointF> & pts, const QPointF apt);

    static int  circleLineIntersectionPoints(const QGraphicsItem & circle, qreal radius, const QLineF & line, QPointF & aa, QPointF & bb);
    static int  circleLineIntersectionPoints(QPointF center,               qreal radius, const QLineF & line, QPointF & aa, QPointF & bb);
    static int  findLineCircleIntersections(QPointF centre,                qreal radius,       QLineF   line, QPointF & intersection1, QPointF & intersection2);

    static bool pointInCircle(QPointF pt, Circle c);
    static bool pointOnCircle(QPointF pt, Circle c, qreal tolerance = Sys::NEAR_TOL);

    static int     circleIntersects(Circle c1, Circle c2);
    static QPointF circleTouchPt(Circle c0, Circle c1);
    static int     circleCircleIntersectionPoints(Circle c1, Circle c2, QPointF & p1, QPointF & p2);

    static QLineF  normalVectorP1(QLineF line);
    static QLineF  normalVectorP2(QLineF line);
    static QPointF getClosestPoint(QLineF line, QPointF p);
    static QPointF getClosestPointB(const QPointF & A, const QPointF & B, const QPointF & P);
    static QPointF getClosestPointC(const QPointF & A, const QPointF & B, const QPointF & P);
    static qreal   angle(const QLineF &l0,const QLineF &l);

    static QVector<QLineF> rectToLines(QRectF & box);
    static QVector<QLineF> polyToLines(const QPolygonF &poly);
    static bool            pointOnLine(QLineF l, QPointF p);
    static bool            pointAtEndOfLine(QLineF l, QPointF p);
    static QPointF         snapTo(QPointF to, QPointF from, int precision = 2);
    static bool            canSnapTo(QPointF to, QPointF from, int precision = 2);
    static QPointF         snapTo(QPointF pt, QLineF trackLine);
    static qreal           calcArea(QPolygonF & poly);

    static bool            isClockwise(const QPolygonF &poly);
    static bool            isClockwiseKaplan(QPolygonF &  poly);
    static bool            pointInPolygon(const QPointF & point, const QPolygonF & polygon);
    static bool            point_in_polygon(QPointF point, const QPolygonF & polygon);
    static bool            point_on_poly_edge(QPointF p, const QPolygonF & poly);

    static void reverseOrder(QPolygonF & poly);
    static void reverseOrder(EdgePoly & ep);

    static bool rectContains(const QRectF &rect, QPointF p);

    static bool isColinearAndTouching(const EdgePoly & ep1, const EdgePoly & ep2, qreal tolerance = Sys::NEAR_TOL);
    static bool isColinearAndTouching(const QLineF & l1, const QLineF & l2, qreal tolerance = Sys::NEAR_TOL);
    static bool isColinear(const QLineF & l1, const QLineF & l2, qreal tolerance);

private:
    static int      findLineCircleIntersections(qreal cx, qreal cy, qreal radius, QPointF point1, QPointF point2, QPointF & intersection1, QPointF & intersection2);
    static int      circleIntersects(qreal x1, qreal y1, qreal x2,  qreal y2, qreal r1, qreal r2);
    static void     circleTouchPt(qreal x0, qreal x1, qreal & x3,  qreal y0, qreal y1, qreal & y3, qreal r0, qreal r1);
    static QPointF  rotatePoint(QPointF fp, QPointF pt, qreal a);
    static qreal    acossafe(qreal x);
};
#endif


