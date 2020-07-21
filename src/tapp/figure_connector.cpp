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

#include "figure_connector.h"
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
    for (auto vert : unitMap->getVertices())
    {
        QPointF pos = vert->getPosition();
        qDebug() << "test" << pos << tip_pos;
        if( Loose::equals( pos, tip_pos ) )
        {
            tip = vert;
            break;
        }
    }
    Q_ASSERT(tip != nullptr);
    qDebug() << "tip is: " << tip->getPosition();

    // Scale the unit
    unitMap->scale(rp->getFigureScale());

    qDebug() << "tip is: " << tip->getPosition();

    tip_pos = tip->getPosition();
    dmap->insertDebugMark(tip_pos,"tip");

    // Build the clipping polygon
    QPolygonF border;
    for( int idx = 0; idx < n; ++idx )
    {
        border << rp->getArc(static_cast<qreal>(idx) * don);
    }

    // Locate the other vertex of the segment we're going to extend.
    VertexPtr below_tip;

    QPointF pos = tip->getPosition();

    NeighbourMap & nmap     = unitMap->getNeighbourMap();
    NeighboursPtr np        = nmap.getNeighbours(tip);
    QVector<EdgePtr> & qvep = np->getNeighbours();

    for (auto edge : qvep)
    {
        VertexPtr ov = edge->getOtherV(pos);
        if (ov->getPosition().y() < 0.0)
        {
            below_tip = ov;
            break;
        }
    }

    Q_ASSERT(below_tip != nullptr);

    // Extend and clip.
    QPointF bpos = below_tip->getPosition();
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

        QPointF ep = Intersect::getIntersection(tip_pos, seg_end, poly_a, poly_b );
        if( !ep.isNull() )
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
        QPointF isect = Intersect::getIntersection(tip_pos, endpoint, neg_start, neg_end);
        if( isect.isNull() )
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

    unitMap->verifyMap("RosetteConnectFigure");
}

qreal FigureConnector::computeScale(MapPtr cunit)
{
    // Find the vertex at (1,0), extend its incoming edges, intersect
    // with rotations of same, and use the location of the intersection
    // to compute a scale factor.

    QPointF tip_pos( 1.0, 0.0 );

    // Find the tip, i.e. the vertex at (1,0)
    NeighbourMap & nmap     = cunit->getNeighbourMap();
    for (auto vert : cunit->getVertices())
    {
        QPointF pos = vert->getPosition();
        if( Loose::equals( pos, tip_pos ))
        {
            NeighboursPtr np        = nmap.getNeighbours(vert);
            QVector<EdgePtr> & qvep = np->getNeighbours();
            for (auto edge : qvep)
            {
                VertexPtr ov = edge->getOtherV(pos);
                if( ov->getPosition().y() < 0.0 )
                {
                    QPointF bpos = ov->getPosition();
                    QPointF tmp = tip_pos - bpos;
                    tmp = Point::normalize(tmp);
                    tmp *= 100.0;
                    QPointF seg_end = tip_pos + tmp;
                    QPointF neg_seg(seg_end.x(), -seg_end.y());

                    QPointF ra = rp->getTransform().map( tip_pos );
                    QPointF rb = rp->getTransform().map( neg_seg );

                    QPointF isect = Intersect::getIntersection( tip_pos, seg_end, ra, rb );
                    if( isect.isNull() )
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
          qDebug() << s  << i.key()->getPosition() << ": " << i.value()->getPosition();
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

    for (auto vert : cunit->getVertices())
    {
        if( (vert->getPosition().y() + Loose::TOL) > 0.0 )
        {
            movers.insert( vert, vert );
        }
    }

    //dumpM("one",movers);

    QList<VertexPtr> keys = movers.keys();
    for (auto e2 = keys.begin(); e2 != keys.end(); e2++)
    {
        VertexPtr vert = *e2;
        VertexPtr nvert = cunit->insertVertex(Tp.map(vert->getPosition()));
        movers.insert( vert, nvert );   // DAC - this should replace
    }

    //dumpM("two",movers);

    QVector<EdgePtr> eadds;

    for (auto edge : cunit->getEdges())
    {
        if (   movers.contains(edge->getV1())
            && movers.contains(edge->getV2()))
        {
            eadds << edge;
        }
    }

    for(auto e4 = eadds.begin(); e4 != eadds.end(); e4++)
    {
        EdgePtr edge = *e4;
        cunit->insertEdge(movers.value(edge->getV1()), movers.value(edge->getV2()));
    }

    //cunit->verify("rotateHalf end penultimate",false);

    QMap<VertexPtr, VertexPtr>::const_iterator i = movers.constBegin();
    while (i != movers.constEnd())
    {
        VertexPtr v = i.key();
        if( v->getPosition().y() > Loose::TOL )
        {
            cunit->removeVertex(v);
        }
        ++i;
    }

    //dumpM("three",movers);

    cunit->verifyMap("rotateHalf end");
}

void FigureConnector::scaleToUnit(MapPtr cunit )
{
    VertexPtr vmax = nullptr;
    qreal xmax = 0.0;

    for (auto vert : cunit->getVertices())
    {
        if( vmax == nullptr )
        {
            vmax = vert;
            xmax = vert->getPosition().x();
        }
        else
        {
            qreal x = vert->getPosition().x();
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
