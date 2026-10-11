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
#include <QtMath>
#include "model/motifs/rosette.h"
#include "sys/geometry/debug_map.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/intersect.h"

typedef std::shared_ptr<Rosette>  RosettePtr;

using std::make_shared;

Rosette::Rosette(int nsides, qreal qq, int ss) : RadialMotif(nsides)
{
    q = q_clamp(qq);
    s = s_clamp(ss);
    setMotifType(Type::Rosette);
}

Rosette::Rosette(const Rosette & other) : RadialMotif(other)
{
    q     = other.q;
    s     = other.s;
    setMotifType(Type::Rosette);
}

Rosette::Rosette(const Motif & other) : RadialMotif(other)
{
    setMotifType(Type::Rosette);
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

    if (!Motif::equals(other))
        return false;

     return true;
}

void Rosette::setQ(qreal qq)
{
    q = q_clamp(qq);
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

// key point (p1) to stable intersection (p2), in unit space
QLineF Rosette::getKeyLine() const
{
    QPointF tip(1.0, 0.0 );         // The point to build from
    QPointF rtip = getArc(don());     // The next point over.

    // Consider an equilateral triangle formed by the origin,
    // up_outer and a vertical edge extending down from up_outer.
    // The center of the bottom edge of that triangle defines the
    // bisector of the angle leaving up_outer that we care about.
    qreal   qr_outer     = 1.0 / qCos( M_PI * don() );
    QPointF r_outer      = QPointF(0.0, qr_outer);
    QPointF up_outer     = getArc( 0.5 * don() ) * qr_outer;
    QPointF down_outer   = up_outer - r_outer;
    QPointF bisector     = down_outer * 0.5;
    QPointF apoint       = rtip - tip;
    QPointF stable_isect =  up_outer + (Geo::normalize(up_outer) * (-up_outer.y())) ;
    QPointF apoint2      = stable_isect - tip;
    qreal stable_angle   = Geo::getAngle(apoint2);
    //qDebug() << "stable_angle:"  << stable_angle  << qRadiansToDegrees(stable_angle);

    qreal theta = Geo::getAngle(apoint);
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

    return QLineF(key_point, stable_isect);
}

void Rosette::buildUnitRays()
{
    //qDebug().noquote() << "Rosette::buildUnit n="  << getN() << "q=" << q << "s=" << s << "scale" << getMotifScale() << "rot" << getMotifRotate();

    setMotifDebug(0);
    if (getMotifDebug() > 0)
    {
        Sys::debugMapCreate->wipeout();
    }

    QPointF tip(1.0, 0.0 );         // The point to build from

    QLineF  keyLine   = getKeyLine();
    QPointF key_point = keyLine.p1();
    QPointF key_end   = Geo::convexSum(key_point, keyLine.p2(), 10.0);

    // r means something like reverse or the other side of the center line
    QPointF key_r_point(key_point.x(), -key_point.y());
    QPointF key_r_end(  key_end.x(),   -key_end.y());

    // fill the ray
    RadialRay ray1;
    RadialRay ray2;
    ray1.addTip(tip);
    ray2.addTip(tip);
    ray1.addTail(QPointF(key_point.x(),-key_point.y()));
    ray2.addTail(key_point);

    int sclamp = s_clamp(s);

    for( int idx = 1; idx <= sclamp; ++idx )
    {
        QPointF point    = getRadialTransform(idx).map(key_r_point);
        QPointF end      = getRadialTransform(idx).map(key_r_end);

        QLineF key_line(key_point,key_end);
        QLineF key_r_line(point,end);

        QPointF isect;
        if (key_line.intersects(key_r_line,&isect) == QLineF::BoundedIntersection)
        {
            ray1.addTail(QPointF(isect.x(),-isect.y()));
            ray2.addTail(isect);
        }
    }

    unitRays.add(ray1);
    unitRays.add(ray2);

    if (inscribe)
    {
        qreal circumradius = 1.0 / (cos(M_PI / (qreal)getN()));
        QTransform t = QTransform::fromScale(circumradius,circumradius);
        unitRays.transform(t);
    }

    if (onPoint)
    {
        qreal angle = 180.0/qreal(getN());
        QTransform t;
        t.rotate(angle);
        unitRays.transform(t);
    }

    // apply motif scale and rotation
    if (getMotifDebug() & 0x01) unitRays.debug(0);
    QTransform t = getMotifTransform();
    unitRays.transform(t);
    if (getMotifDebug() & 0x01) unitRays.debug(0);

    //qDebug().noquote() << "Rosette::buildUnit n="  << getN() << "q=" << q << "s=" << s << "scale" << getMotifScale() << "rot" << getMotifRotate() << "AGAIN";
}


