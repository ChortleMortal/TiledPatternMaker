#pragma once

#include "sys/geometry/edge.h"
#include "sys/geometry/vertex.h"
#include "sys/geometry/neighbours.h"

typedef std::shared_ptr<class NeighbourMap> NeighbourMapPtr;
typedef std::shared_ptr<class Neighbours>   NeighboursPtr;

class MapBase
{
    friend class MapCleanser;
    friend class MapVerifier;

public:
    MapBase() {}

    void paint(QPainter * painter, QTransform & tr, bool shoWDirn, bool showArcCenters, bool showVertices , bool showEdges);

    // getters
    const QVector<VertexPtr>  & getVertices()   { return vertices; }
    const EdgeSet             & getEdges()      { return edges; }

    bool isEmpty() const;

    virtual void    wipeout();
    virtual QString info() const;

    int vertexIndex(const VertexPtr & v) const { return vertices.indexOf(v); }
    int edgeIndex(const EdgePtr & e)     const { return edges.indexOf(e); }

protected:
    QVector<VertexPtr> vertices;
    QVector<EdgePtr>   edges;

};
