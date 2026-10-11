////////////////////////////////////////////////////////////////////////////
//
// Star.java
//
// The classic [n/d]s star construction.  See the paper for more details.

#include <QtMath>
#include <QDebug>
#include "model/motifs/star2.h"
#include "model/tilings/tile.h"
#include "sys/geometry/debug_map.h"
#include "sys/sys.h"

typedef std::shared_ptr<Star2> Star2Ptr;

using std::make_shared;

Star2::Star2(int nsides, qreal theta, int intersects) : RadialMotif(nsides)
{
    this->theta = theta;
    s = intersects;
    setMotifType(Type::Star2);
}

Star2::Star2(const Star2 & other) : RadialMotif(other)
{
    theta = other.theta;
    s     = other.s;
    setMotifType(Type::Star2);
}

Star2::Star2(const Motif & motif) : RadialMotif(motif)
{
    this->theta = 60.0;
    s = 1;
    setMotifType(Type::Star2);
}

bool Star2::equals(const MotifPtr other)
{
    Star2Ptr otherp = std::dynamic_pointer_cast<Star2>(other);
    if (!otherp)
        return  false;

    if (theta != otherp->theta)
        return  false;

    if (s != otherp->s)
        return false;

    if (!Motif::equals(other))
        return  false;

    return true;
}

void Star2::buildUnitRays()
{
    // int s = number of intersects, qreal theta  = angle

    //qDebug() << "Star2::buildUnitMap" << "n:" << getN() << "theta:" << theta << "intersects:" << s;

    setMotifDebug(0x0);
    if (getMotifDebug() > 0)
    {
        Sys::debugMapCreate->wipeout();
    }

    Tile tile(getN());
    corners = tile.getPoints();
    mids    = tile.getMids();

    RadialRay ray1;
    RadialRay ray2;

    // tip
    QPointF pt = mids[getN()-1];
    ray1.addTip(pt);
    ray2.addTip(pt);

    QLineF branchRay = getRay(getN()-1,theta,false);
    for (int side = 0; side < s; ++side)
    {
        QPointF isect;
        QLineF otherRay = getRay(side,theta,true);
        if (branchRay.intersects(otherRay,&isect) == QLineF::BoundedIntersection)
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

    if (getMotifDebug() & 0x01) unitRays.debug(0);
    QTransform t = getMotifTransform();
    unitRays.transform(t);
    if (getMotifDebug() & 0x01) unitRays.debug(0);
}

QLineF Star2::getRay(int side, qreal theta, bool sign)
{
    //side = modulo(side,getN());

    //qDebug() << "Ray:" << "n" << getN() << "side:" << side << "sign:" << sign << "theta" << theta;

    qreal angle;
    QLineF branchRay(mids[side], corners[side]);

    if (getMotifDebug() & 0x02) Sys::debugMapCreate->insertDebugMark(mids[side],QString::number(side),Qt::red);

    if (sign)
    {
        angle = branchRay.angle();
        angle += theta;
    }
    else
    {
        angle = branchRay.angle() + 180.0;
        angle -= theta;
    }
    branchRay.setAngle(angle);

    qreal len = branchRay.length();
    len *= 100;
    branchRay.setLength(len);

    if (getMotifDebug() & 0x02) Sys::debugMapCreate->insertDebugLine(branchRay,Qt::blue);

    return branchRay;
}
