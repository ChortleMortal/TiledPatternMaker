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
// The classic [n/d]s star construction.  See the paper for more
// details.

#include "tapp/Star.h"
#include "geometry/Loose.h"
#include "geometry/Intersect.h"
#include "tile/Feature.h"

Star::Star( int n, qreal d, int s, qreal r ) : RadialFigure(n, r)
{
    this->d = d;
    this->s = s;
    setFigType(FIG_TYPE_STAR);
}

Star::Star(const Figure & fig,  int n, qreal d, int s, qreal r ) : RadialFigure(fig, n, r)
{
    this->d = d;
    this->s = s;
    setFigType(FIG_TYPE_STAR);
}


void Star::setD( qreal d )
{
    this->d = d;
}

void Star::setS( int s )
{
    this->s = s;
}

MapPtr Star::buildUnit()
{
    buildExtBoundary();

    unitMap = make_shared<Map>();

    const bool debug = true;

    // s = number of intersects
    // d = angle (each integer is the next point around the star)
    qDebug() << "Star::buildUnit"  << n << d << s;

    debugMap = make_shared<Map>();

    qreal clamp_d = qMax( 1.0, qMin( d, 0.5 * dn - 0.01 ) );    // range limits d

    qreal did   = qFloor( clamp_d );
    qreal dfrac = clamp_d - did;
    int   di    = int(did);
    bool d_is_int = false;

    int clamp_s = qMin( s, di );
    int outer_s = qMin( s, di - 1 );

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

    qDebug() << "d:"  << d <<  "clamp_d:" << clamp_d  << "s-intersects:" << s << "clamp_s:" << clamp_s << "outer_s" << outer_s << "d_is_int:" << d_is_int;


    points.erase(points.begin(),points.end());

    QPointF a( 1.0, 0.0 );
    QPointF b = getArc( clamp_d * don );

    for( int idx = 1; idx <= outer_s; ++idx )
    {
        QPointF ar = getArc(  qreal(idx) * don );
        QPointF br = getArc( (qreal(idx) - clamp_d) * don );

        debugMap->insertDebugLine(a,b);
        debugMap->insertDebugLine(ar,br);
        QPointF mid = Intersect::getIntersection( a, b, ar, br );
        debugMap->insertDebugMark(mid,"mid");
        points.push_back(mid);
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

    if( clamp_s == di )
    {
        QPointF midr = Tr.map( top_prev->getPosition() );
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

            debugMap->insertDebugLine(ar,br);
            debugMap->insertDebugLine(a,c);
            QPointF   cent = Intersect::getIntersection( ar, br, a, c );
            debugMap->insertDebugMark(cent,"cent");

            VertexPtr vcent = unitMap->insertVertex(cent);
            unitMap->insertEdge( top_prev, vcent, debug);
            unitMap->insertEdge( vcent, v4, debug);
        }
    }

    //qDebug().noquote() << "Star: points =" << points.size() << unitMap->getInfo();
    //unitMap->verify("Star::buildUnit",false);

    // rotate
    qreal rotate = qDegreesToRadians(r);
    unitMap->rotate(rotate);
    debugMap->rotate(rotate);

    // scale
    unitMap->scale(getFigureScale());
    debugMap->scale(getFigureScale());

    return unitMap;
}

