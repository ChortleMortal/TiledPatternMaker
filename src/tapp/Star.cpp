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

////////////////////////////////////////////////////////////////////////////
//
// Star.java
//
// The classic [n/d]s star construction.  See the paper for more details.

#include "tapp/star.h"
#include "geometry/loose.h"
#include "geometry/intersect.h"
#include "tile/feature.h"
#include "geometry/map.h"
#include "geometry/vertex.h"

typedef std::shared_ptr<Star>             StarPtr;

using std::make_shared;

Star::Star( int nsides, qreal dd, int ss, qreal rr ) : RadialFigure(nsides, rr)
{
    d = clamp_d(dd);
    s = clamp_s(ss);
    setFigType(FIG_TYPE_STAR);
}

Star::Star(const Figure & fig,  int nsides, qreal dd, int ss, qreal rr ) : RadialFigure(fig, nsides, rr)
{
    d = clamp_d(dd);
    s = clamp_s(ss);
    setFigType(FIG_TYPE_STAR);
}

bool Star::equals(const FigurePtr other)
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

     if (getFigureRotate() != otherp->getFigureRotate())
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

MapPtr Star::buildUnit()
{
    buildExtBoundary();

    unitMap = make_shared<Map>("star unit map");

    const bool debug = false;

    // s = number of intersects
    // d = angle (each integer is the next point around the star)
    qDebug() << "Star::buildUnit"  << n << d << s;

    debugMap = make_shared<Map>("star debug map");

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

    points.erase(points.begin(),points.end());

    QPointF a( 1.0, 0.0 );
    QPointF b = getArc(d * don);
    debugMap->insertDebugMark(a,"a");
    debugMap->insertDebugMark(b,"b");
    debugMap->insertDebugLine(a,b);
    QLineF ab(a,b);

    for( int idx = 1; idx <= outer_s; ++idx )
    {
        QPointF ar = getArc(  qreal(idx) * don );
        QPointF br = getArc( (qreal(idx) - d) * don );
        QLineF arbr(ar,br);
    //    debugMap->insertDebugLine(a,b);
    //    debugMap->insertDebugLine(ar,br);
        QPointF mid;
        if (ab.intersects(arbr,&mid) == QLineF::BoundedIntersection)
        {
    //        debugMap->insertDebugMark(mid,QString("mid%1").arg(idx));
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

        unitMap->insertEdge( top_prev, top, debug);
        unitMap->insertEdge( bot_prev, bot, debug);

        top_prev = top;
        bot_prev = bot;
    }

    if (s == di )
    {
        QPointF midr = Tr.map( top_prev->pt );
        VertexPtr v4 = unitMap->insertVertex(midr);

        if( d_is_int )
        {
            unitMap->insertEdge( top_prev, v4, debug);
        }
        else
        {
            QPointF ar = getArc( did * don );
            QPointF br = getArc( -dfrac * don );

            QPointF c  = getArc( d * don );

  //          debugMap->insertDebugLine(ar,br);
  //          debugMap->insertDebugLine(a,c);
            QPointF cent;
            Intersect::getIntersection(ar, br, a, c, cent );
  //          debugMap->insertDebugMark(cent,"cent");

            VertexPtr vcent = unitMap->insertVertex(cent);
            unitMap->insertEdge( top_prev, vcent, debug);
            unitMap->insertEdge( vcent, v4, debug);
        }
    }

    //unitMap->dumpMap(false);
    //unitMap->verify("Star::buildUnit");


    // rotate
    qreal rotate = qDegreesToRadians(getFigureRotate());
    unitMap->rotate(rotate);
    debugMap->rotate(rotate);

    // scale
    unitMap->scale(getFigureScale());
    //debugMap->scale(getFigureScale());

    return unitMap;
}

