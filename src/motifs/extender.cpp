#include <QDebug>
#include "motifs/extender.h"
#include "motifs/motif.h"
#include "geometry/map.h"
#include "geometry/edge.h"
#include "geometry/vertex.h"
#include "geometry/neighbours.h"
#include "geometry/geo.h"
#include "geometry/transform.h"
#include "tile/tile.h"

Extender::Extender()
{
    extendPeripheralVertices  = false;
    extendFreeVertices        = false;
    connectBoundaryVertices   = false;
}

Extender::Extender(const Extender & other)
{
    extendPeripheralVertices  = other.extendPeripheralVertices;
    extendFreeVertices        = other.extendFreeVertices;
    connectBoundaryVertices   = other.connectBoundaryVertices;
}

void Extender::extend(Motif * motif, QTransform Tr)
{
    this->motif = motif;
    newVertices.clear();

    auto map = motif->getMotifMap();

    map->verify();

    if (extendPeripheralVertices)
    {
        extendPeripheralMap(map);
    }

    if (extendFreeVertices)
    {
        extendFreeMap(map,Tr);
    }

    if (connectBoundaryVertices)
    {
        connectOuterVertices(map);
    }

    map->verify();
}

void Extender::extendPeripheralMap(MapPtr motifMap)  // is same as extend Peripheral Map - used by rosette
{
    qDebug().noquote() << "Extender::extendPeripheralMap" << motifMap->summary();

    const ExtendedBoundary & eb = motif->getExtendedBoundary();

    qreal radius = eb.getScale();
    QGraphicsEllipseItem circle(-radius,-radius,radius * 2.0, radius * 2.0);

    const QPolygonF & eboundary = eb.getPoly();
    const QPolygonF & fboundary = motif->getMotifBoundary();

    // extend the lines to extended boundary and clip
    // extendLine and clipLine both assume that p1 is closest to the center
    // and p0 is closest to the edge

    // the shortest line is the one to insert
    QVector<EdgePtr> edges = motifMap->getEdges();
    for (auto & edge  : std::as_const(edges))
    {
        VertexPtr v1 = edge->v1;   // outer
        VertexPtr v2 = edge->v2;   // inner
        QLineF l1(v2->pt,v1->pt);

        QPointF intersect;
        if (extendPeripheralVertices && Geo::intersectPoly(l1,fboundary,intersect))
        {
            // l1 intersects boundary
            if (!eb.isCircle())
            {
                // polygonal boundary
                // dont extend lines which already touch boundary
                QPointF intersect2;
                if (Geo::intersectPoly(l1,eboundary,intersect2))
                {
                    continue;
                }
                // extend lines
                QLineF l2  = Geo::extendLine(l1,10.0);      // extends
                l2         = Geo::clipLine(l2,eboundary);   // clips
                QPointF pt = l2.p2();                         // outer
                // test if this new point is already a vertex
                VertexPtr newv = motifMap->insertVertex(pt);
                newVertices.push_back(newv);
                motifMap->insertEdge(v1,newv);
            }
            else
            {
                // circular boundary
                QPointF a;
                QPointF b;
                int points = Geo::circleLineIntersectionPoints(circle,radius,l1,a,b);
                if (points)
                {
                    VertexPtr newv;
                    if (!a.isNull())
                    {
                        newv = motifMap->insertVertex(a);
                        newVertices.push_back(newv);
                        motifMap->insertEdge(v1,newv);
                    }
                    if (!b.isNull())
                    {
                        newv = motifMap->insertVertex(b);
                        newVertices.push_back(newv);
                        motifMap->insertEdge(v1,newv);
                    }
                }
            }
        }
    }
}

void Extender::extendFreeMap(MapPtr motifMap, QTransform unitRotationTr)
{
    qDebug() << "Tile transform" << Transform::toInfoString(unitRotationTr);
    const ExtendedBoundary & eb = motif->getExtendedBoundary();
    int n = motif->getN();
    if (n != eb.getSides())
    {
        qWarning("Cannot extend - no matching boundary");
        return;
    }

    QPointF tip(1,0);
    QPointF e_tip(1,0);

    QTransform t1 = motif->getDELTransform();
    tip = t1.map(tip);
    VertexPtr v1 = motifMap->getVertex(tip);
    Q_ASSERT(v1);

    QTransform t2 = motif->getTile()->getTransform();
    QTransform t3 = eb.getTransform();
    QTransform t4 = t2 * t3;
    e_tip = t4.map(e_tip);
    auto newv = motifMap->insertVertex(e_tip);
    Q_ASSERT(newv);

    newVertices.push_back(newv);
    motifMap->insertEdge(v1,newv);

    for (int idx = 1; idx < n; idx++)
    {
        tip   = unitRotationTr.map(tip);
        v1    = motifMap->getVertex(tip);
        e_tip = unitRotationTr.map(e_tip);
        auto newv = motifMap->insertVertex(e_tip);
        newVertices.push_back(newv);
        motifMap->insertEdge(v1,newv);
    }
}

// conenct vertices on boundary
void Extender::connectOuterVertices(MapPtr motifMap)
{
    // this algorithm connects vertices on the same edge
    const ExtendedBoundary & eb = motif->getExtendedBoundary();
    QVector<QLineF> blines = Geo::polyToLines(eb.getPoly());

    for (const auto & line : std::as_const(blines))
    {
        VertexPtr v1;
        VertexPtr v2;
        auto it = newVertices.begin();
        while (it != newVertices.end())
        {
            if (!v1)
            {
                if (it == newVertices.end())
                    continue;
                v1  = *it++;
                if (!Geo::pointOnLine(line,v1->pt))
                {
                    v1.reset();
                    continue;
                }
            }
            // we have v1
            if (!v2)
            {
                if (it == newVertices.end())
                    continue;
                v2  = *it++;
                if (!Geo::pointOnLine(line,v2->pt))
                {
                    v2.reset();
                    continue;
                }
            }
            // we have v1 and v2
            motifMap->insertEdge(v1,v2);
            v1.reset();
            v2.reset();
        }
    }
}

qreal Extender::len(VertexPtr v1, VertexPtr v2)
{
    return QLineF(v1->pt,v2->pt).length();
}




