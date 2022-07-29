#include <QDebug>

#include "geometry/neighbours.h"
#include "geometry/map.h"
#include "geometry/vertex.h"
#include "geometry/edge.h"

Neighbours::Neighbours(const VertexPtr & vp)
{
    vertex = vp;
}

Neighbours::Neighbours(const Neighbours & other) : QVector<WeakEdgePtr>(other)
{
    vertex = other.vertex;
}

Neighbours::~Neighbours()
{
}

bool Neighbours::contains(const EdgePtr & e) const
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

BeforeAndAfter Neighbours::getBeforeAndAfter(const EdgePtr &edge )
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

void Neighbours::insertNeighbour(const EdgePtr & e)
{
    //qDebug() << "insert neighbour: vertex=" << Utils::addr(this) << "edge=" << Utils::addr(e.get());

    if (contains(e))
    {
        //qWarning("inserting existing Neighbour");
        return;
    }

    if (size() == 0)
    {
        push_back(e);
        return;
    }

    qreal a = vertex.lock()->getAngle(e);

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
        qreal b = vertex.lock()->getAngle(wep.lock());
        if (a > b)
        {
            insert(pos,e);
            return;
        }
    }

    push_back(e);
}

void Neighbours::dumpNeighbours() const
{
    qDebug() << "vertex: " << vertex.lock()->pt << "num neighbour:" << size();
    for (auto & wedge : *this)
    {
        auto edge = wedge.lock();
        if (edge)
        {
            QPointF v1 = edge->v1->pt;
            QPointF v2 = edge->v2->pt;
            qDebug() << "      edge: " << edge.get() << " " << v1 << " " << v2;
        }
        else
        {
            qWarning("WEAK EDGE DOES NOT LOCK in dumpNeighbours");
        }
    }
}

EdgePtr Neighbours::getFirstNonvisitedNeighbour(const EdgePtr & home)
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
        if (vertex.lock() != e.lock()->v1 && vertex.lock() != e.lock()->v2)
        {
            qWarning() << "Neighbours: vertex:" << vertex.lock()->pt << "has invalid edge:" << e.lock()->v1->pt  << e.lock()->v2->pt;
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
        if (vertex.lock() != e.lock()->v1  && vertex.lock() != e.lock()->v2)
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


