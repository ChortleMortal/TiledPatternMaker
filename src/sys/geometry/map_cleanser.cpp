#include <cmath>
#include <QDebug>
#include <QStack>

#include "sys/geometry/map.h"
#include "sys/geometry/map_cleanser.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/vertex.h"
#include "sys/geometry/neighbours.h"
#include "sys/geometry/neighbour_map.h"
#include "sys/geometry/loose.h"
#include "sys/geometry/geo.h"
#include "sys/qt/timers.h"

// cleanse just cleanses - it does not verify
void MapCleanser::cleanse(uint options, qreal sensitivity)
{
    if (options == 0)
        return;

    qDebug().noquote() << "Map::cleanse opions =" << Qt::hex << options << "- BEGIN";

    AQElapsedTimer timer;

    if (options & coalescePoints)
    {
        deDuplicateVertices(sensitivity);
    }

    if (options & joinupColinearEdges)
    {
        joinColinearEdges();
    }

    if (options & divideupIntersectingEdges)
    {
        divideIntersectingEdges();
        deDuplicateEdgesUsingNeighbours();
    }

    if (options & coalesceEdges)
    {
        removeBadEdges();
        deDuplicateEdgesUsingNeighbours();
    }

    // do this first, before badVertices_0
    if (options & badVertices_1)
    {
        removeVerticesWithEdgeCount(1);
    }

    if (options & badVertices_0)
    {
        removeVerticesWithEdgeCount(0);
    }

    qDebug().noquote() << "Map::cleanse - END" << timer.getElapsed();
}

uint MapCleanser::analyze(qreal sensitivity)
{
    uint level = 0;

    int vcount = map->vertices.size();
    int ecount = map->edges.size();

    deDuplicateVertices(sensitivity);

    if (vcount != map->vertices.size() || ecount != map->edges.size() )
    {
        vcount = map->vertices.size();
        ecount = map->edges.size();
        level |= coalescePoints;
    }

    joinColinearEdges();

    if (vcount != map->vertices.size() || ecount != map->edges.size() )
    {
        vcount = map->vertices.size();
        ecount = map->edges.size();
        level |= joinupColinearEdges;
    }

    divideIntersectingEdges();
    deDuplicateEdgesUsingNeighbours();

    if (vcount != map->vertices.size() || ecount != map->edges.size() )
    {
        vcount = map->vertices.size();
        ecount = map->edges.size();
        level |= divideupIntersectingEdges;
    }

    removeBadEdges();
    deDuplicateEdgesUsingNeighbours();
    if (vcount != map->vertices.size() || ecount != map->edges.size() )
    {
        vcount = map->vertices.size();
        ecount = map->edges.size();
        level |= coalesceEdges;
    }

    // do this first, before badVertices_0
    removeVerticesWithEdgeCount(1);
    if (vcount != map->vertices.size() || ecount != map->edges.size() )
    {
        vcount = map->vertices.size();
        ecount = map->edges.size();
        level |= badVertices_1;
    }

    removeVerticesWithEdgeCount(0);
    if (vcount != map->vertices.size() || ecount != map->edges.size() )
    {
        vcount = map->vertices.size();
        ecount = map->edges.size();
        level |= badVertices_0;
    }

    deDuplicateEdgesUsingNeighbours();
    if (vcount != map->vertices.size() || ecount != map->edges.size() )
    {
        level |= cleanupEdges;
    }

    //verify();

    return level;
}

void MapCleanser::removeBadEdges()
{
    qDebug().noquote() << "Map::removeBadEdges" << map->info();

    EdgeSet baddies;

    for (const auto & e : std::as_const(map->edges))
    {
        auto v1 = e->v1;
        auto v2 = e->v2;

        if (!v1 || !map->vertices.contains(v1))
        {
            //qDebug() << "not really an edge (1)" << edgeIndex(e);
            baddies.push_back(e);
        }
        else if (!v2 || !map->vertices.contains(v2))
        {
            //qDebug() << "not really an edge (2)" << edgeIndex(e);
            baddies.push_back(e);
        }
        else if (v1 == v2)
        {
            //qDebug() << "removing null edge (3)" << edgeIndex(e);
            baddies.push_back(e);
        }
        else if (v1->pt == v2->pt)
        {
            //qDebug() << "removing null edge (4)" << edgeIndex(e);
            baddies.push_back(e);
        }
    }

    for (auto & e : std::as_const(baddies))
    {
        map->removeEdge(e);
    }

    qDebug().noquote() << "Map::removeBadEdges removed" << baddies.size() << "edges" << map->info();
}

