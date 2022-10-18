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

typedef std::shared_ptr<Star>             StarPtr;

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

    if (n != otherp->n)
        return  false;

     if (getMotifRotate() != otherp->getMotifRotate())
         return false;

     if (getMotifScale() != otherp->getMotifScale())
         return false;

     return true;
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

qreal Star::clamp_d(qreal d)
{
    return qMax(1.0, qMin(d, 0.5 * dn - 0.01));    // range limits d
}

int Star::clamp_s(int s)
{
    qreal did   = qFloor(d);
    int   di    = int(did);
    return qMin(s, di);
}

void Star::buildUnitMap()
{
    const bool debug = false;

    buildRadialBoundaries();

    unitMap = make_shared<Map>("star unit map");

    // s = number of intersects
    // d = angle (each integer is the next point around the star)
    qDebug() << "Star::buildUnit"  << n << d << s;

    if (debug) debugMap = make_shared<DebugMap>("star debug map");

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

    qDebug() << "d:"  << d << "s-intersects:" << s << "s:" << s << "outer_s" << outer_s << "d_is_int:" << d_is_int;

    points.clear();

    QPointF a( 1.0, 0.0 );
    QPointF b = getArc(d * don);
    QLineF ab(a,b);
    if (debug)
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
        if (debug)
        {
            debugMap->insertDebugLine(a,b);
            debugMap->insertDebugLine(ar,br);
        }
        QPointF mid;
        if (ab.intersects(arbr,&mid) == QLineF::BoundedIntersection)
        {
            if (debug) debugMap->insertDebugMark(mid,QString("mid%1").arg(idx));
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
            if (debug)
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

    if (debug)
    {
        debugMap->rotate(rotate);
        debugMap->scale(getMotifScale());
    }
}

