#include <QDebug>
#include <QtMath>
#include <QGraphicsItem>

#ifdef __linux__
#include <cfloat>
#endif

#include "geometry/edge.h"
#include "geometry/edgepoly.h"
#include "geometry/geo.h"
#include "geometry/loose.h"
#include "geometry/vertex.h"

bool Geo::isValid(QPointF &pt)
{
    qreal x = pt.x();
    bool is_nan = (x != x);
    bool is_inf = (x < -DBL_MAX || x > DBL_MAX);
    if (is_nan || is_inf)
        return false;

    qreal y = pt.y();
    is_nan = (y != y);
    is_inf = (y < -DBL_MAX || y > DBL_MAX);
    if (is_nan || is_inf)
        return false;

    return true;
}

// Equality.
bool Geo::isNear(const QPointF &pt,  const QPointF & other )
{
    return (dist2(pt,other) < 49.0);
}

// Less-than compares x coordinates first, then y, using the default tolerance.
bool Geo::lessThan(const QPointF & a, QPointF & other)
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

qreal Geo::mag2(const QPointF & pt)
{
    return pt.x() * pt.x() + pt.y() * pt.y();
}

qreal Geo::mag(const QPointF &pt)
{
    return qSqrt( mag2(pt) );
}

qreal Geo::dist2(const QPointF & pt, const QPointF & other )
{
    qreal dx = pt.x() - other.x();
    qreal dy = pt.y() - other.y();

    return dx * dx + dy * dy;
}

qreal Geo::dist( QPointF pt, QPointF other )
{
    return qSqrt( dist2(pt,other));
}

// DAC - this is probably same as Kaplan's convex sum
QLineF Geo::extendLine(QLineF line, qreal scale)
{
    line.setLength(line.length() * scale);
    return line;
}

QLineF Geo::extendAsRay(QLineF line, qreal scale)
{
    qreal len = line.length();
    line.setLength(len * scale);
    QLineF l = reversed(line);
    l.setLength(2 * len * scale);
    return reversed(l);
}

