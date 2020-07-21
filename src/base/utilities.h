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

#ifndef UTILITIES_H
#define UTILITIES_H

#include <QtCore>
#include "base/layer.h"
#include "geometry/circle.h"

class QGraphicsItem;

class Utils
{
public:
    static QString  addr(void * address);
    static QString  addr(const void * address);

    static void identify(Layer * layer, QPolygonF * poly);
    static int  circleLineIntersectionPoints(const QGraphicsItem & circle, qreal radius, const QLineF & line, QPointF & aa, QPointF & bb);
    static int  circleLineIntersectionPoints(QPointF center,               qreal radius, const QLineF & line, QPointF & aa, QPointF & bb);
    static int  findLineCircleLineIntersections(QPointF centre, qreal radius, QLineF line, QPointF & intersection1, QPointF & intersection2);

    static bool pointInCircle(QPointF pt, CirclePtr c);
    static bool pointOnCircle(QPointF pt, CirclePtr c);

    static int circleIntersects(CirclePtr c1, CirclePtr c2);
    static QPointF circleTouchPt(CirclePtr c0, CirclePtr c1);
    static int circleCircleIntersectionPoints(CirclePtr c1, CirclePtr c2, QPointF & p1, QPointF & p2);

    static QLineF  normalVectorP1(QLineF line);
    static QLineF  normalVectorP2(QLineF line);
    static QPointF getClosestPoint(QLineF line, QPointF p);

    static QVector<QLineF> rectToLines(QRectF & box);
    static QVector<QLineF> polyToLines(QPolygonF & poly);
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

private:
    static int      findLineCircleLineIntersections(qreal cx, qreal cy, qreal radius, QPointF point1, QPointF point2, QPointF & intersection1, QPointF & intersection2);
    static int      circleIntersects(qreal x1, qreal y1, qreal x2,  qreal y2, qreal r1, qreal r2);
    static void     circleTouchPt(qreal x0, qreal x1, qreal & x3,  qreal y0, qreal y1, qreal & y3, qreal r0, qreal r1);
    static QPointF  rotatePoint(QPointF fp, QPointF pt, qreal a);
    static qreal    acossafe(qreal x);
};

#endif
