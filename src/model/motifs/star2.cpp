////////////////////////////////////////////////////////////////////////////
//
// Star.java
//
// The classic [n/d]s star construction.  See the paper for more details.

#include <QtMath>
#include <QDebug>
#include "model/motifs/star2.h"
#include "sys/geometry/map.h"
#include "model/tilings/tile.h"
#include "gui/viewers/debug_view.h"

typedef std::shared_ptr<Star2> Star2Ptr;

using std::make_shared;

Star2::Star2(int nsides, qreal theta, int intersects) : RadialMotif(nsides)
{
    this->theta = theta;
    s = intersects;
    setMotifType(MOTIF_TYPE_STAR2);
}

Star2::Star2(const Motif & fig,  int nsides, qreal theta, int intersects) : RadialMotif(fig, nsides)
{
    this->theta = theta;
    s = intersects;
    setMotifType(MOTIF_TYPE_STAR2);
}

Star2::Star2(const Star2 & other) : RadialMotif(other)
{
    theta = other.theta;
    s     = other.s;
    setMotifType(MOTIF_TYPE_STAR2);
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

void Star2::buildUnitMap()
{
    // int s = number of intersects, qreal theta  = angle

    //qDebug() << "Star2::buildUnitMap" << "n:" << getN() << "theta:" << theta << "intersects:" << s;

    motifDebug = 0x0;
    if (motifDebug > 0)
    {
        Sys::debugMapCreate->wipeout();
    }

    Tile tile(getN());
    corners = tile.getPoints();
    mids    = tile.getMids();

    raySet1.clear();
    raySet2.clear();

    // tip
    QPointF pt = mids[getN()-1];
    raySet1.ray1.addTip(pt);
    raySet1.ray2.addTip(pt);

    QLineF branchRay = getRay(getN()-1,theta,false);
    for (int side = 0; side < s; ++side)
    {
        QPointF isect;
        QLineF otherRay = getRay(side,theta,true);
        if (branchRay.intersects(otherRay,&isect) == QLineF::BoundedIntersection)
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

    if (motifDebug & 0x01) raySet1.debug();
    QTransform t = getDELTransform();
    raySet1.transform(t);
    if (motifDebug & 0x01) raySet1.debug();

    auto tr = getUnitRotationTransform();
    raySet2 = raySet1;
    raySet2.transform(tr);
}

QLineF Star2::getRay(int side, qreal theta, bool sign)
{
    //side = modulo(side,getN());

    //qDebug() << "Ray:" << "n" << getN() << "side:" << side << "sign:" << sign << "theta" << theta;

    qreal angle;
    QLineF branchRay(mids[side], corners[side]);

    if (motifDebug & 0x02) Sys::debugMapCreate->insertDebugMark(mids[side],QString::number(side),Qt::red);

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

    if (motifDebug & 0x02) Sys::debugMapCreate->insertDebugLine(branchRay,Qt::blue);

    return branchRay;
}
