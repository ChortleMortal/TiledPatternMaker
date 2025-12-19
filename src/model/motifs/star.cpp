////////////////////////////////////////////////////////////////////////////
//
// Star.java
//
// The classic [n/d]s star construction.  See the paper for more details.

#include <QtMath>
#include <QDebug>
#include "model/motifs/star.h"
#include "sys/geometry/intersect.h"
#include "sys/geometry/map.h"
#include "model/tilings/tile.h"
#include "gui/viewers/debug_view.h"

typedef std::shared_ptr<Star> StarPtr;

using std::make_shared;

Star::Star(int nsides, qreal dd, int ss) : RadialMotif(nsides)
{
    d = clamp_d(dd);
    s = clamp_s(ss);
    setMotifType(MOTIF_TYPE_STAR);
}

Star::Star(const Motif & fig,  int nsides, qreal dd, int ss) : RadialMotif(fig, nsides)
{
    d = clamp_d(dd);
    s = clamp_s(ss);
    setMotifType(MOTIF_TYPE_STAR);
}

Star::Star(const Star & other) : RadialMotif(other)
{
    d = other.d;
    s = other.s;
    setMotifType(MOTIF_TYPE_STAR);
}

bool Star::equals(const MotifPtr other)
{
    StarPtr otherp = std::dynamic_pointer_cast<Star>(other);
    if (!otherp)
        return  false;

    if (d != otherp->d)
        return  false;

    if (s != otherp->s)
        return false;

    if (!Motif::equals(other))
        return  false;

    return true;
}

void Star::setN(int n)
{
    qDebug() << "Star::setN()" << n;
    RadialMotif::setN(n);

    tile        = make_shared<Tile>(n);
    auto corners = tile->getPoints();

    mids.clear();
    auto prev = corners[n-1];
    for (int i=0; i < n; i++)
    {
        auto next = corners[i];
        mids << QLineF(prev,next).pointAt(0.5);
        prev = next;
    }
}

void Star::setD(qreal dd)
{
    d = clamp_d(dd);
    s = clamp_s(s);
}

void Star::setS(int ss)
{
    s = clamp_s(ss);
}

#define SCLAMP

qreal Star::clamp_d(qreal d)
{
#ifdef SCLAMP
    return qMax(1.0, qMin(d, 0.5 * qreal(getN()) - 0.01));    // range limits d
#else
    return d;
#endif
}

int Star::clamp_s(int s)
{
#ifdef SCLAMP
    qreal did   = qFloor(d);
    int   di    = int(did);
    return qMin(s, di);
#else
    return s;
#endif
}

void Star::buildUnitMap()
{
    motifDebug    = 0x0;
    if (motifDebug > 0)
    {
        Sys::debugMapCreate->wipeout();
    }

    raySet1.clear();
    raySet2.clear();

    if (getVersion() == 2)
        buildUnitV2();
    else
        buildv1();

    auto tr = getUnitRotationTransform();
    raySet2 = raySet1;
    raySet2.transform(tr);
}

