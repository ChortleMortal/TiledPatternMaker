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


    bool changed = false;
    do
    {
        NeighbourMap nmap(map);     // kep rebuilding nmap
        changed = joinOneColinearEdge(nmap);
        //verifyMap("joinColinearEdges",false,true);
    } while (changed);
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
            qDebug() << angle;
            if (Loose::zero(angle,Sys::NEAR_TOL) || Loose::equals(angle,180.0, Sys::NEAR_TOL))
            {
                // need to remove one edge, extend the other, and remove vertex
                qDebug() << "HIT";
                combineLinearEdges(n->getNeighbour(0),n->getNeighbour(1),vp);
                return true;
            }
        }
    }
    return false;
}

// combine two edges which are in a straight line with common vertex
void MapCleanser::combineLinearEdges(const EdgePtr & a, const EdgePtr & b, const VertexPtr & common)
{
    VertexPtr newV1 = a->getOtherV(common);
    VertexPtr newV2 = b->getOtherV(common);
    map->insertEdge(newV1,newV2);
    map->removeEdge(a);
    map->removeEdge(b);
    map->removeVertexSimple(common);
}

// coalesce identical vertices to eliminate duplicates.
bool MapCleanser::coalesceVertices(qreal tolerance)
{
    qDebug().noquote() << "coalesceVertices-start" <<  map->summary() << "Tolerance = " << tolerance;
    qsizetype start = map->vertices.size();

    QMap<VertexPtr,VertexPtr> replacements;
    qsizetype size = map->vertices.size();
    for (int i = 0; i < size; i++)
    {
        for (int j = i+1 ;  j < size; j++)
        {
            if (Geo::dist2(map->vertices[i]->pt,map->vertices[j]->pt) < tolerance)
            {
                replacements[map->vertices[i]] = map->vertices[j];
            }
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
        map->vertices.removeOne(vert);
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
    while (coalesceVertices(tolerance))
        ;
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
        map->vertices.removeOne(v);
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
