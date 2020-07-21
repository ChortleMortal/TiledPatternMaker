#include "neighbours.h"
#include "base/utilities.h"
#include "geometry/map.h"

Neighbours::Neighbours()
{
    qWarning() << "why am I here";
}

Neighbours::Neighbours(VertexPtr vp)
{
    v = vp;
}

Neighbours::Neighbours(const Neighbours & other)
{
    v    = other.v;
    list = other.list;
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
    if (numEdges() < 2)
    {
        qDebug() << "getBeforeAndAfter - edge=" << Utils::addr(edge.get()) << "count=" << numEdges();
    }

    BeforeAndAfter ret;

    list.push_back(list.at(0));      // temp addition
    list.push_back(list.at(1));      // temp addition

    for (int i=1; i < (list.size()-1); i++)
    {
        if (list.at(i) == edge)
        {
            ret.before = list.at(i-1);
            ret.after  = list.at(i+1);
            list.removeLast();     //back out addition
            list.removeLast();     //back out addition
            return ret;
        }
    }
    qCritical("getBeforeAndAfter - should not reach here");
    return ret;
}

QVector<EdgePtr> & Neighbours::getNeighbours()
{
    return list;
}

EdgePtr Neighbours::getNeighbour(const VertexPtr other)
{
    for (auto edge : list)
    {
        VertexPtr vp = edge->getOtherV(v);
        if (vp == other)
        {
            //qDebug() << "        MATCH";
            return edge;
        }
        else if (!vp)
        {
            qWarning() << "MISSING VERTEX in getNeighbour";
        }
        //else  qDebug() << "        NO match";
    }

    qWarning("getNeighbour - no neighbouring edge");
    EdgePtr e;
    return e;
}

#if 0
bool Neighbours::connectsTo( VertexPtr other)
{
    return (getNeighbour(other) != nullptr);
}
#endif

bool Neighbours::isNear(VertexPtr other)
{
    return Point::isNear(v->getPosition(),other->getPosition());
}

void Neighbours::insertEdgeSimple(EdgePtr edge)
{
    list.push_back(edge);
    //qDebug() << "vertex:" << v->getPosition() << "adding edge" << Utils::addr(edge.get());
}

/*
 * Insert the edge into the vertex's neighbour list.  Edges
 * are stored in sorted order by angle (though the starting point
 * isn't important).  So traverse the list until there's a spot
 * where the arc swept by consecutive edges contains the new edge.
 */

void Neighbours::insertEdge(EdgePtr edge)
{
    insertNeighbour(edge);
}

void Neighbours::insertNeighbour(EdgePtr e)
{
    //qDebug() << "insert neighbour: vertex=" << Utils::addr(this) << "edge=" << Utils::addr(e.get());

    if (list.contains(e))
    {
        qWarning("inserting existing Neighbour");
        return;
    }

    if (list.size() == 0)
    {
        list.push_back(e);
        return;
    }

    qreal a = v->getAngle(e);

    for (int i=0; i < list.size(); i++)
    {
        EdgePtr ep = list.at(i);
        qreal    b = v->getAngle(ep);
        if (a > b)
        {
            list.insert(i,e);
            return;
        }
    }

    list.push_back(e);
}

