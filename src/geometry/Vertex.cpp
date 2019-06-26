/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

////////////////////////////////////////////////////////////////////////////
//
// Vertex.java
//
// The vertex abstraction for planar maps.  A Vertex has the usual graph
// component, a list of adjacent edges.  It also has the planar component,
// a position.  Finally, there's a user data field for applications.

#include "geometry/Vertex.h"
#include "geometry/Edge.h"
#include "geometry/Point.h"
#include "style/Interlace.h"
#include "base/utilities.h"
#include <iterator>

int Vertex::refs = 0;

Vertex::Vertex( QPointF pos )
{
    refs++;
    this->pos       = pos;
    interlaceData   = nullptr;
    tmpIndex        = -1;
}

Vertex::~Vertex()
{
    refs--;
    //qDebug() << "Vertex destructor";
    copy.reset();
    neighbours.clear();
    if (interlaceData)
    {
       delete interlaceData;
    }
}

interlaceInfo * Vertex::getInterlaceInfo()
{
    return interlaceData;
}

void Vertex::setInterlaceInfo(interlaceInfo * info)
{
    if (interlaceData)
        delete interlaceData;
    interlaceData = info;
}


// though the original taprats version of this method seemed deceptively simple
// it has been hard to discern how it was really meant to work
// for the corener case of when a vertex has 0, 1, or 2, neighbours
// so this code, while ugly, is a slavish attempt to emulate the original
QVector<EdgePtr> Vertex::getBeforeAndAfter(EdgePtr edge )
{
    if (numEdges() < 2)
    {
        qDebug() << "getBeforeAndAfter - edge=" << Utils::addr(edge.get()) << "count=" << numEdges();
    }

    QVector<EdgePtr> ret;

    neighbours.push_back(neighbours[0]);      // temp addition
    neighbours.push_back(neighbours[1]);      // temp addition

    for (int i=1; i < (neighbours.size()-1); i++)
    {
        if (neighbours[i] == edge)
        {
            ret = { neighbours[i-1], neighbours[i+1]};
            neighbours.removeLast();     //back out addition
            neighbours.removeLast();     //back out addition
            return ret;
        }
    }
    qCritical("getBeforeAndAfter - should not reach here");
    return ret;
}

QVector<EdgePtr> & Vertex::getNeighbours()
{
    return neighbours;
}

QVector<EdgePtr> & Vertex::getEdges()
{
    return neighbours;
}

EdgePtr Vertex::getNeighbour(const VertexPtr other)
{
    for (auto it = neighbours.begin(); it != neighbours.end(); it++)
    {
        EdgePtr edge = *it;
        VertexPtr vp = edge->getOther(pos);
        if (vp == other)
        {
            return edge;
        }
        else if (!vp)
        {
            qWarning() << "MISSING VERTEX in getNeighbour";
        }
    }

    qWarning("getNeighbour - no neighbouring edge");
    EdgePtr e;
    return e;
}

bool Vertex::connectsTo( VertexPtr other)
{
    return (getNeighbour(other) != nullptr);
}

bool Vertex::isNear(VertexPtr other)
{
    return Point::isNear(pos,other->pos);
}

/*
 * Insert the edge into the vertex's neighbour list.  Edges
 * are stored in sorted order by angle (though the starting point
 * isn't important).  So traverse the list until there's a spot
 * where the arc swept by consecutive edges contains the new edge.
 */

void Vertex::insertEdge(EdgePtr edge)
{
    insertNeighbour(edge);
}


void Vertex::insertNeighbour(EdgePtr e)
{
    //qDebug() << "insert neighbour: vertex=" << Utils::addr(this) << "edge=" << Utils::addr(e.get());
    if(neighbours.size() == 0)
    {
        neighbours.push_back(e);
        return;
    }

    qreal a = getAngle(e);

    for (int i=0; i < neighbours.size(); i++)
    {
        EdgePtr ep = neighbours[i];
        qreal b = getAngle(ep);
        if (a > b)
        {
            neighbours.insert(i,e);
            return;
        }
    }

    neighbours.push_back(e);
}

void Vertex::sortEdgesByAngle()
{
    if (numEdges() < 2)
    {
        return;     // returns for 0 or 1
    }

    QVector<EdgePtr> vec;

    for (int i=0; i < neighbours.size(); i++)
    {
        EdgePtr e = neighbours[i];
        qreal   a = getAngle(e);
        bool found = false;
        for (int j = 0; j < vec.size(); j ++)
        {
            EdgePtr evec = vec[j];
            qreal b = getAngle(evec);
            if (a > b)
            {
                vec.insert(j,e);
                found = true;
                break;
            }
        }
        if (!found)
        {
            vec.push_back(e);
        }
    }
    neighbours = vec;    // overwrites
}

//Removing is always easier than adding.  Just splice the edge out of the list.
void Vertex::removeEdge(EdgePtr edge )
{
    neighbours.removeAll(edge);
}

// Apply a transform.  Recalculate all the angles.  The order
// doesn't change, although the list might need to get reversed
// if the transform flips.  CSKFIXME -- make this work.
// Fortunately, the rigid motions we'll apply in Islamic design
// won't contain flips.  So we're okay for now.
// casper: don't need to recalc angle
void Vertex::applyRigidMotion(Transform T)
{
    pos = T.apply(pos);
}


// When an edge is split in the map, this vertex's adjacency list
// needs to be updated -- some neighbour will now have a new
// edge instance to represent the new half of an edge that was
// created.  Do a hacky rewrite here.
// casper - another hacky rewrite on top of the other
// TODO - maybe resort now?
void Vertex::swapEdge(VertexPtr other, EdgePtr nedge)
{
    for (auto it = neighbours.begin(); it != neighbours.end(); it++)
    {
        EdgePtr edge = *it;
        VertexPtr vp = edge->getOther(pos);
        if (vp == other)
        {
            *it = nedge;        // this replaces edge
            break;
        }
    }
}

qreal Vertex::getAngle(EdgePtr edge)
{
    VertexPtr other = edge->getOther(pos);
    QPointF pd  = other->getPosition() - pos;
    Point::normalizeD(pd);
    qreal angle = qAtan2(pd.x(), pd.y());
    return angle;
}


QString Vertex::dumpNeighbours()
{
    QString astring;
    QDebug  deb(&astring);
    for (auto it=neighbours.begin(); it!= neighbours.end(); it++)
    {
        EdgePtr ep = *it;
        deb << "edge: " << Utils::addr(ep.get());
#if 1
        QPointF v1 = ep->getV1()->getPosition();
        QPointF v2 = ep->getV2()->getPosition();
        deb << " " << v1 << " " << v2 << endl;
#endif
    }
    return astring;
}
