#include <QDebug>

#include "model/motifs/adaptive_extender.h"
#include "model/motifs/motif.h"
#include "sys/geometry/map_verifier.h"
#include "sys/geometry/neighbour_map.h"
#include "sys/geometry/neighbours.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/transform.h"
#include "sys/geometry/intersect.h"
#include "model/tilings/tile.h"

///////////////////////////////////////////////////////////////////////////////
///
///  AdaptiveExtender
///
///////////////////////////////////////////////////////////////////////////////

AdaptiveExtender::AdaptiveExtender(Motif *motif) : Extender(motif)
{
}

AdaptiveExtender::AdaptiveExtender(const Extender & other) : Extender(other)
{
}

void AdaptiveExtender::extend()
{
    Q_ASSERT(motif);
    Q_ASSERT(motif->getTile());

    QTransform t;
    extend(t);
}

void AdaptiveExtender::extend(QTransform Tr)
{
    Q_ASSERT(motif);
    Q_ASSERT(motif->getTile());

    newVertices.clear();

    MapPtr map;
    if (motif->hasValidMotifMap())
    {
        map = motif->getExistingMotifMap();
    }
    else
    {
        return; // can't extend something that doesn't exist
    }

    buildExtendedBoundary();

    MapVerifier mv(map);
    mv.verify();

    if (getExtendRays())   // extend rays
    {
        extendMotifRays(map);
    }

    if (getExtendTipsToBound())  // tips
    {
        extendTipsToBoundary(map,Tr);
    }

    if (getExtendBoundaryToTile())  // points on boundary
    {
        extendBoundaryMap(map,motif->getTile());
    }

    if (getConnectRays())  // connect to each other
    {
        connectOuterVertices(map);
    }

    if (getEmbedBoundary())
    {
        embedBoundaryInMap(map);
    }

    if (getEmbedTile())
    {
        embedTileInMap(map,motif->getTile());
    }

    mv.verify();
}

void AdaptiveExtender::extendMotifRays(MapPtr motifMap)
{
    Q_ASSERT(getExtendRays());

    qDebug().noquote() << "Extender::extendMotifRays" << motifMap->summary();

    buildExtendedBoundary();

    const ExtendedBoundary & eb = getExtendedBoundary();

    qreal radius = eb.getRadius();
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
        if (Geo::intersectPoly(l1,fboundary,intersect))
        {
            // l1 intersects boundary
            if (!eb.isCircle())
            {
                // polygonal boundary
                // dont extend lines which already touch boundary
                QPointF intersect2;
                if (!Geo::intersectPoly(l1,eboundary,intersect2))
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

void AdaptiveExtender::extendTipsToBoundary(MapPtr motifMap, QTransform unitRotationTr)
{
    if (motif->isIrregular())
    {
        // FIXME implement Extender::extendFreeMap for irregular
        qWarning() << "Extender::extendTipsToBoundary - FOR IRREGULAR NOT YET SUPPORTEDf";
        return;
    }

    qDebug() << "Tile transform" << Transform::info(unitRotationTr);
    const ExtendedBoundary & eb = getExtendedBoundary();
    int n = motif->getN();
    if (n != eb.getSides())
    {
        qWarning("Cannot extend - no matching boundary");
        return;
    }

    QPointF tip(1,0);
    QPointF e_tip(1,0);

    QTransform t1 = motif->getMotifTransform();
    tip = t1.map(tip);
    VertexPtr v1 = motifMap->getVertex(tip);
    Q_ASSERT(v1);

    QTransform t2 = motif->getTile()->getEpolyTransform();
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
void AdaptiveExtender::connectOuterVertices(MapPtr motifMap)
{
    // this algorithm connects vertices on the same edge
    const ExtendedBoundary & eb = getExtendedBoundary();
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

void AdaptiveExtender::extendBoundaryMap(MapPtr motifMap, TilePtr tile)
{
    Q_ASSERT(getExtendBoundaryToTile());
    Q_ASSERT(tile);

    const ExtendedBoundary & eb = getExtendedBoundary();
    const QVector<QLineF> blines = Geo::polyToLines(eb.getPoly());

    NeighbourMap nmap(motifMap);

    MapPtr newMap = std::make_shared<Map>("Temp");

    const QVector<VertexPtr> & vertices = motifMap->getVertices();
    for (auto & v : vertices)
    {
        NeighboursPtr np = nmap.getNeighbours(v);
        if (np->numNeighbours() == 1)
        {
            // is a tip - which is on a boundary
            // draw line normal to the boundary from this point
            // until it intersects the tile
            for (const QLineF & bline : blines)
            {
                if (Geo::pointOnLine(bline,v->pt))
                {
                    QPointF apt;
                    qreal dist = 1000;
                    QLineF pline = perpLine(bline,v->pt);
                    for (const EdgePtr & edge : tile->getEdgePoly().get())
                    {
                        QPointF isect;
                        if (Intersect::getIntersection(edge->getLine(),pline,isect))
                        {
                            qreal len = QLineF(v->pt,isect).length();
                            if (len < dist)
                            {
                                dist = len;
                                apt  = isect;
                            }
                        }
                    }
                    newMap->insertVertex(v);
                    VertexPtr v2 = newMap->insertVertex(apt);
                    newMap->insertEdge(v,v2);
                    break;
                }
            }
        }
    }
    motifMap->mergeMap(newMap);
}


