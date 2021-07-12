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

#include "tapp/figure_connector.h"
#include "geometry/map.h"
#include "geometry/vertex.h"
#include "geometry/edge.h"
#include "geometry/neighbours.h"
#include "tapp/radial_figure.h"
#include "geometry/intersect.h"
#include "geometry/loose.h"

FigureConnector::FigureConnector(RadialFigure * rp)
{
    this->rp = rp;
}

void FigureConnector::connectFigure(MapPtr unitMap)
{
    int n     = rp->getN();
    qreal don = rp->get_don();

    MapPtr dmap = rp->useDebugMap();
    dmap->wipeout();  // start again

    VertexPtr tip;
    QPointF tip_pos(1.0,0.0);

    // Find the tip, i.e. the vertex at (1,0)
    for (const auto & vert : unitMap->getVertices())
    {
        QPointF pos = vert->pt;
        qDebug() << "test" << pos << tip_pos;
        if (Loose::equalsPt(pos, tip_pos))
        {
            tip = vert;
            break;
        }
    }
    Q_ASSERT(tip != nullptr);
    qDebug() << "tip is: " << tip->pt;

    // Scale the unit
    unitMap->scale(rp->getFigureScale());

    qDebug() << "tip is: " << tip->pt;

    tip_pos = tip->pt;
    dmap->insertDebugMark(tip_pos,"tip");

    // Build the clipping polygon
    QPolygonF border;
    for( int idx = 0; idx < n; ++idx )
    {
        border << rp->getArc(static_cast<qreal>(idx) * don);
    }

    // Locate the other vertex of the segment we're going to extend.
    VertexPtr below_tip;

    QPointF pos = tip->pt;

    NeighboursPtr ntip = unitMap->getBuiltNeighbours(tip);
    std::vector<WeakEdgePtr> * wedges = dynamic_cast<std::vector<WeakEdgePtr>*>(ntip.get());
    for (auto it = wedges->begin(); it != wedges->end(); it++)
    {
        WeakEdgePtr wedge = *it;
        EdgePtr edge = wedge.lock();
        VertexPtr ov = edge->getOtherV(pos);
        if (ov->pt.y() < 0.0)
        {
            below_tip = ov;
            break;
        }
    }

    Q_ASSERT(below_tip != nullptr);

    // Extend and clip.
    QPointF bpos = below_tip->pt;
    dmap->insertDebugMark(bpos,"bpos");

    QPointF tmp  = tip_pos - bpos;
    tmp = Point::normalize(tmp);
    tmp *= 100.0;
    QPointF seg_end = tip_pos + tmp;

    QPointF endpoint = tip_pos;

    for( int idx = 0; idx < n; ++idx )
    {
        QPointF poly_a = border[ idx ];
        QPointF poly_b = border[ (idx + 1) % n ];

        QPointF ep;
        if (Intersect::getIntersection(tip_pos, seg_end, poly_a, poly_b, ep))
        {
            endpoint = ep;
            dmap->insertDebugMark(endpoint,"endpoint");
            break;
        }
    }

    // Now add the extended edge and its mirror image by first
    // intersecting against rotated versions.

    QPointF neg_start = rp->getTransform().map( tip_pos );
    QPointF neg_end   = rp->getTransform().map(QPointF(endpoint.x(), -endpoint.y()));

    VertexPtr last_top    = tip;
    VertexPtr last_bottom = tip;

    for( int idx = 0; idx < (n+1)/2; ++idx )
    {
        QPointF isect;
        if (!Intersect::getIntersection(tip_pos, endpoint, neg_start, neg_end, isect))
        {
            break;
        }

        VertexPtr iv = unitMap->insertVertex( isect);
        EdgePtr ep = unitMap->insertEdge( last_top, iv);
        dmap->insertDebugLine(ep);
        last_top = iv;

        iv = unitMap->insertVertex(QPointF(isect.x(), -isect.y()));
        ep = unitMap->insertEdge( last_bottom, iv);
        dmap->insertDebugLine(ep);
        last_bottom = iv;

        neg_start = rp->getTransform().map( neg_start );
        neg_end   = rp->getTransform().map( neg_end );
    }

    VertexPtr iv = unitMap->insertVertex( endpoint);
    if( iv !=last_top )
    {
        unitMap->insertEdge( last_top, iv);
        iv = unitMap->insertVertex(QPointF(endpoint.x(), -endpoint.y()));
        unitMap->insertEdge( last_bottom, iv);
    }

    // rotate the unit
    unitMap->rotate(rp->getFigureRotate());

    unitMap->buildNeighbours();

    unitMap->verify();
}

