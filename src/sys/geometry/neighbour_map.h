#pragma once
#ifndef NEIGHBOURMAP_H
#define NEIGHBOURMAP_H

#include <QVector>
#include <QMap>

#include "sys/geometry/map.h"

typedef std::shared_ptr<class Edge>         EdgePtr;
typedef QVector<EdgePtr>                    EdgeSet;
typedef std::shared_ptr<class Vertex>       VertexPtr;
typedef std::shared_ptr<class Neighbours>   NeighboursPtr;

class NeighbourMap
{
public:
    NeighbourMap(MapBase * map);
    NeighbourMap(MapPtr map);

    NeighboursPtr getNeighbours(const VertexPtr & v);
    void          dumpNeighbours(const VertexPtr & v, const EdgePtr edge);

    uint          rawSize();
    void          examine();

protected:
    NeighboursPtr  getRawNeighbours(const VertexPtr & v);

private:
    QMap<VertexPtr,NeighboursPtr>   nmap;
    MapBase *                       map;
};

#endif // NEIGHBOURMAP_H