bool Geo::intersectPoly(QLineF line, const QPolygonF & bounds, QPointF & intersect)
{
    Q_ASSERT(bounds.isClosed());
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

QLineF Geo::clipLine(QLineF line,QPolygonF bounds)
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

QLineF Geo::createLine(QLineF line, QPointF mid, qreal angle, qreal length)
{
    QPointF p1 = line.p1();
    QPointF p2 = line.p2();
    qreal angle2 = Geo::getAngleRadians(p1,p2);

    qreal new_angle = angle2 - angle;
    qreal x2 = mid.x() + (length * qCos(new_angle));
    qreal y2 = mid.y() + (length * qSin(new_angle));

    qreal x1 = mid.x() - (length * qCos(new_angle));
    qreal y1 = mid.y() - (length * qSin(new_angle));

    return QLineF(QPointF(x1,y1), QPointF(x2,y2));
}

QPointF Geo::normalize(QPointF & pt)
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

void Geo::normalizeD(QPointF & pt)
{
    double m = mag(pt);
    if (m != 0.0)
    {
        pt *= ( 1.0 / m );
    }
}

qreal Geo::dot(QPointF & pt, QPointF & other )
{
    return pt.x() * other.x() + pt.y() * other.y();
}

// Return the absolute angle of the edge from this to other, in the
// range -PI to PI.
qreal Geo::getAngleRadians(QPointF pt, QPointF other )
{
    return qAtan2(other.y() - pt.y(), other.x() - pt.x());
}

qreal Geo::getAngleDegrees(QPointF pt, QPointF other )
{
    qreal angle = qRadiansToDegrees(qAtan2(-(other.y() - pt.y()), other.x() - pt.x()));
    return (angle < 0 ? angle + 360.0 : angle);
}

// Angle wrt the origin.
qreal Geo::getAngle(QPointF pt)
{
    return qAtan2( pt.y(), pt.x() );
}

QPointF Geo::perp(QPointF pt)
{
    // Return a vector ccw-perpendicular to this.libglu1-mesa-dev
    return QPointF( -pt.y(), pt.x() );
}

void Geo::perpD(QPointF & pt)
{
    qreal x = pt.x();
    qreal y = pt.y();
    pt.setX(-y);
    pt.setY(x);
}

QPointF Geo::convexSum(const QPointF &pt, const QPointF &other, double t )
{
    qreal mt = 1.0 - t;
    return QPointF( (mt * pt.x()) + (t * other.x()), (mt * pt.y()) + (t * other.y()) );
}


qreal Geo::cross(QPointF & pt, QPointF & other )
{
    return (pt.x() * other.y()) - (pt.y() * other.x());
}

// Get the section of arc swept out between the edges this ==> from
// and this ==> to.
qreal Geo::sweep(QPointF joint, QPointF from, QPointF to)
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

qreal Geo::distToLine(QPointF pt, QLineF line)
{
    return qSqrt( dist2ToLine(pt, line.p1(), line.p2() ) );
}

qreal Geo::distToLine(QPointF pt, QPointF p, QPointF q )
{
    return qSqrt( dist2ToLine(pt, p, q ) );
}

qreal Geo::dist2ToLine(QPointF pt,  QPointF p, QPointF q )
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

bool Geo::isOnLine(QPointF pt, QLineF line)
{
    qreal d2 = dist2ToLine(pt, line.p1(), line.p2());
    return Loose::zero(d2);
}

QPointF Geo::center(const QPolygonF & pts)
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

QPointF Geo::irregularCenter(const QPolygonF & poly)
{
    //Q_ASSERT(!poly.isClosed());

    double area = 0.0;
    double x0   = 0.0; // Current vertex X
    double y0   = 0.0; // Current vertex Y
    double x1   = 0.0; // Next vertex X
    double y1   = 0.0; // Next vertex Y
    double a    = 0.0; // Partial signed area
    double sumX = 0.0;
    double sumY = 0.0;

 // For all vertices in a loop
    auto prev = poly.constLast();
    for (const auto & next : std::as_const(poly))
	{
        x0    = prev.x();
        y0    = prev.y();
        x1    = next.x();
        y1    = next.y();
        a     = x0*y1 - x1*y0;
        area += a;
        sumX += ((x0 + x1)*a);
        sumY += ((y0 + y1)*a);
        prev  = next;
    }

    area *= 0.5;

    QPointF centroid(sumX,sumY);
    centroid /= (6.0 * area);

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

QPointF Geo::reflectPoint(QPointF p, QLineF line)
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

    //qDebug() << p << line << p4;
    return p4;
}

QPolygonF Geo::reflectPolygon(QPolygonF & p, QLineF & line)
{
    QPolygonF poly;
    for (auto pt : std::as_const(p))
    {
        auto pt2 = reflectPoint(pt,line);
        poly << pt2;
    }
    qDebug() << "Polygon reflected about" << line;
    return poly;
}

QPointF Geo::perpPt(QLineF line, QPointF C)
{
    return perpPt(line.p1(),line.p2(),C);
}

QPointF Geo::perpPt(QPointF A, QPointF B, QPointF C)
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

QLineF Geo::reversed(QLineF & line)
{
    return QLineF(line.p2(),line.p1());
}

QLineF Geo::shiftParallel(QLineF bline, qreal offset)
{
    //https://stackoverflow.com/questions/2825412/draw-a-parallel-line
    qreal x1 = bline.p1().x();
    qreal y1 = bline.p1().y();
    qreal x2 = bline.p2().x();
    qreal y2 = bline.p2().y();
    qreal L = qSqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
    // This is the second line
    qreal x1p = x1 + ((offset * (y2-y1)) / L);
    qreal x2p = x2 + ((offset * (y2-y1)) / L);
    qreal y1p = y1 + ((offset * (x1-x2)) / L);
    qreal y2p = y2 + ((offset * (x1-x2)) / L);
    return QLineF(x1p,y1p,x2p,y2p);
}

QPointF Geo::findNearestPoint(const QVector<QPointF> &pts, const QPointF apoint)
{

    QPointF nearest;
    qreal len  = 1000.0;
    for (auto pt : std::as_const(pts))
    {
        qreal d =  dist2(apoint, pt);
        if (d < len)
        {
            len = d;
            nearest = pt;
        }
    }
    return nearest;
}



int Geo::circleLineIntersectionPoints(const QGraphicsItem & circle, qreal radius, const QLineF & line, QPointF & aa, QPointF & bb)
{
    int points = 0;

    //float x0 = (circle.mapToScene(circle.shape().controlPointRect().center())).x();
    //float y0 = (circle.mapToScene(circle.shape().controlPointRect().center())).y();
    qreal x0=0;
    qreal y0=0;

    if (!Loose::equals(line.x2(),line.x1()))
    {
        QPointF yaxis_intersection;
        line.intersects( QLineF(QPointF(0, 10000), QPointF(0, -10000)), &yaxis_intersection);

        qreal a = (line.y2() - line.y1())/(line.x2() - line.x1());
        qreal b = yaxis_intersection.y();

        qreal A = 1 + a*a;
        qreal B = 2*(a*b - a*y0 - x0);
        qreal C = x0 * x0 + y0*y0 + b*b - 2*b*y0 - radius*radius;

        qreal Q = B*B - 4*A*C;
        if (Q > 0)
        {
            qreal s1 = (-1)*(B + sqrt(Q))/(2*A);
            qreal s2 = (sqrt(Q) - B)/(2*A);
            QPointF ps1(s1, a*s1 + b);
            QPointF ps2(s2, a*s2 + b);

            if(circle.contains(ps1))
            {
                points = 1;
                aa = ps1;
            }
            if (circle.contains(ps2))
            {
                if (points == 0)
                {
                    points = 1;
                    aa = ps2;
                }
                else
                {
                    points = 2;
                    bb = ps2;
                }
            }
        }
        else
        {
            qreal s0 = -B/2*A;
            points = 1;
            aa = QPointF(s0, a*s0 + b);
        }
    }
    else
    {
        qreal x = line.x1();
        // yy - 2y0y + (x - x0)(x - x0) + y0y0 - R*R = 0

        qreal C = (x - x0)*(x - x0) + y0*y0 - radius*radius;
        qreal Q = 4*y0*y0 - 4*C;

        if (Q > 0)
        {
            qreal s1 = y0 - sqrt(Q)/2;
            qreal s2 = y0 + sqrt(Q)/2 ;
            QPointF ps1(x, s1);
            QPointF ps2(x, s2);

            if(circle.contains(ps1))
            {
                points = 1;
                aa = ps1;
            }
            if (circle.contains(ps2))
            {
                if (points == 0)
                {
                    points = 1;
                    aa = ps2;
                }
                else
                {
                    points=2;
                    bb = ps2;
                }
            }
        }
        else
        {
            points = 1;
            aa = QPointF(x, y0);
        }
    }
    return points;
}

// This is derived from chmike's response in https://stackoverflow.com/questions/1073336/circle-line-segment-collision-detection-algorithm
int Geo::circleLineIntersectionPoints(QPointF center, qreal radius, const QLineF & line, QPointF & aa, QPointF & bb)
{
    QPointF A = line.p1();
    QPointF B = line.p2();
    QPointF C = center;  // circle center
    qreal   R = radius;  // radius

    // compute the euclidean distance between A and B
    qreal LAB = Geo::dist(A,B);

    // compute the direction vector D from A to B
    qreal Dx = (B.x() - A.x()) / LAB;
    qreal Dy = (B.y() - A.y()) / LAB;

    // the equation of the line AB is x = Dx*t + Ax, y = Dy*t + Ay with 0 <= t <= LAB.
    // compute the distance between the points A and E, where
    // E is the point of AB closest the circle center (Cx, Cy)
    qreal t = (Dx*(C.x()-A.x())) + (Dy*(C.y()-A.y()));

    // compute the coordinates of the point E
    qreal Ex = t * Dx + A.x();
    qreal Ey = t * Dy + A.y();
    QPointF E(Ex,Ey);

    // compute the euclidean distance between E and C
    qreal LEC = Geo::dist(E,C);

    // test if the line intersects the circle
    if (LEC < R)
    {
        // compute distance from t to circle intersection point
        qreal dt = qSqrt( (R*R) - (LEC*LEC));

        // compute first intersection point
        qreal Fx = (t-dt)*Dx + A.x();
        qreal Fy = (t-dt)*Dy + A.y();

        aa = QPointF(Fx,Fy);

        // compute second intersection point
        qreal Gx = (t+dt)*Dx + A.x();
        qreal Gy = (t+dt)*Dy + A.y();

        bb = QPointF(Gx,Gy);

        return 2;
    }
    // else test if the line is tangent to circle
    else if (Loose::equals(LEC,R))
    {
        // tangent point to circle is E
        aa = E;
        return 1;
    }
    else
    {
        // line doesn't touch circle
        return 0;
    }
}

int Geo::findLineCircleIntersections(QPointF centre, qreal radius, QLineF line, QPointF & intersection1, QPointF & intersection2)
{
    qreal cx = centre.x();
    qreal cy = centre.y();

    QPointF point1(line.p1());
    QPointF point2(line.p2());

    qreal dx, dy, A, B, C, det, t;

    dx = point2.x() - point1.x();
    dy = point2.y() - point1.y();

    A = dx * dx + dy * dy;
    B = 2 * (dx * (point1.x() - cx) + dy * (point1.y() - cy));
    C = (point1.x() - cx) * (point1.x() - cx) +
        (point1.y() - cy) * (point1.y() - cy) -
        radius * radius;

    det = B * B - 4 * A * C;
    //qDebug() << "A=" << A <<  "B=" << B << "C=" << C << "det=" << det;

    int count = 0;
    if ((A <= 0.0000001) || (det < 0))
    {
        // No real solutions.
        return count;
    }
    else if (Loose::equals(det,0))
    {
        // One solution.
        t = -B / (2 * A);
        auto isect = QPointF(point1.x() + t * dx, point1.y() + t * dy);
        if (pointOnLine(QLineF(point1,point2),isect))
        {
            intersection1 = isect;
            count = 1;
        }
        return count;
    }
    else
    {
        // Two solutions.
        t = (-B + qSqrt(det)) / (2 * A);
        auto isect = QPointF(point1.x() + t * dx, point1.y() + t * dy);
        if (pointOnLine(QLineF(point1,point2),isect))
        {
            intersection1 = isect;
            count++;
        }
        t = (-B - qSqrt(det)) / (2 * A);
        isect = QPointF(point1.x() + t * dx, point1.y() + t * dy);
        if (pointOnLine(QLineF(point1,point2),isect))
        {
            if (count == 0)
            {
                intersection1 = isect;
            }
            else
            {
                intersection2 = isect;
            }
            count++;
        }
        return count;
    }
}



bool Geo::pointInCircle(QPointF pt, Circle c)
{
    qreal d2  = Geo::dist2(pt,c.centre);
    qreal r2 = c.radius * c.radius;
    return d2 < r2;
}

bool Geo::pointOnCircle(QPointF pt, Circle c, qreal tolerance)
{
    qreal d  = Geo::dist(pt,c.centre);
    qreal r = c.radius;
    //qDebug() << d << r << qAbs(d-r) << tolerance;
    return Loose::equals(d,r,tolerance);
}

bool circCollide(QGraphicsItem * item, QList<QGraphicsItem *> items)
{
    QPointF c1 = item->boundingRect().center();                                             // ok
    foreach (QGraphicsItem * t, items)
    {
        qreal distance = QLineF(c1, t->boundingRect().center()).length();                   // ok
        qreal radii    = (item->boundingRect().width() + t->boundingRect().width()) / 2;    // ok
        if ( distance <= radii )
            return true;
    }
    return false;
}

/*
Distance between centers C1 and C2 is calculated as
    C1C2 = sqrt((x1 - x2)2 + (y1 - y2)2).
    There are three condition arises.
    1. If C1C2 == R1 + R2
       Circle A and B are touch to each other.
    2. If C1C2 > R1 + R2
       Circle A and B are not touch to each other.
    3. If C1C2 < R1 + R2
       Circle intersects each other.
  */

int Geo::circleIntersects(qreal x1, qreal y1, qreal x2,  qreal y2, qreal r1, qreal r2)
{
    qreal distSq   = (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
    qreal radSumSq = (r1 + r2) * (r1 + r2);
    if (Loose::equals(distSq,radSumSq))
    {
        qDebug() << "Circle touch to";
        return 1;
    }
    else if (distSq > radSumSq)
    {
        qDebug() << "Circle not touch";
        return 0;
    }
    else
    {
        qDebug() << "Circle intersect";
        return 2;
    }
}

int Geo::circleIntersects(Circle c1, Circle c2)
{
    return circleIntersects(c1.centre.x(), c1.centre.y(), c2.centre.x(), c2.centre.y(),c1.radius,c2.radius);
}

// Given two circles this method finds the intersection
// point(s) of the two circles (if any exists)
// https://stackoverflow.com/questions/3349125/circle-circle-intersection-points
int Geo::circleCircleIntersectionPoints(Circle c1, Circle c2, QPointF & p1, QPointF & p2)
{
    const qreal EPS = 0.0000001; // Let EPS (epsilon) be a small value

    qreal r, R, cx, cy, Cx, Cy;
    if (c1.radius < c2.radius)
    {
        r  = c1.radius;  R = c2.radius;
        cx = c1.x();    cy = c1.y();
        Cx = c2.x();    Cy = c2.y();
    }
    else
    {
        r  = c2.radius; R  = c1.radius;
        Cx = c1.x();    Cy = c1.y();
        cx = c2.x();    cy = c2.y();
    }

    // Compute the vector <dx, dy>
    qreal dx = cx - Cx;
    qreal dy = cy - Cy;

    // Find the distance between two points.
    qreal d = qSqrt( dx*dx + dy*dy );

    if (d < EPS && abs(R-r) < EPS)
    {
        // There are an infinite number of solutions
        // Seems appropriate to also return null
        return 0;
    }
    else if (d < EPS)
    {
        // No intersection (circles centered at the
        // same place with different size)
        return 0;
    }

    qreal x = (dx / d) * R + Cx;
    qreal y = (dy / d) * R + Cy;
    QPointF P(x, y);

    // Single intersection (kissing circles)
    if (abs((R+r)-d) < EPS || abs(R-(r+d)) < EPS)
    {
        p1 = P;
        return 1;
    }

    // No intersection. Either the small circle contained within
    // big circle or circles are simply disjoint.
    if ( (d+r) < R || (R+r < d) )
    {
        return 0;
    }

    QPointF C(Cx, Cy);
    qreal angle = acossafe((r*r-d*d-R*R)/(-2.0*d*R));
    p1 = rotatePoint(C, P, +angle);
    p2 = rotatePoint(C, P, -angle);
    return 2;
}

// Why not just use 7 lines of your favorite procedural language (or programmable calculator!) as below.
// Assuming you are given P0 coords (x0,y0), P1 coords (x1,y1), r0 and r1 and you want to find P3 coords (x3,y3):
// https://stackoverflow.com/questions/3349125/circle-circle-intersection-points
void Geo::circleTouchPt(qreal x0, qreal x1, qreal & x3,  qreal y0, qreal y1, qreal & y3, qreal r0, qreal r1)
{
    qreal d  = qSqrt( ((x1-x0)*(x1-x0)) + ((y1-y0)*(y1-y0)) );
    qreal a  = ((r0*r0) - (r1*r1) + (d*d))/(2*d);
    qreal h  = qSqrt((r0*r0)- (a*a));
    qreal x2 = x0+a*(x1-x0)/d;
    qreal y2 = y0+a*(y1-y0)/d;
    x3 = x2+h*(y1-y0)/d;       // also x3=x2-h*(y1-y0)/d
    y3 = y2-h*(x1-x0)/d;       // also y3=y2+h*(x1-x0)/d
}

QPointF Geo::circleTouchPt(Circle c0, Circle c1)
{
    qreal x3;
    qreal y3;

    circleTouchPt(c0.x(), c1.x(),x3,c0.y(), c1.y(), y3, c0.radius, c1.radius);

    QPointF pt(x3,y3);

    return pt;
}

QVector<QLineF> Geo::rectToLines(QRectF &box)
{
    QVector<QLineF> res;
    QLineF  a(box.topLeft(),box.topRight());
    res << a;
    QLineF  b(box.topRight(),box.bottomRight());
    res << b;
    QLineF  c(box.bottomRight(),box.bottomLeft());
    res << c;
    QLineF  d(box.bottomLeft(),box.topLeft());
    res << d;
    return res;
}

QVector<QLineF> Geo::polyToLines(const QPolygonF & poly)
{
    QVector<QLineF> lines;
    for (int i=0; i < (poly.size()-1); i++)
    {
        QLineF line(poly[i],poly[i+1]);
        lines << line;
    }
    if (!poly.isClosed())
    {
        QLineF line(poly.last(), poly.first());
        lines <<  line;
    }
    return lines;
}

bool Geo::pointOnLine(QLineF l, QPointF p)
{
    //https://stackoverflow.com/questions/7050186/find-if-point-lies-on-line-segment
    qreal AB = sqrt((l.x2()-l.x1())*(l.x2()-l.x1())+(l.y2()-l.y1())*(l.y2()-l.y1()));
    qreal AP = sqrt((p.x()-l.x1())*(p.x()-l.x1())+(p.y()-l.y1())*(p.y()-l.y1()));
    qreal PB = sqrt((l.x2()-p.x())*(l.x2()-p.x())+(l.y2()-p.y())*(l.y2()-p.y()));
    qreal diff (AB - (AP + PB));

    return Loose::zero(diff);
}

bool Geo::pointAtEndOfLine(QLineF l, QPointF p)
{
    QPointF pt = l.p1();
    if (pt == p)
        return true;

    pt = l.p2();
    if (pt == p)
        return true;

    return false;
}

QPointF Geo::snapTo(QPointF to, QPointF from, int precision)
{
    if (canSnapTo(to,from,precision))
    {
        return to;
    }
    else
    {
        return from;
    }
}

bool Geo::canSnapTo(QPointF to, QPointF from, int precision)
{
    int mult = 18 * precision;
    qreal qtox = to.x() * mult;
    qreal qtoy = to.y() * mult;
    qreal qfrx = from.x() * mult;
    qreal qfry = from.y() * mult;
    int itox = qRound(qtox);
    int itoy = qRound(qtoy);
    int ifrx = qRound(qfrx);
    int ifry = qRound(qfry);

    if ( (qAbs(itox-ifrx) < 2)  && (qAbs(itoy-ifry) < 2) )
    {
        qDebug() << "canSnapTo: from=" << from << "to=" << to;
        return true;
    }
    else
    {
        return false;
    }
}

QPointF Geo::snapTo(QPointF pt, QLineF trackLine)
{
    double APx = pt.x() - trackLine.x1();
    double APy = pt.y() - trackLine.y1();
    double ABx = trackLine.x2() - trackLine.x1();
    double ABy = trackLine.y2() - trackLine.y1();
    double magAB2 = ABx*ABx + ABy*ABy;
    double ABdotAP = ABx*APx + ABy*APy;
    double t = ABdotAP / magAB2;

    QPointF newPoint;

    if ( t < 0)
    {
        newPoint = trackLine.p1();
    }
    else if (t > 1)
    {
        newPoint = trackLine.p2();
    }
    else
    {
        newPoint.setX(trackLine.x1() + ABx*t);
        newPoint.setY(trackLine.y1() + ABy*t);
    }

    return newPoint;
}

qreal Geo::calcArea(QPolygonF &poly)
{
    // taken from https://www.geeksforgeeks.org/area-of-a-polygon-with-given-n-ordered-vertices/
    qreal area = 0.0;
    int size = poly.size();
    if (poly.isClosed())
    {
        size--;
    }
    int j = size - 1;
    for (int i = 0; i < size; i++)
    {
        area += (poly[j].x() + poly[i].x())  * (poly[j].y() - poly[i].y());
        j = i;  // j is previous vertex to i
    }
    area =  qAbs(area/qreal(2.0));
    return area;
}

// Due to double rounding precision the value passed into the Math.acos
// function may be outside its domain of [-1, +1] which would return
// the value NaN which we do not want.
qreal Geo::acossafe(qreal x)
{
    if (x >= +1.0) return 0;
    if (x <= -1.0) return M_PI;
    return qAcos(x);
}

// Rotates a point about a fixed point at some angle 'a'
QPointF Geo::rotatePoint(QPointF fp, QPointF pt, qreal a)
{
    qreal x    = pt.x() - fp.x();
    qreal y    = pt.y() - fp.y();
    qreal xRot = (x * qCos(a)) + (y * qSin(a));
    qreal yRot = (y * qCos(a)) - (x * qSin(a));
    return QPointF(fp.x()+xRot,fp.y()+yRot);
}


QLineF Geo::normalVectorP1(QLineF line)
{
    return QLineF(line.p1(), line.p1() + QPointF(line.dy(), -line.dx()));
}

QLineF Geo::normalVectorP2(QLineF line)
{
    return QLineF(line.p2(), line.p2() - QPointF(line.dy(), -line.dx()));

}

QPointF Geo::getClosestPoint(QLineF line, QPointF p)
{
    QPointF a = line.p1();
    QPointF b = line.p2();
    double APx = p.x() - a.x();
    double APy = p.y() - a.y();
    double ABx = b.x() - a.x();
    double ABy = b.y() - a.y();
    double magAB2  = ABx*ABx + ABy*ABy;
    double ABdotAP = ABx*APx + ABy*APy;
    double t       = ABdotAP / magAB2;

    double xret;
    double yret;

    if ( t < 0)
    {
        xret = a.x();
        yret = a.y();
    }
    else if (t > 1)
    {
        xret = b.x();
        yret = b.y();
    }
    else
    {
        xret = a.x() + ABx *t;
        yret = a.y() + ABy *t;
    }
    return QPointF(xret,yret);
}

bool Geo::isClockwise(const QPolygonF & poly)
{
    //   Q_ASSERT(poly.isClosed());

    double sum = 0.0;
    for (int i = 0; i < poly.count(); i++)
    {
        QPointF v1 = poly[i];
        QPointF v2 = poly[(i + 1) % poly.count()];
        sum += (v2.x() - v1.x()) * (v2.y() + v1.y());
    }

    return sum < 0.0;
}

bool Geo::isClockwiseKaplan(QPolygonF & poly)
{
    // Is a polygon (given here as a vector of Vertex instances) clockwise?
    // We can answer that question by finding a spot on the polygon where
    // we can distinguish inside from outside and looking at the edges
    // at that spot.  In this case, we look at the vertex with the
    // maximum X value.  Whether the polygon is clockwise depends on whether
    // the edge leaving that vertex is to the left or right of the edge
    // entering the vertex.  Left-or-right is computed using the sign of the cross product.

    int sz = poly.size();

    // First, find the vertex with the greatest X coordinate.

    int imax = 0;
    qreal xmax = poly[0].x();

    for( int idx = 1; idx < sz; ++idx )
    {
        qreal x = poly[idx].x();
        if( x > xmax )
        {
            imax = idx;
            xmax = x;
        }
    }

    QPointF pmax  = poly[imax];
    QPointF pnext = poly[(imax+1) % sz];
    QPointF pprev = poly[(imax+sz-1) %sz];

    QPointF dprev = pmax -  pprev;
    QPointF dnext = pnext - pmax;

    return Geo::cross(dprev, dnext ) <= 0.0;
}


void Geo::reverseOrder(QPolygonF & poly)
{
    qDebug() << "reverse order: polygon";
    QPolygonF ret;
    for (int i = poly.size()-1; i >= 0; i--)
    {
        ret << poly[i];
    }
    poly = ret;
}

void Geo::reverseOrder(EdgePoly & ep)
{
    qDebug() << "reverse order: edge poly";
    EdgePoly ret;
    for (int i = ep.size()-1; i >= 0; i--)
    {
        EdgePtr edge = ep[i];
        VertexPtr v1 = edge->v1;
        VertexPtr v2 = edge->v2;
        edge->setV1(v2);
        edge->setV2(v1);
        ret << edge;
    }
    ep = ret;
}

#ifndef M_2PI
#define M_2PI 6.28318530717958647692528676655900576
#endif

qreal Geo::angle(const QLineF &l0,const QLineF &l)
{
    if (l0.isNull() || l.isNull())
        return 0;
    qreal cos_line = (l0.dx()*l.dx() + l0.dy()*l.dy()) / (l0.length()*l.length());
    qreal rad = 0;
    // only accept cos_line in the range [-1,1], if it is outside, use 0 (we return 0 rather than PI for those cases)
    if (cos_line >= -1.0 && cos_line <= 1.0) rad = qAcos( cos_line );
    return rad * 360 / M_2PI;
}

inline QPointF vec(QPointF a, QPointF b )
{
    return QPointF(b.x() - a.x(), b.y()-a.y());
}

bool Geo::rectContains(const QRectF & rect, QPointF p)
{
    QPointF a = rect.topLeft();
    QPointF b = rect.bottomRight();

    qreal x = p.x();
    qreal y = p.y();

    if (  (Loose::greaterThan(x,a.x()) ||  Loose::equals(x,a.x()))
        &&(Loose::lessThan   (x,b.x()) ||  Loose::equals(x,b.x()))
        &&(Loose::greaterThan(y,a.y()) ||  Loose::equals(y,a.y()))
        &&(Loose::lessThan   (y,b.y()) ||  Loose::equals(y,b.y())))
        return true;

    return false;
}


bool Geo::isColinearAndTouching(const EdgePoly & ep1, const EdgePoly & ep2, qreal tolerance)
{
    QPolygonF p1 = ep1.getPoly();
    QPolygonF p2 = ep2.getPoly();
    if (p1.intersects(p2))
    {
        QPolygonF p3 = p1.intersected(p2);
        qreal area = Geo::calcArea(p3);
        qDebug() << "area =" << area;
    }

    Q_UNUSED(tolerance);
    for (const EdgePtr & edge1 : std::as_const(ep1))
    {
        for (const EdgePtr &  edge2 : std::as_const(ep2))
        {
            if (isColinearAndTouching(edge1->getLine(),edge2->getLine()))
            {
                return true;
            }
        }
    }
    return false;
}

bool Geo::isColinearAndTouching(const QLineF & l1, const QLineF & l2, qreal tolerance)
{
    if (isColinear(l1,l2,tolerance))
    {
        if (l1.length() < l2.length())
        {
            // l1 shorter
            return (Geo::isOnLine(l1.p1(),l2) || Geo::isOnLine(l1.p2(),l2));
        }
        else
        {
            // l2 shorter, l1 longer or equal
            return  (Geo::isOnLine(l2.p1(),l1) || Geo::isOnLine(l2.p2(),l1));
        }
    }
    return false;
}

bool Geo::isColinear(const QLineF & l1, const QLineF & l2, qreal tolerance)
{
    qreal angle = Geo::angle(l1,l2);
    if ((qAbs(angle) < tolerance) || (qAbs(angle-180.0) < tolerance))
    {
        return true;
    }
    return false;
}

// Calculate the closest point on the line segment AB to point P
QPointF Geo::getClosestPointB(const QPointF& A, const QPointF& B, const QPointF& P)
{
    // Convert to line formula: y = ax + b
    double a = (B.y() - A.y()) / (B.x() - A.x());
    double b = -a * A.x() + A.y();

    // Get normal line formula: y = a2x + b2
    double a2 = -1 / a; // Corrected formula for normal line slope
    double b2 = -a2 * P.x() + P.y();

    // Solve for x
    double a3 = a - a2;
    double b3 = b2 - b;
    double x = b3 / a3;

    // Calculate y
    double y = a * x + b;

    return QPointF(x, y);
}

// Calculate the closest point on the line segment AB to point P using dot product
QPointF Geo::getClosestPointC(const QPointF& A, const QPointF& B, const QPointF& P)
{
    QPointF AB(B.x() - A.x(), B.y() - A.y());
    QPointF AP(P.x() - A.x(), P.y() - A.y());

    double lengthSqrAB = AB.x() * AB.x() + AB.y() * AB.y();
    double t = (AP.x() * AB.x() + AP.y() * AB.y()) / lengthSqrAB;

    QPointF closest(A.x() + AB.x() * t, A.y() + AB.y() * t);
    return closest;
}

#if 1
// recoded from stack overflow code (see below) becuse of issues wih precision of floatting point
// compilations.  Linux compiler is different to MSVC comiler.
bool Geo::pointInPolygon(const QPointF & point, const QPolygonF & polygon)
{
    int i, j, nvert = polygon.size();
    bool c = false;
    //qDebug() << "pointInPolygon" << nvert << point << polygon;

    for(i = 0, j = nvert - 1; i < nvert; j = i++)
    {
        //bool t1  = polygon[i].y() >= point.y();
        bool t1  = (polygon[i].y() > point.y()) || Loose::equals(polygon[i].y(),point.y());
        //bool t2  = polygon[j].y() >= point.y();
        bool t2  = (polygon[j].y() >= point.y()) || Loose::equals(polygon[j].y(),point.y());
        bool t3  = t1 != t2;

        qreal v1 = polygon[j].x() - polygon[i].x();
        qreal v2 = point.y() - polygon[i].y();
        qreal v3 = (polygon[j].y() - polygon[i].y());
        qreal v4 = v1 * v2 / v3;
        qreal v5 = v4 + polygon[i].x();
        //bool  t4 = point.x() <= v5;
        bool  t4 = (point.x() < v5) || Loose::equals(point.x(),v5);
        if ( t3 && t4)
        {
            c = !c;
        }
        //qDebug() << t1 << t2 <<  t3 << t4 << v1 << v2 << v3 << v4 << v5;
    }

    return c;
}
#else
//  Taken from
//  https://stackoverflow.com/questions/11716268/point-in-polygon-algorithm
//
bool Geo::pointInPolygon(const QPointF & point, const QPolygonF & polygon)
{
    int i, j, nvert = polygon.size();
    bool c = false;
    qDebug() << "pointInPolygon" << nvert << point << polygon;

    for(i = 0, j = nvert - 1; i < nvert; j = i++)
    {
        if( ( (polygon[i].y() >= point.y() ) != (polygon[j].y() >= point.y()) ) &&
            (point.x() <= (polygon[j].x() - polygon[i].x()) * (point.y() - polygon[i].y()) / (polygon[j].y() - polygon[i].y()) + polygon[i].x()) )
        {
            c = !c;
        }
    }
    return c;
}
#endif

bool Geo::point_in_polygon(QPointF point, const QPolygonF &polygon)
{
    // from https://www.geeksforgeeks.org/how-to-check-if-a-given-point-lies-inside-a-polygon/
    int num_vertices = polygon.size();

    qreal x = point.x(), y = point.y();

    bool inside = false;

    // Store the first point in the polygon and initialize the second point
    QPointF p1 = polygon[0], p2;

    // Loop through each edge in the polygon
    for (int i = 1; i <= num_vertices; i++)
    {
        // Get the next point in the polygon
        p2 = polygon[i % num_vertices];

        // Check if the point is above the minimum y coordinate of the edge
        if (y > qMin(p1.y(), p2.y()))
        {
            // Check if the point is below the maximum y coordinate of the edge
            if (y <= qMax(p1.y(), p2.y()))
            {
                // Check if the point is to the left of the maximum x coordinate of the edge
                if (x <= qMax(p1.x(), p2.x()))
                {
                    // Calculate the x-intersection of the line connecting the point to the edge
                    qreal x_intersection = (y - p1.y()) * (p2.x() - p1.x()) / (p2.y() - p1.y()) + p1.x();

                    // Check if the point is on the same line as the edge or to the left of the x-intersection
                    if (p1.x() == p2.x() || x <= x_intersection)
                    {
                        // Flip the inside flag
                        inside = !inside;
                    }
                }
            }
        }
        // Store the current point as the first point for the next iteration
        p1 = p2;
    }
    return inside;
}

bool Geo::point_on_poly_edge(QPointF p, const QPolygonF & poly)
{
    EdgePoly ep(poly);
    for (EdgePtr edge : std::as_const(ep))
    {
        if (Loose::zero(distToLine(p,edge->getLine())))
        {
            return true;
        }
    }
    return false;
}
