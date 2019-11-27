#ifndef EDGEPOLY_H
#define EDGEPOLY_H

#include "base/shared.h"
#include "geometry/Edge.h"

class QPainter;
class GeoGraphics;

class EdgePoly : public QVector<EdgePtr>
{
public:
    EdgePoly();
    EdgePoly(QPolygonF & poly);
    EdgePoly(PolyPtr pp);

    void     mapD(QTransform T);
    EdgePoly map(QTransform T);

    bool equals(const EdgePoly & other);
    bool isClockwise();    // should be anti-clockwise

    QPolygonF  getPoly();
    qreal      getAngle(int edge);

    void    paint(QPainter *painter, QTransform T);
    void    draw(GeoGraphics * gg, QPen pen);

private:
    void    init(QPolygonF & poly);
};

#endif // EDGEPOLY_H
