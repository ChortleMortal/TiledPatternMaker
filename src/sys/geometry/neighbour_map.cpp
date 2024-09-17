#include "sys/geometry/neighbour_map.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/neighbours.h"

NeighbourMap::NeighbourMap(const QVector<EdgePtr> & edges)
{
    for (const auto & edge : std::as_const(edges))
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
