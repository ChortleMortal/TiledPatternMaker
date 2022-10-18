#include <QDebug>
#include <QtMath>

#include "motifs/motif_connector.h"
#include "geometry/map.h"
#include "geometry/vertex.h"
#include "geometry/edge.h"
#include "geometry/neighbours.h"
#include "motifs/radial_motif.h"
#include "geometry/intersect.h"
#include "geometry/loose.h"

MotifConnector::MotifConnector()
{
}

void MotifConnector::connectMotif(RadialMotif * fig)
{
    Q_ASSERT(fig);

    int n     = fig->getN();
    qreal don = fig->get_don();

    DebugMapPtr dbgmap = fig->getDebugMap();
    if (dbgmap)
        dbgmap->wipeout();  // start again

    VertexPtr tip;
    QPointF tip_pos(1.0,0.0);

    // Find the tip, i.e. the vertex at (1,0)
    auto map = fig->getUnitMap();
    for (const auto & vert : qAsConst(map->getVertices()))
    {
        QPointF pos = vert->pt;
        qDebug() << "test" << pos << tip_pos;
        if (Loose::equalsPt(pos, tip_pos))
        {
            tip = vert;
            break;
        }
    }
    Q_ASSERT(tip);
    qDebug() << "tip is: " << tip->pt;

    // Scale the unit
    map->scale(fig->getMotifScale());

    qDebug() << "tip is: " << tip->pt;

    tip_pos = tip->pt;
    if (dbgmap)
        dbgmap->insertDebugMark(tip_pos,"tip");

    // Build the clipping polygon
    QPolygonF border;
    for( int idx = 0; idx < n; ++idx )
    {
        border << fig->getArc(static_cast<qreal>(idx) * don);
    }

    // Locate the other vertex of the segment we're going to extend.
    VertexPtr below_tip;

    QPointF pos = tip->pt;

    NeighboursPtr ntip = map->getNeighbours(tip);
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

    Q_ASSERT(below_tip);

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

    QPointF neg_start = fig->getTransform().map( tip_pos );
    QPointF neg_end   = fig->getTransform().map(QPointF(endpoint.x(), -endpoint.y()));

    VertexPtr last_top    = tip;
    VertexPtr last_bottom = tip;

    for( int idx = 0; idx < (n+1)/2; ++idx )
    {
        QPointF isect;
        if (!Intersect::getIntersection(tip_pos, endpoint, neg_start, neg_end, isect))
        {
            break;
        }

        VertexPtr iv = map->insertVertex( isect);
        EdgePtr ep   = map->insertEdge( last_top, iv);
        if (dbgmap)
            dbgmap->insertDebugLine(ep);
        last_top = iv;

        iv = map->insertVertex(QPointF(isect.x(), -isect.y()));
        ep = map->insertEdge( last_bottom, iv);
        if (dbgmap)
            dbgmap->insertDebugLine(ep);
        last_bottom = iv;

        neg_start = fig->getTransform().map( neg_start );
        neg_end   = fig->getTransform().map( neg_end );
    }

    VertexPtr iv = map->insertVertex( endpoint);
    if( iv !=last_top )
    {
        map->insertEdge( last_top, iv);
        iv = map->insertVertex(QPointF(endpoint.x(), -endpoint.y()));
        map->insertEdge( last_bottom, iv);
    }

    // rotate the unit
    map->rotate(fig->getMotifRotate());
    map->resetNeighbourMap();
    map->getNeighbourMap(); // rebuilds
    map->verify();
}

qreal MotifConnector::computeScale(RadialMotif * fig)
{
    Q_ASSERT(fig);

    // Find the vertex at (1,0), extend its incoming edges, intersect
    // with rotations of same, and use the location of the intersection
    // to compute a scale factor.

    QPointF tip_pos( 1.0, 0.0 );

    // Find the tip, i.e. the vertex at (1,0)
    auto map = fig->getUnitMap();
    for (const auto & vert : qAsConst(map->getVertices()))
    {
        QPointF pos = vert->pt;
        if (Loose::equalsPt(pos, tip_pos))
        {
            NeighboursPtr n = map->getNeighbours(vert);
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

                    QPointF ra = fig->getTransform().map( tip_pos );
                    QPointF rb = fig->getTransform().map( neg_seg );

                    QPointF isect;
                    if (!Intersect::getIntersection(tip_pos, seg_end, ra, rb, isect))
                    {
                        qDebug() << "computeConnectScale = 1.0";
                        return 1.0;
                    }
                    else
                    {
                        qreal alpha = qCos(M_PI / fig->get_dn()) / Point::mag(isect);
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

void MotifConnector::dumpM(QString s,  QMap<VertexPtr,VertexPtr> & movers)
{
    QMap<VertexPtr, VertexPtr>::const_iterator i = movers.constBegin();
    while (i != movers.constEnd())
    {
        qDebug() << s  << i.key()->pt << ": " << i.value()->pt;
        ++i;
    }
}

#if 0
// Assume that the result from scaling is a figure with no apex at
// the boundary of the enclosing n-gon, but rather edges that leave for
// different n-gon edges.  Chop the basic unit in half and reassemble
// the bottom half underneath the top half to solve this problem.

void MotifConnector::rotateHalf(RadialFigure * fig)
{
    Q_ASSERT(fig);

    auto map = fig->getUnitMap();
    //map->verify("rotate half",false);

    QMap<VertexPtr,VertexPtr> movers;

    QTransform Tp = QTransform().rotateRadians(-2.0 * M_PI * fig->get_don());

    for (const auto &vert : qAsConst(map->getVertices()))
    {
        if( (vert->pt.y() + Loose::TOL) > 0.0 )
        {
            movers.insert(vert,vert);
        }
    }

    QList<VertexPtr> keys = movers.keys();
    for (auto & e2 : keys)
    {
        VertexPtr vert = e2;
        VertexPtr nvert = map->insertVertex(Tp.map(vert->pt));
        movers.insert(vert,nvert);   // DAC - this should replace
    }

    QVector<EdgePtr> eadds;

    for (auto & edge : qAsConst(map->getEdges()))
    {
        if (   movers.contains(edge->v1)
            && movers.contains(edge->v2))
        {
            eadds << edge;
        }
    }

    for(auto & e4 : qAsConst(eadds))
    {
        EdgePtr edge = e4;
        map->insertEdge(movers.value(edge->v1), movers.value(edge->v2));
    }

    QMap<VertexPtr, VertexPtr>::const_iterator i = movers.constBegin();
    while (i != movers.constEnd())
    {
        VertexPtr v = i.key();
        if( v->pt.y() > Loose::TOL )
        {
            map->removeVertex(v);
        }
        ++i;
    }

    map->verify();
}
#endif

void MotifConnector::scaleToUnit(RadialMotif * fig)
{
    Q_ASSERT(fig);

    VertexPtr vmax;
    qreal xmax = 0.0;

    auto map = fig->getUnitMap();
    for (const auto &vert : qAsConst(map->getVertices()))
    {
        if(!vmax)
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

    if (vmax)
    {
        qDebug() << "xmax=" << xmax;
        map->scale( 1.0 / xmax );
    }
}
