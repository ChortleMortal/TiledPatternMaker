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

#include "geometry/intersect.h"
#include "geometry/loose.h"

bool Intersect::getIntersection(QLineF line1, QLineF line2, QPointF & intersect)
{
    return getIntersection(line1.p1(), line1.p2(), line2.p1(), line2.p2(), intersect);
}

bool Intersect::getIntersection( QPointF p1, QPointF q1, QPointF p2, QPointF q2, QPointF & intersect)
{
    // Get the position of the intersection by interpolating.
    // Returns null if parallel or if it ends up outside of the segments.

    QPointF isect;
    if (getIntersectionParams( p1, q1, p2, q2, isect) && stayOnSegments(isect))
    {
        intersect = Point::convexSum(p1, q1, isect.x());
        return true;
    }
    else
    {
        return false;
    }

}

bool Intersect::getTrueIntersection(QLineF  l1, QLineF  l2, QPointF &intersect)
{
    return getTrueIntersection(l1.p1(),l1.p2(),l2.p1(),l2.p2(),intersect);
}

bool Intersect::getTrueIntersection(QPointF p1, QPointF q1, QPointF p2, QPointF q2, QPointF & intersect)
{
    // Don't return the intersection if it is at the enpoints of both segments.

    QPointF isect;
    if (getIntersectionParams( p1, q1, p2, q2, isect) && stayOnSegments(isect))
    {
        qreal s = isect.x();
        qreal t = isect.y();

        if( s < Loose::TOL )
        {
            if( (t < Loose::TOL) || ((1.0-t) < Loose::TOL) )
            {
                return false;
            }
        }
        else if( (1.0-s) < Loose::TOL )
        {
            if( (t < Loose::TOL) || ((1.0-t) < Loose::TOL) )
            {
                return false;
            }
        }

        intersect =  Point::convexSum(p1, q1, s );
        return true;
    }
    else
    {
        return false;
    }
}

bool Intersect::getIntersectionParams(QPointF p1, QPointF q1, QPointF p2, QPointF q2 , QPointF & intersect)
{
    // Return a point (s,t), where s is the fraction of the from p1 to
    // q1 where an intersection occurs.  t is defined similarly for p2 and q2.
    // If there's no intersection, return null.

    qreal p1x = p1.x();
    qreal p1y = p1.y();

    qreal q1x = q1.x();
    qreal q1y = q1.y();

    qreal p2x = p2.x();
    qreal p2y = p2.y();

    qreal q2x = q2.x();
    qreal q2y = q2.y();

    qreal d1x = q1x - p1x;
    qreal d1y = q1y - p1y;
    qreal d2x = q2x - p2x;
    qreal d2y = q2y - p2y;

    qreal det = (d1x * d2y) - (d1y * d2x);

    if( Loose::zero( det ) )
    {
        // Parallel.  We won't worry about cases where endpoints touch
        // and we certainly won't worry about overlapping lines.
        // That leaves, um, nothing.  Done!
        return false;
    }

    // These two lines are adapted from O'Rourke's
    // _Computational Geometry in C_ segment-segment intersection code.
    qreal is = -((p1x*d2y) + p2x*(p1y-q2y) + q2x*(p2y-p1y)) / det;
    qreal it = ((p1x*(p2y-q1y)) + q1x*(p1y-p2y) + p2x*d1y) / det;

    intersect = QPointF( is, it );
    return true;
}

bool Intersect::stayOnSegments(QPointF ip)
{
    // Coerce the point to be null if not on both segments.

    QPointF null;
    if ( ip == null )
        return false;

    qreal is = ip.x();
    if( (is < -Loose::TOL) || (is > (1.0 + Loose::TOL)) )
        return false;

    qreal it = ip.y();
    if( (it < -Loose::TOL) || (it > (1.0 + Loose::TOL)) )
        return false;

    return true;
}


