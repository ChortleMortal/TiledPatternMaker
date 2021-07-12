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

typedef std::weak_ptr<class Vertex>         WeakVertexPtr;
typedef std::shared_ptr<class Vertex>       VertexPtr;
typedef std::shared_ptr<class Edge>         EdgePtr;
typedef std::weak_ptr<class Edge>           WeakEdgePtr;
typedef std::shared_ptr<class Thread>       ThreadPtr;
typedef std::shared_ptr<class Face>         FacePtr;

enum eEdgeType
{
    EDGETYPE_NULL,
    EDGETYPE_POINT,     // an incomplete edge
    EDGETYPE_LINE,
    EDGETYPE_CURVE,
};

struct arcData
{
    QRectF  rect;
    qreal   start;
    qreal   end;    // used internally
    qreal   span;
};


class Edge
{
public:
    Edge();
    Edge(VertexPtr V1);
    Edge(VertexPtr V1, VertexPtr V2 );
    Edge(VertexPtr V1, VertexPtr V2, QPointF arcCenter, bool convex);
    Edge(EdgePtr other, QTransform T);
    ~Edge();

    double    angle() const;

    VertexPtr getOtherV(VertexPtr vert) const;
    VertexPtr getOtherV(QPointF pos) const;
    QPointF   getOtherP(VertexPtr vert) const;
    QPointF   getOtherP(QPointF pos) const;
    QLineF    getLine();
    QPointF   getMidPoint()  { return getLine().pointAt(0.50); }
    eEdgeType getType()      { return type; }
    qreal     getAngle();
    EdgePtr   getSwappedEdge();

    void      setV1(VertexPtr v)  { v1 = v;}
    void      setV2(VertexPtr v)  { v2 = v;  if ((type == EDGETYPE_NULL) || (type == EDGETYPE_POINT)) type = EDGETYPE_LINE; }

    void      setArcCenter(QPointF ac, bool convex);
    void      calcArcCenter(bool convex);
    void      calcMagnitude();
    void      setArcMagnitude(qreal magnitude);
    void      setConvex(bool convexCurve) { convex = convexCurve; }
    void      resetCurve();

    QPointF   getArcCenter() { return arcCenter; }
    qreal     getArcMagnitude() { return arcMagnitude; }
    bool      isConvex()     { return convex; }

    void      setSwapState(bool swap) { isSwapped = swap; }
    bool      getSwapState()          { return isSwapped; }

    bool      isColinearAndTouching(EdgePtr e);
    bool      isColinear(EdgePtr e);

    static arcData  calcArcData(QPointF p1, QPointF p2, QPointF c, bool convex);
           arcData  calcArcData();

    bool      contains(VertexPtr v);
    bool      sameAs(EdgePtr other);
    bool      sameAs(VertexPtr ov1, VertexPtr ov2);
    bool      equals(EdgePtr other);

    // Used to sort the edges in the map.
    qreal     getMinX();


    // dcel stuff
    bool operator < (const Edge & other) const;

    EdgePtr    prev() { return twin.lock()->next.lock(); }

    WeakEdgePtr     twin;
    WeakEdgePtr     next;
    FacePtr         incident_face;
    bool            dvisited;       // used by dcel

    //
    QString   dump();

    static int refs;

    VertexPtr       v1;
    VertexPtr       v2;

    bool            visited;        // used by interlace
    bool            start_under;    // used by interlace
    ThreadPtr       thread;         // used by interlace

protected:
    eEdgeType       type;

    bool            isSwapped;
    bool            convex;
    QPointF         arcCenter;      // inside or outside the polygon
    qreal           arcMagnitude;   // range 0 to 1
};

#endif

