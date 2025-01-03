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
#include "sys/geometry/geo.h"
#include "sys/geometry/intersect.h"
#include "sys/geometry/map.h"
#include "gui/viewers/debug_view.h"

typedef std::shared_ptr<Rosette>  RosettePtr;

using std::make_shared;

Rosette::Rosette(const Motif & fig,  int nsides, qreal qq, int ss) : RadialMotif(fig, nsides)
{
    q = q_clamp(qq);
    s = s_clamp(ss);
    setMotifType(MOTIF_TYPE_ROSETTE);
}

Rosette::Rosette(int nsides, qreal qq, int ss) : RadialMotif(nsides)
{
    q = q_clamp(qq);
    s = s_clamp(ss);
    setMotifType(MOTIF_TYPE_ROSETTE);
}

Rosette::Rosette(const Rosette & other) : RadialMotif(other)
{
    q     = other.q;
    s     = other.s;
    setMotifType(MOTIF_TYPE_ROSETTE);
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

void Rosette::buildUnitMap()
{
    qDebug().noquote() << "Rosette::buildUnit"  << getN() << q << s << "scale" << getMotifScale() << "rot" << getMotifRotate();

    dbgVal = 0x100;
    debugMap = nullptr;
    if (dbgVal > 0)
    {
        debugMap = Sys::debugView->getMap();
        debugMap->wipeout();
    }

    raySet1.clear();

    //QPointF center(0.0,0.0);
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

    QPointF apoint       = rtip - tip;
    //QPointF norm_up_outer= Geo::normalize(up_outer);
    QPointF stable_isect = (up_outer + (Geo::normalize(up_outer)) * (-up_outer.y()) );
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
    
    QPointF key_end = Geo::convexSum(key_point, stable_isect, 10.0);

    // r means something like reverse or the other side of the center line
    QPointF key_r_point(key_point.x(), -key_point.y());
    QPointF key_r_end(  key_end.x(),   -key_end.y());

    // fill the ray
    raySet1.ray1.addTip(tip);
    raySet1.ray2.addTip(tip);
    raySet1.ray1.addTail(key_point);
    raySet1.ray2.addTail(QPointF(key_point.x(),-key_point.y()));

    int sclamp = s_clamp(s);

    for( int idx = 1; idx <= sclamp; ++idx )
    {
        key_r_point    = radialRotationTr.map(key_r_point);
        key_r_end      = radialRotationTr.map(key_r_end);

        QLineF key_line(key_point,key_end);
        QLineF key_r_line(key_r_point,key_r_end);

        QPointF isect;
        if (key_line.intersects(key_r_line,&isect) == QLineF::BoundedIntersection)
        {
            raySet1.ray1.addTail(isect);
            raySet1.ray2.addTail(QPointF(isect.x(),-isect.y()));
        }
    }

    if (inscribe)
    {
        qreal circumradius = 1.0 / (cos(M_PI / (qreal)getN()));
        QTransform t = QTransform::fromScale(circumradius,circumradius);
        raySet1.transform(t);
    }

    if (onPoint)
    {
        qreal angle = 180.0/qreal(getN());
        QTransform t;
        t.rotate(angle);
        raySet1.transform(t);
    }

    if (dbgVal & 0x01) raySet1.debug();
    QTransform t = getDELTransform();
    raySet1.transform(t);
    if (dbgVal & 0x01) raySet1.debug();

    auto tr = getUnitRotationTransform();
    raySet2 = raySet1;
    raySet2.transform(tr);
    if (dbgVal & 0x01) raySet2.debug();
}


