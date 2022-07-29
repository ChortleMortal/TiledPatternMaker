#include <QDebug>
#include <QtMath>

#include "figures/figure_connector.h"
#include "geometry/map.h"
#include "geometry/vertex.h"
#include "geometry/edge.h"
#include "geometry/neighbours.h"
#include "figures/radial_figure.h"
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

    MapPtr dbgmap = rp->useDebugMap();
    if (dbgmap)
        dbgmap->wipeout();  // start again

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
    if (dbgmap)
        dbgmap->insertDebugMark(tip_pos,"tip");

    // Build the clipping polygon
    QPolygonF border;
    for( int idx = 0; idx < n; ++idx )
    {
        border << rp->getArc(static_cast<qreal>(idx) * don);
    }

    // Locate the other vertex of the segment we're going to extend.
    VertexPtr below_tip;

    QPointF pos = tip->pt;

    NeighboursPtr ntip = unitMap->getNeighbours(tip);
    for (auto & wedge : *ntip)
    {
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
    if (dbgmap)
        dbgmap->insertDebugMark(bpos,"bpos");

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
            if (dbgmap)
                dbgmap->insertDebugMark(endpoint,"endpoint");
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
        if (dbgmap)
            dbgmap->insertDebugLine(ep);
        last_top = iv;

        iv = unitMap->insertVertex(QPointF(isect.x(), -isect.y()));
        ep = unitMap->insertEdge( last_bottom, iv);
        if (dbgmap)
            dbgmap->insertDebugLine(ep);
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
    unitMap->resetNeighbourMap();
    unitMap->getNeighbourMap(); // rebuilds
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
            NeighboursPtr n = cunit->getNeighbours(vert);
            for (auto & wedge : *n)
            {
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

    for (auto & edge : cunit->getEdges())
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
