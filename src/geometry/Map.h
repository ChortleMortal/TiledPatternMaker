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

#include "misc/unique_qvector.h"
#include "geometry/circle.h"
#include "geometry/edgepoly.h"
#include "geometry/loose.h"
#include "legacy/shapes.h"
#include "geometry/neighbours.h"
#include "geometry/neighbour_map.h"

typedef std::shared_ptr<class Map>          MapPtr;
typedef std::shared_ptr<class NeighbourMap> NeighbourMapPtr;
typedef std::shared_ptr<const class Map>    constMapPtr;
typedef std::shared_ptr<class ShapeFactory> ShapeFPtr;
typedef std::shared_ptr<class Neighbours>   NeighboursPtr;
typedef std::shared_ptr<class DCEL>         DCELPtr;
typedef std::weak_ptr<class Edge>           WeakEdgePtr;

typedef std::weak_ptr<class DCEL>           WeakDCELPtr;

typedef QVector<QTransform>                 Placements;
enum eCompare
{
    COMP_LESS    = -1,
    COMP_EQUAL   = 0,
    COMP_GREATER = 1
};

enum eMCOptions
{
    badVertices_0               = 0x01,
    badVertices_1               = 0x02,
    badEdges                    = 0x04,
    joinupColinearEdges         = 0x10,
    divideupIntersectingEdges   = 0x20,
    cleanupNeighbours           = 0x40,
    buildNeighbours             = 0x80,
};

#define default_cleanse (badEdges | badVertices_0| cleanupNeighbours)

enum eMapError
{
    MAP_EMPTY,
    MAP_NO_EDGES,
    MAP_NO_VERTICES,

    EDGE_TRIVIAL_VERTICES,
    EDGE_TRIVIAL_POINTS,
    EDGE_VERTEX_UNKNOWN,
    EDGE_DUPLICATED,

    VERTEX_DUPLICATED,

    NEIGHBOUR_NO_EDGE,
    NEIGHBOUR_INVALID_EDGE,
    NEIGHBOUR_EXPIRED,
    NEIGHBOUR_BAD,

    MERR_EDGES_INTERSECTING
};

class Isect
{
public:
    Isect() {}
    Isect(EdgePtr e, EdgePtr c, VertexPtr v) { vertex=v; edge=e; cutter = c; }
    WeakEdgePtr edge;    // the interseted edge
    WeakEdgePtr cutter;  // the cutting edge
    VertexPtr vertex;    // the vertex for the intersection point
};

class Map
{
    #define MAP_EDGECOUNT_MAX 16

    friend class DCEL;

public:
    Map(const QString & name);
    Map(const QString & name, const QPolygonF & poly);
    Map(QString Name, const EdgePoly & poly);
    ~Map();

    void        draw(QPainter * painter);

    void        wipeout();          // reclaim memory
    void        set(const constMapPtr & other);  // replace contents
    MapPtr      copy() const;       // duplictes the contents
    MapPtr      recreate() const;   // make a new map with similar content
    MapPtr      getTransformed(const QTransform & T) const;

    // neighours
    NeighboursPtr getNeighbours(const VertexPtr & vert);

    //  verify/cleanse
    bool        verify(bool force = false);
    bool        verifyAndFix(bool force = false, bool confirm = false);
    void        cleanse(unsigned int options);
    uint        cleanseAnalysis();

    VertexPtr   insertVertex(const QPointF & pt);
    VertexPtr   getVertex(const QPointF & pt) const;

    EdgePtr     insertEdge(const QLineF & line);
    EdgePtr     insertEdge(const QPointF   & p1, const QPointF   & p2);
    EdgePtr     insertEdge(const VertexPtr & v1, const VertexPtr & v2);
    void        insertEdge(const EdgePtr & cutter);

    void        addShapeFactory(ShapeFPtr sf);

    // deletions
    void        removeVertex(const VertexPtr & v);
    void        removeVertexSimple(const VertexPtr & v);
    void        removeEdge(const EdgePtr & e);

    // modifications
    void        embedCrop(const QRectF & rect);
    void        embedCrop(const Circle & circle);

    void        cropOutside(const QRectF & rect);
    void        cropOutside(const QPolygonF & poly);
    void        cropOutside(Circle &circle);

    void        scale(qreal s);
    void        rotate(qreal r);
    void        translate(qreal x, qreal y);
    void        transformMap(QTransform T);

    void        splitEdge(EdgePtr e);

    void        mergeMap(const constMapPtr & other, qreal tolerance = Loose::TOL);
    void        mergeMany(const constMapPtr & other, const Placements & placements);
    EdgePtr     makeCopy(const EdgePtr & e);
    EdgePtr     makeCopy(const EdgePtr & e, QTransform T);

    void        removeMap(MapPtr other);

    // back-door
    void        XmlInsertDirect(VertexPtr v);
    void        XmlInsertDirect(EdgePtr e);

