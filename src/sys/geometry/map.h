#pragma once

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

#include <QHash>
#include <QPair>

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
    void        transform(const QTransform & T);

    void        insertVertex(VertexPtr v);
    VertexPtr   insertVertex(const QPointF & pt);
    VertexPtr   getVertex(const QPointF & pt) const;

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

    void        cropOutside2(const QPolygonF & poly);
    void        cropOutside3(const QPolygonF & poly);

    void        splitEdge(EdgePtr e);

    void        mergeMap(const Map * other, qreal tolerance = Loose::TOL);
    void        mergeMap(const constMapPtr & other, qreal tolerance = Loose::TOL);
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

    void        _mergeVertices(const constMapPtr & other, qreal tolerance = Loose::TOL);
    void        _joinEdges(const EdgePtr & e1, const EdgePtr & e2);

    // getters
    VertexPtr   _getOrCreateVertex(const QPointF &pt);

    // debug
    void        _dumpVertices(bool full);
    void        _dumpEdges(bool full) const;

    void        _cleanCopy() const;

    // utilities
    static eCompare  comparePoints(const QPointF &a, const QPointF &b, qreal tolerance = Loose::TOL);
    static bool      vertexAngleGreaterThan(const VertexPtr & a, const VertexPtr & b);

    // spatial index for fast vertex lookup during merge-heavy construction.
    // Rebuilt lazily (on next _getOrCreateVertex call) whenever invalidated.
    // GRID_CELL must be > sqrt(Loose::TOL) so a matching vertex can never be
    // more than one cell away from pt's own cell (hence the 3x3 search).
    static constexpr qreal GRID_CELL = 0.001;
    // Each bucket entry pairs a vertex with its insertion index within
    // `vertices` at the time the grid was (re)built, so that when several
    // candidates in the neighborhood all match within tolerance, we can
    // deterministically pick the same one the old linear scan over
    // `vertices` would have picked (the lowest index / first inserted) -
    // rather than whichever the grid happens to visit first.
    mutable QHash<qint64, QVector<QPair<int,VertexPtr>>> vertexGrid;
    mutable bool                                         gridValid = false;

    static qint64 _gridKey(qint64 cx, qint64 cy) { return (cx << 32) ^ (cy & 0xffffffffLL); }
    void          _invalidateGrid() const { gridValid = false; }
    void          _rebuildGrid() const;
    bool          _isVertexIndexed(const VertexPtr & v) const;

    // adjacency index for fast edgeExists(v1,v2) lookup during merge-heavy
    // construction. Edge::sameAs(v1,v2) compares vertex identity only (not
    // position), so any edge matching a given (v1,v2) pair is guaranteed to
    // have v1 as one of its own endpoints - keying by raw Vertex* is safe
    // and avoids needing a shared_ptr hash. Maintained incrementally by
    // _insertEdgeSimple when valid; rebuilt lazily (full O(E) pass) on the
    // next edgeExists call otherwise. NOTE: only speeds up the
    // edgeExists(VertexPtr,VertexPtr) overload - edgeExists(QPointF,QPointF)
    // compares raw positions, not vertex identity, and still does the
    // original O(E) scan; it isn't on the insertEdge/mergeMap hot path.
    mutable QHash<Vertex*, QVector<EdgePtr>> edgeAdjacency;
    mutable bool                             adjacencyValid = false;

    void          _invalidateAdjacency() const { adjacencyValid = false; }
    void          _rebuildAdjacency() const;

    // spatial index for findIntersections: buckets edges by every cell
    // their bounding box overlaps, so a cutter only needs exact geometric
    // testing against edges near it instead of the whole map. Unlike
    // vertexGrid's fixed GRID_CELL (tolerance-driven), edgeGridCell is
    // sized adaptively from the map's current extent when the grid is
    // (re)built, since edge length varies with design scale.
    //
    // Must stay correct incrementally, not just lazily-invalidated-and-
    // rebuilt like edgeAdjacency: a stale grid wouldn't just be slow, it
    // would silently miss real intersections. New edges are bucketed at
    // insertion (_insertEdgeSimple); edges whose endpoint is reassigned by
    // a split (_splitTwoEdgesByVertex, processIntersections) are moved from
    // their old bucket set to their new one at the same call sites that
    // already patch edgeAdjacency for the same reason.
    mutable QHash<qint64, QVector<EdgePtr>> edgeGrid;
    mutable bool                            edgeGridValid = false;
    mutable qreal                           edgeGridCell  = 1.0;
    // edges.size() at the last rebuild, so findIntersections can tell when
    // the map has grown enough since then that edgeGridCell (average edge
    // length at that point) is worth re-measuring - see findIntersections.
    mutable qsizetype                       edgeGridBuiltAtCount = 0;

    void          _invalidateEdgeGrid() const { edgeGridValid = false; }
    void          _rebuildEdgeGrid() const;
    static void   _edgeBBox(const EdgePtr & e, qreal & minX, qreal & maxX, qreal & minY, qreal & maxY);
    void          _edgeCellRange(const EdgePtr & e, qint64 & cx0, qint64 & cx1, qint64 & cy0, qint64 & cy1) const;
    void          _addEdgeToGrid(const EdgePtr & e) const;
    void          _removeEdgeFromGrid(const EdgePtr & e) const;

    QString                     mname;
    WeakDCELPtr                 derivedDCEL;
    static QPointF              tmpCenter;
};
