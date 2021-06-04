#include "geometry/neighbours.h"
#include "base/utilities.h"
#include "geometry/map.h"

#if 0
Neighbours::Neighbours()
{
    _built = false;
}
#endif

Neighbours::Neighbours(VertexPtr vp)
{
    parent = vp;
    _built = false;
}

Neighbours::Neighbours(const Neighbours & other)
{
    parent = other.parent;
    _built = false;
}

Neighbours::~Neighbours()
{
    //clear();
}

bool Neighbours::contains(EdgePtr e) const
{
   for (auto pos = begin(); pos != end(); pos++)
   {
       WeakEdgePtr wep = *pos;
       if (e == wep.lock())
       {
           return true;
       }
   }
   return false;
}

BeforeAndAfter Neighbours::getBeforeAndAfter(EdgePtr edge )
{
    //qDebug() << "getBeforeAndAfter - edge:" << edge.get() << "count:" << numNeighbours();

    BeforeAndAfter ret;

    int sz = (int)size();

    for (int i=0; i < sz; i++)
    {
        if (at(i).lock() == edge)
        {
            if (i == 0)
            {
                ret.before = back().lock();
            }
            else
            {
                ret.before = at(i-1).lock();
            }

            if (i == sz-1)
            {
                ret.after = front().lock();
            }
            else
            {
                ret.after  = at(i+1).lock();
            }
            return ret;
        }
    }
    qCritical("getBeforeAndAfter - should not reach here");
    return ret;
}

/*
 * Insert the edge into the vertex's neighbour list.  Edges
 * are stored in sorted order by angle (though the starting point
 * isn't important).  So traverse the list until there's a spot
 * where the arc swept by consecutive edges contains the new edge.
 */

void Neighbours::insertNeighbour(EdgePtr e)
{
    //qDebug() << "insert neighbour: vertex=" << Utils::addr(this) << "edge=" << Utils::addr(e.get());

    if (contains(e))
    {
        qWarning("inserting existing Neighbour");
        return;
    }

    if (size() == 0)
    {
        push_back(e);
        return;
    }

    qreal a = parent.lock()->getAngle(e);

    //for (int i=0; i < (int)neighbours.size(); i++)
    for (auto pos = begin(); pos != end(); pos++)
    {
        WeakEdgePtr wep = *pos;
        EdgePtr ep = wep.lock();
        if (!ep)
        {
            qWarning() << "bad neighbour 1";
            continue;
        }
        qreal b = parent.lock()->getAngle(wep.lock());
        if (a > b)
        {
            insert(pos,e);
            return;
        }
    }

    push_back(e);
}

struct cmpangle
{
    cmpangle(VertexPtr parent) { this->parent = parent; }

    bool operator () (WeakEdgePtr ep1, WeakEdgePtr ep2)
    {
        EdgePtr edge1 = ep1.lock();
        qreal    a = parent->getAngle(edge1);
        EdgePtr  edge2 = ep2.lock();
        qreal      b = parent->getAngle(edge2);
        return (a > b);
    }

    VertexPtr parent;
};

void Neighbours::sortNeighboursByAngle()
{
    if (numNeighbours() < 2)
    {
        return;     // returns for 0 or 1
    }

    std::sort(begin(),end(),cmpangle(parent.lock()));
}

void Neighbours::dumpNeighbours() const
{
    qDebug() << "vertex: " << parent.lock()->pt << "num neighbour:" << size();
    for (auto & edge : *this)
    {
        QPointF v1 = edge.lock()->v1->pt;
        QPointF v2 = edge.lock()->v2->pt;
        qDebug() << "      edge: " << edge.lock().get() << " " << v1 << " " << v2;
    }
}

//Removing is always easier than adding.  Just splice the edge out of the list.
void Neighbours::removeNeighbour(EdgePtr edge )
{
    const auto pos = std::find_if(begin(), end(), [&edge](const WeakEdgePtr& ptr1) {
        return ptr1.lock() == edge;
    });

    if (pos != end())
    {
        erase(pos);
    }
}

// When an edge is split in the map, this vertex's adjacency list
// needs to be updated -- some neighbour will now have a new
// edge instance to represent the new half of an edge that was
// created.  Do a hacky rewrite here.
// casper - another hacky rewrite on top of the other
// TODO - maybe resort now?

void Neighbours::replaceNeighbour(EdgePtr old_edge, EdgePtr new_edge)
{
    Q_ASSERT(new_edge);

    // replace
    for (auto it = begin(); it != end(); it++)
    {
        WeakEdgePtr wedge = *it;
        if  (wedge.lock() == old_edge)
        {
            *it = new_edge;        // this replaces edge
            break;
        }
    }

    // remove dead neighbours
    std::vector<WeakEdgePtr> new_neighbours;
    for (auto it = begin(); it != end(); it++)
    {
        WeakEdgePtr wedge = *it;
        if  (wedge.lock())
        {
            new_neighbours.push_back(wedge);
        }
        else
            qDebug() <<  "removed wedge";
    }

    std::vector<WeakEdgePtr> * neighbours = dynamic_cast<std::vector<WeakEdgePtr> *>(this);
    *neighbours = new_neighbours;

    // resort
    sortNeighboursByAngle();
}

EdgePtr Neighbours::getFirstNonvisitedNeighbour(EdgePtr home)
{
    QVector<qreal> angles;
    qreal hangle = home->getAngle();
    angles.push_back(hangle);
    WeakEdgePtr ep;
    for (auto & edge : *this)
    {
        if (edge.lock()->visited)
        {
            continue;
        }
        angles.push_back(edge.lock()->getAngle());
        if (!ep.lock())
            ep = edge;
    }
    //qDebug() << "Angles:" << angles;
    return ep.lock();
}

bool Neighbours::verify()
{
    bool rv = true;
    for (auto & e : *this)
    {
        if (parent.lock() != e.lock()->v1  && parent.lock() != e.lock()->v2)
        {
            qWarning() << "Neighbours: vertex:" << parent.lock()->pt << "has invalid edge:" << e.lock()->v1->pt  << e.lock()->v2->pt;
            rv = false;
        }
    }
    return rv;
}

void Neighbours::cleanse()
{
    std::vector<WeakEdgePtr> baddies;

    for (auto & e : *this)
    {
        if (parent.lock() != e.lock()->v1  && parent.lock() != e.lock()->v2)
        {
            qDebug() << "Neighbours: cleaning invalid edge";
            baddies.push_back(e);
        }
    }

    for (auto & e2 : baddies)
    {
        EdgePtr ee2 = e2.lock();
        for (auto pos = begin(); pos != end(); pos++)
        {
            WeakEdgePtr wep = *pos;
            if (ee2 == wep.lock())
            {
                erase(pos);
                break;
            }
        }
    }
}

EdgePtr Neighbours::getNeighbour(int index)
{
    WeakEdgePtr wep = (*this)[index];
    return wep.lock();
}
