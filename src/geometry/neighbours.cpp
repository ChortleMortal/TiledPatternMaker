#include "geometry/neighbours.h"
#include "base/utilities.h"
#include "geometry/map.h"

Neighbours::Neighbours()
{
    parent = nullptr;
}

Neighbours::Neighbours(Vertex * vp)
{
    parent = vp;
}

Neighbours::Neighbours(const Neighbours & other)
{
    parent      = other.parent;
    neighbours  = other.neighbours;
}

Neighbours::~Neighbours()
{
    //clear();
}

// though the original taprats version of this method seemed deceptively simple
// it has been hard to discern how it was really meant to work
// for the corener case of when a vertex has 0, 1, or 2, neighbours
// so this code, while ugly, is a slavish attempt to emulate the original
BeforeAndAfter Neighbours::getBeforeAndAfter(EdgePtr edge )
{
    if (numNeighbours() < 2)
    {
        qDebug() << "getBeforeAndAfter - edge:" << edge.get() << "count:" << numNeighbours();
    }

    BeforeAndAfter ret;

    QVector<EdgePtr> list = neighbours;     // local copy
    list.push_back(list.at(0));
    list.push_back(list.at(1));

    for (int i=1; i < (list.size()-1); i++)
    {
        if (list.at(i) == edge)
        {
            ret.before = list.at(i-1);
            ret.after  = list.at(i+1);
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

    if (neighbours.contains(e))
    {
        qWarning("inserting existing Neighbour");
        return;
    }

    if (neighbours.size() == 0)
    {
        neighbours.push_back(e);
        return;
    }

    qreal a = parent->getAngle(e);

    for (int i=0; i < neighbours.size(); i++)
    {
        EdgePtr ep = neighbours.at(i);
        qreal    b = parent->getAngle(ep);
        if (a > b)
        {
            neighbours.insert(i,e);
            return;
        }
    }

    neighbours.push_back(e);
}

void Neighbours::sortNeighboursByAngle()
{
    if (numNeighbours() < 2)
    {
        return;     // returns for 0 or 1
    }

    UniqueQVector<EdgePtr> vec;

    for (const auto & edge : qAsConst(neighbours))
    {
        qreal    a = parent->getAngle(edge);
        bool found = false;

        for (int j = 0; j < vec.size(); j ++)
        {
            EdgePtr evec = vec[j];
            qreal      b = parent->getAngle(evec);
            if (a > b)
            {
                vec.insert(j,edge);
                found = true;
                break;
            }
        }
        if (!found)
        {
            vec.push_back(edge);
        }
    }

    neighbours = vec;    // overwrites
}

void Neighbours::dumpNeighbours() const
{
    qDebug() << "vertex: " << parent->pt << "neighbours:" << neighbours.size();
    for (auto edge : neighbours)
    {
        QPointF v1 = edge->v1->pt;
        QPointF v2 = edge->v2->pt;
        qDebug() << "      edge: " << edge.get() << " " << v1 << " " << v2;
    }
}

//Removing is always easier than adding.  Just splice the edge out of the list.
void Neighbours::removeNeighbour(EdgePtr edge )
{
    neighbours.removeAll(edge);
}

// When an edge is split in the map, this vertex's adjacency list
// needs to be updated -- some neighbour will now have a new
// edge instance to represent the new half of an edge that was
// created.  Do a hacky rewrite here.
// casper - another hacky rewrite on top of the other
// TODO - maybe resort now?

void Neighbours::replaceNeighbour(EdgePtr old_edge, EdgePtr new_edge)
{
    for (auto it = neighbours.begin(); it != neighbours.end(); it++)
    {
        EdgePtr edge = *it;
        if  (edge == old_edge)
        {
            *it = new_edge;        // this replaces edge
            break;
        }
    }
    sortNeighboursByAngle();
}

EdgePtr Neighbours::getFirstNonvisitedNeighbour(EdgePtr home)
{
    QVector<qreal> angles;
    qreal hangle = home->getAngle();
    angles.push_back(hangle);
    EdgePtr ep;
    for (auto edge : qAsConst(neighbours))
    {
        if (edge->visited)
        {
            continue;
        }
        angles.push_back(edge->getAngle());
        if (!ep)
            ep = edge;
    }
    //qDebug() << "Angles:" << angles;
    return ep;
}

bool Neighbours::verify()
{
    bool rv = true;
    for (auto e : qAsConst(neighbours))
    {
        if (parent != e->v1.get()  && parent != e->v2.get())
        {
            qWarning() << "Neighbours: vertex:" << parent->pt << "has invalid edge:" << e->v1->pt  << e->v2->pt;
            rv = false;
        }
    }
    return rv;
}

void Neighbours::cleanse()
{
    QVector<EdgePtr> baddies;
    for (auto e : qAsConst(neighbours))
    {
        if (parent != e->v1.get()  && parent != e->v2.get())
        {
            qDebug() << "Neighbours: cleaning invalid edge";
            baddies.push_back(e);
        }
    }
    for (const auto & e : baddies)
    {
        neighbours.removeAll(e);
    }
}
