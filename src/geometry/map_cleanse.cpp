#include "geometry/map.h"
#include <QDebug>

#include "geometry/edge.h"
#include "geometry/vertex.h"
#include "geometry/neighbours.h"
#include "geometry/intersect.h"
#include "geometry/loose.h"
#include "misc/utilities.h"

// cleanse just cleanses - it does not verify
void Map::cleanse(unsigned int options)
{
    qDebug() << "cleanse......";
    const bool debug = false;

    qDebug() << "Map::cleanse - start";

    if (options & joinupColinearEdges)
    {
        joinColinearEdges();
    }

    if (options & divideupIntersectingEdges)
    {
        divideIntersectingEdges();
        deDuplicateNeighbours();
        //sortVertices();
        //sortEdges();
        nMap.reset();
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

    if (options & badEdges)
    {
        removeBadEdges();
    }

    if (options & BuildNeighbours)
    {
        nMap.reset();
    }

    if (options & cleanupNeighbours)
    {
        deDuplicateNeighbours();
    }

    if (debug)
    {
        nMap = std::make_shared<NeighbourMap>(edges);
        verify();
    }
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
    for(const auto & edge : qAsConst(edges))
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
    for (const auto & vp : qAsConst(vertices))
    {
        NeighboursPtr n = getNeighbours(vp);
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
    qDebug().noquote() << "coalesceVertices-start" <<  summary2() << "Tolerance = " << tolerance;
    qsizetype start = vertices.size();

    QMap<VertexPtr,VertexPtr> replacements;
    qsizetype size = vertices.size();
    for (int i = 0; i < size; i++)
    {
        for (int j = i+1 ;  j < size; j++)
        {
            if (Point::dist2(vertices[i]->pt,vertices[j]->pt) < tolerance)
            {
                replacements[vertices[i]] = vertices[j];
            }
        }
    }
    qDebug() << "Replacements =" << replacements.size();
    if (replacements.size() == 0)
    {
        qDebug().noquote() << "coalesceVertices-end  " <<  summary2();
        return false;
    }

    UniqueQVector<EdgePtr> changedEdges;
    for (const auto & edge : qAsConst(edges))
    {
        QMap<VertexPtr,VertexPtr>::iterator it1;
        QMap<VertexPtr,VertexPtr>::iterator it2;
        it1 = replacements.find(edge->v1);
        if (it1 != replacements.end())
        {
            auto oldv = edge->v1;
            edge->v1 = *it1;
            changedEdges.push_back(edge);
            vertices.removeOne(oldv);
        }
        it2 = replacements.find(edge->v2);
        if (it2 != replacements.end())
        {
            auto oldv = edge->v2;
            edge->v2 = *it2;
            changedEdges.push_back(edge);
            vertices.removeOne(oldv);
        }
    }

    QVector<EdgePtr> edgesToDelete;
    for (const auto & edge : qAsConst(changedEdges))
    {
        if (edge->isTrivial(tolerance))
        {
            //qDebug() << "deleting trivial edge";
            edgesToDelete.push_back(edge);
            continue;
        }
        for (auto & existingEdge : edges)
        {
            if (existingEdge == edge)
            {
                //qDebug() << "SKIPPED: changed" << edge->v1->pt  << edge->v2->pt << "existing" << existingEdge->v1->pt << existingEdge->v2->pt;
                continue;
            }
            //qDebug() << "changed" << edge->v1->pt  << edge->v2->pt << "existing" << existingEdge->v1->pt << existingEdge->v2->pt;
            if (edge->sameAs(existingEdge))
            {
                //qDebug() << "deleting duplicate edge";
                edgesToDelete.push_back(edge);
            }
        }
    }

    for (auto & edge : edgesToDelete)
    {
        edges.removeOne(edge);
    }

    nMap.reset();
    _cleanseVertices();

    qDebug().noquote() << "coalesceVertices-end  " <<  summary2();

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

void Map::deDuplicateNeighbours()
{
    qDebug() << "deDuplicateNeighbours BEGIN" << "vertices =" << vertices.size() << "edges =" << edges.size();

    for (const auto & v :  qAsConst(vertices))
    {
        // examining a vertex
        NeighboursPtr n = getNeighbours(v);
        deDuplicateEdges(n);
    }
    qDebug() << "deDuplicateNeighbours END  "  << "vertices =" << vertices.size() << "edges =" << edges.size();
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

void Map::removeVerticesWithEdgeCount(int edgeCount)
{
    qDebug() << "removeVerticesWithEdgeCount" << edgeCount << "......";

    QVector<VertexPtr> verts;
    for (const auto & v : qAsConst(vertices))
    {
        NeighboursPtr n = getNeighbours(v);
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
