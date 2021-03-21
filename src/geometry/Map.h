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

class Map : public std::enable_shared_from_this<Map>
{
    #define MAP_EDGECOUNT_MAX 16

    friend class MosaicLoader;
    friend class MosaicWriter;
    friend class MapCleanser;

public:
    Map(QString Name);
    Map(QString Name, QPolygonF & poly);
    Map(QString Name, EdgePoly & poly);
    Map(const Map & map);     // duplictes the contents
    ~Map();

    void wipeout();     // reclaim memory
    bool verifyMap(QString mapname, bool force = false);

    // Some obvious getters.
    int numEdges() const;
    int numVertices() const;

    EdgePoly getEdgePoly();

    bool isEmpty();

    // Remove stuff from the map.
    void removeEdge(EdgePtr e);
    void removeVertex(VertexPtr v);
    void removeVertexSimple(VertexPtr v) { vertices.removeOne(v);  }
    void crop(QRectF rect);

    MapPtr  recreate();                // makes a new map with similar content
    MapPtr  compress();                // join all straight lines

    EdgePtr insertEdge(VertexPtr  v1, VertexPtr v2, bool debug = false);
    void    insertEdge(EdgePtr e, bool debug = false);
    EdgePtr insertCurvedEdge(VertexPtr  v1, VertexPtr v2, QPointF center, bool isConvex, bool debug = false);
    void    splitEdge(EdgePtr e);

    VertexPtr insertVertex(QPointF pt);

    int  vertexIndex(VertexPtr v) { return vertices.indexOf(v); }
    int  edgeIndex(EdgePtr e)     { return edges.indexOf(e); }

    void insertDebugMark(QPointF m, QString txt, qreal size = 0.05 , QPointF offset = QPointF());
    void insertDebugLine(EdgePtr edge);
    void insertDebugLine(QPointF p1, QPointF p2);
    void insertDebugLine(QLineF l1);
    void insertDebugPolygon(QPolygonF & poly);

    void scale(qreal s);
    void rotate(qreal r);
    void translate(qreal x, qreal y);
    void transformMap(QTransform T);

    void mergeMap(MapPtr other);
    void mergeSimpleMany(constMapPtr other, const QVector<QTransform> & transforms);

    void sortVertices();
    void sortEdges();
    void buildNeighbours();

    void    dumpMap(bool full=true);
    QString name() { return mname; }
    QString getInfo() const;
    QString summary();
    QString calcVertexEdgeCounts();

    bool contains (VertexPtr v) { return vertices.contains(v); }
    bool contains (EdgePtr e)   { return edges.contains(e); }
    bool edgeSameAs(VertexPtr v1, VertexPtr v2);

    void addShapeFactory(ShapeFPtr sf);

    DCELPtr getDCEL();

    QVector<sText>  texts;

    static int refs;

    UniqueQVector<VertexPtr>  vertices;
    UniqueQVector<EdgePtr>    edges;
    DCELPtr                   dcel;

protected:
    // Make map from DAC structures
    void insertPolygon(Polyform  * poly);
    void insertPolyline(Polyform * poly);

    VertexPtr getOrCreateVertex( QPointF pt );

    void insertEdge_Simple(EdgePtr edge );

    void splitEdgesByVertex(VertexPtr vert);
    bool splitTwoEdgesByVertex(VertexPtr vert);

    void mergeVertices(MapPtr other);

    void applyTrivialRigidMotion(QTransform T);

    void applyGeneralRigidMotion(QTransform T );

    static int lexCompareEdges( qreal a, qreal b );
    static int lexComparePoints( QPointF a, QPointF b );
    static bool edgeLessThan(EdgePtr a, EdgePtr b );
    static bool vertexLessThan( VertexPtr  a, VertexPtr b );

    bool joinOneColinearEdgeIgnoringIntersects();
    void joinEdges(EdgePtr e1, EdgePtr e2);

    void dumpVertices(bool full);
    void dumpEdges(bool full);

    void cleanCopy();

private:
    bool verifyVertices();
    bool verifyEdges();
    bool verifyNeighbours();

    QString             mname;
    Configuration     * config;

    QDebug  * deb;
    QString astring;
};

#endif
