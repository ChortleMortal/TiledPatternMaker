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
class Neighbours : public std::vector<WeakEdgePtr>
{
public:
    //Neighbours();
    Neighbours(VertexPtr vp);
    Neighbours(const Neighbours & other);
    ~Neighbours();

    void setParent(VertexPtr v) { parent = v; }

    int numNeighbours()  { return (int)size(); }

    void sortNeighboursByAngle();
    void insertNeighbour(EdgePtr np);
    void removeNeighbour(EdgePtr edge );
    bool contains(EdgePtr e) const;
    void replaceNeighbour(EdgePtr old_edge, EdgePtr new_edge);
    void eraseNeighbours() { clear(); }

    BeforeAndAfter getBeforeAndAfter(EdgePtr edge);
    EdgePtr        getNeighbour(int index);
    EdgePtr        getFirstNonvisitedNeighbour(EdgePtr home);

    bool verify();
    void cleanse();
    void dumpNeighbours() const;

    bool _built;

private:
    WeakVertexPtr parent;
};

#endif // NEIGHBOURS_H
