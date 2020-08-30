#ifndef NEIGHBOURS_H
#define NEIGHBOURS_H

#include <QtCore>
#include "base/shared.h"
#include "geometry/edge.h"
#include "geometry/vertex.h"

struct BeforeAndAfter
{
    EdgePtr before;
    EdgePtr after;
};

// DAC: OMG  in taprats this was a forward linked list of neigbour classes
// IMHO this added complexity but no value and is replaced by a vector of edges
// The vector needs to be sorted by angle
class Neighbours
{
public:
    Neighbours();
    Neighbours(VertexPtr vp);
    Neighbours(const Neighbours & other);
    ~Neighbours();

    void      setVertex(VertexPtr vp) { v = vp; }
    VertexPtr getVertex() const { return v; }

    int numEdges()       { return list.size(); }
    int numNeighbours()  { return list.size(); }

    bool contains(EdgePtr e) { return  list.contains(e); }
    bool connectsTo(VertexPtr other);
    bool isNear(VertexPtr other);

    void insertNeighbour(EdgePtr np);
    void insertEdge(EdgePtr edge);
    void insertEdgeSimple(EdgePtr edge);
    void removeEdge(EdgePtr edge );
    void swapEdge(VertexPtr other, EdgePtr nedge);
    void swapEdge2(EdgePtr old_edge, EdgePtr new_edge);

    bool verify();
    void cleanse();
    void sortEdgesByAngle();
    void dumpNeighbours() const;

    BeforeAndAfter   getBeforeAndAfter(EdgePtr edge);
    EdgePtr          getNeighbour(const VertexPtr other);
    EdgePtr          getEdge(int index) { return list[index]; }
    EdgePtr          getFirstNonvisitedNeighbour(EdgePtr home);
    QVector<EdgePtr> & getNeighbours();

private:
    VertexPtr        v;
    QVector<EdgePtr> list;   // this is the list neighbours for the vertex
};

class NeighbourMap
{
    friend class MosaicWriter;
    friend class MosaicLoader;
    friend class Map;

public:
    NeighbourMap(Map * parentMap);
    ~ NeighbourMap();

    bool contains(VertexPtr v) { return neighbours.contains(v); }
    NeighboursPtr getNeighbours(VertexPtr v);

    void clear();

    void insertVertex(VertexPtr v);
    void removeVertex(VertexPtr v);
    int  numNeighbours(VertexPtr v);

    void insertNeighbour(VertexPtr v, EdgePtr e);
    void removeNeighbour(VertexPtr v, EdgePtr e);
    void replaceNeighbour(VertexPtr v, EdgePtr existingEdge, EdgePtr newEdge);

    int  size() { return neighbours.size(); }
    int  countNeighbouringEdges();

    void sortByAngle();
    bool verify();
    void cleanse();
    void dump();

protected:
    QMap<VertexPtr,NeighboursPtr> & get() { return neighbours; }
private:
    QMap<VertexPtr,NeighboursPtr> neighbours;
    Map * parent;
};

#endif // NEIGHBOURS_H
