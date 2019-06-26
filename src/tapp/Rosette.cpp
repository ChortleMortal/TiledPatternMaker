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
// Rosette.java
//
// The rosette is a classic feature of Islamic art.  It's a star
// shape surrounded by hexagons.
//
// This class implements the rosette as a RadialFigure using the
// geometric construction given in Lee [1].
//
// [1] A.J. Lee, _Islamic Star Patterns_.  Muqarnas 4.

#include "tapp/Rosette.h"
#include "tapp/Star.h"
#include "geometry/Point.h"
#include "geometry/Intersect.h"
#include "base/shapefactory.h"
#include "base/misc.h"
#include "base/canvas.h"
#include "tile/Feature.h"

Rosette::Rosette(const Figure & fig,  int n, qreal q, int s, qreal k, qreal r)
    : RadialFigure(fig, n, r)
{
    this->q = q;
    //this->s = qMin( s, (n-1) / 2 ); // DAC remove limiting here - is done in buildUnit
    this->s = s;
    this->k = k;
    setFigType(FIG_TYPE_ROSETTE);
    count = 0;
}

Rosette::Rosette(int n, qreal q, int s, qreal k, qreal r) : RadialFigure(n, r)
{
    this->q = q;
    //this->s = qMin( s, (n-1) / 2 ); // DAC remove limiting here - is done in buildUnit
    this->s = s;
    this->k = k;
    setFigType(FIG_TYPE_ROSETTE);
    count = 0;
}

void Rosette::setQ( qreal q )
{
    this->q = q;
}

void Rosette::setK( qreal k )
{
    this->k = k;
}

void Rosette::setS( int s )
{
    //this->s = qMin( s, (n-1) / 2 ); // DAC remove limiting here - is done in buildUnit
    this->s = s;
}

void Rosette::setN(int n)
{
    RadialFigure::setN(n);
    //this->s = qMin( s, (n-1) / 2 );  // DAC remove limiting here - is done in buildUnit
}

