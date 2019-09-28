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
#include "designs/shapes.h"
#include "geometry/Edge.h"
#include "geometry/Vertex.h"

using std::make_shared;

struct sText
{
    QPointF pt;
    QString txt;
};

class Map
{
    #define MAP_EDGECOUNT_MAX 16

    friend class XmlLoader;
    friend class XmlWriter;

public:
    Map();
    ~Map();

    static int refs;

    void wipeout();     // reclaim memory

    QVector<sText>      texts;

    // Some obvious getters.
    int numEdges() const;
    int numVertices() const;

    const QVector<VertexPtr> * getVertices() const { return  &vertices;}
    const QVector<EdgePtr>   * getEdges()    const { return  &edges;}

    QString getInfo() const;
    bool    isEmpty();

    // Remove stuff from the map.
    void removeEdge(EdgePtr e);
    void removeVertex(VertexPtr v);
    void removeVertexSimple(VertexPtr v) { vertices.removeOne(v); }

    void    duplicate(MapPtr ret);     // duplictes the contents
    MapPtr  recreate();                // makes a new map with similar content

    // Insert the edge connecting two vertices, including updating
    // the neighbour lists for the vertices.
    EdgePtr insertEdge( VertexPtr  v1, VertexPtr v2, bool debug = false);
    void    splitEdge(EdgePtr e);

    // The publically-accessible version.
    VertexPtr insertVertex(QPointF pt);

    // Make map from DAC structures
    void insertPolygon(Polyform  * poly);
    void insertPolyline(Polyform * poly);

    void insertDebugMark(QPointF m, QString txt, qreal size = 0.05 );
    void insertDebugLine(EdgePtr edge);
    void insertDebugLine(QPointF p1, QPointF p2);
    void insertDebugLine(QLineF l1);
    void insertDebugPolygon(QPolygonF & poly);

    void scale(qreal s);
    void rotate(qreal r);
    void translate(qreal x, qreal y);
    void transformMap(QTransform T);

    // Transform a single vertex.
    void transformVertex(Vertex v, QTransform T);

    // Merge two maps.  The bread and butter of the Map class.  This is a
    // complicated computational geometry algorithm with a long and
    // glorious tradition :^)
    //
    // The goal is to form a map from the union of the two sets of
    // vertices (eliminating duplicates) and the union of the two sets
    // of edges (splitting edges whenever intersections occur).
    //
    // There are very efficient ways to do this, reporting edge-edge
    // intersections using a plane-sweep algorithm.  Implementing
    // this merge code in all its glory would be too much work.  Since
    // I have to use all my own code, I'm going to resort to a simplified
    // (and slower) version of the algorithm.
    //void mergeMap(MapPtr other, bool consume );
    void mergeMap(MapPtr other);

    // A simpler merge routine that assumes that the two maps
    // don't interact except at vertices, so edges don't need to
    // be checked for intersections.
    void mergeSimple( Map other );

    // It's often the case that we want to merge a transformed copy of
    // a map into another map, or even a collection of transformed copies.
    // Since transforming a map requires a slow cloning, we can save lots
    // of time and memory by transforming and merging simultaneously.
    // Here, we transform vertices as they are put into the current map.
    void mergeSimpleMany(MapPtr other, const QVector<QTransform> & transforms);

    // These methods fix maps.  It may be better not to make maps that need
    // to be fixed in the first place, but these routines come around afterwards
    // and clean things up

    void cleanse();                 // does all of the fucntions below
    void removeNullEdges();
    void removeDanglingVertices();
    void divideIntersectingEdges(); // if two edges cross, make a new vertex and have four edges
    void joinColinearEdges();       // if two lines are straight but have a vertex, then combine lines and delete vertex
    void cleanNeighbours();
    void sortAllNeighboursByAngle();
    void sortVertices();
    void sortEdges();

    // Print a text version of the map.
    QString summary();
    void dump(bool full=true) const;
    void dumpVertices(bool full=true) const;
    void dumpEdges(bool full=true) const;
    QString verticesToString();
    QString vptrsToString();

    bool contains (VertexPtr v) { return vertices.contains(v); }
    bool contains (EdgePtr e)   { return edges.contains(e); }

    QRectF calcBoundingRect();

    // A big 'ole sanity check for maps.  Throw together a whole collection
    // of consistency checks.  When debugging maps, call early and call often.
    //
    // It would probably be better to make this function provide the error
    // messages through a return value or exception, but whatever.
    bool verify(QString mapname, bool verbose, bool detailed = false, bool doDump = false) const;

    // shape factory
    void addShapeFactory(ShapeFactory * sf);

    // edge counts as per A.J. Lee
    bool analyzeVertices() const;     // returns true if changed
    void calcVertexEdgeCounts() const;
    void removeVerticesWithEdgeCount(int edgeCount);

    VertexPtr getVertex(int index)  { return vertices[index]; }

protected:
    // Get a Map Vertex given that we're asserting the vertex
    // doesn't lie on an edge in the map.
    VertexPtr getVertex_Simple( QPointF pt );

    // Insert an edge given that we know the edge doesn't interact
    // with other edges or vertices other than its endpoints.
    void insertEdge_Simple(EdgePtr edge );

    // Split any edge (there is at most one) that intersects
    // this new vertex.  You'll want to make sure this vertex isn't
    // a duplicate of one already in the map.  That would just
    // make life unpleasant.
    void splitEdgesByVertex(VertexPtr vert);

    // The "correct" version of inserting a vertex.  Make sure the
    // map stays consistent.
    VertexPtr getVertex_Complex(QPointF pt );

    // Given another vector of vertices, add them to the vertices of the
    // current map.  We can do this in linear time with a simple merge
    // algorithm.  Note that we want to coalesce identical vertices to
    // eliminate duplicates.
    void mergeVertices(QVector<VertexPtr> & your_verts );

    // Applying a motion made up only of uniform scales and translations,
    // Angles don't change.  So we can just transform each vertex.
    void applyTrivialRigidMotion(QTransform T);

    // In the general case, the vertices and edges must be re-sorted.
    void applyGeneralRigidMotion(QTransform T );

    // combine two edges which are in a straight line with common vertex
    void combineLinearEdges(EdgePtr a, EdgePtr b,VertexPtr common);

    // Routines used for spatial sorting of edges and vertices.
    static int lexCompareEdges( qreal a, qreal b );
    static int lexComparePoints( QPointF a, QPointF b );
    static bool edgeLessThan(EdgePtr a, EdgePtr b );
    static bool vertexLessThan( VertexPtr  a, VertexPtr b );

    bool joinOneColinearEdge();
    void deDuplicateEdges(QVector<EdgePtr> & vec);
    void deDuplicateVertices(QVector<VertexPtr> & vec);

    void cleanCopy();


private:
    QVector<VertexPtr> 	vertices;
    QVector<EdgePtr>    edges;
};

#endif
