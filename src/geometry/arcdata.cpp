#include "geometry/arcdata.h"
#include "geometry/edge.h"
#include "geometry/point.h"
#include "geometry/loose.h"
#include "geometry/vertex.h"

ArcData::ArcData(QPointF p1, QPointF p2, QPointF c, bool convex)
{
    trace = 0;
    span = 0;
    this->convex = convex;
    //testCenters(p1,p2,c);
    //testEnds(p1,p2,c);

    if (convex)
    {
        calcConvex(p1,p2,c);
    }
    else
    {
        calcConcave(p1,p2,c);
    }
}

ArcData::ArcData(Edge * edge)
{
    span = 0;
    trace = 0;
    convex = edge->isConvex();
    if (convex)
    {
        calcConvex(edge->v1->pt, edge->v2->pt, edge->getArcCenter());
    }
    else
    {
        calcConcave(edge->v1->pt, edge->v2->pt, edge->getArcCenter());
    }
}

void ArcData::calcConvex(QPointF p1, QPointF p2, QPointF c)
{
    if (trace >=3) qDebug()  << "convex " << p1 << p2 << c;

    start = Point::getAngleDegrees(c,p1);
    end   = Point::getAngleDegrees(c,p2);

    // should be a -ve span  (meaning CCW)
    span  = end-start;

    if (trace >= 2) qDebug() << "convex start" << start << "end" << end << "span" << span;

    if (span > 0)
    {
        //Q_ASSERT(span > 180);
        span = -(start + (360-end));

    }
    if (span < -180.0)
    {
        //span += 180.0;  // this breaks FirstCurve.v4
        //span = (360 - span);  // also breaks
        //span = 360 + span; // not tested
        span = -(360 + span); // not tested
        //qWarning() << "something needed here";
    }

    qreal radius = QLineF(p1,c).length();
    //qDebug() <<  "radius"  << radius;

    rect = QRectF(c.x() - radius, c.y() - radius, radius*2, radius*2);

    if (trace >= 1) qInfo() << "ArcData convex" << rect << "start=" << start << "end=" << end  << "span=" << span;

    Q_ASSERT(span <= 0);
}

