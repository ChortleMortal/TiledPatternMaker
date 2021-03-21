#include "geometry/map_cleanser.h"
#include "geometry/map.h"
#include "geometry/loose.h"
#include "geometry/intersect.h"
#include "base/configuration.h"
#include "base/misc.h"
#include "base/utilities.h"

MapCleanser::MapCleanser(MapPtr map)
{
    this->map = map;

    deb = new QDebug(&astring);
}

MapCleanser::~MapCleanser()
{
}

bool MapCleanser::cleanse(unsigned int options, bool forceVerify)
{
    qDebug() << "cleanse......";
    const bool debug = false;

    qDebug() << "Map::cleanse - start";

    map->verifyMap("cleanse-start");

    if (options & joinupColinearEdges)
    {
        joinColinearEdges();
        if (debug) map->verifyMap("Map:joinColinearEdges()");
    }

    if (options & divideupIntersectingEdges)
    {
        divideIntersectingEdges();
        if (debug) map->verifyMap("Map:divideIntersectingEdges()");
    }

    if (options & badVertices_0)
    {
        removeVerticesWithEdgeCount(0);
        if (debug) map->verifyMap("Map:removeVerticesWithEdgeCount(0)");
    }

    if (options & badVertices_1)
    {
        removeVerticesWithEdgeCount(1);
        if (debug) map->verifyMap("Map:removeVerticesWithEdgeCount(1)");
    }

    if (options & badEdges)
    {
        removeBadEdges();
        if (debug) map->verifyMap("Map:removeBadEdges");
    }

    if (options & badNeighbours)
    {
        fixNeighbours();
        if (debug) map->verifyMap("Map:fixNeighbours");
    }

    if (options & cleanupNeighbours)
    {
        cleanNeighbours();
        if (debug) map->verifyMap("Map:cleanNeighbours");
    }

    bool rv = map->verifyMap("cleanse-end",forceVerify);
    qDebug() << "Map::cleanse - end";
    return rv;
}

void MapCleanser::removeBadEdges()
{
    qDebug() << "removeBadEdges";

    QVector<EdgePtr> baddies;

    for (const auto & e : qAsConst(map->edges))
    {
        // examining a vertex
        if (e->v1 == e->v2)
        {
            qDebug() << "removing null edge (1)" << map->edgeIndex(e);
            baddies.push_back(e);
        }
        else if (e->v1->pt == e->v2->pt)
        {
            qDebug() << "removing null edge (2)" << map->edgeIndex(e);
            baddies.push_back(e);
        }
        if (!e->v1)
        {
            qDebug() << "not really an edge (3)" << map->edgeIndex(e);
            baddies.push_back(e);
        }
        if (!e->v2)
        {
            qDebug() << "not really an edge (4)" << map->edgeIndex(e);
            baddies.push_back(e);
        }
    }

    for (auto & e : baddies)
    {
        map->removeEdge(e);
    }

    qDebug() << "removed" << baddies.size() << "edges";
}

void MapCleanser::divideIntersectingEdges()
{
    qDebug() << "divideIntersectingEdges......";
    qDebug().noquote() << map->summary();

    UniqueQVector<QPointF> intersects;
    for(auto edge : qAsConst(map->edges))
    {
        // To check all intersections of this edge with edges in
        // the current map, we can use the optimization that
        // edges are stored in sorted order by min x.  So at the
        // very least, skip the suffix of edges whose minimum x values
        // are past the max x of the edge to check.
        // Casper 12DEC02 - removed optimisation and simplified code

        QLineF e = edge->getLine();
        for (auto cur : qAsConst(map->edges))
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
    for (QPointF pt : intersects)
    {
        //qDebug() << "New split at" << pt;
        map->insertVertex(pt);
    }

    qDebug().noquote() << map->summary();
    qDebug() << "divideIntersectingEdges - done";
}

void MapCleanser::joinColinearEdges()
{
    qDebug() << "joinColinearEdges.........";

    bool changed = false;
    do
    {
        changed = joinOneColinearEdge();
        //verifyMap("joinColinearEdges",false,true);
    } while (changed);

}

bool MapCleanser::joinOneColinearEdge()
{
    for (auto& vp : map->vertices)
    {
        int count = vp->numNeighbours();
        if (count == 2)
        {
            QLineF a = vp->getNeighbour(0)->getLine();
            QLineF b = vp->getNeighbour(1)->getLine();
            qreal angle = Utils::angle(a,b);
            if (Loose::zero(angle) || Loose::equals(angle,180.0))
            {
                // need to remove one edge, extend the other, and remove vertex
                combineLinearEdges(vp->getNeighbour(0),vp->getNeighbour(1),vp);
                return true;
            }
        }
    }
    return false;
}

// combine two edges which are in a straight line with common vertex
void MapCleanser::combineLinearEdges(EdgePtr a, EdgePtr b,VertexPtr common)
{
    VertexPtr newV1 = a->getOtherV(common);
    VertexPtr newV2 = b->getOtherV(common);

    map->insertEdge(newV1,newV2);
    map->removeEdge(a);
    map->removeEdge(b);
}

void MapCleanser::cleanNeighbours()
{
    qDebug() << "cleanNeighbours BEGIN edges=" << map->edges.size()  << "vertices=" << map->vertices.size();

    for (auto & v :  map->vertices)
    {
        // examining a vertex
        const QVector<EdgePtr> & list = v->getNeighbours();
        deDuplicateEdges(list);
    }
    qDebug() << "cleanNeighbours END   edges=" << map->edges.size()  << "vertices=" << map->vertices.size();
}

void MapCleanser::fixNeighbours()
{
    qDebug() << "fixNeighbours";

    for (const auto & e : qAsConst(map->edges))
    {
        VertexPtr v1 = e->v1;
        if (!map->vertices.contains(v1))
        {
            map->vertices.push_back(v1);
        }

        VertexPtr v2 = e->v2;
        if (!map->vertices.contains(v2))
        {
            map->vertices.push_back(v1);
        }

        if (!v1->contains(e))
        {
            v1->insertNeighbour(e);
        }

        if (!v2->contains(e))
        {
            v2->insertNeighbour(e);
        }
    }
}

void MapCleanser::deDuplicateEdges(const QVector<EdgePtr> & vec)
{
    // examining the positions edges of associated with the vertex
    QVector<EdgePtr> duplicateEdges;
    for (auto it = vec.begin(); it != vec.end(); it++)
    {
        EdgePtr e = *it;
        auto it2 = it;
        it2++;
        for ( ; it2 != vec.end(); it2++)
        {
            EdgePtr e2 = *it2;
            if (e->sameAs(e2))
            {
                duplicateEdges.push_back(e2);
            }
        }
    }

    // remove duplicates from the map
    for (const auto & e : duplicateEdges)
    {
        map->removeEdge(e);
    }
}

void MapCleanser::removeVerticesWithEdgeCount(int edgeCount)
{
    qDebug() << "removeVerticesWithEdgeCount" << edgeCount << "......";
    QVector<VertexPtr> verts;
    for (auto & v : map->vertices)
    {
        if (v->numNeighbours() == edgeCount)
        {
            verts.push_back(v);
        }
    }

    for (auto & v : verts)
    {
        map->removeVertex(v);
    }

    if (verts.size())
        qInfo() << "removed" << verts.size() << "vertices with edge count" << edgeCount;
    else
        qDebug() << "removed 0 vertices with edge count" << edgeCount;
}
