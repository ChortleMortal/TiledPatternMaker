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

#include "enums/edgetype.h"
#include "geometry/arcdata.h"
#include "misc/sys.h"

typedef std::shared_ptr<class Vertex>       VertexPtr;
typedef std::shared_ptr<class Edge>         EdgePtr;
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
    Edge(const VertexPtr & V1, const VertexPtr & V2, const QPointF & arcCenter, bool convex, bool chord);
    Edge(const EdgePtr & other);
    Edge(const EdgePtr & other, QTransform T);
    Edge(const Edge & other);
    ~Edge();

    eEdgeType getType()      { return type; }
    bool      isLine()       { return (type == EDGETYPE_LINE); }
    bool      isCurve()      { return (type == EDGETYPE_CURVE) || type == EDGETYPE_CHORD; }

    EdgePtr   createTwin();

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

    void      resetCurveToLine();
    void      convertToConvexCurve();

    void      setCurvedEdge(QPointF arcCenter, bool convex, bool chord);
    void      setConvex(bool convex);
    void      setArcMagnitude(qreal magnitude);
    void      calcMagnitude();

    inline ArcData & getArcData()             { return arcData; }
    inline bool      isConvex()         const { return arcData.convex(); }
    inline QPointF   getArcCenter()     const { return arcData.getCenter(); }
    inline qreal     getArcMagnitude()  const { return arcData.magnitude; }
    inline qreal     getArcSpan()       const { return arcData.span(); }

    qreal     getRadius();
    bool      pointWithinArc(QPointF pt)  { return arcData.pointWithinArc(pt); }

    bool      isTrivial(qreal tolerance = Sys::TOL);
    bool      isColinearAndTouching(const EdgePtr & e, qreal tolerance = Sys::NEAR_TOL);
    bool      isColinear(const EdgePtr & e, qreal tolerance = Sys::NEAR_TOL);

    bool      contains(const VertexPtr & v);
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
    bool            visited;        // used by interlace and interlace threads
    bool            v1_under;       // used by interlace
    WeakThreadPtr   thread;         // used by interlace

    static int      refs;

protected:
    eEdgeType       type;

private:
    ArcData         arcData;
};

#endif

