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

#include "tapp/ExtendedStar.h"
#include "geometry/Loose.h"
#include "geometry/Intersect.h"
#include "tile/Feature.h"
#include "base/utilities.h"

ExtendedStar::ExtendedStar(const Figure & fig,
                           int n, qreal d, int s, qreal r,
                           bool extendPeripherals, bool extendFreeVertices)
    : Star(fig, n, d, s, r)
{
    setFigType(FIG_TYPE_EXTENDED_STAR);
    this->extendPeripheralVertices  = extendPeripherals;
    this->extendFreeVertices        = extendFreeVertices;
}

ExtendedStar::ExtendedStar(int n, qreal d, int s, qreal r,
                           bool extendPeripherals, bool extendFreeVertices)
    : Star(n,d,s,r)
{
    setFigType(FIG_TYPE_EXTENDED_STAR);
    this->extendPeripheralVertices  = extendPeripherals;
    this->extendFreeVertices        = extendFreeVertices;
}

void ExtendedStar::buildMaps()
{
    Star::buildMaps();
    if (extendFreeVertices || extendPeripheralVertices)
    {
        extendMap();
    }
}

void ExtendedStar::extendMap()
{
    qDebug() << "ExtendedStar::extendMap";

    qreal radius = extBoundaryScale;
    QGraphicsEllipseItem circle(-radius,-radius,radius * 2.0, radius * 2.0);

    // extend the lines to exteneded boundary and clip
    // extendLine and clipLine both assume that p1 is closest to the center
    // and p0 is closest to the edge

    QVector<EdgePtr> ledges = *figureMap->getEdges();    // local copy of vector
    for(auto e = ledges.begin(); e != ledges.end(); e++)
    {
        EdgePtr edge = *e;
        VertexPtr v1 = edge->getV1();   // outer
        VertexPtr v2 = edge->getV2();   // inner
        QLineF l1 = QLineF(v2->getPosition(),v1->getPosition());

        bool extend = false;
        if (!Point::intersectPoly(l1,radialFigBoundary))
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
            if (!hasCircleBoundary)
            {
                // dont extend lines which already touch boundary
                if (!Point::intersectPoly(l1,extBoundary))
                {
                    // extend lines
                    QLineF l2 = Point::extendLine(l1,10.0);      // extends
                    l2        = Point::clipLine(l2,extBoundary); // clips
                    // insert Vertex and Insert edge
                    VertexPtr newv = figureMap->insertVertex(l2.p2()); // outer
                    figureMap->insertEdge(newv,v1);
                    //qDebug() << "extension" << newv->getPosition() << v1->getPosition();
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
    figureMap->verify("Extended figure - after",false,true,false);
}
