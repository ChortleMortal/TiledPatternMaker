#pragma once
#ifndef EDGEPOLY_H
#define EDGEPOLY_H

#include <QPolygonF>
#include <QPen>

class QPainter;
class GeoGraphics;
class Circle;

typedef std::shared_ptr<QPolygonF>       PolyPtr;
typedef std::shared_ptr<class Edge>      EdgePtr;
typedef std::shared_ptr<class Vertex>    VertexPtr;

class EdgePoly : public QVector<EdgePtr>
{
public:
    EdgePoly();
    EdgePoly(const QPolygonF & poly);
    EdgePoly(QRectF & rect);
    EdgePoly(const Circle & circle);
    EdgePoly(PolyPtr pp);
    EdgePoly(const QVector<EdgePtr> & qvep);

    void set(const QPolygonF &poly);
    void set(QRectF & rect);
    void set(const Circle & circle);

    EdgePoly recreate() const;          // makes a new edge poly

    void     rotate(qreal angle);
    void     scale(qreal delta);
    void     mapD(QTransform T);        // maps this EdgePoly
    EdgePoly map(QTransform T) const;   // creates a new EdgePoly

    bool equals(const EdgePoly & other);

    bool isCorrect();
    bool isValid(bool rigorous = false);
    bool isClockwise() const;
    bool isClockwiseK();
    void reverseWindingOrder();
    void relink();

    QVector<VertexPtr> getVertices();
    QVector<QLineF>    getLines();
    QPolygonF   getPoly() const;        // closed
    QPolygonF   getPoints() const;      // not closed
    QPolygonF   getMids() const;        // not closed
    QRectF      getRect() const;
    QPointF     calcCenter();
    QPointF     calcIrregularCenter();
    QLineF      getEdge(int edge);
    qreal       getAngle(int edge);

    void    paint(QPainter *painter, QTransform T, bool annotate=false) const;
    void    draw(GeoGraphics * gg, QPen & pen);
    void    drawPts(GeoGraphics * gg, QPen & pen);

    void    dump() const;

protected:

private:
    void    init(const QPolygonF & poly);
};

#endif // EDGEPOLY_H
