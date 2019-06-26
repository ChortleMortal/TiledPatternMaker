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
// Edge.java
//
// The edge component of the planar map abstraction.

#ifndef EDGE_H
#define EDGE_H

#include <QtCore>
#include "base/shared.h"

class Map;
class interlaceInfo;

class Edge
{
public:
    Edge();
    Edge(VertexPtr V1, VertexPtr V2 );
    ~Edge();

    VertexPtr getV1();
    VertexPtr getV2();
    VertexPtr getOther(QPointF pos) const;
    QLineF    getLine();
    QPointF   getMidPoint() { return getLine().pointAt(0.50); }

    void setV1(VertexPtr v) {v1 = v;}
    void setV2(VertexPtr v) {v2 = v;}

    bool sameAs(EdgePtr other);

    interlaceInfo * getInterlaceInfo();
    void setInterlaceInfo(interlaceInfo * info);

    // Used to sort the edges in the map.
    qreal getMinX();

    static int refs;

    void setTmpIndex(int i) { tmpIndex = i; }
    int  getTmpIndex()      { return tmpIndex; }

protected:
    VertexPtr       v1;
    VertexPtr       v2;
    interlaceInfo * interlaceData;
    int tmpIndex;
};

#endif

