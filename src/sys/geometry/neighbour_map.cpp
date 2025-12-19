#include "sys/geometry/edge.h"
#include "sys/geometry/map_base.h"
#include "sys/geometry/neighbour_map.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/map_base.h"
#include "sys/geometry/neighbours.h"
#include "sys/geometry/vertex.h"

NeighbourMap::NeighbourMap(MapPtr map)
{
    this->map = map.get();

    for (const auto & edge : std::as_const(map->getEdges()))
    {
        NeighboursPtr n = getRawNeighbours(edge->v1);
        n->insertNeighbour(edge);
        NeighboursPtr n2 = getRawNeighbours(edge->v2);
        n2->insertNeighbour(edge);
    }
}

NeighbourMap::NeighbourMap(MapBase *map)
{
    this->map = map;

    for (const auto & edge : std::as_const(map->getEdges()))
    {
        NeighboursPtr n = getRawNeighbours(edge->v1);
        n->insertNeighbour(edge);
        NeighboursPtr n2 = getRawNeighbours(edge->v2);
        n2->insertNeighbour(edge);
    }
}
NeighboursPtr  NeighbourMap::getNeighbours(const VertexPtr & v)
{
    return getRawNeighbours(v);
}

NeighboursPtr  NeighbourMap::getRawNeighbours(const VertexPtr & v)
{
    NeighboursPtr np = nmap.value(v);
    if (!np)
    {
        np = std::make_shared<Neighbours>(v);
        nmap.insert(v,np);
    }
    return np;
}

uint NeighbourMap::rawSize()
{
    uint ncount = 0;

    QMapIterator<VertexPtr, NeighboursPtr> i(nmap);
    while (i.hasNext())
    {
        i.next();
        auto neighbours = i.value();
        ncount += neighbours->numNeighbours();
    }
    return ncount;
}

void NeighbourMap::examine()
{
    QMapIterator<VertexPtr, NeighboursPtr> i(nmap);
    while (i.hasNext())
    {
        i.next();
        auto neighbours = i.value();
        uint ncount = neighbours->numNeighbours();
        if (ncount > 4)
            qDebug() << "LOG2" << ncount;
    }
}

void NeighbourMap::dumpNeighbours(const VertexPtr & v, const EdgePtr edge)
{
    NeighboursPtr np = getNeighbours(v);
    qDebug() << "vertex: " << v->pt << "num neighbours:" << np->size() << "angle" << edge->getLine().angle();    

    for (auto & wedge : *std::as_const(np))
    {
        auto edge = wedge.lock();
        if (edge)
        {
            QPointF v1  = edge->v1->pt;
            QPointF v2  = edge->v2->pt;
            qreal angle = edge->getLine().angle();
            qDebug() << "      edge: " << map->edgeIndex(edge) << " " << v1 << " " << v2 << " " << angle;
        }
        else
        {
            qWarning("WEAK EDGE DOES NOT LOCK in dumpNeighbours");
        }
    }
}
