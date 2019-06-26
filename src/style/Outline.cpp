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

#include "style/Outline.h"
#include "geometry/Point.h"
#include <QPainter>
////////////////////////////////////////////////////////////////////////////
//
// Outline.java
//
// The simplest non-trivial rendering style.  Outline just uses
// some trig to fatten up the map's edges, also drawing a line-based
// outline for the resulting fat figure.
//
// The same code that computes the draw elements for Outline can
// be used by other "fat" styles, such as Emboss.


// Creation.

Outline::Outline(PrototypePtr proto, PolyPtr bounds ) : Thick (proto,bounds)
{
}

Outline::Outline(const Style *other ) : Thick(other)
{
}

Outline::~Outline()
{
#ifdef EXPLICIT_DESTRUCTOR
    qDebug() << "deleting outline";
    pts3.clear();
#endif
}

// Style overrides.

void Outline::resetStyleRepresentation()
{
    pts3.erase(pts3.begin(),pts3.end());
}

void Outline::createStyleRepresentation()
{
    if (pts3.size())
    {
        return;
    }

    setupStyleMap();

    for (auto e = getReadOnlyMap()->getEdges()->begin(); e != getReadOnlyMap()->getEdges()->end(); e++)
    {
        EdgePtr edge = *e;
        VertexPtr v1 = edge->getV1();
        VertexPtr v2 = edge->getV2();

        QVector<QPointF> top   = getPoints(edge, v1, v2, width );
        QVector<QPointF> fromp = getPoints(edge, v2, v1, width );

        QPolygonF poly;
        poly <<  top[0] << v2->getPosition() << top[1] << fromp[0] << v1->getPosition() << fromp[1];

        pts3 << poly;
    }
}

void Outline::draw(GeoGraphics *gg)
{
    if (!isVisible())
    {
        return;
    }

    if( pts3.size() != 0)
    {
        gg->pushAndCompose(*getLayerTransform());
        for( int idx = 0; idx < pts3.size(); idx++)
        {
            QPolygonF  poly = pts3[idx];
            gg->setColor(colors.getNextColor().color);
            gg->drawPolygon(poly,true);
            if ( draw_outline )
            {
                gg->setColor(Qt::black);
                gg->drawLine( poly[2], poly[3] );
                gg->drawLine( poly[5], poly[0] );
            }
        }
        gg->pop();
    }
}

////////////////////////////////////////////////////////////////////////////
//
// Helpers.

// Do a mitered join of the two fat lines (a la postscript, for example).
// The join point on the other side of the joint can be computed by
// reflecting the point returned by this function through the joint.

QPointF  Outline::getJoinPoint(QPointF joint, QPointF a, QPointF b, qreal width )
{
    qreal th = Point::sweep(joint, a, b );

    if( qAbs( th - M_PI ) < 1e-7 )
    {
        return QPointF(0,0);
    }


    QPointF d1 = joint - a;
    Point::normalizeD(d1);
    QPointF d2 = joint - b;
    Point::normalizeD(d2);

    qreal l = width / qSin( th );
    qreal isx = joint.x() - (d1.x() + d2.x()) * l;
    qreal isy = joint.y() - (d1.y() + d2.y()) * l;
    return QPointF( isx, isy );

}

// Look at a given edge and construct a plausible set of points
// to draw at the edge's 'to' vertex.  Call this twice to get the
// complete outline of the hexagon to draw for this edge.

QVector<QPointF> Outline::getPoints(EdgePtr edge, VertexPtr from, VertexPtr to, qreal width )
{
    QPointF pfrom = from->getPosition();
    QPointF pto = to->getPosition();

    QPointF dir = pto - pfrom;
    Point::normalizeD(dir);
    QPointF perp = Point::perp(dir);

    int nn = to->numNeighbours();

    QPointF above;
    QPointF below;

    if( nn == 1 )
    {
        below = pto - (perp * width);
        above = pto + (perp * width);
    }
    else if( nn == 2 )
    {
        QVector<EdgePtr> ba = to->getBeforeAndAfter( edge );
        VertexPtr ov = ba[0]->getOther( to->getPosition() );
        QPointF pov = ov->getPosition();

        QPointF jp = getJoinPoint( pto, pfrom, pov, width );

        if (jp == QPointF(0,0))
        {
            below = pto - (perp * width);
            above = pto + (perp * width);
        }
        else
        {
            below = jp;
            above = Point::convexSum(jp, pto, 2.0 );
        }
    }
    else
    {
        QVector<EdgePtr> ba = to->getBeforeAndAfter( edge );
        QPointF before_pt = ba[0]->getOther( to->getPosition() )->getPosition();
        QPointF after_pt  = ba[1]->getOther( to->getPosition() )->getPosition();

        below = getJoinPoint( pto, pfrom, after_pt, width );
        if ( below == QPointF(0,0))
        {
            below = pto - (perp * width);
        }
        above = getJoinPoint( pto, before_pt, pfrom, width );
        if ( above == QPointF(0,0 ))
        {
            above = pto + (perp * width);
        }
    }

    QVector<QPointF> ret;
    ret.push_back(below);
    ret.push_back(above);
    return ret;
}


