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

// Some routines for testing line segment intersections.

#ifndef INTERSECT_H
#define INTERSECT_H

#include <QPointF>
#include <QLineF>

class Intersect
{
public:
    // Get the position of the intersection by interpolating.
    // Returns null if parallel or if it ends up outside of the segments.
    static  QPointF getIntersection(QLineF line1, QLineF line2 );
    static  QPointF getIntersection(QPointF p1, QPointF q1, QPointF p2, QPointF q2 );

    // Get the position of the intersection by interpolating.
    // Returns null if parallel or if the point is too far off.
    static  QPointF getNearIntersection(QPointF &p1, QPointF &q1, QPointF &p2, QPointF &q2 );

    // Don't return the intersection if it is at the enpoints of both segments.
    static  QPointF getTrueIntersection( QPointF & p1, QPointF & q1, QPointF & p2, QPointF & q2 );

protected:
    // Return a point (s,t), where s is the fraction of the from p1 to
    // q1 where an intersection occurs.  t is defined similarly for p2 and q2.
    // If there's no intersection, return null.
    static QPointF getIntersectionParams(QPointF p1, QPointF q1, QPointF p2, QPointF q2 );

    // Coerce the point to be null if not on both segments.
    static QPointF stayOnSegments(QPointF ip );

    // Coerce the point to be null if too far off both segments.
    static  QPointF stayNearSegments(QPointF ip );
};

#endif