    // getters
    const QVector<VertexPtr>    & getVertices() { return vertices; }
    const QVector<EdgePtr>      & getEdges()    { return edges; }
    const QVector<QPair<QPointF,QString>> & getTexts()    { return debugTexts; }

    void            resetNeighbourMap() { nMap.reset(); }
    NeighbourMapPtr getNeighbourMap();
    void            setDerivedDCEL(DCELPtr dcel) { derivedDCEL = dcel; }
    DCELPtr         getDerivedDCEL()             { return derivedDCEL.lock(); }
    EdgePoly        getEdgePoly() const;

    // info
    QString     name() const { return mname; }
    QString     namedSummary() const;
    QString     summary() const;
    QString     displayVertexEdgeCounts();

    bool        isEmpty() const;
    int         numEdges() const;
    int         numVertices() const;
    bool        contains (const VertexPtr & v) const { return vertices.contains(v); }
    bool        contains (const EdgePtr & e)  const  { return edges.contains(e); }
    bool        hasIntersectingEdges() const;
    EdgePtr     edgeExists(const EdgePtr & edge) const;
    EdgePtr     edgeExists(const VertexPtr &  v1, const VertexPtr & v2) const;

    //debug
    QVector<eMapError> _verify(bool force = false);
    void        dumpMap(bool full=true);
    void        dumpErrors(const QVector<eMapError> &theErrors);

    static int  refs;

    void        deDuplicateEdges(const NeighboursPtr & vec);
    void        deDuplicateNeighbours();
    void        deDuplicateVertices(qreal tolerance);

protected:
    class Configuration       * config;

    UniqueQVector<VertexPtr>         vertices;
    UniqueQVector<EdgePtr>           edges;
    QVector<QPair<QPointF,QString>>  debugTexts;

private:
    // insertions
    void        _insertEdge(const EdgePtr & e);
    EdgePtr     _insertCurvedEdge(const VertexPtr & v1, const VertexPtr & v2, const QPointF & center, bool isConvex, bool isChord, bool debug);
    void        _insertEdge_Simple(const EdgePtr & edge );
    void        _insertPolygon(Polyform  * poly);
    void        _insertPolyline(Polyform * poly);

    // modifications
    void        _applyGeneralRigidMotion(QTransform T);
    void        _applyTrivialRigidMotion(QTransform T);

    void        _splitEdgesByVertex(const VertexPtr & vert);
    bool        _splitTwoEdgesByVertex(const VertexPtr & vert);

    void        _mergeVertices(const constMapPtr & other, qreal tolerance = Loose::TOL);
    void        _joinEdges(const EdgePtr & e1, const EdgePtr & e2);

    // getters
    VertexPtr   _getOrCreateVertex(const QPointF &pt);

    // debug
    void        _dumpVertices(bool full);
    void        _dumpEdges(bool full) const;

    // cleanse
    void        _cleanCopy() const;
    void        _cleanseVertices();

    // cleanse operations
    void        joinColinearEdges();       // if two lines are straight but have a vertex, then combine lines and delete vertex
    bool        joinOneColinearEdge();
    void        divideIntersectingEdges(); // if two edges cross, make a new vertex and have four edges
    void        combineLinearEdges(const EdgePtr & a, const EdgePtr & b, const VertexPtr & common);
    void        removeVerticesWithEdgeCount(uint edgeCount);
    bool        coalesceVertices(qreal tolerance = Loose::TOL);
    void        removeBadEdges();

    // debug verify operations
    void        verifyEdges();
    void        verifyNeighbours();
    bool        procErrors(const QVector<eMapError> &errors);
    bool        isSevereError(eMapError err);
    bool        isMinorError(eMapError err);

    // utilities
    static eCompare  comparePoints(const QPointF &a, const QPointF &b, qreal tolerance = Loose::TOL);
    static bool      vertexAngleGreaterThan(const VertexPtr & a, const VertexPtr & b);

    int         vertexIndex(const VertexPtr & v) const { return vertices.indexOf(v); }
    int         edgeIndex(const EdgePtr & e)     const { return edges.indexOf(e); }

    QString                     mname;
    WeakDCELPtr                 derivedDCEL;
    UniqueQVector<eMapError>    errors;
    static QPointF              tmpCenter;
    NeighbourMapPtr             nMap;
};

class DebugMap : public Map
{
public:
    DebugMap(const QString & name);
//    DebugMap(const QString & name, const QPolygonF & poly);
//    DebugMap(QString Name, const EdgePoly & poly);

    void        insertDebugMark(QPointF m, QString txt);
    void        insertDebugLine(EdgePtr edge);
    void        insertDebugLine(QPointF p1, QPointF p2);
    void        insertDebugLine(QLineF l1);
    void        insertDebugPolygon(QPolygonF & poly);
    void        insertDebugPoints(QPolygonF & poly);
};

#endif
