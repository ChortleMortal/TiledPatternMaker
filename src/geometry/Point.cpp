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

#include "geometry/point.h"
#include "geometry/edgepoly.h"
#include "geometry/loose.h"
#include "base/shared.h"

// Point in a geometrical map.

qreal Point::TOLERANCE = 1.0E-7;
qreal Point::TOLERANCE2 = TOLERANCE * TOLERANCE;

QPointF Point::ORIGIN = QPointF(0.0,0.0);


// Equality.

bool Point::isNear(const QPointF &pt,  const QPointF & other )
{
    return (dist2(pt,other) < 49.0);
}

// Useful maths on QPointFs.

qreal Point::mag2(QPointF & pt)
{
    return pt.x() * pt.x() + pt.y() * pt.y();
}

qreal Point::mag(QPointF & pt)
{
    return qSqrt( mag2(pt) );
}

qreal Point::dist2(const QPointF & pt, const QPointF & other )
{
    qreal dx = pt.x() - other.x();
    qreal dy = pt.y() - other.y();

    return dx * dx + dy * dy;
}

qreal Point::dist( QPointF pt, QPointF other )
{
    return qSqrt( dist2(pt,other));
}

// DAC - this is probably same as Kaplan's convex sum
QLineF Point::extendLine(QLineF line, qreal scale)
{
    qreal len = line.length() * scale;
    line.setLength(len);
    return line;
}

bool Point::intersectPoly(QLineF line, QPolygonF bounds)
{
    if (!bounds.isClosed())
    {
        bounds << bounds[0];
    }
    Q_ASSERT(bounds.isClosed());

    for (int i=0; i < (bounds.size()-1); i++)
    {
        QPointF intersect;
        QLineF l2(bounds[i],bounds[i+1]);
#if (QT_VERSION >= QT_VERSION_CHECK(5,15,0))
        QLineF::IntersectType it = line.intersects(l2,&intersect);
#else
        QLineF::IntersectType it = line.intersect(l2,&intersect);
#endif
        if (it == QLineF::BoundedIntersection)
        {
            return true;
        }

        if (it == QLineF::UnboundedIntersection)
        {
            if (Loose::equals(intersect,line.p1()) || Loose::equals(intersect,line.p2()))
            {
                return true;
            }
        }
    }

    return false;
}

QLineF Point::clipLine(QLineF line,QPolygonF bounds)
{
    if (!bounds.isClosed())
    {
        bounds << bounds[0];
    }

    QPointF clipped;
    for (int i=0; i < (bounds.count()-1); i++)
    {
        QLineF l2 = QLineF(bounds.at(i), bounds.at(i+1));
#if (QT_VERSION >= QT_VERSION_CHECK(5,15,0))
        if (line.intersects(l2,&clipped) == QLineF::BoundedIntersection)
#else
        if (line.intersect(l2,&clipped) == QLineF::BoundedIntersection)
#endif
        {
            line.setP2(clipped);
            return line;
        }
    }
    return line;
}

QPointF Point::normalize(QPointF & pt)
{
    qreal m = mag(pt);
    if( m != 0.0 )
    {
        qreal mult = 1.0 / m;
        QPointF apt = pt * mult;
        return apt;
    }
    else
    {
        return pt;
    }
}

void Point::normalizeD(QPointF & pt)
{
    double m = mag(pt);
    if( m != 0.0 )
    {
        pt *= ( 1.0 / m );
    }
}

qreal Point::dot(QPointF & pt, QPointF & other )
{
    return pt.x() * other.x() + pt.y() * other.y();
}

// Return the absolute angle of the edge from this to other, in the
// range -PI to PI.
qreal Point::getAngle(QPointF & pt, QPointF & other )
{
    return qAtan2( other.y() - pt.y(), other.x() - pt.x() );
}

// Angle wrt the origin.
qreal Point::getAngle(QPointF & pt)
{
    return qAtan2( pt.y(), pt.x() );
}

QPointF Point::perp(QPointF & pt)
{
    // Return a vector ccw-perpendicular to this.libglu1-mesa-dev
    return QPointF( -pt.y(), pt.x() );
}

void Point::perpD(QPointF & pt)
{
    qreal x = pt.x();
    qreal y = pt.y();
    pt.setX(-y);
    pt.setY(x);
}

QPointF Point::convexSum(QPointF pt, QPointF other, double t )
{
    qreal mt = 1.0 - t;
    return QPointF( (mt * pt.x()) + (t * other.x()), (mt * pt.y()) + (t * other.y()) );
}


qreal Point::cross(QPointF & pt, QPointF & other )
{
    return (pt.x() * other.y()) - (pt.y() * other.x());
}

// Get the section of arc swept out between the edges this ==> from
// and this ==> to.
qreal Point::sweep(QPointF joint, QPointF from, QPointF to )
{
    qreal afrom = getAngle(joint, from );
    qreal ato = getAngle( joint,  to );

    qreal res = ato - afrom;

    while( res < 0.0 )
    {
        res += 2.0 * M_PI;
    }

    return res;
}

qreal Point::distToLine(QPointF pt, QLineF line)
{
    return qSqrt( dist2ToLine(pt, line.p1(), line.p2() ) );
}

qreal Point::distToLine(QPointF pt, QPointF p, QPointF q )
{
    return qSqrt( dist2ToLine(pt, p, q ) );
}

qreal Point::dist2ToLine(QPointF pt,  QPointF p, QPointF q )
{
    QPointF qmp = q  - p;
    QPointF apt = pt - p;
    qreal t = dot(apt,qmp ) / dot(qmp,qmp );
    if( t >= 0.0 && t <= 1.0 )
    {
        double ox = p.x() + t * ( q.x() - p.x() );
        double oy = p.y() + t * ( q.y() - p.y() );
        return ((pt.x()- ox)*(pt.x()-ox)) + ((pt.y()-oy)*(pt.y()-oy));
    }
    else if( t < 0.0 )
    {
        return dist2(p,pt);
    }
    else
    {
        return dist2( q,pt );
    }
}

QPointF Point::center(QPolygonF & pts )
{
    QPointF cent;
    int count = 0;
    for( int i = 0; i < pts.size(); ++i )
    {
        if ( !pts[i].isNull())
        {
            cent += pts[i];
            count++;
        }
    }

    cent = cent / static_cast<qreal>(count);
    return cent;
}

QPointF Point::center(EdgePoly & epoly)
{
    QPointF accum;
    int count = epoly.size();
    for( int i = 0; i < count; ++i )
    {
        accum += epoly[i]->getV1()->getPosition();
    }
    QPointF cent = accum / static_cast<qreal>(count);
    return cent;
}

QPolygonF Point::recenter( QPolygonF pts, QPointF center )
{
    QPolygonF new_pts(pts.size());

    if ( pts.isEmpty() )
        return new_pts;

    if ( center.isNull() )
        return pts;

    for( int i = 0; i < pts.size(); ++i )
    {
        if ( !pts[i].isNull() )
        {
            new_pts[i] = pts[i] - center;
        }
    }
    return new_pts;
}

