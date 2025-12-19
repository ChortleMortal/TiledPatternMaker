#pragma once
#ifndef MAP_H
#define MAP_H

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

#include "legacy/shapes.h"
#include "sys/geometry/circle.h"
#include "sys/geometry/edge_poly.h"
#include "sys/geometry/fill_region.h"
#include "sys/geometry/map_base.h"

typedef std::shared_ptr<class Map>          MapPtr;
typedef std::shared_ptr<const class Map>    constMapPtr;
typedef std::shared_ptr<class ShapeFactory> ShapeFPtr;
typedef std::shared_ptr<class DCEL>         DCELPtr;

typedef std::weak_ptr<class Map>            WeakMapPtr;
typedef std::weak_ptr<class Edge>           WeakEdgePtr;
typedef std::weak_ptr<class DCEL>           WeakDCELPtr;

enum eCompare
{
    COMP_LESS    = -1,
    COMP_EQUAL   = 0,
    COMP_GREATER = 1
};

class Isect
{
public:
    Isect() {}
    Isect(EdgePtr e, EdgePtr c, VertexPtr v) { vertex=v; edge=e; cutter = c; }

    void dump() const;

    EdgePtr     edge;    // the interseted edge
    EdgePtr     cutter;  // the cutting edge
    VertexPtr   vertex;  // the vertex for the intersection point
};

class Map : public MapBase
{
    #define MAP_EDGECOUNT_MAX 16

    friend class DCEL;
    friend class DebugMap;
    friend class MapVerifier;

public:
    Map(const QString & name);
    Map(const QString & name, const QPolygonF & poly);
    Map(QString Name, const EdgePoly & poly);
    virtual ~Map();

    void        set(const constMapPtr & other);  // replace contents
    MapPtr      copy() const;       // duplictes the contents
    MapPtr      recreate() const;   // make a new map with similar content
    void        rebuildD();         // rebuild from scratch
    void        clear();            // reclaim memory
    MapPtr      getTransformed(const QTransform & T) const;

    void        insertVertex(VertexPtr v) { vertices.push_back(v); }
    VertexPtr   insertVertex(const QPointF & pt);
    VertexPtr   getVertex(const QPointF & pt) const;

#if 0
    EdgePtr     insertEdge(const QLineF & line);
    EdgePtr     insertEdge(const QPointF   & p1, const QPointF   & p2);
#endif
    EdgePtr     insertEdge(const VertexPtr & v1, const VertexPtr & v2);
    EdgePtr     insertEdge(const VertexPtr & v1, const VertexPtr & v2, const QPointF & arcCenter, eCurveType ctype);

    void        addShapeFactory(ShapeFPtr sf);

    // deletions
    void        removeVertex(const VertexPtr & v);
    void        removeVertexSimple(const VertexPtr & v);
    void        removeEdge(const EdgePtr & e);

    // modifications
    void        embedCrop(const QRectF & rect);
    void        embedCrop(const Circle & circle);
    void        embedCrop(const QPolygonF & poly);

    void        cropOutside(const QRectF & rect);
    void        cropOutside(const QPolygonF & poly);
    void        cropOutside(const Circle & circle);

    void        splitEdge(EdgePtr e);

    void        mergeMap(const Map * other, qreal tolerance = Sys::TOL);
    void        mergeMap(const constMapPtr & other, qreal tolerance = Sys::TOL);
    void        mergeMany(const constMapPtr & other, const Placements & placements);
    void        mergeSimpleMany(constMapPtr & other, const Placements & transforms);

    QStack<Isect> findIntersections(EdgePtr cutter);
    void          processIntersections(QStack<Isect> & isects);

    EdgePtr     makeCopy(const EdgePtr & e, QTransform T);

    void        addMap(MapPtr other);       // for debug
    void        removeMap(MapPtr other);

    // back-door
    void        XmlInsertDirect(VertexPtr v);
    void        XmlInsertDirect(EdgePtr e);

    const QVector<QPointF>  getPoints();

    // DCEL
    void        setDerivedDCEL(DCELPtr dcel) { derivedDCEL = dcel; }
    DCELPtr     getDerivedDCEL()             { return derivedDCEL.lock(); }

    // info
    QString             name() const { return mname; }
    virtual QString     summary() const;
    virtual QString     info() const;
    virtual void        dump(bool full=false);
    QString             displayVertexEdgeCounts();

    int         numEdges() const;
    int         numVertices() const;
    bool        contains (const VertexPtr & v) const { return vertices.contains(v); }
    bool        contains (const EdgePtr & e)  const  { return edges.contains(e); }
    bool        hasIntersectingEdges() const;
    EdgePtr     edgeExists(const EdgePtr & edge) const;
    EdgePtr     edgeExists(const VertexPtr &  v1, const VertexPtr & v2) const;
    EdgePtr     edgeExists(const QPointF &  p1, const QPointF & v2) const;

    //debug
    void        private_insertEdge(const EdgePtr & e) { _insertEdgeSimple(e); }

    static int  refs;

protected:

private:
    // insertions
    void        _insertEdge(const EdgePtr & cutter);
    EdgePtr     _insertCurvedEdge(const VertexPtr & v1, const VertexPtr & v2, const QPointF & center, eCurveType ctype);
    void        _insertEdgeSimple(const EdgePtr & edge );
    void        _insertPolygon(Polyform  * poly);
    void        _insertPolyline(Polyform * poly);

    // modifications
    void        _splitEdgesByVertex(const VertexPtr & vert);
    bool        _splitTwoEdgesByVertex(const VertexPtr & vert);

    void        _mergeVertices(const constMapPtr & other, qreal tolerance = Sys::TOL);
    void        _joinEdges(const EdgePtr & e1, const EdgePtr & e2);

    // getters
    VertexPtr   _getOrCreateVertex(const QPointF &pt);

    // debug
    void        _dumpVertices(bool full);
    void        _dumpEdges(bool full) const;

    void        _cleanCopy() const;

    // utilities
    static eCompare  comparePoints(const QPointF &a, const QPointF &b, qreal tolerance = Sys::TOL);
    static bool      vertexAngleGreaterThan(const VertexPtr & a, const VertexPtr & b);

    QString                     mname;
    WeakDCELPtr                 derivedDCEL;
    static QPointF              tmpCenter;
};

#endif
