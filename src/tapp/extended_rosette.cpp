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

#include "tapp/extended_rosette.h"
#include "tapp/radial_figure.h"
#include "tile/feature.h"
#include "base/utilities.h"

ExtendedRosette::ExtendedRosette(const Figure & fig,
                int nsides, qreal q, int s, qreal k,
                qreal r,
                bool  extendPeripheralVertices,
                bool  extendFreeVertices,
                bool  connectBoundaryVertices)
    : Rosette(fig, nsides,q,s,k,r)
{
    setFigType(FIG_TYPE_EXTENDED_ROSETTE);
    this->extendPeripheralVertices  = extendPeripheralVertices;
    this->extendFreeVertices        = extendFreeVertices;
    this->connectBoundaryVertices   = connectBoundaryVertices;
}

ExtendedRosette::ExtendedRosette(int nsides, qreal q, int s, qreal k, qreal r,
                                 bool extendPeripheralVertices, bool extendFreeVertices, bool connectBoundaryVertices)
    : Rosette(nsides,q,s,k,r)
{
    setFigType(FIG_TYPE_EXTENDED_ROSETTE);
    this->extendPeripheralVertices  = extendPeripheralVertices;
    this->extendFreeVertices        = extendFreeVertices;
    this->connectBoundaryVertices   = connectBoundaryVertices;
}

void ExtendedRosette::buildMaps()
{
    Rosette::buildMaps();
    if (extendFreeVertices || extendPeripheralVertices)
    {
        extendMap();
    }
}

void ExtendedRosette::extendMap()
{
    qDebug() << "ExtendedRosette::extendMap";

    figureMap->verify();

    qreal radius = getExtBoundaryScale();
    QGraphicsEllipseItem circle(-radius,-radius,radius * 2.0, radius * 2.0);

    // extend the lines to exteneded boundary and clip
    // extendLine and clipLine both assume that p1 is closest to the center
    // and p0 is closest to the edge

    // the shortest line is the one to insert
    QMap<VertexPtr,VertexPtr> new_edges;    // key is new vertex

    QVector<EdgePtr> local_edges = figureMap->getEdges();    // local copy of vector
    for (auto & edge  : local_edges)
    {
        VertexPtr v1 = edge->v1;   // outer
        VertexPtr v2 = edge->v2;   // inner
        QLineF l1 = QLineF(v2->pt,v1->pt);

        bool extend = false;
        QPointF intersect;
        bool touches = Point::intersectPoly(l1,getRadialFigBoundary(),intersect);

        if (!touches && extendFreeVertices)
            extend = true; // point doesn't touch the figure boundary
        else if (touches && extendPeripheralVertices)
            extend = true; // point does touch the figure boundary

        if (!extend)
            continue;

        if (!hasExtCircleBoundary())
        {
            // dont extend lines which already touch boundary
            QPointF intersect2;
            touches = Point::intersectPoly(l1,getExtBoundary(),intersect2);
            if (touches)
                continue;

            // extend lines
            QLineF l2  = Point::extendLine(l1,10.0);      // extends
            l2         = Point::clipLine(l2,getExtBoundary()); // clips
            QPointF pt = l2.p2();                         // outer
            // test if this new point is already a vertex
            VertexPtr newv = figureMap->insertVertex(pt);
            if (new_edges.contains(newv))
            {
                VertexPtr old1 = new_edges[newv];
                if (len(v1,newv) < len(old1,newv))
                {
                    new_edges[newv] = v1;
                }
            }
            else
            {
                new_edges[newv] = v1;
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

    for (QMap<VertexPtr,VertexPtr>::const_iterator it = new_edges.cbegin(), end = new_edges.cend(); it != end; ++it)
    {
        figureMap->insertEdge(it.value(),it.key());
    }

    if (connectBoundaryVertices)
    {
        connectOuterVertices(figureMap);
    }

    figureMap->verify();
}

void ExtendedRosette::connectOuterVertices(MapPtr map)
{
    // conenct verices on boundary
    //qDebug() << "Boundary=" << extBoundary;
    QVector<QLineF> blines = Utils::polyToLines(getExtBoundary());

    QVector<VertexPtr> edgeVerts;

    for (const auto & v : map->getVertices())
    {
        //qDebug() << "num neigbours=" << v->numNeighbours();
        NeighboursPtr n = map->getBuiltNeighbours(v);
        if (n->numNeighbours() < 2)
        {
            bool doInsert = false;
            for (int i=0; i < blines.size(); i++)
            {
                QLineF line = blines[i];
                if (!Utils::pointAtEndOfLine(line,v->pt))
                {
                    doInsert = true;
                }
            }
            if (doInsert)
            {
                //qDebug() << "insert";
                //debugMap->insertDebugMark(v->pt,"v0");
                edgeVerts.push_back(v);
            }
            else
            {
                //qDebug() << "skip";
            }
        }
    }

    for (int i=0; i < blines.size(); i++)
    {
        QLineF line = blines[i];
        qDebug() << "Boundary Line"  << line;
        VertexPtr v1;
        VertexPtr v2;

        auto it = edgeVerts.begin();
        while (it != edgeVerts.end())
        {
            if (!v1)
            {
                if (it == edgeVerts.end())
                    continue;
                v1  = *it++;
                if (!Utils::pointOnLine(line,v1->pt))
                {
                    v1.reset();
                    continue;
                }
            }
            // we have v1
            if (!v2)
            {
                if (it == edgeVerts.end())
                    continue;
                v2  = *it++;
                if (!Utils::pointOnLine(line,v2->pt))
                {
                    v2.reset();
                    continue;
                }
            }
            // we have v1 and v2

            map->insertEdge(v1,v2);
            qDebug() << "newLine=" << v1->pt << v2->pt;
            debugMap->insertDebugMark(v1->pt,"v1");
            debugMap->insertDebugMark(v2->pt,"v2");
            v1.reset();
            v2.reset();
        }
    }

    map->verify();
}
