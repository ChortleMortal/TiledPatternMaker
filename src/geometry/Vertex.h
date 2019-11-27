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
#include "base/shared.h"
#include "style/InterlaceInfo.h"

using std::list;

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

    interlaceInfo & getInterlaceInfo() {  return interlaceData; }
    void            initInterlaceInfo() { interlaceData.init(); }

    qreal getAngle(EdgePtr edge);


    void applyRigidMotion(QTransform T);

    static int refs;

    void    setTmpVertexIndex(int i) { tmpVertexIndex = i; }
    int     getTmpVertexIndex()      { return tmpVertexIndex; }

private:
    QPointF       pos;
    VertexPtr     copy; 	// Used when cloning the map.
    interlaceInfo interlaceData;
    int           tmpVertexIndex;
};

#endif

