#include <QDebug>

#include "sys/geometry/map.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/vertex.h"
#include "sys/geometry/neighbours.h"
#include "sys/geometry/intersect.h"
#include "sys/geometry/loose.h"
#include "sys/geometry/geo.h"
#include "sys/qt/timers.h"

// cleanse just cleanses - it does not verify
void Map::cleanse(uint options, qreal sensitivity)
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
        nMap.reset();
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

    if (options & cleanupNeighbours)
    {
        deDuplicateEdgesUsingNeighbours();
    }

    qDebug().noquote() << "Map::cleanse - END" << timer.getElapsed();
}

uint Map::cleanseAnalysis(qreal sensitivity)
{
    uint level = 0;

    int vcount = vertices.size();
    int ecount = edges.size();

    deDuplicateVertices(sensitivity);
    if (vcount != vertices.size() || ecount != edges.size() )
    {
        vcount = vertices.size();
        ecount = edges.size();
        level |= coalescePoints;
    }

    joinColinearEdges();
    if (vcount != vertices.size() || ecount != edges.size() )
    {
        vcount = vertices.size();
        ecount = edges.size();
        level |= joinupColinearEdges;
    }

    divideIntersectingEdges();
    deDuplicateEdgesUsingNeighbours();
    nMap.reset();
    if (vcount != vertices.size() || ecount != edges.size() )
    {
        vcount = vertices.size();
        ecount = edges.size();
        level |= divideupIntersectingEdges;
    }

    removeBadEdges();
    deDuplicateEdgesUsingNeighbours();
    if (vcount != vertices.size() || ecount != edges.size() )
    {
        vcount = vertices.size();
        ecount = edges.size();
        level |= coalesceEdges;
    }

    // do this first, before badVertices_0
    removeVerticesWithEdgeCount(1);
    if (vcount != vertices.size() || ecount != edges.size() )
    {
        vcount = vertices.size();
        ecount = edges.size();
        level |= badVertices_1;
    }

    removeVerticesWithEdgeCount(0);
    if (vcount != vertices.size() || ecount != edges.size() )
    {
        //vcount = vertices.size();
        //ecount = edges.size();
        level |= badVertices_0;
    }

    uint ncount = 0;
    if (nMap)
    {
        ncount = nMap->rawSize();
    }
    nMap = std::make_shared<NeighbourMap>(edges);
    if (nMap->rawSize() != ncount)
    {
        ncount = nMap->rawSize();
        level |= buildNeighbours;
    }

    deDuplicateEdgesUsingNeighbours();
    if (nMap->rawSize() != ncount)
    {
        //ncount = nMap->rawSize();
        level |= cleanupNeighbours;
    }

    //verify();

    return level;
}

void Map::removeBadEdges()
{
    qDebug().noquote() << "Map::removeBadEdges" << info();

    QVector<EdgePtr> baddies;

    for (const auto & e : std::as_const(edges))
    {
        auto v1 = e->v1;
        auto v2 = e->v2;

        if (!v1 || !vertices.contains(v1))
        {
            //qDebug() << "not really an edge (1)" << edgeIndex(e);
            baddies.push_back(e);
        }
        else if (!v2 || !vertices.contains(v2))
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
        removeEdge(e);
    }

    qDebug().noquote() << "Map::removeBadEdges removed" << baddies.size() << "edges" << info();
}

void Map::divideIntersectingEdges()
{
    qDebug() << "Map::divideIntersectingEdges......";
    qWarning("Does not handle curves");
    dump();

    UniqueQVector<QPointF> intersects;
    for (const auto & edge : std::as_const(edges))
    {
        // To check all intersections of this edge with edges in
        // the current map, we can use the optimization that
        // edges are stored in sorted order by min x.  So at the
        // very least, skip the suffix of edges whose minimum x values
        // are past the max x of the edge to check.
        // Casper 12DEC02 - removed optimisation and simplified code

        QLineF e = edge->getLine();
        for (const auto & cur : std::as_const(edges))
        {
            if (cur == edge)
            {
                continue;
            }

            QLineF  c  = cur->getLine();
            QPointF apt;
            if (Intersect::getTrueIntersection(e, c, apt))
            {
                intersects.push_back(apt);
            }
        }
    }
    // now split the edges
    qDebug() << "divideIntersectingEdges - splitting at" << intersects.count() << "points";
    for (QPointF pt : std::as_const(intersects))
    {
        qDebug() << "New split at" << pt;
        qWarning("Does not work");
        insertVertex(pt);
    }
    
    dump();
    qDebug() << "divideIntersectingEdges - done";
}

