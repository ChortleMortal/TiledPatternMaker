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
#include "geometry/Vertex.h"
#include "style/InterlaceInfo.h"

class Map;


enum eEdgeType
{
    EDGE_NULL,
    EDGE_POINT,     // an incomplete edge
    EDGE_LINE,
    EDGE_CURVE,
};

struct arcData
{
    QRectF  rect;
    qreal   start;
    qreal   span;
};


class Edge
{
public:
    Edge();
    Edge(VertexPtr V1);
    Edge(VertexPtr V1, VertexPtr V2 );
    Edge(VertexPtr V1, VertexPtr V2, QPointF arcCenter, bool convex);
    ~Edge();

    VertexPtr getV1();
    VertexPtr getV2();
    VertexPtr getOtherV(VertexPtr vert) const;
    VertexPtr getOtherV(QPointF pos) const;
    QPointF   getOtherP(VertexPtr vert) const;
    QPointF   getOtherP(QPointF pos) const;
    QLineF    getLine();
    QPointF   getMidPoint()  { return getLine().pointAt(0.50); }
    eEdgeType getType()      { return type; }

    QPointF   getArcCenter() { return arcCenter; }
    bool      isConvex()     { return convex; }

    void resetCurve();
    void setV1(VertexPtr v)  { v1 = v;}
    void setV2(VertexPtr v)  { v2 = v;  if ((type == EDGE_NULL) || (type == EDGE_POINT)) type = EDGE_LINE; }
    void setArcCenter(QPointF ac, bool convex);

    QPointF  calcDefaultArcCenter(bool convex);
    static arcData  calcArcData(QPointF V1, QPointF V2, QPointF ArcCenter, bool Convex);

    bool contains(VertexPtr v);
    bool sameAs(EdgePtr other);
    bool equals(EdgePtr other);

    interlaceInfo & getInterlaceInfo() {  return interlaceData; }
    void            initInterlaceInfo() { interlaceData.init(); }

    // Used to sort the edges in the map.
    qreal getMinX();

    static int refs;

    void setTmpEdgeIndex(int i) { tmpEdgeIndex = i; }
    int  getTmpEdgeIndex()      { return tmpEdgeIndex; }

protected:
    eEdgeType       type;
    VertexPtr       v1;
    VertexPtr       v2;

    QPointF         arcCenter;   // inside or outside the polygon
    bool            convex;

    interlaceInfo   interlaceData;
    int             tmpEdgeIndex;
};

#endif

