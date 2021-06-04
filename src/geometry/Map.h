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
// Map.java
//
// The implementation of a planar map abstraction.  A planar map is
// an (undirected) graph represented on the plane in such a way that
// edges don't cross vertices or other edges.
//
// This is one of the big daddy structures of computational geometry.
// The right way to do it is with a doubly-connected edge list structure,
// complete with half edges and a face abstraction.  Because I'm lazy
// and because of the sheer coding involved, I'm doing something simpler,
// more like an ordinary graph.  The disadvantage is that I don't maintain
// faces explicitly, which can make face colouring for islamic patterns
// tricky later.  But it's more tractable than computing overlays of
// DCELs.


#ifndef MAP_H
#define MAP_H

#include <QtCore>
#include "base/shared.h"
#include "base/misc.h"
#include "designs/shapes.h"
#include "geometry/edge.h"
#include "geometry/edgepoly.h"
#include "geometry/vertex.h"
#include "geometry/neighbours.h"

struct sText
{
    QPointF pt;
    QString txt;
};

enum eMCOptions
{
    badVertices_0               = 0x01,
    badVertices_1               = 0x02,
    badEdges                    = 0x04,
    joinupColinearEdges         = 0x10,
    divideupIntersectingEdges   = 0x20,
    cleanupNeighbours           = 0x40,
};

#define default_cleanse (badEdges | badVertices_0| cleanupNeighbours)

class MapStatus
{
public:
    MapStatus()  { reset(); }

    void reset() { verticesSorted = false; edgesSorted = false; neighboursBuilt = false; }

    bool fullyBuilt() { return neighboursBuilt && edgesSorted && verticesSorted; }

    bool neighboursBuilt;
    bool verticesSorted;
    bool edgesSorted;
};


class Map : public std::enable_shared_from_this<Map>
{
    #define MAP_EDGECOUNT_MAX 16

    friend class DCEL;

public:
    Map(QString Name);
    Map(QString Name, QPolygonF & poly);
    Map(QString Name, EdgePoly & poly);
    Map(const Map & map);     // duplictes the contents

    ~Map();
    void        wipeout();     // reclaim memory

    // make a new map with similar content
    MapPtr      recreate() const;

    // cleanse
    bool        cleanse(unsigned int options, bool forceVerify = true);

    // insertions
    void        insertDirect(VertexPtr v);
    void        insertDirect(EdgePtr e);
    VertexPtr   insertVertex(QPointF pt);
    EdgePtr     insertEdge(VertexPtr  v1, VertexPtr v2, bool debug = false);

    void        addShapeFactory(ShapeFPtr sf);

    // deletions
    void        removeVertex(VertexPtr v);
    void        removeVertexSimple(VertexPtr v);
    void        removeEdge(EdgePtr e);

    // modifications
    void        addCropBorder(QRectF rect);
    void        removeOutisde(QRectF rect);

    void        scale(qreal s);
    void        rotate(qreal r);
    void        translate(qreal x, qreal y);
    void        transformMap(QTransform T);

    void        splitEdge(EdgePtr e);

    void        mergeMap(MapPtr other);
    void        mergeSimpleMany(constMapPtr other, const QVector<QTransform> & transforms);
    void        mergeMany(constMapPtr other, const QVector<QTransform> & transforms);

    void        sortVertices();
    void        sortEdges();
    void        buildNeighbours();

    // getters
    const QVector<VertexPtr> & getVertices() { return vertices; }
    const QVector<EdgePtr>   & getEdges()    { return edges; }
    const QVector<sText>     & getTexts()    { return texts; }

    NeighboursPtr getBuiltNeighbours(VertexPtr v);
    NeighboursPtr getRawNeighbours(VertexPtr v);

    DCELPtr     getDCEL();
    EdgePoly    getEdgePoly() const;

    // info
    QString     name() const { return mname; }
    QString     summary() const;
    QString     displayVertexEdgeCounts();

    bool        isEmpty() const;
    int         numEdges() const;
    int         numVertices() const;
    bool        contains (VertexPtr v) const { return vertices.contains(v); }
    bool        contains (EdgePtr e)  const  { return edges.contains(e); }
    bool        hasIntersectingEdges() const;

    //debug
    bool        verify(bool force = false);
    void        dumpMap(bool full=true);

    void        insertDebugMark(QPointF m, QString txt, qreal size = 0.05 , QPointF offset = QPointF());
    void        insertDebugLine(EdgePtr edge);
    void        insertDebugLine(QPointF p1, QPointF p2);
    void        insertDebugLine(QLineF l1);
    void        insertDebugPolygon(QPolygonF & poly);

    static int  refs;

protected:
    UniqueQVector<VertexPtr>    vertices;
    UniqueQVector<EdgePtr>      edges;

private:
    // insertions
    void        _insertEdge(EdgePtr e, bool debug = false);
    EdgePtr     _insertCurvedEdge(VertexPtr  v1, VertexPtr v2, QPointF center, bool isConvex, bool debug = false);
    void        _insertEdge_Simple(EdgePtr edge );
    void        _insertPolygon(Polyform  * poly);
    void        _insertPolyline(Polyform * poly);

    // modifications
    void        _applyGeneralRigidMotion(QTransform T );
    void        _applyTrivialRigidMotion(QTransform T);

    void        _splitEdgesByVertex(VertexPtr vert);
    bool        _splitTwoEdgesByVertex(VertexPtr vert);

    void        _mergeVertices(MapPtr other);
    void        _joinEdges(EdgePtr e1, EdgePtr e2);

    // getters
    VertexPtr   _getOrCreateVertex(QPointF pt);
    VertexPtr   _getVertex(QPointF pt) const;

    // info
    bool        _edgeExists(VertexPtr v1, VertexPtr v2) const;

    // debug
    void        _dumpVertices(bool full);
    void        _dumpEdges(bool full) const;

    // cleanse
    void        _cleanCopy() const;
    void        _cleanseVertices();

    // cleanse operations
    void        joinColinearEdges();       // if two lines are straight but have a vertex, then combine lines and delete vertex
    void        divideIntersectingEdges(); // if two edges cross, make a new vertex and have four edges
    void        removeVerticesWithEdgeCount(int edgeCount);
    void        deDuplicateNeighbours();
    void        removeBadEdges();
    bool        joinOneColinearEdge();
    void        combineLinearEdges(EdgePtr a, EdgePtr b,VertexPtr common);
    void        deDuplicateEdges(const NeighboursPtr vec);

    // debug verify operations
    bool        verifyVertices();
    bool        verifyEdges();
    bool        verifyNeighbours();

    // utilities
    static int  lexCompareEdges(qreal a, qreal b);
    static int  lexComparePoints(QPointF a, QPointF b);
    static bool edgeLessThan(EdgePtr a, EdgePtr b);
    static bool vertexLessThan( VertexPtr  a, VertexPtr b );

    int         vertexIndex(VertexPtr v) const { return vertices.indexOf(v); }
    int         edgeIndex(EdgePtr e)     const { return edges.indexOf(e); }

    Configuration                   * config;
    QString                           mname;

    std::map<VertexPtr,NeighboursPtr> neighbours;
    DCELPtr                           dcel;
    QVector<sText>                    texts;
    MapStatus                         status;
};

#endif
