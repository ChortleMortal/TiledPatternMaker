#ifndef EDGEPOLY_H
#define EDGEPOLY_H

#include <QPolygonF>
#include <QPen>

class QPainter;
class GeoGraphics;

typedef std::shared_ptr<QPolygonF>       PolyPtr;
typedef std::shared_ptr<class Edge>      EdgePtr;
typedef std::shared_ptr<class Vertex>    VertexPtr;


class EdgePoly : public QVector<EdgePtr>
{
public:
    EdgePoly();
    EdgePoly(QPolygonF & poly);
    EdgePoly(PolyPtr pp);

    EdgePoly recreate() const;          // makes a new edge poly

    void     rotate(qreal angle);
    void     scale(qreal delta);
    void     mapD(QTransform T);        // maps this EdgePoly
    EdgePoly map(QTransform T) const;   // creates a new EdgePoly

    bool equals(const EdgePoly & other);

    bool isValid(bool rigorous = false);
    bool isClockwise() const;
    void reverseWindingOrder();
    void relink();

    QVector<VertexPtr> getVertices();
    QPolygonF  getPoly() const;
    qreal      getAngle(int edge);

    int     numSwapped();

    void    paint(QPainter *painter, QTransform T);
    void    draw(GeoGraphics * gg, QPen pen);

    void    dump() const;

private:
    void    init(QPolygonF & poly);
};

#endif // EDGEPOLY_H