void MapCleanser::divideIntersectingEdges()
{
    qDebug() << "divideIntersectingEdges - start edges =" << map->edges.count();

    QStack<Isect> isects;
    for (const auto & edge : std::as_const(map->edges))
    {
        isects += map->findIntersections(edge);
    }

    map->processIntersections(isects);

    qDebug() << "divideIntersectingEdges - done edges =" << map->edges.count();
}

void MapCleanser::joinColinearEdges()
{
    qDebug() << "joinColinearEdges.........";

    // Built once and maintained incrementally by combineLinearEdges below,
    // rather than rebuilt from scratch (an O(E) scan) after every single
    // join - a map with many colinear chains to collapse could otherwise
    // cost O(k*E) for k joins.
    NeighbourMap nmap(map);
    while (joinOneColinearEdge(nmap))
        ;
}

bool MapCleanser::joinOneColinearEdge(NeighbourMap & nmap)
{
    for (const auto & vp : std::as_const(map->vertices))
    {
        NeighboursPtr n = nmap.getNeighbours(vp);
        int count = n->numNeighbours();
        if (count == 2)
        {
            QLineF a = n->getNeighbour(0)->getLine();
            QLineF b = n->getNeighbour(1)->getLine();
            qreal angle = Geo::angle(a,b);
            if (Loose::zero(angle,Loose::NEAR_TOL) || Loose::equals(angle,180.0, Loose::NEAR_TOL))
            {
                // need to remove one edge, extend the other, and remove vertex
                qDebug() << "HIT";
                combineLinearEdges(n->getNeighbour(0),n->getNeighbour(1),vp,nmap);
                return true;
            }
        }
    }
    return false;
}

// combine two edges which are in a straight line with common vertex
void MapCleanser::combineLinearEdges(const EdgePtr & a, const EdgePtr & b, const VertexPtr & common, NeighbourMap & nmap)
{
    VertexPtr newV1 = a->getOtherV(common);
    VertexPtr newV2 = b->getOtherV(common);

    EdgePtr newEdge = map->insertEdge(newV1,newV2);

    map->removeEdge(a);
    map->removeEdge(b);
    map->removeVertexSimple(common);

    // Patch nmap's entries for the two surviving endpoints in place instead
    // of leaving them (and everything else in nmap) stale for the next
    // caller to rebuild. common's own entry doesn't need patching - it's
    // gone from map->vertices and joinOneColinearEdge only ever looks up
    // vertices still in that list.
    if (newEdge)
    {
        auto patch = [&](const VertexPtr & v, const EdgePtr & oldEdge)
        {
            NeighboursPtr n = nmap.getNeighbours(v);
            n->removeIf([&](const WeakEdgePtr & w) { return w.lock() == oldEdge; });
            n->insertNeighbour(newEdge);
        };
        patch(newV1, a);
        patch(newV2, b);
    }
}

// coalesce identical vertices to eliminate duplicates.
bool MapCleanser::coalesceVertices(qreal tolerance)
{
    qDebug().noquote() << "coalesceVertices-start" <<  map->summary() << "Tolerance = " << tolerance;
    qsizetype start = map->vertices.size();

    QMap<VertexPtr,VertexPtr> replacements;
    qsizetype size = map->vertices.size();

    // Was an O(V^2) all-pairs scan - the same pattern Map::_getOrCreateVertex
    // had before its grid fix. Replaced with a call-scoped spatial hash,
    // sized directly from this call's own `tolerance` (compared as squared
    // distance, so linear cell size is sqrt(tolerance) - not sampled from
    // data, so the stale-cell-size failure mode that hit the incremental
    // Map grids doesn't apply here). This can't reuse Map's own vertexGrid:
    // `tolerance` varies ~14 orders of magnitude across call sites
    // (1e-16 to 0.5, see DlgCleanse/mapedMergeSensitivity/motif defaults),
    // incompatible with vertexGrid's fixed GRID_CELL.
    qreal cellSize = qMax(std::sqrt(tolerance) * 1.01, 1e-9);
    auto gridKey = [](qint64 cx, qint64 cy) -> qint64
    {
        return (cx << 32) ^ (cy & 0xffffffffLL);
    };
    auto cellOf = [cellSize](const QPointF & pt) -> QPair<qint64,qint64>
    {
        return { static_cast<qint64>(std::floor(pt.x() / cellSize)),
                 static_cast<qint64>(std::floor(pt.y() / cellSize)) };
    };

    QHash<qint64, QVector<int>> grid;
    for (int i = 0; i < size; i++)
    {
        auto [cx,cy] = cellOf(map->vertices[i]->pt);
        grid[gridKey(cx,cy)].push_back(i);
    }

    for (int i = 0; i < size; i++)
    {
        QPointF pt = map->vertices[i]->pt;
        auto [cx,cy] = cellOf(pt);

        // Among all candidates j > i within tolerance in the 3x3 cell
        // neighborhood, keep the highest index - reproduces the nested
        // loop's "last match in increasing-j order wins" behaviour exactly.
        int best = -1;
        for (qint64 dx = -1; dx <= 1; dx++)
        {
            for (qint64 dy = -1; dy <= 1; dy++)
            {
                auto it = grid.constFind(gridKey(cx+dx,cy+dy));
                if (it == grid.constEnd())
                    continue;

                for (int j : it.value())
                {
                    if (j > i && j > best && Geo::dist2(pt,map->vertices[j]->pt) < tolerance)
                    {
                        best = j;
                    }
                }
            }
        }

        if (best != -1)
        {
            replacements[map->vertices[i]] = map->vertices[best];
        }
    }
    qDebug() << "Vertices to replace =" << replacements.size();
    if (replacements.size() == 0)
    {
        qDebug().noquote() << "coalesceVertices-end  " <<  map->summary();
        return false;
    }

    QMap<VertexPtr,VertexPtr>::iterator it;
    UniqueQVector<VertexPtr> deletions;
    for (const auto & edge : std::as_const(map->edges))
    {
        it = replacements.find(edge->v1);
        if (it != replacements.end())
        {
            edge->setV1(it.value());
            deletions.push_back(it.key());
        }
        it = replacements.find(edge->v2);
        if (it != replacements.end())
        {
            edge->setV2(it.value());
            deletions.push_back(it.key());
        }
    }

    for (auto & vert : std::as_const(deletions))
    {
        map->removeVertexSimple(vert);
    }

    cleanseVertices();

    qDebug().noquote() << "coalesceVertices-end  " <<  map->summary();

    if (map->vertices.size() != start)
    {
        return true;    // has coalesced
    }
    return false;
}

