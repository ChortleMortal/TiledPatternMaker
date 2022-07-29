#ifndef NEIGHBOURS_H
#define NEIGHBOURS_H

#include <memory>
#include <QVector>
#include <QMap>

typedef std::shared_ptr<class Edge>         EdgePtr;
typedef std::weak_ptr<class Edge>           WeakEdgePtr;
typedef std::shared_ptr<class Vertex>       VertexPtr;
typedef std::weak_ptr<class Vertex>         WeakVertexPtr;
typedef std::shared_ptr<class Neighbours>   NeighboursPtr;

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

    void insertNeighbour(const EdgePtr &np);

    int numNeighbours()  { return (int)size(); }

    BeforeAndAfter getBeforeAndAfter(const EdgePtr & edge);
    EdgePtr        getNeighbour(int index);
    EdgePtr        getFirstNonvisitedNeighbour(const EdgePtr & home);

    bool verify();
    void cleanse();
    void dumpNeighbours() const;

protected:
    bool contains(const EdgePtr & e) const;

private:
    WeakVertexPtr vertex;
};

#endif // NEIGHBOURS_H
