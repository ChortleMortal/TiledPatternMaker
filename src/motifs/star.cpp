////////////////////////////////////////////////////////////////////////////
//
// Star.java
//
// The classic [n/d]s star construction.  See the paper for more details.

#include <QtMath>
#include <QDebug>
#include "motifs/star.h"
#include "geometry/loose.h"
#include "geometry/intersect.h"
#include "geometry/map.h"
#include "geometry/vertex.h"
#include "tile/tile.h"

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
    placedTile  = tile->getEdgePoly();
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
    s = clamp_s(d);
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
    if (getVersion() == 2)
        buildV2();
    else
        buildV1();
}

void Star::buildV1()
{
    qDebug() << "Star::buildV2";

    unitMap = make_shared<Map>("star unit map");

    // int s    = number of intersects
    // qreal d  = angle (each integer is the next edge around the star)
    // qreal don = 1/n
    qDebug() << "Star::buildUnit";
    qDebug() << "n:" << getN() << "d:" << d << "s:" << s;

#if 1
    debugMap = make_shared<DebugMap>("star debug map");
#endif

    qreal did   = qFloor(d);
    qreal dfrac = d - did;
    int   di    = int(did);
    bool d_is_int = false;

    int outer_s = qMin(s, di - 1 );

    if( dfrac < Loose::TOL )
    {
        dfrac = 0.0;
        d_is_int = true;
    }
    else if( (1.0 - dfrac) < Loose::TOL )
    {
        dfrac = 0.0;
        di = int(did) + 1;
        d_is_int = true;
    }

    qDebug() << "n:" << getN() << "d:" << d << "s:" << s << "di:" << di <<  "d_is_int:" << d_is_int;

    points.clear();

    QPointF a( 1.0, 0.0 );
    QPointF b = getArc(d * don);
    QLineF ab(a,b);
    qDebug() << "Ray" << ab;
    if (debugMap)
    {
        debugMap->insertDebugMark(a,"a");
        debugMap->insertDebugMark(b,"b");
        debugMap->insertDebugLine(a,b);
    }

    for( int idx = 1; idx <= outer_s; ++idx )
    {
        QPointF ar = getArc(  qreal(idx) * don );
        QPointF br = getArc( (qreal(idx) - d) * don );
        QLineF arbr(ar,br);
        if (debugMap)
        {
            debugMap->insertDebugLine(a,b);
            debugMap->insertDebugLine(ar,br);
        }
        QPointF mid;
        if (ab.intersects(arbr,&mid) == QLineF::BoundedIntersection)
        {
            if (debugMap)
                debugMap->insertDebugMark(mid,QString("mid%1").arg(idx));
            points.push_back(mid);
        }
    }

    VertexPtr vt = unitMap->insertVertex(a);
    VertexPtr top_prev = vt;
    VertexPtr bot_prev = vt;

    for( int idx = 0; idx < points.size(); ++idx )
    {
        VertexPtr top = unitMap->insertVertex( points[ idx ]);
        VertexPtr bot = unitMap->insertVertex( QPointF(points[idx].x(), -points[idx].y()));  // mirror

        unitMap->insertEdge( top_prev, top);
        unitMap->insertEdge( bot_prev, bot);

        top_prev = top;
        bot_prev = bot;
    }

    if (s == di )
    {
        QPointF midr = Tr.map( top_prev->pt );
        VertexPtr v4 = unitMap->insertVertex(midr);

        if( d_is_int )
        {
            unitMap->insertEdge( top_prev, v4);
        }
        else
        {
            QPointF ar = getArc( did * don );
            QPointF br = getArc( -dfrac * don );
            QPointF c  = getArc( d * don );

            QPointF cent;
            Intersect::getIntersection(ar, br, a, c, cent );
            if (debugMap)
            {
                debugMap->insertDebugLine(ar,br);
                debugMap->insertDebugLine(a,c);
                debugMap->insertDebugMark(cent,"cent");
            }

            VertexPtr vcent = unitMap->insertVertex(cent);
            unitMap->insertEdge(top_prev, vcent);
            unitMap->insertEdge(vcent, v4);
        }
    }

    //unitMap->dumpMap(false);
    //unitMap->verify("Star::buildUnit");

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

void Star::buildV2()
{
    qDebug() << "Star::buildV2";

    unitMap = make_shared<Map>("star unit map");

    // int s    = number of intersects
    // qreal d  = angle (each integer is the next edge around the star)
    // qreal don = 1/n
    qDebug() << "n:" << getN() << "d:" << d << "s:" << s;

#if 1
    debugMap = make_shared<DebugMap>("star debug map");
#endif

    int  di       = qFloor(d);          // di is equivalent to the mids index, equivalent to the edge index;
    qreal dfrac   = d - qreal(di);
    bool d_is_int = false;
    if (dfrac < Loose::TOL)
    {
        d_is_int = true;
    }
    else if ((1.0 - dfrac) < Loose::TOL)
    {
        d_is_int = true;
        di++;
    }

    qDebug() << "n:" << getN() << "d:" << d << "s:" << s << "di:" << di <<  "d_is_int:" << d_is_int;

    QLineF branchRay = getRay(0,d,1);
    qDebug() << "Ray" << branchRay;
    if (debugMap)
    {
        qreal angle = branchRay.angle();
        debugMap->insertDebugMark(branchRay.p1(),"a0");
        debugMap->insertDebugMark(branchRay.p2(),QString::number(angle));
        debugMap->insertDebugLine(branchRay);
    }

    QVector<QPointF> isects;
    for (int side = 1; side < di; ++side)
    {
        QLineF otherRay = getRay(side,d,-1);
        if (debugMap)
        {
            qreal angle = otherRay.angle();
            debugMap->insertDebugMark(otherRay.p1(),QString("a%1").arg(side));
            debugMap->insertDebugMark(otherRay.p2(),QString::number(angle));
            debugMap->insertDebugLine(otherRay);
        }
        QPointF mid;
        if (branchRay.intersects(otherRay,&mid) == QLineF::BoundedIntersection)
        {
            if (debugMap)
                debugMap->insertDebugMark(mid,QString("mid%1").arg(side));
            isects.push_back(mid);
        }
    }

    VertexPtr vt = unitMap->insertVertex(mids[0]);
    VertexPtr top_prev = vt;
    VertexPtr bot_prev = vt;

    for( int idx = 0; idx < isects.size() && idx < s; ++idx )
    {
        VertexPtr top = unitMap->insertVertex(isects[idx]);
        VertexPtr bot = unitMap->insertVertex(QPointF(isects[idx].x(), -isects[idx].y()));  // mirror

        unitMap->insertEdge(top_prev, top);
        unitMap->insertEdge(bot_prev, bot);

        top_prev = top;
        bot_prev = bot;
    }

    if (s == di)
    {
        QPointF midr = Tr.map(top_prev->pt);
        VertexPtr v4 = unitMap->insertVertex(midr);

        if (d_is_int)
        {
            unitMap->insertEdge(top_prev, v4);
            if (debugMap)
            {
                debugMap->insertDebugMark(top_prev->pt,"top_prev");
                debugMap->insertDebugMark(v4->pt,"v4");
                debugMap->insertDebugLine(top_prev->pt,v4->pt);
            }
        }
        else
        {
            QLineF arbr = getRay(di,d,-1);
            QLineF mc = getRay(0,d,1);
            QPointF isect;
            bool rv = Intersect::getIntersection(arbr, mc,isect);
            if (debugMap)
            {
                debugMap->insertDebugMark(arbr.p1(),"ar");
                debugMap->insertDebugMark(arbr.p2(),"br");
                debugMap->insertDebugMark(mc.p2(),"c");
                debugMap->insertDebugLine(arbr);
                debugMap->insertDebugLine(mc);
                debugMap->insertDebugMark(isect,"isect");
            }
            if (rv)
            {
                VertexPtr vcent = unitMap->insertVertex(isect);
                unitMap->insertEdge(top_prev, vcent);
                unitMap->insertEdge(vcent, v4);
                if (debugMap)
                {
                    debugMap->insertDebugMark(top_prev->pt,"top_prev");
                    debugMap->insertDebugMark(isect,"isect");
                    debugMap->insertDebugMark(v4->pt,"v4");
                    debugMap->insertDebugLine(top_prev->pt,vcent->pt);
                    debugMap->insertDebugLine(vcent->pt,v4->pt);
                }
            }
        }
    }

    //unitMap->dumpMap(false);
    //unitMap->verify("Star::buildUnit");

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

    if (dfrac < Loose::TOL)
    {
        dfrac = 0.0;
    }
    else if ((1.0 - dfrac) < Loose::TOL)
    {
        dfrac    = 0.0;
        di++;
    }

    di = modulo(di,getN());

    QPointF   a =  mids[side];
    QLineF edge =  placedTile.getEdge(di);
    QPointF   b =  edge.pointAt(dfrac);
    QLineF branchRay(a,b);

    //qDebug() << "Ray:" << "n" << getN() << "side:" << side << "sign:" << sign << "d" << d << "di" << di  << "dfrac"  << dfrac;

    return branchRay;
}


