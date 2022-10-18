#include <QDebug>
#include <QtMath>

#include "geometry/point.h"
#include "geometry/loose.h"

// Point in a geometrical map.

qreal Point::TOLERANCE = 1.0E-7;
qreal Point::TOLERANCE2 = TOLERANCE * TOLERANCE;

QPointF Point::ORIGIN = QPointF(0.0,0.0);

// Equality.
bool Point::isNear(const QPointF &pt,  const QPointF & other )
{
    return (dist2(pt,other) < 49.0);
}

// Less-than compares x coordinates first, then y, using the default tolerance.
bool Point::lessThan(const QPointF & a, QPointF & other)
{
   if (Loose::lessThan(a.x(), other.x()))
      return true;

   if (Loose::greaterThan(a.x(), other.x()))
      return false;

   if (Loose::lessThan(a.y(), other.y()))
      return true;

   return false;
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

bool Point::intersectPoly(QLineF line, const QPolygonF & bounds, QPointF & intersect)
{
    for (int i=0; i < (bounds.size()-1); i++)
    {
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
            if (Loose::equalsPt(intersect,line.p1()) || Loose::equalsPt(intersect,line.p2()))
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

QLineF Point::createLine(QLineF line, QPointF mid, qreal angle, qreal length)
{
    QPointF p1 = line.p1();
    QPointF p2 = line.p2();
    qreal angle2 = Point::getAngleRadians(p1,p2);

    qreal new_angle = angle2 - angle;
    qreal x2 = mid.x() + (length * qCos(new_angle));
    qreal y2 = mid.y() + (length * qSin(new_angle));

    qreal x1 = mid.x() - (length * qCos(new_angle));
    qreal y1 = mid.y() - (length * qSin(new_angle));

    return QLineF(QPointF(x1,y1), QPointF(x2,y2));
}

QPointF Point::normalize(QPointF & pt)
{
    qreal m = mag(pt);
    if (m != 0.0)
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
    if (m != 0.0)
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
qreal Point::getAngleRadians(QPointF pt, QPointF other )
{
    return qAtan2(other.y() - pt.y(), other.x() - pt.x());
}

qreal Point::getAngleDegrees(QPointF pt, QPointF other )
{
    qreal angle = qRadiansToDegrees(qAtan2(-(other.y() - pt.y()), other.x() - pt.x()));
    return (angle < 0 ? angle + 360.0 : angle);
}

// Angle wrt the origin.
qreal Point::getAngle(QPointF pt)
{
    return qAtan2( pt.y(), pt.x() );
}

QPointF Point::perp(QPointF pt)
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

QPointF Point::convexSum(const QPointF &pt, const QPointF &other, double t )
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
qreal Point::sweep(QPointF joint, QPointF from, QPointF to)
{
    qreal afrom = getAngleRadians(joint, from);
    qreal ato   = getAngleRadians(joint, to);
    qreal res   = ato - afrom;

    while (res < 0.0)
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

bool Point::isOnLine(QPointF pt, QLineF line)
{
    qreal d2 = dist2ToLine(pt, line.p1(), line.p2());
    return Loose::zero(d2);
}

QPointF Point::center(const QPolygonF & pts)
{
    QPointF cent;
    int count = (pts.isClosed()) ? pts.size() -1 : pts.size();
    for (int i = 0; i < count; i++)
    {
        cent += pts[i];
    }

    cent = cent / static_cast<qreal>(count);
    return cent;
}

QPointF Point::irregularCenter(QPolygonF & poly)
{
    QPointF centroid;
    double signedArea = 0.0;
    double x0 = 0.0; // Current vertex X
    double y0 = 0.0; // Current vertex Y
    double x1 = 0.0; // Next vertex X
    double y1 = 0.0; // Next vertex Y
    double a  = 0.0;  // Partial signed area

    // For all vertices in a loop
    auto prev = poly.constLast();
    for (const auto & next : poly)
    {
        x0 = prev.x();
        y0 = prev.y();
        x1 = next.x();
        y1 = next.y();
        a = x0*y1 - x1*y0;
        signedArea += a;
        centroid.setX(centroid.x() + ((x0 + x1)*a));
        centroid.setY(centroid.y() + ((y0 + y1)*a));
        prev = next;
    }

    signedArea *= 0.5;
    centroid /= (6.0*signedArea);

    return centroid;
}

/*
Uing: https://stackoverflow.com/questions/3306838/algorithm-for-reflecting-a-point-across-a-line
Given point (x1, y1) and a line that passes through (x2,y2) and (x3,y3), we can first define the line as y = mx + c, where:
slope m is (y3-y2)/(x3-x2)
y-intercept c is (x3*y2-x2*y3)/(x3-x2)
If we want the point (x1,y1) reflected through that line, as (x4, y4), then:
set d = (x1 + (y1 - c)*m)/(1 + m^2) and then:
x4 = 2*d - x1
y4 = 2*d*m - y1 + 2*c
*/

QPointF Point::reflectPoint(QPointF & p, QLineF & line)
{
    qreal x1 = p.x();
    qreal x2 = line.x1();
    qreal x3 = line.x2();
    qreal x4 = 0;
    qreal y1 = p.y();
    qreal y2 = line.y1();
    qreal y3 = line.y2();
    qreal y4 = 0;
    if (Loose::equals(x3,x2))
    {
        // reflect accross vertical line
        if (x1 < x2)
        {
            x4 = x2 + (x2-x1);
        }
        else
        {
            x4 = x2 - (x1-x2);
        }
        y4 =  y1;
    }
    else
    {
        qreal  m = (y3 - y2) / (x3 -x2);     // slope
        qreal  c = ((x3*y2) - (x2*y3)) / (x3-x2);
        qreal  d =( x1 + ((y1-c)*m)) / (1.0 + (m*m));

        x4 = (2*d) - x1;
        y4 = (2*d*m) - y1 + (2*c);
    }

    QPointF p4(x4,y4);

    qDebug() << p << line << p4;
    return p4;
}


QPolygonF Point::reflectPolygon(QPolygonF & p, QLineF & line)
{
    QPolygonF poly;
    for (auto pt : p)
    {
        auto pt2 = reflectPoint(pt,line);
        poly << pt2;
    }
    qDebug() << "Polygon reflected about" << line;
    return poly;
}

QPointF Point::perpPt(QPointF &A, QPointF &B, QPointF &C)
{
    // https://stackoverflow.com/questions/1811549/perpendicular-on-a-line-from-a-given-point
    qreal x1 = A.x();
    qreal y1 = A.y();
    qreal x2 = B.x();
    qreal y2 = B.y();
    qreal x3 = C.x();
    qreal y3 = C.y();
    qreal k = ((y2-y1) * (x3-x1) - (x2-x1) * (y3-y1)) / ( qPow((y2-y1),2.0) + qPow((x2-x1),2.0) );
    qreal x4 = x3 - k * (y2-y1);
    qreal y4 = y3 + k * (x2-x1);
    return QPointF(x4,y4);
}

QLineF Point::reversed(QLineF & line)
{
    return QLineF(line.p2(),line.p1());
}

QLineF Point::shiftParallel(QLineF bline, qreal offset)
{
    //https://stackoverflow.com/questions/2825412/draw-a-parallel-line
    qreal x1 = bline.p1().x();
    qreal y1 = bline.p1().y();
    qreal x2 = bline.p2().x();
    qreal y2 = bline.p2().y();
    qreal L = qSqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
    // This is the second line
    qreal x1p = x1 + offset * (y2-y1) / L;
    qreal x2p = x2 + offset * (y2-y1) / L;
    qreal y1p = y1 + offset * (x1-x2) / L;
    qreal y2p = y2 + offset * (x1-x2) / L;
    return QLineF(x1p,y1p,x2p,y2p);
}