void Star::buildv1()
{
    // int s    = number of intersects
    // qreal d  = angle (each integer is the next edge around the star)
    // qreal don = 1/n
    qDebug() << "Star::buildV1" << "n:" << getN() << "d:" << d << "s:" << s;

    qreal did   = qFloor(d);
    qreal dfrac = d - did;
    int   di    = int(did);
    bool d_is_int = false;

    int outer_s = qMin(s, di - 1 );

    if( dfrac < Sys::TOL )
    {
        dfrac = 0.0;
        d_is_int = true;
    }
    else if( (1.0 - dfrac) < Sys::TOL )
    {
        dfrac = 0.0;
        di = int(did) + 1;
        d_is_int = true;
    }

    QPointF a( 1.0, 0.0 );
    raySet1.ray1.addTip(a);
    raySet1.ray2.addTip(a);

    QPointF b = getArc(d * don);
    QLineF ab(a,b);

    for( int idx = 1; idx <= outer_s; ++idx )
    {
        QPointF ar = getArc(  qreal(idx) * don );
        QPointF br = getArc( (qreal(idx) - d) * don );
        QLineF arbr(ar,br);
        QPointF isect;
        if (ab.intersects(arbr,&isect) == QLineF::BoundedIntersection)
        {
            raySet1.ray1.addTail(isect);
            raySet1.ray2.addTail(QPointF(isect.x(),-isect.y()));
        }
    }

    if (s == di )
    {
        if( d_is_int )
        {
            QPointF midr = radialRotationTr.map(raySet1.ray1.getTail());
            raySet1.ray1.addTail(midr);
            raySet1.ray2.addTail(QPointF(midr.x(),-midr.y()));
        }
        else
        {
            QPointF ar = getArc( did * don );
            QPointF br = getArc( -dfrac * don );
            QPointF c  = getArc( d * don );

            QPointF isect;
            bool rv = Intersect::getIntersection(ar, br, a, c, isect);
            if (rv)
            {
                raySet1.ray1.addTail(isect);
                raySet1.ray2.addTail(QPointF(isect.x(),-isect.y()));
            }
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
}

void Star::buildUnitV2()
{
    // int s    = number of intersects
    // qreal d  = angle (each integer is the next edge around the star)
    // qreal don = 1/n

    qDebug() << "Star::buildV2"  << "n:" << getN() << "d:" << d << "s:" << s;
    //qDebug() << "mids" << mids;

    int  di       = qFloor(d);          // di is equivalent to the mids index, equivalent to the edge index;
    qreal dfrac   = d - qreal(di);
    bool d_is_int = false;
    if (dfrac < Sys::TOL)
    {
        d_is_int = true;
    }
    else if ((1.0 - dfrac) < Sys::TOL)
    {
        d_is_int = true;
        di++;
    }

    QPointF vt = mids[0];
    raySet1.ray1.addTip(vt);
    raySet1.ray2.addTip(vt);

    QLineF branchRay = getRay(0,d,1);
    for (int side = 1; side < di; ++side)
    {
        QLineF otherRay = getRay(side,d,-1);
        QPointF mid;
        if (branchRay.intersects(otherRay,&mid) == QLineF::BoundedIntersection)
        {
            raySet1.ray1.addTail(mid);
            raySet1.ray2.addTail(QPointF(mid.x(),-mid.y()));
        }
    }

    if (s == di)
    {
        if (d_is_int)
        {
            QPointF midr = radialRotationTr.map(raySet1.ray1.getTail());
            raySet1.ray1.addTail(midr);
            raySet1.ray2.addTail(QPointF(midr.x(),-midr.y()));
        }
        else
        {
            QLineF arbr = getRay(di,d,-1);
            QLineF mc   = getRay(0,d,1);
            QPointF isect;
            bool rv = Intersect::getIntersection(arbr, mc,isect);
            if (rv)
            {
                raySet1.ray1.addTail(isect);
                raySet1.ray2.addTail(QPointF(isect.x(),-isect.y()));
            }
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
}

QLineF Star::getRay(int side, qreal d, int sign)
{
    //qDebug() << "IrregularStar" << this << getN();

    side = modulo(side,getN());

    qreal d2;
    int di;
    qreal dfrac;
    if (sign == 1)
    {
        d2    = d  + side - 0.5;
        di    = qFloor(d2);          // di is equivalent to the mids index, equivalent to the edge index;
        dfrac = d2 - qreal(di);
    }
    else
    {
        d2    = getN() - d  + side - 0.5;
        di    = qFloor(d2);          // di is equivalent to the mids index, equivalent to the edge index;
        dfrac = d2 - qreal(di);
    }

    if (dfrac < Sys::TOL)
    {
        dfrac = 0.0;
    }
    else if ((1.0 - dfrac) < Sys::TOL)
    {
        dfrac    = 0.0;
        di++;
    }

    di = modulo(di,getN());

    QPointF   a =  mids[side];
    QLineF edge =  tile->getEdge(di);
    QPointF   b =  edge.pointAt(dfrac);
    QLineF branchRay(a,b);

    qDebug() << "Ray:" << "n" << getN() << "side:" << side << "sign:" << sign << "d" << d << "di" << di  << "dfrac"  << dfrac;
    qDebug() << branchRay;

    return branchRay;
}


