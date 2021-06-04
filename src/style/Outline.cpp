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

#include "style/outline.h"
#include "geometry/point.h"
#include "viewers/view.h"
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

Outline::Outline(PrototypePtr proto) : Thick (proto)
{
}

Outline::Outline(StylePtr other) : Thick(other)
{
}

Outline::~Outline()
{
#ifdef EXPLICIT_DESTRUCTOR
    qDebug() << "deleting outline";
    pts4.clear();
#endif
}

// Style overrides.

void Outline::resetStyleRepresentation()
{
    pts4.clear();
    eraseStyleMap();
}

void Outline::createStyleRepresentation()
{
    if (pts4.size())
    {
        return;
    }

    MapPtr map = getMap();

    for (auto edge : map->getEdges())
    {
        VertexPtr v1 = edge->v1;
        VertexPtr v2 = edge->v2;

        BelowAndAbove top   = getPoints(map, edge, v1, v2, width);
        BelowAndAbove fromp = getPoints(map, edge, v2, v1, width);

        BelowAndAboveEdge bae;
        bae.type     = edge->getType();
        bae.v2.below = top.below;
        bae.v2.v     = v2->pt;
        bae.v2.above = top.above;
        bae.v1.below = fromp.below;
        bae.v1.v     = v1->pt;
        bae.v1.above = fromp.above;
        if (bae.type == EDGETYPE_CURVE)
        {
            bae.convex    = edge->isConvex();
            bae.arcCenter = edge->getArcCenter();
        }

        pts4 << bae;
    }
}

void Outline::draw(GeoGraphics *gg)
{
    if (!isVisible())
    {
        return;
    }

    if( pts4.size() != 0)
    {
        for( int idx = 0; idx < pts4.size(); idx++)
        {
            BelowAndAboveEdge bae = pts4[idx];

            if (bae.type == EDGETYPE_LINE)
            {
                QPolygonF  poly = bae.getPoly();
                QColor color = colors.getNextColor().color;
                gg->fillPolygon(poly,color);
                QPen pen(Qt::black);
                if ( draw_outline )
                {
                    gg->drawLine( bae.v2.above, bae.v1.below, pen);
                    gg->drawLine( bae.v1.above, bae.v2.below, pen);
                }
            }
            else if (bae.type == EDGETYPE_CURVE)
            {

                QPointF center = bae.arcCenter;
                bool    convex = bae.convex;
#if 0
                gg->setColor(Qt::red);
                gg->drawLine(bae.v1.below, bae.v2.above);  // inside
                gg->setColor(Qt::blue);
                gg->drawLine(bae.v1.above, bae.v2.below );  // outside
                gg->setColor(Qt::black);
                gg->drawLine(bae.v1.v, bae.v2.v);  // outside
#endif
                QPolygonF  poly = bae.getPoly();
                QColor color = colors.getNextColor().color;
                QPen pen(color);
                //gg->drawPolygon(poly,false);

                if (convex)
                {
                    gg->drawPie(bae.v1.above, bae.v2.below, center, pen, QBrush(color), convex);

                    View * view = View::getInstance();
                    QColor c = view->getBackgroundColor();
                    QBrush br(c);
                    gg->drawPie(bae.v1.below, bae.v2.above, center, pen, br, convex);
                }
                //gg->drawPolygon(poly,false);
#if 1
                if ( draw_outline )
                {
                    //gg->drawLine( bae.v2.above, bae.v1.below );
                    //gg->drawLine( bae.v1.above, bae.v2.below );
                    QPen pen(Qt::black);
                    //gg->setColor(Qt::red);
                    gg->drawChord(bae.v1.below, bae.v2.above, center, pen, QBrush(), convex);  // inside
                    //gg->setColor(Qt::blue);
                    gg->drawChord(bae.v1.above, bae.v2.below, center, pen, QBrush(), convex);  // outside
                }
#endif
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////
//
// Helpers.

// Do a mitered join of the two fat lines (a la postscript, for example).
// The join point on the other side of the joint can be computed by
// reflecting the point returned by this function through the joint.

QPointF  Outline::getJoinPoint(QPointF joint, QPointF a, QPointF b, qreal qwidth )
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

    qreal l   = qwidth / qSin(th);
    qreal isx = joint.x() - (d1.x() + d2.x()) * l;
    qreal isy = joint.y() - (d1.y() + d2.y()) * l;
    return QPointF( isx, isy );

}

// Look at a given edge and construct a plausible set of points
// to draw at the edge's 'to' vertex.  Call this twice to get the
// complete outline of the hexagon to draw for this edge.

BelowAndAbove Outline::getPoints(MapPtr map, EdgePtr edge, VertexPtr from, VertexPtr to, qreal qwidth )
{
    QPointF pfrom = from->pt;
    QPointF pto   = to->pt;

    QPointF dir   = pto - pfrom;
    Point::normalizeD(dir);
    QPointF perp = Point::perp(dir);

    BelowAndAbove ret;

    NeighboursPtr nto = map->getBuiltNeighbours(to);
    int nn = nto->numNeighbours();

    if( nn == 1 )
    {
        ret.below = pto - (perp * qwidth);
        ret.above = pto + (perp * qwidth);
    }
    else if( nn == 2 )
    {
        BeforeAndAfter ba = nto->getBeforeAndAfter(edge);
        QPointF       pov = ba.before->getOtherP(to);
        QPointF        jp = getJoinPoint(pto, pfrom, pov, qwidth);

        if (jp.isNull())
        {
            ret.below = pto - (perp * qwidth);
            ret.above = pto + (perp * qwidth);
        }
        else
        {
            ret.below = jp;
            ret.above = Point::convexSum(jp, pto, 2.0 );
        }
    }
    else
    {
        BeforeAndAfter ba = nto->getBeforeAndAfter(edge);
        QPointF before_pt = ba.before->getOtherP(to);
        QPointF after_pt  = ba.after->getOtherP(to);

        ret.below = getJoinPoint( pto, pfrom, after_pt, qwidth );
        if (ret.below.isNull())
        {
            ret.below = pto - (perp * qwidth);
        }
        ret.above = getJoinPoint( pto, before_pt, pfrom, qwidth );
        if (ret.above.isNull())
        {
            ret.above = pto + (perp * qwidth);
        }
    }

    return ret;
}


QPolygonF BelowAndAboveEdge::getPoly()
{
    QPolygonF p;
    p << v2.below << v2.v << v2.above << v1.below << v1.v << v1.above;
    return p;
}


