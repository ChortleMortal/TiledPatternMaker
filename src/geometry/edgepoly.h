#ifndef EDGEPOLY_H
#define EDGEPOLY_H

#include "base/shared.h"
#include "geometry/edge.h"

class QPainter;
class GeoGraphics;

class EdgePoly : public QVector<EdgePtr>
{
public:
    EdgePoly();
    EdgePoly(QPolygonF & poly);
    EdgePoly(PolyPtr pp);

    EdgePoly recreate() const;          // makes a new edge poly

    void     rotate(qreal angle);
    void     mapD(QTransform T);        // maps this EdgePoly
    EdgePoly map(QTransform T) const;   // creates a new EdgePoly

    bool equals(const EdgePoly & other);

    bool isValid();
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