void ArcData::calcConcave(QPointF p1, QPointF p2, QPointF c)
{
    // for concave we need the center reflection accross p1p2
    // and the direction must be CCW - meaning a positsive span

    if (trace >=3) qDebug() << "concave" << p1 << p2 << c;
    c = getCenterOldConcave(p1,p2,c);
    //qDebug() << "center (mod)" << c;

    start = Point::getAngleDegrees(c,p1);
    end   = Point::getAngleDegrees(c,p2);

    // should be a +ve span (meaning CW)
    span = end-start;
    if (trace >= 2) qDebug() << "concave start" << start << "end" << end << "span" << span;

    if (span < 0)
    {
        //_span = start + (360-end);
        span += 360;
    }

    if (span > 180.0)
    {
        span  = 360.0 - span;   // fixes FirstCurrve.v4 and petal
    }

    qreal radius = QLineF(p1,c).length();
    //qDebug() <<  "radius"  << radius;

    rect = QRectF(c.x() - radius, c.y() - radius, radius*2, radius*2);

    if (trace >= 1) qInfo() << "ArcData concave" << rect << "start=" << start << "end=" << end  << "span=" << span;

    Q_ASSERT(span >= 0);
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

QPointF ArcData::reflectPoint(QPointF p, QLineF line)
{
    qreal x1 = p.x();
    qreal x2 = line.x1();
    qreal x3 = line.x2();
    qreal x4 = 0;
    qreal y1 = p.y();
    qreal y2 = line.y1();
    qreal y3 = line.y2();
    qreal y4 = 0;
    if (equals(x3,x2,1e-7))
    {
        y4 =  y1;
        x4 += (x2-x1);
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

void ArcData::dump()
{
    qInfo() << "ArcData" << rect << start << end << "span=" << span;
}

QPointF ArcData::getCenterOldConcave(QPointF p1, QPointF p2, QPointF c)
{
    QPointF amid = QLineF(p1,p2).pointAt(0.5);
    QLineF  mid  = QLineF(c,amid);
    QPointF c2 = mid.pointAt(2.0);
    return c2;
}

QPointF ArcData::getCenterNew(QPointF p1, QPointF p2, qreal radius, bool convex)
{
    QLineF edge(p1,p2);

    QPointF vp1 = edge.pointAt(0.5);

    // calc normal vector
    QLineF baseline(vp1,p2);
    QPointF vp2;
    if (!convex)        // DAC - why is this fliipped
    {
        vp2.setX(-baseline.dy());
        vp2.setY(baseline.dx());
    }
    else
    {
        vp2.setX(baseline.dy());
        vp2.setY(-baseline.dx());
    }
    QLineF normalVector(vp1,vp1+vp2);

    // adjust len using Pythagoras
    qreal len = qSqrt( (radius * radius) - ((edge.length()/2.0) * (edge.length()/2.0)));
    normalVector.setLength(len);

    return normalVector.p2();   // the center
}

void ArcData::testCenters(QPointF p1, QPointF p2, QPointF c)
{
    qreal radius  = QLineF(c,p1).length();
    qreal radius2 = QLineF(c,p2).length();
    if (!Loose::equals(radius,radius2))
    {
        qWarning() << "radii mismatch (1)" << radius << radius2;
    }

    QPointF newConvex  = getCenterNew(p1,p2,radius,true);
    QPointF newConcave = getCenterNew(p1,p2,radius,false);
    QPointF oldConcave = getCenterOldConcave(p1,p2,c);

    if (c != newConvex)
    {
        qWarning() << "convex center mismatch" << c << newConvex;
    }
    if (oldConcave != newConcave)
    {
        qWarning() << "concave center mismatch" << oldConcave << newConcave;
    }

    qreal radius3 = QLineF(oldConcave,p1).length();
    qreal radius4 = QLineF(oldConcave,p2).length();
    if (!Loose::equals(radius3,radius4))
    {
        qWarning() << "radii mismatch (2)" << radius3 << radius4;
    }
    if (!Loose::equals(radius,radius3))
    {
        qWarning() << "radii mismatch (2)" << radius << radius3;
    }
}

void  ArcData::testEnds(QPointF p1, QPointF p2, QPointF c)
{
    qreal convex_start = Point::getAngleDegrees(c,p1);
    qreal convex_end   = Point::getAngleDegrees(c,p2);

    QPointF c2 = getCenterOldConcave(p1,p2,c);
    qreal concave_start = Point::getAngleDegrees(c2,p1);
    qreal concave_end   = Point::getAngleDegrees(c2,p2);

    qInfo() << "convex:" << convex_start << convex_end << "concave:" << concave_start << concave_end;

    convex_start  = QLineF(c,p1).angle();
    convex_end    = QLineF(c,p2).angle();
    concave_start = QLineF(c2,p1).angle();
    concave_end   = QLineF(c2,p2).angle();

    qInfo() << "convex:" << convex_start << convex_end << "concave:" << concave_start << concave_end;
}

void ArcData::test1()
{
    QPointF pt[8];
    pt[0] = QPointF(1,0);
    pt[1] = QPointF(1,1);
    pt[2] = QPointF(0,1);
    pt[3] = QPointF(-1,1);
    pt[4] = QPointF(-1,0);
    pt[5] = QPointF(-1,-1);
    pt[6] = QPointF(0,-1);
    pt[7] = QPointF(1,-1);

    for (int i=0; i < 8; i++)
    {
        qDebug()  << QLineF(QPointF(),pt[i]).angle();
        qDebug()  << Point::getAngleDegrees(QPointF(),pt[i]);
    }
}

void ArcData::test2()
{
    //Convex  ArcData QRectF(-588.231,-982.346 1100x1100) center QPointF(-38.2307,-432.346) 322.824 284.556 span= -38.2679
    //Concave ArcData QRectF(-11.7693,-117.654 1100x1100) center QPointF(538.231,432.346) 104.556 142.824 span= 38.2679

    //Convex  QPointF(400,-100) QPointF(100,100) center QPointF(-38.2307,-432.346) 322.824 284.556 span= -38.2679
    //Concave QPointF(400,-100) QPointF(100,100) center QPointF(538.231,432.346) 104.556 142.824 span= 38.2679

    QPointF p1(400,-100);
    QPointF p2(100,100);
    QPointF center(-38.2307,-432.346);

    ArcData ad1(p1,p2,center,true);
    ArcData ad2(p1,p2,center,false);

    //Convex  QPointF(200,100) QPointF(100,200) center QPointF(-235.681,-235.681) 322.387 307.613 span= -14.7733
    //Concave QPointF(200,100) QPointF(100,200) center QPointF(535.681,535.681) 127.613 142.387 span= 14.7733

    p1 = QPointF(200,100);
    p2 = QPointF(100,200);
    center = QPointF(-235.681,-235.681);

    ArcData ad3(p1,p2,center,true);
    ArcData ad4(p1,p2,center,false);

    //Convex  QPointF(200,80) QPointF(120,200) center QPointF(125.805,117.203) 26.6303 265.99 span= -120.641
    //Concave QPointF(200,80) QPointF(120,200) center QPointF(194.195,162.797) 85.9896 206.63 span= 120.641 (is wrong)

    p1 = QPointF(200,80);
    p2 = QPointF(120,200);
    center = QPointF(125.805,117.203);

    ArcData ad5(p1,p2,center,true);
    ArcData ad6(p1,p2,center,false);
}