qreal FigureConnector::computeScale(MapPtr cunit)
{
    // Find the vertex at (1,0), extend its incoming edges, intersect
    // with rotations of same, and use the location of the intersection
    // to compute a scale factor.

    QPointF tip_pos( 1.0, 0.0 );

    // Find the tip, i.e. the vertex at (1,0)
    for (const auto & vert : cunit->getVertices())
    {
        QPointF pos = vert->pt;
        if (Loose::equalsPt(pos, tip_pos))
        {
            NeighboursPtr n = cunit->getBuiltNeighbours(vert);
            std::vector<WeakEdgePtr> * wedges = dynamic_cast<std::vector<WeakEdgePtr>*>(n.get());
            for (auto it = wedges->begin(); it != wedges->end(); it++)
            {
                WeakEdgePtr wedge = *it;
                EdgePtr edge = wedge.lock();
                VertexPtr ov = edge->getOtherV(pos);
                if( ov->pt.y() < 0.0 )
                {
                    QPointF bpos = ov->pt;
                    QPointF tmp = tip_pos - bpos;
                    tmp = Point::normalize(tmp);
                    tmp *= 100.0;
                    QPointF seg_end = tip_pos + tmp;
                    QPointF neg_seg(seg_end.x(), -seg_end.y());

                    QPointF ra = rp->getTransform().map( tip_pos );
                    QPointF rb = rp->getTransform().map( neg_seg );

                    QPointF isect;
                    if (!Intersect::getIntersection(tip_pos, seg_end, ra, rb, isect))
                    {
                        qDebug() << "computeConnectScale = 1.0";
                        return 1.0;
                    }
                    else
                    {
                        qreal alpha = qCos(M_PI / rp->get_dn()) / Point::mag(isect);
                        qDebug() << "computeConnectScale =" << alpha;
                        return alpha;
                    }
                }
            }
        }
    }

    qDebug() << "computeConnectScale = 1.0";
    return 1.0;
}

void FigureConnector::dumpM(QString s,  QMap<VertexPtr,VertexPtr> & movers)
{
        QMap<VertexPtr, VertexPtr>::const_iterator i = movers.constBegin();
        while (i != movers.constEnd())
        {
          qDebug() << s  << i.key()->pt << ": " << i.value()->pt;
          ++i;
      }
}

// Assume that the result from scaling is a figure with no apex at
// the boundary of the enclosing n-gon, but rather edges that leave for
// different n-gon edges.  Chop the basic unit in half and reassemble
// the bottom half underneath the top half to solve this problem.

void FigureConnector::rotateHalf( MapPtr cunit )
{
    //cunit->verify("rotate half",false);

    QMap<VertexPtr,VertexPtr> movers;

    QTransform Tp = QTransform().rotateRadians(-2.0 * M_PI * rp->get_don());

    for (const auto &vert : cunit->getVertices())
    {
        if( (vert->pt.y() + Loose::TOL) > 0.0 )
        {
            movers.insert( vert, vert );
        }
    }

    QList<VertexPtr> keys = movers.keys();
    for (auto e2 = keys.begin(); e2 != keys.end(); e2++)
    {
        VertexPtr vert = *e2;
        VertexPtr nvert = cunit->insertVertex(Tp.map(vert->pt));
        movers.insert( vert, nvert );   // DAC - this should replace
    }

    QVector<EdgePtr> eadds;

    for (auto edge : cunit->getEdges())
    {
        if (   movers.contains(edge->v1)
            && movers.contains(edge->v2))
        {
            eadds << edge;
        }
    }

    for(auto e4 = eadds.begin(); e4 != eadds.end(); e4++)
    {
        EdgePtr edge = *e4;
        cunit->insertEdge(movers.value(edge->v1), movers.value(edge->v2));
    }

    QMap<VertexPtr, VertexPtr>::const_iterator i = movers.constBegin();
    while (i != movers.constEnd())
    {
        VertexPtr v = i.key();
        if( v->pt.y() > Loose::TOL )
        {
            cunit->removeVertex(v);
        }
        ++i;
    }

    cunit->verify();
}

void FigureConnector::scaleToUnit(MapPtr cunit )
{
    VertexPtr vmax = nullptr;
    qreal xmax = 0.0;

    for (const auto &vert : cunit->getVertices())
    {
        if( vmax == nullptr )
        {
            vmax = vert;
            xmax = vert->pt.x();
        }
        else
        {
            qreal x = vert->pt.x();
            if( x > xmax )
            {
                xmax = x;
                vmax = vert;
            }
        }
    }

    if ( vmax != nullptr)
    {
        qDebug() << "xmax=" << xmax;
        cunit->scale( 1.0 / xmax );
    }
}
