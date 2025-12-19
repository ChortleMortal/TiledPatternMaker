#ifndef MAP_VERIFIER_H
#define MAP_VERIFIER_H

#include <QtTypes>
#include "sys/geometry/map.h"

enum eMapError
{
    MAP_EMPTY,
    MAP_NO_EDGES,
    MAP_NO_VERTICES,

    EDGE_TRIVIAL_VERTICES,
    EDGE_TRIVIAL_POINTS,
    EDGE_VERTEX_UNKNOWN,
    EDGE_DUPLICATED,

    VERTEX_DUPLICATED,

    NEIGHBOUR_NO_EDGE,
    NEIGHBOUR_INVALID_EDGE,
    NEIGHBOUR_EXPIRED,
    NEIGHBOUR_BAD,

    MERR_EDGES_INTERSECTING
};

class MapVerifier
{
public:
    MapVerifier(MapPtr map) { this->map = map.get(); }
    MapVerifier(Map *  map) { this->map = map; }

    bool        verify(bool force = false);
    bool        verifyAndFix(bool force = false, bool confirm = false);

    void        verifyEdges();
    void        verifyNeighbours(NeighbourMap *nMap);
    bool        procErrors(const QVector<eMapError> &errors);

protected:

private:
    QVector<eMapError> _verify();
    void        dumpErrors(const QVector<eMapError> &theErrors);

    Map * map;
    UniqueQVector<eMapError>    errors;

};

#endif // MAP_VERIFIER_H