void Map::joinColinearEdges()
{
    qDebug() << "joinColinearEdges.........";

    bool changed = false;
    do
    {
        changed = joinOneColinearEdge();
        //verifyMap("joinColinearEdges",false,true);
    } while (changed);
}

bool Map::joinOneColinearEdge()
{
    for (const auto & vp : std::as_const(vertices))
    {
        NeighboursPtr n = getNeighbours(vp);
        int count = n->numNeighbours();
        if (count == 2)
        {
            QLineF a = n->getNeighbour(0)->getLine();
            QLineF b = n->getNeighbour(1)->getLine();
            qreal angle = Geo::angle(a,b);
            if (Loose::zero(angle) || Loose::equals(angle,180.0))
            {
                // need to remove one edge, extend the other, and remove vertex
                combineLinearEdges(n->getNeighbour(0),n->getNeighbour(1),vp);
                nMap.reset();
                return true;
            }
        }
    }
    return false;
}

// combine two edges which are in a straight line with common vertex
void Map::combineLinearEdges(const EdgePtr & a, const EdgePtr & b, const VertexPtr & common)
{
    VertexPtr newV1 = a->getOtherV(common);
    VertexPtr newV2 = b->getOtherV(common);

    insertEdge(newV1,newV2);
    removeEdge(a);
    removeEdge(b);

    nMap.reset();
}

// coalesce identical vertices to eliminate duplicates.
bool Map::coalesceVertices(qreal tolerance)
{
    qDebug().noquote() << "coalesceVertices-start" <<  summary() << "Tolerance = " << tolerance;
    qsizetype start = vertices.size();

    QMap<VertexPtr,VertexPtr> replacements;
    qsizetype size = vertices.size();
    for (int i = 0; i < size; i++)
    {
        for (int j = i+1 ;  j < size; j++)
        {
            if (Geo::dist2(vertices[i]->pt,vertices[j]->pt) < tolerance)
            {
                replacements[vertices[i]] = vertices[j];
            }
        }
    }
    qDebug() << "Vertices to replace =" << replacements.size();
    if (replacements.size() == 0)
    {
        qDebug().noquote() << "coalesceVertices-end  " <<  summary();
        return false;
    }

    QMap<VertexPtr,VertexPtr>::iterator it;
    UniqueQVector<VertexPtr> deletions;
    for (const auto & edge : std::as_const(edges))
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
        vertices.removeOne(vert);
    }

    nMap.reset();

    _cleanseVertices();

    qDebug().noquote() << "coalesceVertices-end  " <<  summary();

    if (vertices.size() != start)
    {
        return true;    // has coalesced
    }
    return false;
}

void Map::deDuplicateVertices(qreal tolerance)
{
    while (coalesceVertices(tolerance))
        ;
}

void Map::deDuplicateEdgesUsingNeighbours(bool silent)
{
    if (!silent) qDebug().noquote() << "Map::deDuplicateEdgesUsingNeighbours BEGIN" << info();

    for (const auto & v :  std::as_const(vertices))
    {
        // examining a vertex
        NeighboursPtr n = getNeighbours(v);
        deDuplicateEdges(n);
    }
    if (!silent) qDebug().noquote() << "Map::deDuplicateEdgesUsingNeighbours END"  << info();
}

void Map::deDuplicateEdges(const NeighboursPtr & vec)
{
    // examining the positions edges of associated with the vertex
    QVector<EdgePtr> duplicateEdges;
    for (auto it = vec->begin(); it != vec->end(); it++)
    {
        WeakEdgePtr wep = *it;
        auto it2 = it;
        it2++;
        for ( ; it2 != vec->end(); it2++)
        {
            WeakEdgePtr wep2 = *it2;
            if (wep.lock()->sameAs(wep2.lock()))
            {
                duplicateEdges.push_back(wep2.lock());
            }
        }
    }

    // remove duplicates from the map
    for (const auto & e : duplicateEdges)
    {
        removeEdge(e);
    }
}

void Map::removeVerticesWithEdgeCount(uint edgeCount)
{
    qDebug() << "removeVerticesWithEdgeCount" << edgeCount << "......";

    QVector<VertexPtr> verts;
    for (const auto & v : std::as_const(vertices))
    {
        NeighboursPtr n = getNeighbours(v);
        if (n->numNeighbours() == edgeCount)
        {
            verts.push_back(v);
        }
    }

    for (auto & v : std::as_const(verts))
    {
        removeVertex(v);
    }

    if (verts.size())
        qInfo() << "removed" << verts.size() << "vertices with edge count" << edgeCount;
    else
        qDebug() << "removed 0 vertices with edge count" << edgeCount;
}
