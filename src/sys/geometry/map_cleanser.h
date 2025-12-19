#ifndef MAP_CLEANSER_H
#define MAP_CLEANSER_H

#include <QtTypes>
#include "sys/geometry/map.h"

enum eMCOptions
{
    badVertices_0               = 0x01,
    badVertices_1               = 0x02,
    coalesceEdges               = 0x04,
    coalescePoints              = 0x08,
    joinupColinearEdges         = 0x10,
    divideupIntersectingEdges   = 0x20,
    deprecated1                 = 0x40,
    deprecated2                 = 0x80,
    cleanupEdges                = 0x100
};

#define default_cleanse (coalescePoints | coalesceEdges  | badVertices_0| cleanupNeighbours)

class MapCleanser
{
public:
    MapCleanser(MapPtr map) { this->map = map.get(); }
    MapCleanser(Map *  map) { this->map = map; }

    void  cleanse(uint options, qreal sensitivity);
    uint  analyze(qreal sensitivity);

    void cleanseVertices();
    void deDuplicateEdgesUsingNeighbours(bool silent = false);
    void deDuplicateVertices(qreal tolerance);

protected:
    void removeBadEdges();
    void divideIntersectingEdges();
    void joinColinearEdges();
    bool coalesceVertices(qreal tolerance);
    void combineLinearEdges(const EdgePtr & a, const EdgePtr & b, const VertexPtr & common);
    void deDuplicateEdges(const NeighboursPtr & vec);
    void removeVerticesWithEdgeCount(uint edgeCount);

private:
    bool joinOneColinearEdge(NeighbourMap & nmap);

    Map * map;
};

#endif // MAP_CLEANSER_H