MapPtr Rosette::buildUnit()
{
    qDebug().noquote() << "Rosette::buildUnit"  << n << q << s << "Tr:" << Tr.toString();

    buildBoundary();

    debugMap = make_shared<Map>();

    QPointF center(0.0,0.0);
    QPointF tip(1.0, 0.0 );         // The point to build from
    QPointF rtip  = getArc( don );  // The next point over.

    debugMap->insertDebugMark(center,"center");
    debugMap->insertDebugMark(tip,"tip",0.1);
    debugMap->insertDebugMark(rtip,"rtip",0.1);

    qreal q_clamp = qMin( qMax( q, -0.99 ), 0.99 );
    // Consider an equilateral triangle formed by the origin,
    // up_outer and a vertical edge extending down from up_outer.
    // The center of the bottom edge of that triangle defines the
    // bisector of the angle leaving up_outer that we care about.
    qreal   qr_outer    = 1.0 / qCos( M_PI * don );
    QPointF r_outer(0.0, qr_outer);
    QPointF up_outer    = getArc( 0.5 * don ) * qr_outer;
    QPointF down_outer  = up_outer - r_outer;
    QPointF bisector    = down_outer * 0.5;

    debugMap->insertDebugMark(down_outer,"down_outer",0.1);
    debugMap->insertDebugMark(up_outer,"up_outer",0.1);
    debugMap->insertDebugMark(r_outer,"r_outer",0.1);
    debugMap->insertDebugMark(bisector,"bisector",0.1);

    QPointF apoint = rtip - tip;

    debugMap->insertDebugMark(apoint,"apoint",0.1);

    qreal theta = Point::getAngle(apoint);
    qDebug() << "theta:"  << theta  << qRadiansToDegrees(theta);

    QPointF stable_isect = (up_outer + (Point::normalize(up_outer)) * (-up_outer.y()) );
    stable_isect.setY(stable_isect.y() - k);    // uses k

    debugMap->insertDebugMark(stable_isect,"stable_isect",0.1);

    QPointF apoint2      = stable_isect - tip;
    qreal stable_angle   = Point::getAngle(apoint2);
    qDebug() << "stable_angle:"  << stable_angle  << qRadiansToDegrees(stable_angle);

    qreal theta2;
    if( q_clamp >= 0.0 )
    {
        theta2 = (theta * (1.0 - q_clamp)) + ((M_PI * 0.5) * q_clamp);
    }
    else
    {
        //theta2 = theta * (1.0 - (-q_clamp)) + M_PI * (-q_clamp);
        theta2 = (theta * (1.0 + q_clamp)) - (stable_angle * q_clamp);
    }
    qDebug() << "theta=" << qRadiansToDegrees(theta)  << "theta2=" << qRadiansToDegrees(theta2)  << "stable_angle=" << qRadiansToDegrees(stable_angle);

    // Heh heh - you said q-tip - heh heh.
    QPointF qtip( 1.0 + qCos( theta2 ), qSin( theta2 ) );
    debugMap->insertDebugMark(qtip,"qtip",0.1);


    QPointF key_point = Intersect::getIntersection(tip, qtip, up_outer, bisector );
    debugMap->insertDebugLine(tip, qtip);
    debugMap->insertDebugLine(up_outer, bisector);

    QPointF key_end   = Point::convexSum(key_point, stable_isect, 10.0 );

    debugMap->insertDebugMark(key_point,"key_point",0.1);
    debugMap->insertDebugMark(key_end,"key_end",0.1);
    debugMap->insertDebugLine(key_point,key_end);

    QPointF key_r_point( key_point.x(), -key_point.y() );
    QPointF key_r_end(   key_end.x(),   -key_end.y() );

    debugMap->insertDebugMark(key_r_point,"key_r_point_0",0.1);
    debugMap->insertDebugMark(key_r_end,"key_r_end_0",0.1);
    debugMap->insertDebugLine(key_r_point,key_r_end);

    // create new map and points to put into it
    unitMap = make_shared<Map>();
    points.erase(points.begin(),points.end());

    // fill the map
    points.push_back(key_point);

    if (k == 0.0)
    {
        int s_clamp   = qMin( s, (n-1) / 2 );
        for( int idx = 1; idx <= s_clamp; ++idx )
        {
            Tr.applyD(key_r_point);
            Tr.applyD(key_r_end);

            debugMap->insertDebugMark(key_r_point,QString("key_r_point_%1").arg(idx),0.1);
            debugMap->insertDebugMark(key_r_end,QString("key_r_end_%1").arg(idx),0.1);
            debugMap->insertDebugLine(key_r_point,key_r_end);

            QPointF middle = Intersect::getIntersection(key_point, key_end, key_r_point, key_r_end );
            debugMap->insertDebugMark(middle,QString("middle_%1").arg(idx),0.1);
            qDebug().noquote() << QString("middle_%1").arg(idx) << middle;

            // FIXMECSK --
            // For some combinations of n, q and s (for example, n = 12, q = -0.8, s = 4), the intersection fails
            // because the line segments being checked end up parallel.  Rather than knowing mathematically when
            // that happens, I punt and check after the fact whether the intersection test failed.
            if (!middle.isNull())
            {
                points.push_back(middle);
            }
        }
    }
    else
    {
        qreal rot = -Tr.rotation();
        Transform Tr2 = Transform::rotate(rot);

        Tr2.applyD(key_r_point);
        Tr2.applyD(key_r_end);

        debugMap->insertDebugMark(key_r_point,QString("key_r_point_%1").arg(s-1),0.1);
        debugMap->insertDebugMark(key_r_end,QString("key_r_end_%1").arg(s-1),0.1);
        debugMap->insertDebugLine(key_r_point,key_r_end);

        QPointF middle = Intersect::getIntersection(key_point, key_end, key_r_point, key_r_end );
        debugMap->insertDebugMark(middle,QString("middle_%1").arg(s-1),0.1);
        qDebug().noquote() << QString("middle_%1").arg(s-1) << middle;

        if( !middle.isNull())
        {
            points.push_back(middle);
        }
    }

    // setup the map
    VertexPtr vt       = unitMap->insertVertex(tip);
    VertexPtr top_prev = vt;
    VertexPtr bot_prev = vt;

    for( int idx = 0; idx < points.size(); ++idx )
    {
        VertexPtr top = unitMap->insertVertex( points[idx]);
        VertexPtr bot = unitMap->insertVertex(QPointF( points[idx].x(), -points[idx].y()));

        unitMap->insertEdge( top_prev, top, true);
        unitMap->insertEdge( bot_prev, bot, true);

        top_prev = top;
        bot_prev = bot;
    }

    //qDebug().noquote() << "Rosette: points =" << points.size() << unitMap->getInfo();
    //unitMap->verify("Rosette::buildUnit",false);

    // rotate
    qreal rotate = qDegreesToRadians(r);
    unitMap->rotate(rotate);
    debugMap->rotate(rotate);

    // scale
    unitMap->scale(figureScale);
    debugMap->scale(figureScale);

    unitMap->verify("rosette unitMap", false,true);
    return unitMap;
}
