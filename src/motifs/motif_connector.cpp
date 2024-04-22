#include <QDebug>
#include <QtMath>

#include "motifs/motif_connector.h"
#include "geometry/map.h"
#include "geometry/debug_map.h"
#include "geometry/vertex.h"
#include "geometry/edge.h"
#include "geometry/neighbours.h"
#include "motifs/radial_motif.h"
#include "geometry/intersect.h"
#include "geometry/loose.h"
#include "geometry/geo.h"

MotifConnector::MotifConnector()
{
}

void MotifConnector::connectMotif(RadialMotif * motif, qreal scale)
{
    Q_ASSERT(motif);

    int n     = motif->getN();
    qreal don = motif->get_don();

    DebugMapPtr dbgmap = motif->getDebugMap();
    if (dbgmap) dbgmap->wipeout();  // start again

    QPointF tip_pos(1.0,0.0);
    //QTransform t = motif->getDELTransform();
    //tip_pos = t.map(tip_pos);

    // Find the tip, i.e. the vertex at (1,0)
    auto map = motif->getUnitMap();
    VertexPtr tip = map->getVertex(tip_pos);
    Q_ASSERT(tip);
    qDebug() << "tip is: " << tip->pt;

    // Scale the unit
    //qreal scale = motif->getMotifScale();
    map->transform(QTransform().scale(scale,scale));

    qDebug() << "tip is: " << tip->pt;

    tip_pos = tip->pt;
    if (dbgmap) dbgmap->insertDebugMark(tip_pos,"tip");

    // Build the clipping polygon
    QPolygonF border;
    for( int idx = 0; idx < n; ++idx )
    {
        border << motif->getArc(static_cast<qreal>(idx) * don);
    }

    // Locate the other vertex of the segment we're going to extend.
    VertexPtr below_tip;

    QPointF pos = tip->pt;

    NeighboursPtr ntip = map->getNeighbours(tip);
    for (auto & wedge : std::as_const(*ntip))
    {
        EdgePtr edge = wedge.lock();
        VertexPtr ov = edge->getOtherV(pos);
        if (ov->pt.y() < 0.0)
        {
            below_tip = ov;
            break;
        }
    }

    if (!below_tip)
    {
        qWarning() << "MotifConnector::connectMotif - cannot make a connection";
        return;
    }

    Q_ASSERT(below_tip);

    // Extend and clip.
    QPointF bpos = below_tip->pt;
    if (dbgmap) dbgmap->insertDebugMark(bpos,"bpos");

    QPointF tmp  = tip_pos - bpos;
    tmp = Geo::normalize(tmp);
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
            if (dbgmap) dbgmap->insertDebugMark(endpoint,"endpoint");
            break;
        }
    }

    // Now add the extended edge and its mirror image by first
    // intersecting against rotated versions.
    
    QPointF neg_start = motif->getUnitRotationTransform().map( tip_pos );
    QPointF neg_end   = motif->getUnitRotationTransform().map(QPointF(endpoint.x(), -endpoint.y()));

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
        if (dbgmap) dbgmap->insertDebugEdge(ep);
        last_top = iv;

        iv = map->insertVertex(QPointF(isect.x(), -isect.y()));
        ep = map->insertEdge( last_bottom, iv);
        if (dbgmap) dbgmap->insertDebugEdge(ep);
        last_bottom = iv;
        
        neg_start = motif->getUnitRotationTransform().map( neg_start );
        neg_end   = motif->getUnitRotationTransform().map( neg_end );
    }

    VertexPtr iv = map->insertVertex( endpoint);
    if( iv !=last_top )
    {
        map->insertEdge( last_top, iv);
        iv = map->insertVertex(QPointF(endpoint.x(), -endpoint.y()));
        map->insertEdge( last_bottom, iv);
    }

    // rotate the unit
    qreal rotate = motif->getMotifRotate();
    map->transform(QTransform().rotate(rotate));
    map->resetNeighbourMap();
    map->verify();
}

qreal MotifConnector::computeScale(RadialMotif * motif)
{
    Q_ASSERT(motif);

    // Find the vertex at (1,0), extend its incoming edges, intersect
    // with rotations of same, and use the location of the intersection
    // to compute a scale factor.

    QPointF tip_pos( 1.0, 0.0 );

    // Find the tip, i.e. the vertex at (1,0)
    auto map = motif->getUnitMap();
    for (const auto & vert : std::as_const(map->getVertices()))
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
                    tmp = Geo::normalize(tmp);
                    tmp *= 100.0;
                    QPointF seg_end = tip_pos + tmp;
                    QPointF neg_seg(seg_end.x(), -seg_end.y());
                    
                    QPointF ra = motif->getUnitRotationTransform().map( tip_pos );
                    QPointF rb = motif->getUnitRotationTransform().map( neg_seg );

                    QPointF isect;
                    if (!Intersect::getIntersection(tip_pos, seg_end, ra, rb, isect))
                    {
                        qDebug() << "computeConnectScale = 1.0";
                        return 1.0;
                    }
                    else
                    {
                        qreal alpha = qCos(M_PI / qreal(motif->getN())) / Geo::mag(isect);
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

void MotifConnector::rotateHalf(RadialMotif * motif)
{
    Q_ASSERT(motif);

    auto map = motif->getUnitMap();
    //map->verify("rotate half",false);

    QMap<VertexPtr,VertexPtr> movers;

    QTransform Tp = QTransform().rotateRadians(-2.0 * M_PI * motif->get_don());

    for (const auto &vert : std::as_const(map->getVertices()))
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

    for (auto & edge : std::as_const(map->getEdges()))
    {
        if (   movers.contains(edge->v1)
            && movers.contains(edge->v2))
        {
            eadds << edge;
        }
    }

    for(auto & e4 : std::as_const(eadds))
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

void MotifConnector::scaleToUnit(RadialMotif * motif)
{
    Q_ASSERT(motif);

    VertexPtr vmax;
    qreal xmax = 0.0;

    auto map = motif->getUnitMap();
    for (const auto &vert : std::as_const(map->getVertices()))
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
        qreal scale = 1.0 / xmax;
        map->transform(QTransform().scale(scale,scale));
    }
}