void Neighbours::sortEdgesByAngle()
{
    if (numEdges() < 2)
    {
        return;     // returns for 0 or 1
    }

    QVector<EdgePtr> vec;

    for (auto edge : list)
    {
        qreal    a = v->getAngle(edge);
        bool found = false;

        for (int j = 0; j < vec.size(); j ++)
        {
            EdgePtr evec = vec[j];
            qreal      b = v->getAngle(evec);
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

    list = vec;    // overwrites
}

void Neighbours::dumpNeighbours() const
{
    qDebug() << "vertex: " << v->getTmpVertexIndex() << "neighbours:" << list.size();
    for (auto edge : list)
    {
#if 0
        QPointF v1 = edge->getV1()->getPosition();
        QPointF v2 = edge->getV2()->getPosition();
        qDebug() << "      edge: " << Utils::addr(edge.get()) << " " << v1 << " " << v2;
#else
        qDebug() << "      edge: " << edge->getTmpEdgeIndex() << "vertices:" << edge->getV1()->getTmpVertexIndex() << " " << edge->getV2()->getTmpVertexIndex();
#endif
    }
}

//Removing is always easier than adding.  Just splice the edge out of the list.
void Neighbours::removeEdge(EdgePtr edge )
{
    list.removeAll(edge);
}

// When an edge is split in the map, this vertex's adjacency list
// needs to be updated -- some neighbour will now have a new
// edge instance to represent the new half of an edge that was
// created.  Do a hacky rewrite here.
// casper - another hacky rewrite on top of the other
// TODO - maybe resort now?
void Neighbours::swapEdge(VertexPtr other, EdgePtr nedge)
{
    for (auto it = list.begin(); it != list.end(); it++)
    {
        EdgePtr edge = *it;
        VertexPtr vp = edge->getOtherV(v);
        if (vp == other)
        {
            *it = nedge;        // this replaces edge
            break;
        }
    }
}

void Neighbours::swapEdge2(EdgePtr old_edge, EdgePtr new_edge)
{
    for (auto it = list.begin(); it != list.end(); it++)
    {
        EdgePtr edge = *it;
        if  (edge == old_edge)
        {
            *it = new_edge;        // this replaces edge
            return;
        }
    }
}

EdgePtr Neighbours::getFirstNonvisitedNeighbour(EdgePtr home)
{
    QVector<qreal> angles;
    qreal hangle = home->getAngle();
    angles.push_back(hangle);
    EdgePtr ep;
    for (auto edge : list)
    {
        if (edge->getInterlaceInfo().visited)
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
    for (auto e : list)
    {
        if (v != e->getV1()  && v != e->getV2())
        {
            qWarning() << "Neighbours: vertex:" << v->getTmpVertexIndex() << "has invalid edge:" << e->getTmpEdgeIndex()
                     << "edge_vertices" << e->getV1()->getTmpVertexIndex()  << e->getV2()->getTmpVertexIndex();
            rv = false;
        }
    }
    return rv;
}

void Neighbours::cleanse()
{
    QVector<EdgePtr> baddies;
    for (auto e : list)
    {
        if (v != e->getV1()  && v != e->getV2())
        {
            qDebug() << "Neighbours: cleaning invalid edge";
            baddies.push_back(e);
        }
    }
    for (auto e : baddies)
    {
        list.removeAll(e);
    }
}

/////////////////////////////////////////////////////////////////////
//
//
//
/////////////////////////////////////////////////////////////////////

NeighbourMap::NeighbourMap(Map * parentMap)
{
    parent = parentMap;
}

NeighbourMap::~ NeighbourMap()
{
    neighbours.clear();
}

void NeighbourMap::clear()
{
    neighbours.clear();
}

#if 0
QMap<VertexPtr,NeighboursPtr> & NeighbourMap::getNMap()
{
    return neighbours;
}
#endif

void NeighbourMap::removeVertex(VertexPtr v)
{
    neighbours.remove(v);
}

void NeighbourMap::insertVertex(VertexPtr v)
{
    if (neighbours.contains(v))
    {
        return;
    }
    NeighboursPtr np = make_shared<Neighbours>(v);
    neighbours.insert(v,np);
}

int  NeighbourMap::numNeighbours(VertexPtr v)
{
    NeighboursPtr np = getNeighbours(v);
    if (np)
        return np->numNeighbours();
    else
        return 0;
}

void NeighbourMap::insertNeighbour(VertexPtr v, EdgePtr e)
{
    if (!neighbours.contains(v))
    {
        insertVertex(v);
    }
    NeighboursPtr np = getNeighbours(v);
    np->insertNeighbour(e);
}

void NeighbourMap::removeNeighbour(VertexPtr v, EdgePtr e)
{
    NeighboursPtr np = getNeighbours(v);
    if (np)
    {
        np->removeEdge(e);
        if (np->numEdges() == 0)
        {
            neighbours.remove(v);
        }
    }
}

void NeighbourMap::replaceNeighbour(VertexPtr v, EdgePtr existingEdge, EdgePtr newEdge)
{
    NeighboursPtr np = getNeighbours(v);
    if (np)
    {
        np->swapEdge2(existingEdge,newEdge);
    }
}

NeighboursPtr NeighbourMap::getNeighbours(VertexPtr v)
{
    NeighboursPtr np;
    if (neighbours.contains(v))
    {
        np = neighbours[v];
    }
    return np;
}

void NeighbourMap::sortByAngle()
{
    QMapIterator<VertexPtr,NeighboursPtr> i(neighbours);
    while (i.hasNext())
    {
        i.next();
        NeighboursPtr np = i.value();
        np->sortEdgesByAngle();
    }
}

int  NeighbourMap::countNeighbouringEdges()
{
    int count = 0;
    QMapIterator<VertexPtr,NeighboursPtr> i(neighbours);
    while (i.hasNext())
    {
        i.next();
        NeighboursPtr np = i.value();
        count += np->numNeighbours();
    }
    return count;
}

bool NeighbourMap::verify()
{
    bool rv = true;
    QMapIterator<VertexPtr,NeighboursPtr> i(neighbours);
    while (i.hasNext())
    {
        i.next();
        VertexPtr v  = i.key();
        qDebug() << "NeighbourMap verifying vertex:" << v->getTmpVertexIndex();
        if (!parent->getVertices().contains(v))
        {
            qDebug() << "unknown vertex" << v->getTmpVertexIndex() << "in NeighbourMap";
            rv = false;
        }

        NeighboursPtr np = i.value();
        if (v != np->getVertex())
        {
            qWarning() << "Neighbours has wrong vertex";
            rv = false;
        }
        if (!np->verify())
        {
            rv = false;
        }
    }
    return rv;
}

void NeighbourMap::cleanse()
{
    qDebug() << "NeighbourMap::cleanse";

    QVector<VertexPtr> baddies;
    QMapIterator<VertexPtr,NeighboursPtr> i(neighbours);
    while (i.hasNext())
    {
        i.next();
        VertexPtr v = i.key();
        if (!parent->getVertices().contains(v))
        {
            qWarning() << "Neighours has unknown vertex";
            baddies.push_back(v);
            continue;
        }

        NeighboursPtr np = i.value();
        if (i.key() != np->getVertex())
        {
            qWarning() << "Neighours has wrong vertex";
            baddies.push_back(v);
            continue;
        }

        if (!np->verify())
        {
            np->cleanse();
        }
    }
    for (auto v : baddies)
    {
        neighbours.remove(v);
    }
}

void NeighbourMap::dump()
{
    qDebug() << "NeighbourMap::dump() - start";
    QMapIterator<VertexPtr,NeighboursPtr> i(neighbours);
    while (i.hasNext())
    {
        i.next();
        qDebug() << "vertex" << i.key()->getTmpVertexIndex();
        NeighboursPtr np = i.value();
        np->dumpNeighbours();
    }
    qDebug() << "NeighbourMap::dump() - end";
}
