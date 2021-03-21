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

#include "base/utilities.h"
#include "base/misc.h"
#include "geometry/loose.h"
#include "geometry/point.h"
#include "geometry/edgepoly.h"
#include <QPainter>

QString Utils::addr(void * address)
{
  return QString::number(reinterpret_cast<uint64_t>(address),16);
}

QString Utils::addr(const void * address)
{
  return QString::number(reinterpret_cast<uint64_t>(address),16);
}

QString Utils::str(QPointF pt)
{
    return QString("(%1,%2)").arg(QString::number(pt.x())).arg(QString::number(pt.y()));
}

QString Utils::str(QSize sz)
{
    return QString("%1 x %2").arg(QString::number(sz.width())).arg(QString::number(sz.height()));
}

void Utils::identify(Layer * layer, QPolygonF * poly)
{
    for (int i=0; i< poly->count(); i++)
    {
        QPointF p   = poly->at(i);
        QString txt = QString("%1: %2 %3").arg(i).arg(p.x()).arg(p.y());
        qDebug() << txt;
        MarkXPtr m   = make_shared<MarkX>(p, QPen(QColor(Qt::green),3), txt);
        layer->addSubLayer(m);
    }
}

int Utils::circleLineIntersectionPoints(const QGraphicsItem & circle, qreal radius, const QLineF & line, QPointF & aa, QPointF & bb)
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
int Utils::circleLineIntersectionPoints(QPointF center, qreal radius, const QLineF & line, QPointF & aa, QPointF & bb)
{
    QPointF A = line.p1();
    QPointF B = line.p2();
    QPointF C = center;  // circle center
    qreal   R = radius;  // radius

    // compute the euclidean distance between A and B
    qreal LAB = Point::dist(A,B);

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
    qreal LEC = Point::dist(E,C);

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

// Find the points of intersection.
int Utils::findLineCircleLineIntersections(qreal cx, qreal cy, qreal radius, QPointF point1, QPointF point2, QPointF & intersection1, QPointF & intersection2)
{
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
    if ((A <= 0.0000001) || (det < 0))
    {
        // No real solutions.
        return 0;
    }
    else if (Loose::equals(det,0))
    {
        // One solution.
        t = -B / (2 * A);
        intersection1 = QPointF(point1.x() + t * dx, point1.y() + t * dy);
        return 1;
    }
    else
    {
        // Two solutions.
        t = (-B + qSqrt(det)) / (2 * A);
        intersection1 = QPointF(point1.x() + t * dx, point1.y() + t * dy);
        t = (-B - qSqrt(det)) / (2 * A);
        intersection2 = QPointF(point1.x() + t * dx, point1.y() + t * dy);
        return 2;
    }
}

int Utils::findLineCircleLineIntersections(QPointF centre, qreal radius, QLineF line, QPointF & intersection1, QPointF & intersection2)
{
    return findLineCircleLineIntersections(centre.x(), centre.y(), radius, line.p1(), line.p2(), intersection1, intersection2);

}

bool Utils::pointInCircle(QPointF pt, CirclePtr c)
{
    QPointF center = c->centre;
    qreal dx = pt.x() - center.x();
    qreal dy = pt.y() - center.y();
    return (((dx*dx) + (dy*dy)) < (c->radius*c->radius));
}

bool Utils::pointOnCircle(QPointF pt, CirclePtr c)
{
    QPointF center = c->centre;
    qreal dx = pt.x() - center.x();
    qreal dy = pt.y() - center.y();
    return ( ( ((dx*dx) + (dy*dy)) - (c->radius*c->radius) ) < Loose::NEAR_TOL);
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

int Utils::circleIntersects(qreal x1, qreal y1, qreal x2,  qreal y2, qreal r1, qreal r2)
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

int Utils::circleIntersects(CirclePtr c1, CirclePtr c2)
{
    return circleIntersects(c1->centre.x(), c1->centre.y(), c2->centre.x(), c2->centre.y(),c1->radius,c2->radius);
}

// Given two circles this method finds the intersection
// point(s) of the two circles (if any exists)
// https://stackoverflow.com/questions/3349125/circle-circle-intersection-points
int Utils::circleCircleIntersectionPoints(CirclePtr c1, CirclePtr c2, QPointF & p1, QPointF & p2)
{
    const qreal EPS = 0.0000001; // Let EPS (epsilon) be a small value

    qreal r, R, cx, cy, Cx, Cy;
    if (c1->radius < c2->radius)
    {
        r  = c1->radius;  R = c2->radius;
        cx = c1->x();    cy = c1->y();
        Cx = c2->x();    Cy = c2->y();
    }
    else
    {
        r  = c2->radius; R  = c1->radius;
        Cx = c1->x();    Cy = c1->y();
        cx = c2->x();    cy = c2->y();
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
void Utils::circleTouchPt(qreal x0, qreal x1, qreal & x3,  qreal y0, qreal y1, qreal & y3, qreal r0, qreal r1)
{
    qreal d  = qSqrt( ((x1-x0)*(x1-x0)) + ((y1-y0)*(y1-y0)) );
    qreal a  = ((r0*r0) - (r1*r1) + (d*d))/(2*d);
    qreal h  = qSqrt((r0*r0)- (a*a));
    qreal x2 = x0+a*(x1-x0)/d;
    qreal y2 = y0+a*(y1-y0)/d;
          x3 = x2+h*(y1-y0)/d;       // also x3=x2-h*(y1-y0)/d
          y3 = y2-h*(x1-x0)/d;       // also y3=y2+h*(x1-x0)/d
}

QPointF Utils::circleTouchPt(CirclePtr c0, CirclePtr c1)
{
    qreal x3;
    qreal y3;

    circleTouchPt(c0->x(), c1->x(),x3,c0->y(), c1->y(), y3, c0->radius, c1->radius);

    QPointF pt(x3,y3);

    return pt;
}

QVector<QLineF> Utils::rectToLines(QRectF &box)
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

QVector<QLineF> Utils::polyToLines(QPolygonF & poly)
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

bool Utils::pointOnLine(QLineF l, QPointF p)
{
    qreal AB = sqrt((l.x2()-l.x1())*(l.x2()-l.x1())+(l.y2()-l.y1())*(l.y2()-l.y1()));
    qreal AP = sqrt((p.x()-l.x1())*(p.x()-l.x1())+(p.y()-l.y1())*(p.y()-l.y1()));
    qreal PB = sqrt((l.x2()-p.x())*(l.x2()-p.x())+(l.y2()-p.y())*(l.y2()-p.y()));
    qreal diff (AB - (AP + PB));

    return Loose::zero(diff);
}

bool Utils::pointAtEndOfLine(QLineF l, QPointF p)
{
    QPointF pt = l.p1();
    if (pt == p)
        return true;

    pt = l.p2();
    if (pt == p)
        return true;

    return false;
}

QPointF Utils::snapTo(QPointF to, QPointF from, int precision)
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

bool Utils::canSnapTo(QPointF to, QPointF from, int precision)
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

QPointF Utils::snapTo(QPointF pt, QLineF trackLine)
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

qreal Utils::calcArea(QPolygonF &poly)
{
    // taken from https://www.geeksforgeeks.org/area-of-a-polygon-with-given-n-ordered-vertices/

    if (poly.isClosed())
        qWarning() << "calculating area on closed polygon";

    qreal area = 0.0;
    int j = poly.size() - 1;
    for (int i = 0; i < poly.size(); i++)
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
qreal Utils::acossafe(qreal x)
{
    if (x >= +1.0) return 0;
    if (x <= -1.0) return M_PI;
    return qAcos(x);
}

// Rotates a point about a fixed point at some angle 'a'
QPointF Utils::rotatePoint(QPointF fp, QPointF pt, qreal a)
{
    qreal x    = pt.x() - fp.x();
    qreal y    = pt.y() - fp.y();
    qreal xRot = (x * qCos(a)) + (y * qSin(a));
    qreal yRot = (y * qCos(a)) - (x * qSin(a));
    return QPointF(fp.x()+xRot,fp.y()+yRot);
}


QLineF Utils::normalVectorP1(QLineF line)
{
    return QLineF(line.p1(), line.p1() + QPointF(line.dy(), -line.dx()));
}

QLineF Utils::normalVectorP2(QLineF line)
{
    return QLineF(line.p2(), line.p2() - QPointF(line.dy(), -line.dx()));

}

QPointF Utils::getClosestPoint(QLineF line, QPointF p)
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
        xret = a.x() + ABx*t;
        yret = a.y() + ABy*t;
    }
    return QPointF(xret,yret);
}

bool Utils::isClockwise(const QPolygonF & poly)
{
    Q_ASSERT(!poly.isClosed());

    double sum = 0.0;
    for (int i = 0; i < poly.count(); i++)
    {
        QPointF v1 = poly[i];
        QPointF v2 = poly[(i + 1) % poly.count()];
        sum += (v2.x() - v1.x()) * (v2.y() + v1.y());
    }

    return sum < 0.0;
}

bool Utils::isClockwiseKaplan(QPolygonF & poly)
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

    return Point::cross(dprev, dnext ) <= 0.0;
}


void Utils::reverseOrder(QPolygonF & poly)
{
    qDebug() << "reverse order: polygon";
    QPolygonF ret;
    for (int i = poly.size()-1; i >= 0; i--)
    {
        ret << poly[i];
    }
    poly = ret;
}

void Utils::reverseOrder(EdgePoly & ep)
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

qreal Utils::angle(const QLineF &l0,const QLineF &l)
{
    if (l0.isNull() || l.isNull())
        return 0;
    qreal cos_line = (l0.dx()*l.dx() + l0.dy()*l.dy()) / (l0.length()*l.length());
    qreal rad = 0;
    // only accept cos_line in the range [-1,1], if it is outside, use 0 (we return 0 rather than PI for those cases)
    if (cos_line >= -1.0 && cos_line <= 1.0) rad = qAcos( cos_line );
    return rad * 360 / M_2PI;
}
