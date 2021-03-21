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
// Point a geometrical map.

#ifndef POINT_DAC_H
#define POINT_DAC_H

#include <QtCore>
#include <QPointF>
#include <QPolygonF>

class EdgePoly;

class Point
{
public:
    static qreal TOLERANCE;
    static qreal TOLERANCE2;
    static QPointF ORIGIN;

    // Equality.
    static bool     isNear(const QPointF &pt, const QPointF &other);

    // Less-than compares x coordinates first, then y, using the default tolerance.
    static bool     lessThan(const QPointF & a, QPointF & other);

    // Useful maths on QPointFs.
    static qreal    mag2(QPointF & pt);
    static qreal    mag(QPointF & pt);
    static qreal    dist2(const QPointF &pt, const QPointF &other );
    static qreal    dist( QPointF pt, QPointF other );

    static QLineF   extendLine(QLineF line, qreal scale);

    static QLineF   clipLine(QLineF line,QPolygonF bounds);

    static bool     intersectPoly(QLineF line, QPolygonF bounds, QPointF &intersect);
 
    static QPointF  normalize(QPointF & pt);
    static void     normalizeD(QPointF & pt);

    static qreal    dot(QPointF & pt, QPointF & other );

    // Return the absolute angle of the edge from this to other, in the
    // range -PI to PI.
    static qreal    getAngle(QPointF & pt, QPointF & other );

    // Angle wrt the origin.
    static qreal    getAngle(QPointF & pt);

    // Return a vector ccw-perpendicular to this.libglu1-mesa-dev
    static QPointF  perp(QPointF & pt);
    static void     perpD(QPointF & pt);

    // Returns a point which is on a line between the two points at fraction t
    static QPointF  convexSum(QPointF pt, QPointF other, double t);
   
    static qreal    cross(QPointF & pt, QPointF & other);

    // Get the section of arc swept out between the edges this ==> from
    // and this ==> to.
    static qreal    sweep(QPointF joint, QPointF from, QPointF to);

    static qreal    distToLine(QPointF pt, QLineF line);
    static qreal    distToLine(QPointF pt, QPointF p, QPointF q);
    static qreal    dist2ToLine(QPointF pt,  QPointF p, QPointF q);

    static QPointF  center(QPolygonF & pts);
    static QPointF  center(EdgePoly & epoly);
    static QPolygonF recenter(QPolygonF pts, QPointF center);
};
#endif