void MapCleanser::deDuplicateVertices(qreal tolerance)
{
    bool coalesced = false;
    while (coalesceVertices(tolerance))
        coalesced = true;

    // coalescing can collapse short edges and make duplicate edges
    if (coalesced)
    {
        removeBadEdges();
        deDuplicateEdgesUsingNeighbours();
    }
}

void MapCleanser::deDuplicateEdgesUsingNeighbours(bool silent)
{
    if (!silent) qDebug().noquote() << "Map::deDuplicateEdgesUsingNeighbours BEGIN" << map->info();

    NeighbourMap nmap(map);

    for (const auto & v :  std::as_const(map->vertices))
    {
        // examining a vertex
        NeighboursPtr n = nmap.getNeighbours(v);
        deDuplicateEdges(n);
    }
    if (!silent) qDebug().noquote() << "Map::deDuplicateEdgesUsingNeighbours END"  << map->info();
}

void MapCleanser::deDuplicateEdges(const NeighboursPtr & vec)
{
    // examining the positions edges of associated with the vertex
    EdgeSet duplicateEdges;
    for (auto it = vec->begin(); it != vec->end(); it++)
    {
        WeakEdgePtr wep = *it;
        EdgePtr ep = wep.lock();
        if (ep)
        {
            auto it2 = it;
            it2++;
            while (it2 != vec->end())
            {
                WeakEdgePtr wep2 = *it2;
                EdgePtr ep2 = wep2.lock();
                if (ep2 && ep->sameAs(ep2))
                {
                    duplicateEdges.push_back(ep2);
                }
                it2++;
            }
        }
    }

    // remove duplicates from the map
    for (const auto & e : duplicateEdges)
    {
        map->removeEdge(e);
    }
}

void MapCleanser::cleanseVertices()
{
    NeighbourMap nmap(map);
    std::vector<VertexPtr> baddies;
    for (const auto & v : std::as_const(map->vertices))
    {
        NeighboursPtr n = nmap.getNeighbours(v);
        if (n->size() == 0)
        {
            baddies.push_back(v);
        }
    }
    qDebug() << "Bad vertices to delete:" << baddies.size();
    for (const auto & v  : std::as_const(baddies))
    {
        map->removeVertexSimple(v);
    }
}

void MapCleanser::removeVerticesWithEdgeCount(uint edgeCount)
{
    qDebug() << "removeVerticesWithEdgeCount" << edgeCount << "......";

    NeighbourMap nmap(map);
    QVector<VertexPtr> verts;
    for (const auto & v : std::as_const(map->vertices))
    {
        NeighboursPtr n = nmap.getNeighbours(v);
        if (n->numNeighbours() == edgeCount)
        {
            verts.push_back(v);
        }
    }

    for (auto & v : std::as_const(verts))
    {
       map->removeVertex(v);
    }

    if (verts.size())
        qInfo() << "removed" << verts.size() << "vertices with edge count" << edgeCount;
    else
        qDebug() << "removed 0 vertices with edge count" << edgeCount;
}
