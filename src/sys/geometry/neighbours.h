#pragma once
#ifndef NEIGHBOURS_H
#define NEIGHBOURS_H

#include <QMap>
#include <QVector>

typedef std::shared_ptr<class Edge>         EdgePtr;
typedef std::weak_ptr<class Edge>           WeakEdgePtr;
typedef std::shared_ptr<class Vertex>       VertexPtr;
typedef std::weak_ptr<class Vertex>         WeakVertexPtr;
typedef std::shared_ptr<class Neighbours>   NeighboursPtr;
typedef std::weak_ptr<class Neighbours>     WeakNeighboursPtr;
typedef std::shared_ptr<class Map>          MapPtr;

struct BeforeAndAfter
{
    EdgePtr before;
    EdgePtr after;
};

// DAC: OMG  in taprats this was a forward linked list of neigbour classes
// IMHO this added complexity but no value and is replaced by a vector of edges
// The vector needs to be sorted by angle
class Neighbours : public QVector<WeakEdgePtr>
{
public:
    Neighbours(const VertexPtr & vp);
    Neighbours(const Neighbours & other);
    ~Neighbours();

    void            insertNeighbour(const EdgePtr &np);

    uint            numNeighbours()  { return (uint)size(); }

    BeforeAndAfter  getBeforeAndAfter(const EdgePtr & edge);
    EdgePtr         nearest(const EdgePtr & edge, bool side1);
    EdgePtr         getContinuation(const EdgePtr edge, int type);

    EdgePtr         getNeighbour(int index);
    VertexPtr       getVertex() { return vertex.lock();}

    bool            verify();
    void            cleanse();
    QString         info(MapPtr &  map);
    QString         casingInfo();

    static int      refs;

protected:
    bool contains(const EdgePtr & e) const;

    WeakVertexPtr   vertex;
};

#endif // NEIGHBOURS_H
