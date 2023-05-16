////////////////////////////////////////////////////////////////////////////
//
// Rosette.java
//
// The rosette is a classic feature of Islamic art.  It's a star
// shape surrounded by hexagons.
//
// This class implements the rosette as a RadialFigure using the
// geometric construction given in Lee [1].
//
// [1] A.J. Lee, _Islamic Star Patterns_.  Muqarnas 4.

#include <QDebug>
#include "motifs/rosette.h"
#include "geometry/point.h"
#include "geometry/intersect.h"
#include "geometry/transform.h"
#include "geometry/map.h"

typedef std::shared_ptr<Rosette>          RosettePtr;

using std::make_shared;

Rosette::Rosette(const Motif & fig,  int nsides, qreal qq, int ss, qreal kk) : RadialMotif(fig, nsides)
{
    q = q_clamp(qq);
    s = s_clamp(ss);
    k = kk;
    setMotifType(MOTIF_TYPE_ROSETTE);
}

Rosette::Rosette(int nsides, qreal qq, int ss, qreal kk) : RadialMotif(nsides)
{
    q = q_clamp(qq);
    s = s_clamp(ss);
    k = kk;
    setMotifType(MOTIF_TYPE_ROSETTE);
}

Rosette::Rosette(const Rosette & other) : RadialMotif(other)
{
    q     = other.q;
    k     = other.k;
    s     = other.s;
}

bool Rosette::equals(const MotifPtr other)
{
    RosettePtr otherp = std::dynamic_pointer_cast<Rosette>(other);
    if (!otherp)
        return  false;

    if (q != otherp->q)
        return  false;

    if (s != otherp->s)
        return false;

    if (k != otherp->k)
        return  false;

    if (!Motif::equals(other))
        return false;

     return true;
}

void Rosette::setQ(qreal qq)
{
    q = q_clamp(qq);
}

void Rosette::setK(qreal kk)
{
    k = kk;
}

void Rosette::setS(int ss)
{
    s = s_clamp(ss);
}

void Rosette::setN(int nn)
{
    RadialMotif::setN(nn);
    s = s_clamp(s);
}

int Rosette::s_clamp(int s)
{
    //return s;
    return qMin(s, (getN()-1)/2);
}

qreal Rosette::q_clamp(qreal q)
{
    //return q;
    return qMin(qMax(q, -0.99), 0.99);
}

