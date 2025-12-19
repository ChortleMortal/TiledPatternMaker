#include <QDebug>
#include "sys/geometry/edge.h"
#include "sys/geometry/map.h"
#include "sys/geometry/neighbours.h"
#include "sys/geometry/vertex.h"

int Neighbours::refs = 0;

extern double angleBetween(EdgePtr a, EdgePtr b);

Neighbours::Neighbours(const VertexPtr & vp)
{
    vertex = vp;
    refs++;
}

Neighbours::Neighbours(const Neighbours & other) : QVector<WeakEdgePtr>(other)
{
    vertex = other.vertex;
    refs++;
}

Neighbours::~Neighbours()
{
    refs--;
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

EdgePtr Neighbours::nearest(const EdgePtr &edge, bool side1)
{
    EdgePtr neaarestEdge;
    qreal   angle = 1000;       // makes compiler happy since it does not understand this code
    for (auto & wedge : std::as_const(*this))
    {
        auto nedge = wedge.lock();
        if (nedge == edge)
            continue;

        qreal theta;
        if (side1)
            theta = angleBetween(nedge,edge);
        else
            theta = angleBetween(edge,nedge);

        if (!neaarestEdge)
        {
            angle = theta;
            neaarestEdge = nedge;
        }
        else
        {
            Q_ASSERT(angle != 1000);
            if (theta < angle)
            {
                angle = theta;
                neaarestEdge    = edge;
            }
        }
    }
    return neaarestEdge;
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

EdgePtr Neighbours::getContinuation(EdgePtr edge, int type)
{
    EdgePtr result;
    qreal   angle;

    for (auto & wedge : *this)
    {
        EdgePtr nedge = wedge.lock();

        if (!nedge)
            continue;
        if (edge == nedge)
            continue;
        if (edge->isTwin(nedge))
            continue;

        if (!result)
        {
            // first time
            angle = qAbs(edge->getLine().angleTo(nedge->getLine()));
            result = nedge;
        }
        else
        {
            qreal angle2 = qAbs(edge->getLine().angleTo(nedge->getLine()));
            switch (type)
            {
            case 1:
                // this gets most acute (smallest) angle, not nearest to 180 degrees)
                if (angle2 < angle)
                {
                    angle  = angle2;
                    result = nedge;
                }
                break;
            case 2:
                // this gets largest angle, not nearest to 180 degrees)
                if (angle2 > angle)
                {
                    angle  = angle2;
                    result = nedge;
                }
                break;
            case 3:
                // this not nearest to 180 degrees)
                if ((180.0 - angle2) >  (180.0 - angle))
                {
                    angle  = angle2;
                    result = nedge;
                }
                break;
            }
        }
    }
    return result;
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
#if 0
    qreal b = e->angle();
    qreal c = e->getLine().angle();
    auto  e2 = e->createTwin();
    qDebug() << qRadiansToDegrees(a) << qRadiansToDegrees(b) << c;
    qreal b2 = e2->angle();
    qreal c2 = e2->getLine().angle();
    qDebug() << qRadiansToDegrees(a) << qRadiansToDegrees(b2) << c2;
#endif
    for (int i=0; i < (int)size(); i++)
    {
        EdgePtr ep = at(i).lock();
        if (!ep)
        {
            qWarning() << "bad neighbour 1";
            continue;
        }
        qreal b = vertex.lock()->getAngle(ep);
        if (a > b)
        {
            insert(i,e);  // here
            return;
        }
    }

    push_back(e);
}

bool Neighbours::verify()
{
    bool rv = true;
    for (auto & e : std::as_const(*this))
    {
        if (vertex.lock() != e.lock()->v1 && vertex.lock() != e.lock()->v2)
        {
            qWarning() << "Neighbours: vertex:" << vertex.lock()->pt << "has invalid edge:" << e.lock()->v1->pt << e.lock()->v2->pt;
            rv = false;
        }
    }
    return rv;
}

void Neighbours::cleanse()
{
    EdgeSet baddies;

    for (const WeakEdgePtr & e : std::as_const(*this))
    {
        EdgePtr e1 = e.lock();
        if (vertex.lock() != e1->v1  && vertex.lock() != e1->v2)
        {
            qDebug() << "Neighbours: cleaning invalid edge";
            baddies.push_back(e1);
        }
    }

    // this seems unnecessarily complex but has to be
    for (const EdgePtr & e2 : std::as_const(baddies))
    {
        for (int i=0; i < (int)size(); i++)
        {
            EdgePtr ep = at(i).lock();
            if (ep == e2)
            {
                removeAt(i);
            }
        }
    }
}

EdgePtr Neighbours::getNeighbour(int index)
{
    WeakEdgePtr wep = (*this)[index];
    return wep.lock();
}

QString Neighbours::info(MapPtr &map)
{
    QString info;
    for (const WeakEdgePtr & e : std::as_const(*this))
    {
        EdgePtr e1 = e.lock();
        info += QString::number(map->edgeIndex(e1));
        info += " ";
    }
    return info;
}

QString Neighbours::casingInfo()
{
    QString info;
    for (const WeakEdgePtr & e : std::as_const(*this))
    {
        EdgePtr e1 = e.lock();
        info += QString::number(e1->casingIndex);
        info += " ";
    }
    return info;
}
