#pragma once
#ifndef EDGEPOLY_H
#define EDGEPOLY_H

#include <QPolygonF>
#include <QPen>
#include <QPainterPath>

class QPainter;
class GeoGraphics;
class Circle;

typedef std::shared_ptr<QPolygonF>       PolyPtr;
typedef std::shared_ptr<class Edge>      EdgePtr;
typedef std::shared_ptr<class Vertex>    VertexPtr;

typedef QVector<EdgePtr> EdgeSet;

class EdgePoly
{
    friend class Tile;

public:
    EdgePoly();
    EdgePoly(const EdgePoly & ep);
    EdgePoly(const EdgeSet & edgeSet);
    EdgePoly(const QPolygonF & poly);
    EdgePoly(const QRectF & rect);
    EdgePoly(const Circle & circle);
    EdgePoly(const PolyPtr pp);
    ~EdgePoly();

    EdgePoly & operator=(const EdgePoly & other);
    bool operator == (const EdgePoly & other) const;
    bool operator != (const EdgePoly & other) const { return !(*this == other); }

    void create(const EdgeSet & edgeSet);
    void create(const QPolygonF & poly);
    void create(QRectF & rect);
    void create(const Circle & circle);
    void create(PolyPtr pp);

    void set(const EdgeSet & edgeSet);
    void set(const QPolygonF &poly);
    void set(QRectF & rect);
    void set(const Circle & circle);

    void compose();
    void decompose();
    void clear();

    EdgePoly recreate() const;          // makes a new edge poly

    void    setRotate(qreal angle);
    void    setScale(qreal not_delta);

    qreal   getScale()      { return scale; }
    qreal   getRotation()   { return rotation; }

    void     mapD(QTransform T);        // maps this epoly
    void     mapDB(QTransform T);       // maps this base
    EdgePoly map(QTransform T) const;   // creates a new EdgePoly

    bool isCorrect();
    bool isValid(bool rigorous = false);
    bool isClockwise() const;
    bool isClockwiseK();
    bool baseContains(const EdgePtr ep);
    bool epolyContains(const EdgePtr ep);

    void reverseWindingOrder();
    void relink();

    QPainterPath        getPainterPath() const;
    void                paint(QPainter *painter, QTransform T, bool annotate=false) const;

    const EdgeSet &     get() const          { return epoly; }
    const EdgeSet &     getBase()  const     { return base;  }
    EdgeSet &           getBaseRW()          { return base;  }
    QVector<VertexPtr>  getVertices();
    QVector<QLineF>     getLines();
    QPolygonF           getPolygon() const;     // closed
    QPolygonF           getPoints() const;      // not closed
    QPolygonF           getBasePoints() const;  // not closed
    QPolygonF           getMids() const;        // not closed
    QRectF              getRect() const;
    QPointF             calcCenter();
    QPointF             calcIrregularCenter();
    QLineF              getEdge(int edge);
    qreal               getAngle(int edge);
    QTransform          getTransform();

    uint                size()              { return epoly.size(); }
    uint                numPoints()         { return epoly.size(); }
    uint                numEdges()          { return epoly.size(); }

    void                dumpPts() const;
    void                dumpBaseEdges() const;
    void                dumpEpolyEdges() const;

protected:
    EdgeSet epoly;
    EdgeSet base;
    qreal   rotation;
    qreal   scale;

private:
    void    createBase(const QPolygonF & poly);
    void    copyFromBase();
    void    recreate(const EdgeSet & oldset, EdgeSet &newset);   // creates new shared aedge and vertex pointers

    QPainterPath        pp;
};

#endif // EDGEPOLY_H
