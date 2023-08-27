#pragma once
#ifndef POINT_DAC_H
#define POINT_DAC_H

#include <QPointF>
#include <QLineF>
#include <QPolygonF>

class Point
{
public:
    static qreal TOLERANCE;
    static qreal TOLERANCE2;
    static QPointF ORIGIN;

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

    static QPointF   reflectPoint(QPointF & p, QLineF & line);
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

    static QPointF  perpPt(QPointF A, QPointF B, QPointF C);

    static QPointF  findNearestPoint(const QVector<QPointF> & pts, const QPointF apt);
};
#endif


