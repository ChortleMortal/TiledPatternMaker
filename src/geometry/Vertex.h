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

#ifndef VERTEX_H
#define VERTEX_H

#include <QtCore>
#include <list>
#include "geometry/Transform.h"
#include "base/shared.h"

using std::list;

class interlaceInfo;

class Vertex
{
    friend class XmlLoader;
    friend class Map;
    friend class Star;

public:
    Vertex(QPointF pos);
    ~Vertex();

    QPointF getPosition() const {return pos;}
    void    setPosition(QPointF pt) {pos = pt;}

    interlaceInfo * getInterlaceInfo();
    void setInterlaceInfo(interlaceInfo * info);

    int numEdges()       { return neighbours.size(); }
    int numNeighbours()  { return neighbours.size(); }

    QVector<EdgePtr> getBeforeAndAfter(EdgePtr edge);

    EdgePtr          getNeighbour(const VertexPtr other);

    QVector<EdgePtr> & getNeighbours();
    QVector<EdgePtr> & getEdges();

    qreal getAngle(EdgePtr edge);

    bool connectsTo(VertexPtr other);
    bool isNear(VertexPtr other);

    void insertNeighbour(EdgePtr np);
    void insertEdge(EdgePtr edge);
    void insertEdgeSimple(EdgePtr edge) { neighbours.push_back(edge); }
    void removeEdge(EdgePtr edge );
	void swapEdge(VertexPtr other, EdgePtr nedge);
    void sortEdgesByAngle();

    void applyRigidMotion(Transform T);

    static int refs;

    QString dumpNeighbours();
    void setTmpIndex(int i) { tmpIndex = i; }
    int  getTmpIndex()      { return tmpIndex; }

private:
    QPointF       pos;
    VertexPtr     copy; 	// Used when cloning the map.
    interlaceInfo * interlaceData;
    int             tmpIndex;

    // DAC: OMG  in taprats this was a forward linked list of neigbour classes
    // IMHO this added complexity but no value and is replaced by a vector of edges
    // This vector needs to be sorted by angle
    QVector<EdgePtr>  neighbours;
};

#endif

