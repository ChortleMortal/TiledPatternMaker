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

#include "tapp/extended_star.h"
#include "geometry/loose.h"
#include "geometry/intersect.h"
#include "geometry/map_cleanser.h"
#include "tile/feature.h"
#include "base/utilities.h"

ExtendedStar::ExtendedStar(const Figure & fig,
                           int nn, qreal d, int s, qreal r,
                           bool extendPeripherals, bool extendFreeVertices)
    : Star(fig, nn, d, s, r)
{
    setFigType(FIG_TYPE_EXTENDED_STAR);
    this->extendPeripheralVertices  = extendPeripherals;
    this->extendFreeVertices        = extendFreeVertices;
}

ExtendedStar::ExtendedStar(int nn, qreal d, int s, qreal r,
                           bool extendPeripherals, bool extendFreeVertices)
    : Star(nn,d,s,r)
{
    setFigType(FIG_TYPE_EXTENDED_STAR);
    this->extendPeripheralVertices  = extendPeripherals;
    this->extendFreeVertices        = extendFreeVertices;
}

void ExtendedStar::buildMaps()
{
    Star::buildMaps();

    if (extendPeripheralVertices)
    {
        extendPeripheralMap();
    }

    if (extendFreeVertices)
    {
        extendFreeMap();
    }
}

void ExtendedStar::extendFreeMap()
{
    if (n != getExtBoundarySides())
    {
        qWarning("Cannot extend - no matching boundary");
        return;
    }

    QPointF tip(1,0);
    QTransform t = QTransform::fromScale(getFigureScale(),getFigureScale());
    tip = t.map(tip);
    VertexPtr v1 = findVertex(tip);

    QPointF e_tip(1,0);
    QTransform t2 = QTransform::fromScale(getExtBoundaryScale(),getExtBoundaryScale());
    e_tip = t2.map(e_tip);
    VertexPtr v2 = figureMap->insertVertex(e_tip);

    figureMap->insertEdge(v1,v2);

    for (int idx = 1; idx < n; idx++)
    {
        tip   = Tr.map(tip);
        v1    = findVertex(tip);

        e_tip = Tr.map(e_tip);
        v2 = figureMap->insertVertex(e_tip);

        figureMap->insertEdge(v1,v2);
    }
}


void ExtendedStar::extendPeripheralMap()
{
    qDebug() << "ExtendedStar::extendFreeMap";

    qreal radius = getExtBoundaryScale();
    QGraphicsEllipseItem circle(-radius,-radius,radius * 2.0, radius * 2.0);

    // extend the lines to exteneded boundary and clip
    // extendLine and clipLine both assume that p1 is closest to the center
    // and p0 is closest to the edge

    QVector<EdgePtr> ledges = figureMap->edges;    // local copy of vector
    for (auto edge : ledges)
    {
        VertexPtr v1 = edge->v1;   // outer
        VertexPtr v2 = edge->v2;   // inner
        QLineF l1 = QLineF(v2->pt,v1->pt);

        bool extend = false;
        QPointF intersect;
        if (!Point::intersectPoly(l1,getRadialFigBoundary(),intersect))
        {
            // point doesn't touch the figure boundary
            if (extendFreeVertices)
            {
                extend = true;
            }
        }
        else
        {
            // point does touch the figure boundary
            if (extendPeripheralVertices)
            {
                extend = true;
            }
        }

        if (extend)
        {
            if (!hasExtCircleBoundary())
            {
                // dont extend lines which already touch boundary
                if (!Point::intersectPoly(l1,getExtBoundary(),intersect))
                {
                    // extend lines
                    QLineF l2 = Point::extendLine(l1,10.0);      // extends
                    l2        = Point::clipLine(l2,getExtBoundary()); // clips
                    // insert Vertex and Insert edge
                    VertexPtr newv = figureMap->insertVertex(l2.p2()); // outer
                    figureMap->insertEdge(newv,v1);
                    //qDebug() << "extension" << newv->pt << v1->pt;
                    //figureMap->verify("Extended figure - mid",true,true,false);
                }
            }
            else
            {
                QPointF a;
                QPointF b;
                int points = Utils::circleLineIntersectionPoints(circle,radius,l1,a,b);
                if (points)
                {
                    VertexPtr newv;
                    if (!a.isNull())
                    {
                        newv = figureMap->insertVertex(a);
                        figureMap->insertEdge(newv,v1);
                    }
                    if (!b.isNull())
                    {
                        newv = figureMap->insertVertex(b);
                        figureMap->insertEdge(newv,v1);
                    }
                }
            }
        }
    }

    figureMap->verifyMap("Extended figure - after");
}

VertexPtr ExtendedStar::findVertex(QPointF pt)
{
    const QVector<VertexPtr> & vertices = figureMap->vertices;
    for (auto v : vertices)
    {
        if (v->pt == pt)
        {
            return v;
        }
    }
    VertexPtr vp;
    return vp;
}
