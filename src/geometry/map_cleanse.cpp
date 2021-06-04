#include "geometry/map.h"
#include "geometry/intersect.h"
#include "base/utilities.h"

bool Map::cleanse(unsigned int options, bool forceVerify)
{
    qDebug() << "cleanse......";
    const bool debug = false;

    qDebug() << "Map::cleanse - start";

    verify();

    if (options & joinupColinearEdges)
    {
        joinColinearEdges();
        if (debug) verify();
    }

    if (options & divideupIntersectingEdges)
    {
        divideIntersectingEdges();
        deDuplicateNeighbours();
        sortVertices();
        sortEdges();
        buildNeighbours();
        if (debug) verify();
    }

    if (options & badVertices_0)
    {
        removeVerticesWithEdgeCount(0);
        if (debug) verify();
    }

    if (options & badVertices_1)
    {
        removeVerticesWithEdgeCount(1);
        if (debug) verify();
    }

    if (options & badEdges)
    {
        removeBadEdges();
        if (debug) verify();
    }

    if (options & cleanupNeighbours)
    {
        deDuplicateNeighbours();
        if (debug) verify();
    }

    bool rv = verify(forceVerify);
    qDebug() << "Map::cleanse - end";
    return rv;
}

void Map::removeBadEdges()
{
    qDebug() << "removeBadEdges";

    QVector<EdgePtr> baddies;

    for (const auto & e : qAsConst(edges))
    {
        // examining a vertex
        if (e->v1 == e->v2)
        {
            qDebug() << "removing null edge (1)" << edgeIndex(e);
            baddies.push_back(e);
        }
        else if (e->v1->pt == e->v2->pt)
        {
            qDebug() << "removing null edge (2)" << edgeIndex(e);
            baddies.push_back(e);
        }
        if (!e->v1)
        {
            qDebug() << "not really an edge (3)" << edgeIndex(e);
            baddies.push_back(e);
        }
        if (!e->v2)
        {
            qDebug() << "not really an edge (4)" << edgeIndex(e);
            baddies.push_back(e);
        }
    }

    for (auto & e : baddies)
    {
        removeEdge(e);
    }

    qDebug() << "removed" << baddies.size() << "edges";
}

void Map::divideIntersectingEdges()
{
    qDebug() << "divideIntersectingEdges......";
    qDebug().noquote() << summary();

    UniqueQVector<QPointF> intersects;
    for(auto edge : qAsConst(edges))
    {
        // To check all intersections of this edge with edges in
        // the current map, we can use the optimization that
        // edges are stored in sorted order by min x.  So at the
        // very least, skip the suffix of edges whose minimum x values
        // are past the max x of the edge to check.
        // Casper 12DEC02 - removed optimisation and simplified code

        QLineF e = edge->getLine();
        for (auto cur : qAsConst(edges))
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
        //qTDebug() << "New split at" << pt;
        insertVertex(pt);
    }

    qDebug().noquote() << summary();
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
    if (!status.neighboursBuilt)
    {
        buildNeighbours();
    }
    for (auto& vp : vertices)
    {
        NeighboursPtr n = getBuiltNeighbours(vp);
        int count = n->numNeighbours();
        if (count == 2)
        {
            QLineF a = n->getNeighbour(0)->getLine();
            QLineF b = n->getNeighbour(1)->getLine();
            qreal angle = Utils::angle(a,b);
            if (Loose::zero(angle) || Loose::equals(angle,180.0))
            {
                // need to remove one edge, extend the other, and remove vertex
                combineLinearEdges(n->getNeighbour(0),n->getNeighbour(1),vp);
                return true;
            }
        }
    }
    return false;
}

// combine two edges which are in a straight line with common vertex
void Map::combineLinearEdges(EdgePtr a, EdgePtr b,VertexPtr common)
{
    VertexPtr newV1 = a->getOtherV(common);
    VertexPtr newV2 = b->getOtherV(common);

    insertEdge(newV1,newV2);
    removeEdge(a);
    removeEdge(b);

    status.neighboursBuilt = false;
}

void Map::deDuplicateNeighbours()
{
    qDebug() << "deDuplicateNeighbours BEGIN edges=" << edges.size()  << "vertices=" << vertices.size();

    if (!status.neighboursBuilt)
    {
        buildNeighbours();
    }

    for (auto & v :  vertices)
    {
        // examining a vertex
        NeighboursPtr n = getBuiltNeighbours(v);
        deDuplicateEdges(n);
    }
    qDebug() << "deDuplicateNeighbours END   edges=" << edges.size()  << "vertices=" << vertices.size();
}

void Map::deDuplicateEdges(const NeighboursPtr vec)
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

void Map::removeVerticesWithEdgeCount(int edgeCount)
{
    qDebug() << "removeVerticesWithEdgeCount" << edgeCount << "......";
    if (!status.neighboursBuilt)
    {
        buildNeighbours();
    }

    QVector<VertexPtr> verts;
    for (auto & v : vertices)
    {
        NeighboursPtr n = getBuiltNeighbours(v);
        if (n->numNeighbours() == edgeCount)
        {
            verts.push_back(v);
        }
    }

    for (auto & v : verts)
    {
        removeVertex(v);
    }

    if (verts.size())
        qInfo() << "removed" << verts.size() << "vertices with edge count" << edgeCount;
    else
        qDebug() << "removed 0 vertices with edge count" << edgeCount;
}
