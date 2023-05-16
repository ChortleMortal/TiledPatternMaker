#include <QDebug>
#include "motifs/extender.h"
#include "motifs/motif.h"
#include "geometry/map.h"
#include "geometry/edge.h"
#include "geometry/vertex.h"
#include "geometry/neighbours.h"
#include "geometry/point.h"
#include "misc/utilities.h"

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

void Extender::extend(Motif * fig, QTransform Tr)
{
    this->fig = fig;
    newVertices.clear();

    auto map = fig->getMotifMap();

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

void Extender::extendPeripheralMap(MapPtr map)  // is same as extend Peripheral Map - used by rosette
{
    qDebug().noquote() << "Extender::extendPeripheralMap" << map->summary();

    const ExtendedBoundary & eb = fig->getExtendedBoundary();

    qreal radius = eb.scale;
    QGraphicsEllipseItem circle(-radius,-radius,radius * 2.0, radius * 2.0);

    const QPolygonF & eboundary = eb.get();
    const QPolygonF & fboundary = fig->getMotifBoundary();

    // extend the lines to exteneded boundary and clip
    // extendLine and clipLine both assume that p1 is closest to the center
    // and p0 is closest to the edge

    // the shortest line is the one to insert
    QVector<EdgePtr> edges = map->getEdges();
    for (auto & edge  : qAsConst(edges))
    {
        VertexPtr v1 = edge->v1;   // outer
        VertexPtr v2 = edge->v2;   // inner
        QLineF l1(v2->pt,v1->pt);

        QPointF intersect;
        if (extendPeripheralVertices && Point::intersectPoly(l1,fboundary,intersect))
        {
            // l1 intersects boundary
            if (!eb.isCircle())
            {
                // polygonal boundary
                // dont extend lines which already touch boundary
                QPointF intersect2;
                if (Point::intersectPoly(l1,eboundary,intersect2))
                {
                    continue;
                }
                // extend lines
                QLineF l2  = Point::extendLine(l1,10.0);      // extends
                l2         = Point::clipLine(l2,eboundary);   // clips
                QPointF pt = l2.p2();                         // outer
                // test if this new point is already a vertex
                VertexPtr newv = map->insertVertex(pt);
                newVertices.push_back(newv);
                map->insertEdge(v1,newv);
            }
            else
            {
                // circular boundary
                QPointF a;
                QPointF b;
                int points = Utils::circleLineIntersectionPoints(circle,radius,l1,a,b);
                if (points)
                {
                    VertexPtr newv;
                    if (!a.isNull())
                    {
                        newv = map->insertVertex(a);
                        newVertices.push_back(newv);
                        map->insertEdge(v1,newv);
                    }
                    if (!b.isNull())
                    {
                        newv = map->insertVertex(b);
                        newVertices.push_back(newv);
                        map->insertEdge(v1,newv);
                    }
                }
            }
        }
    }
}

void Extender::extendFreeMap(MapPtr map, QTransform Tr)
{
    const ExtendedBoundary & eb = fig->getExtendedBoundary();
    int n = fig->getN();
    if (n != eb.sides)
    {
        qWarning("Cannot extend - no matching boundary");
        return;
    }

    QPointF tip(1,0);
    QTransform t = QTransform::fromScale(fig->getMotifScale(),fig->getMotifScale());
    tip = t.map(tip);
    VertexPtr v1 = map->getVertex(tip);

    QPointF e_tip(1,0);
    QTransform t2 = QTransform::fromScale(eb.scale,eb.scale);
    e_tip = t2.map(e_tip);
    auto newv = map->insertVertex(e_tip);
    newVertices.push_back(newv);
    map->insertEdge(v1,newv);

    for (int idx = 1; idx < n; idx++)
    {
        tip   = Tr.map(tip);
        v1    = map->getVertex(tip);
        e_tip = Tr.map(e_tip);
        auto newv = map->insertVertex(e_tip);
        newVertices.push_back(newv);
        map->insertEdge(v1,newv);
    }
}

// conenct vertices on boundary
void Extender::connectOuterVertices(MapPtr map)
{
    // this algorithm connects vertices on the same edge
    const ExtendedBoundary & eb = fig->getExtendedBoundary();
    QVector<QLineF> blines = Utils::polyToLines(eb.get());

    for (auto line : blines)
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
                if (!Utils::pointOnLine(line,v1->pt))
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
                if (!Utils::pointOnLine(line,v2->pt))
                {
                    v2.reset();
                    continue;
                }
            }
            // we have v1 and v2
            map->insertEdge(v1,v2);
            v1.reset();
            v2.reset();
        }
    }
}

qreal Extender::len(VertexPtr v1, VertexPtr v2)
{
    return QLineF(v1->pt,v2->pt).length();
}