void Rosette::buildUnitMap()
{
#if 1
    debugMap = make_shared<DebugMap>("rosette debug map");
#endif

    qDebug().noquote() << "Rosette::buildUnit"  << getN() << q << s << "Tr:" << Transform::toInfoString(Tr) << "rot" << getMotifRotate();

    buildRadialBoundaries();

    QPointF center(0.0,0.0);
    QPointF tip(1.0, 0.0 );         // The point to build from
    QPointF rtip = getArc(don);     // The next point over.

    // Consider an equilateral triangle formed by the origin,
    // up_outer and a vertical edge extending down from up_outer.
    // The center of the bottom edge of that triangle defines the
    // bisector of the angle leaving up_outer that we care about.
    qreal   qr_outer    = 1.0 / qCos( M_PI * don );
    QPointF r_outer     = QPointF(0.0, qr_outer);
    QPointF up_outer    = getArc( 0.5 * don ) * qr_outer;
    QPointF down_outer  = up_outer - r_outer;
    QPointF bisector    = down_outer * 0.5;
    if (debugMap)
    {
        debugMap->insertDebugMark(center,"center");
        debugMap->insertDebugMark(tip,"tip");
        debugMap->insertDebugMark(rtip,"rtip");
        debugMap->insertDebugMark(r_outer,"r_outer");
        debugMap->insertDebugMark(up_outer,"up_outer");
        debugMap->insertDebugMark(down_outer,"down_outer");
        debugMap->insertDebugMark(bisector,"bisector");
    }

    QPointF apoint       = rtip - tip;
    QPointF norm_up_outer= Point::normalize(up_outer);
    QPointF stable_isect = (up_outer + (Point::normalize(up_outer)) * (-up_outer.y()) );
    QPointF apoint2      = stable_isect - tip;
    qreal stable_angle   = Point::getAngle(apoint2);
    //qDebug() << "stable_angle:"  << stable_angle  << qRadiansToDegrees(stable_angle);
    stable_isect.setY(stable_isect.y() - k);    // uses k

    if (debugMap)
    {
        debugMap->insertDebugMark(norm_up_outer,"norm_up_out");
        debugMap->insertDebugMark(apoint,"apoint");
        debugMap->insertDebugMark(stable_isect,"stable_isect");
        debugMap->insertDebugMark(apoint2,"apoint2");
    }

    qreal theta = Point::getAngle(apoint);
    qreal theta2;
    if (q >= 0.0)
    {
        theta2 = (theta * (1.0 - q)) + ((M_PI * 0.5) * q);
    }
    else
    {
        //theta2 = theta * (1.0 - (-q)) + M_PI * (-q);
        theta2 = (theta * (1.0 + q)) - (stable_angle * q);
    }
    //qDebug() << "theta=" << qRadiansToDegrees(theta)  << "theta2=" << qRadiansToDegrees(theta2)  << "stable_angle=" << qRadiansToDegrees(stable_angle);

    // Heh heh - you said q-tip - heh heh.
    QPointF qtip( 1.0 + qCos(theta2), qSin(theta2));

    QPointF key_point;
    Intersect::getIntersection(tip, qtip, up_outer, bisector, key_point);

    QPointF key_end = Point::convexSum(key_point, stable_isect, 10.0);

    // r means something like reverse or the other side of the center line
    QPointF key_r_point(key_point.x(), -key_point.y());
    QPointF key_r_end(  key_end.x(),   -key_end.y());

    if (debugMap)
    {
        debugMap->insertDebugLine(tip, qtip);              // adjusting q adjusts qtip
        debugMap->insertDebugLine(up_outer, bisector);     // this is the line along hich the key_point (aka) shoulder moved
        //debugMap->insertDebugLine(key_point,key_end);
        debugMap->insertDebugLine(key_r_point,key_r_end);

        debugMap->insertDebugMark(qtip,"qtip");
        debugMap->insertDebugMark(key_point,"key_point");
        debugMap->insertDebugMark(key_end,"key_end");
        debugMap->insertDebugMark(key_r_point,"key_r_point_0");
        debugMap->insertDebugMark(key_r_end,"key_r_end_0");
    }

    unitMap = make_shared<Map>("rosette unit map");

    // fill the map
    QPolygonF epoints;

    epoints.push_back(key_point);

    int sclamp = s_clamp(s);

    for( int idx = 1; idx <= sclamp; ++idx )
    {
        if (k == 0.0)
        {
            key_r_point    = Tr.map(key_r_point);
            key_r_end      = Tr.map(key_r_end);
        }
        else
        {
            qreal rot      = -Transform::rotation(Tr);
            QTransform Tr2 = QTransform().rotateRadians(rot);

            key_r_point    = Tr2.map(key_r_point);
            key_r_end      = Tr2.map(key_r_end);
        }
        QLineF key_line(key_point,key_end);
        QLineF key_r_line(key_r_point,key_r_end);

        QPointF isect;
        if (key_line.intersects(key_r_line,&isect) == QLineF::BoundedIntersection)
        {
            //qDebug().noquote() << QString("isect%1").arg(idx) << isect;
            if (debugMap && idx == 1)
            {
                debugMap->insertDebugLine(key_r_point,key_r_end);

                debugMap->insertDebugMark(key_r_point,QString("key_r_point%1").arg(idx));
                debugMap->insertDebugMark(key_r_end,QString("key_r_end%1").arg(idx));
                debugMap->insertDebugMark(isect,QString("isect%1").arg(idx));
            }

            epoints.push_back(isect);
        }
    }

    // setup the map
    VertexPtr vt       = unitMap->insertVertex(tip);
    VertexPtr top_prev = vt;
    VertexPtr bot_prev = vt;

    for( int idx = 0; idx < epoints.size(); ++idx )
    {
        VertexPtr top = unitMap->insertVertex(epoints[idx]);
        VertexPtr bot = unitMap->insertVertex(QPointF(epoints[idx].x(), - epoints[idx].y()));

        unitMap->insertEdge(top_prev, top);
        unitMap->insertEdge(bot_prev, bot);

        top_prev = top;
        bot_prev = bot;
    }

    //qDebug().noquote() << "Rosette: epoints =" << epoints.size() << unitMap->getInfo();
    //unitMap->verify("Rosette::buildUnit",false);

    // rotate and scale
    qreal rotate = qDegreesToRadians(getMotifRotate());
    unitMap->rotate(rotate);
    unitMap->scale(getMotifScale());

    if (debugMap)
    {
        debugMap->rotate(rotate);
        debugMap->scale(getMotifScale());
    }
}
