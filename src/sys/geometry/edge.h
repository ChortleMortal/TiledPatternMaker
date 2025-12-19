#pragma once
#ifndef EDGE_H
#define EDGE_H

////////////////////////////////////////////////////////////////////////////
//
// Edge.java
//
// The edge component of the planar map abstraction.

#include <QPointF>
#include <QLineF>

#include "sys/enums/edgetype.h"
#include "sys/geometry/arcdata.h"
#include "sys/sys.h"

typedef std::shared_ptr<class Edge>         EdgePtr;
typedef QVector<EdgePtr>                    EdgeSet;

typedef std::shared_ptr<class Vertex>       VertexPtr;
typedef std::shared_ptr<class Thread>       ThreadPtr;
typedef std::shared_ptr<class ArcData>      ArcDataPtr;
typedef std::shared_ptr<class Face>         FacePtr;

typedef std::weak_ptr<class Edge>           WeakEdgePtr;
typedef std::weak_ptr<class Vertex>         WeakVertexPtr;
typedef std::weak_ptr<class Face>           WeakFacePtr;
typedef std::weak_ptr<class Thread>         WeakThreadPtr;

class Edge
{
public:
    Edge();
    Edge(const VertexPtr & V1);
    Edge(const VertexPtr & V1, const VertexPtr & V2 );
    Edge(const VertexPtr & V1, const VertexPtr & V2, const QPointF & arcCenter, eCurveType ctype);
    Edge(const EdgePtr & other);
    Edge(const EdgePtr & other, QTransform T);
    Edge(const Edge & other);
    ~Edge();

    eEdgeType getType();
    bool      isLine();
    bool      isCurve();

    bool      isTwin(EdgePtr e) { return ((e->v1 == v2) && (e->v2 == v1)); }
    EdgePtr   createTwin();

    eSide     side(VertexPtr v);
    eSide     side(QPointF p);

    double    angle() const;

    VertexPtr getOtherV(const VertexPtr & vert) const;
    VertexPtr getOtherV(const QPointF & pos) const;
    QPointF   getOtherP(const VertexPtr & vert) const;
    QPointF   getOtherP(const QPointF & pos) const;
    QLineF    getLine();
    QPointF   getMidPoint()  { return getLine().pointAt(0.50); }
    qreal     getAngle();

    void      setV1(const VertexPtr & v);
    void      setV2(const VertexPtr & v);

    void      convertToCurve(eCurveType ctype);

    void      chgangeToCurvedEdge(QPointF arcCenter, eCurveType ctype);
    void      changeCurveType(eCurveType ctype);
    void      resetCurveToLine();

    inline ArcData  & getArcData()             { return arcData; }
    inline eCurveType getCurveType()     const { return arcData.getCurveType(); }
    inline QPointF    getArcCenter()     const { return arcData.getCenter(); }
    inline qreal      getArcMagnitude()  const { return arcData.magnitude(); }
    inline qreal      getArcSpan()       const { return arcData.span(); }
    inline bool       isConvex()         const { return arcData.getCurveType() == CURVE_CONVEX; }
    inline bool       isConcave()        const { return arcData.getCurveType() == CURVE_CONCAVE; }

    qreal     getRadius();
    bool      pointWithinArc(QPointF pt)  { return arcData.pointWithinArc(pt); }

    bool      isTrivial(qreal tolerance = Sys::TOL);
    bool      isColinearAndTouching(const EdgePtr & e, qreal tolerance = Sys::NEAR_TOL);
    bool      isColinear(const EdgePtr & e, qreal tolerance = Sys::NEAR_TOL);

    bool      contains(const VertexPtr & v);
    bool      contains(const QPointF & p);

    bool      sameAs(const EdgePtr & other);
    bool      sameAs(const VertexPtr & ov1, const VertexPtr & ov2);
    bool      sameAs(const QPointF & op1, const QPointF & op2);
    bool      equals(const EdgePtr & other);

    // Used to sort the edges in the map.
    qreal     getMinX();
    qreal     getMaxX();

    QString   info();
    QString   summary();
    void      dump();

    // dcel stuff
    bool operator < (const Edge & other) const;

    EdgePtr    prev() { return twin.lock()->next.lock(); }

public:
    VertexPtr       v1;
    VertexPtr       v2;

    WeakEdgePtr     twin;
    WeakEdgePtr     next;
    WeakFacePtr     incident_face;

    bool            dvisited;       // used by dcel

    static int      refs;

    uint            casingIndex;    // for debug
    static bool     curvesAsLines;  // for debug

protected:

private:
    eEdgeType       _type;
    ArcData         arcData;
};

#endif

