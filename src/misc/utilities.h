#ifndef UTILITIES_H
#define UTILITIES_H

#include "geometry/loose.h"
#include "geometry/circle.h"

class Layer;
class EdgePoly;
class QGraphicsItem;

typedef std::shared_ptr<class Circle> CirclePtr;

class Utils
{
public:
    static QString  addr(void * address);
    static QString  addr(const void * address);
    static QString  str(QPointF pt);
    static QString  str(QSize sz);

    static void identify(Layer * layer, QPolygonF * poly);
    static int  circleLineIntersectionPoints(const QGraphicsItem & circle, qreal radius, const QLineF & line, QPointF & aa, QPointF & bb);
    static int  circleLineIntersectionPoints   (QPointF center,            qreal radius, const QLineF & line, QPointF & aa, QPointF & bb);
    static int  findLineCircleLineIntersections(QPointF centre,            qreal radius,       QLineF line, QPointF & intersection1, QPointF & intersection2);

    static bool pointInCircle(QPointF pt, CirclePtr c);
    static bool pointOnCircle(QPointF pt, CirclePtr c, qreal tolerance = Loose::NEAR_TOL);

    static int     circleIntersects(Circle c1, Circle c2);
    static QPointF circleTouchPt(Circle c0, Circle c1);
    static int     circleCircleIntersectionPoints(CirclePtr c1, CirclePtr c2, QPointF & p1, QPointF & p2);

    static QLineF  normalVectorP1(QLineF line);
    static QLineF  normalVectorP2(QLineF line);
    static QPointF getClosestPoint(QLineF line, QPointF p);
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

    static void reverseOrder(QPolygonF & poly);
    static void reverseOrder(EdgePoly & ep);

    static bool rectContains(const QRectF &rect, QPointF p);

private:
    static int      findLineCircleLineIntersections(qreal cx, qreal cy, qreal radius, QPointF point1, QPointF point2, QPointF & intersection1, QPointF & intersection2);
    static int      circleIntersects(qreal x1, qreal y1, qreal x2,  qreal y2, qreal r1, qreal r2);
    static void     circleTouchPt(qreal x0, qreal x1, qreal & x3,  qreal y0, qreal y1, qreal & y3, qreal r0, qreal r1);
    static QPointF  rotatePoint(QPointF fp, QPointF pt, qreal a);
    static qreal    acossafe(qreal x);
};

#endif
