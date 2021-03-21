#ifndef NEIGHBOURS_H
#define NEIGHBOURS_H

#include <QtCore>
#include "base/shared.h"
#include "base/misc.h"

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
    Neighbours(Vertex * vp);
    Neighbours(const Neighbours & other);
    ~Neighbours();

    int numNeighbours()  { return neighbours.size(); }

    void sortNeighboursByAngle();
    void insertNeighbour(EdgePtr np);
    void removeNeighbour(EdgePtr edge );
    bool contains(EdgePtr e) { return  neighbours.contains(e); }
    void replaceNeighbour(EdgePtr old_edge, EdgePtr new_edge);
    void eraseNeighbours() { neighbours.clear(); }

    BeforeAndAfter getBeforeAndAfter(EdgePtr edge);
    EdgePtr        getNeighbour(int index) { return neighbours[index]; }
    EdgePtr        getFirstNonvisitedNeighbour(EdgePtr home);

    const QVector<EdgePtr> & getNeighbours() { return neighbours; }

    bool verify();
    void cleanse();
    void dumpNeighbours() const;

private:
    Vertex *               parent;
    UniqueQVector<EdgePtr> neighbours;
};

#endif // NEIGHBOURS_H
