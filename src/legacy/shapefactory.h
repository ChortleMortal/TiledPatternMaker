#ifndef SHAPEFACTORY_H
#define SHAPEFACTORY_H

#include "legacy/shapes.h"
#include "viewers/shape_view.h"
#include <QGraphicsItem>
#include <QPainter>

class Canvas;


class ShapeFactory : public ShapeViewer
{
public:
    ShapeFactory(qreal diameter, QPointF loc = QPointF());
    ~ShapeFactory();

    void reset() { polyforms.clear(); Layer::reset(); }

    // points
    void getHexPackingPoints(QPolygonF &pf);
    void getOctGridPoints(QPolygonF &pf);

    // lines
    Polyline2 * addLine(QPen Pen, QLineF line);
    Polyline2 * addLine(QPen Pen, QPointF a, QPointF b);

    // circles
    Circle2 * addCircle(qreal diameter, QPen pen = QPen(Qt::NoPen), QBrush brush = QBrush(Qt::NoBrush));
    Circle2 * addCircle(const Circle2 & Circle);

    // polygons
    Polygon2 * addPolygon(QPen Pen, QBrush Brush, QPolygonF points);
    Polygon2 * addPolygon(const Polygon2 & Poly);

    // polylines
    Polyline2 * addPolyline(QPen Pen, QPolygonF points);
    Polyline2 * addPolyline(const Polyline2 & Poly);

    // triangles
    Polygon2 * addInscribedTriangle(    QPen Pen, QBrush Brush, qreal Angle = 0.0);
    Polygon2 * addCircumscribedTriangle(QPen Pen, QBrush Brush, qreal Angle = 0.0);

    // squares/rectangles
    Polygon2 * addInscribedSquare(            QPen Pen, QBrush Brush, qreal Angle = 0.0);
    Polygon2 * addCircumscribedSquare(        QPen Pen, QBrush Brush, qreal Angle = 0.0);
    Polygon2 * addInscribedRectangleInOctagon(QPen Pen, QBrush Brush, qreal Angle = 0.0);
    Polygon2 * addInscribedSquareInOcatgon(   QPen Pen, QBrush Brush, qreal Angle = 0.0);

    // pentagons
    Polygon2 * addInscribedPentagon(QPen Pen, QBrush Brush, qreal Angle = 0.0);

    // hexagons
    Polygon2 * addInscribedHexagon(        QPen Pen, QBrush Brush, qreal Angle = 0.0);
    Polygon2 * addCircumscribedHexagon(    QPen Pen, QBrush Brush, qreal Angle = 0.0);
    Polygon2 * addExternalHexagon(         QPen Pen, QBrush Brush, qreal Angle = 0.0);
    Polygon2 * addStretchedExternalHexagon(QPen Pen, QBrush Brush, qreal Angle = 0.0);
    void       add6ptStar(                 QPen pen, QBrush brush, qreal Angle = 0.0);

    // septagons
    Polygon2 *  addInscribedHepatgon(QPen Pen, QBrush Brush, qreal Angle = 0.0);

    // octagons
    Polygon2 * addInscribedOctagon(     QPen Pen, QBrush Brush, qreal Angle = 0.0);
    Polygon2 * addCircumscribedOctagon( QPen Pen, QBrush Brush, qreal Angle = 0.0);

    // nonagons
    Polygon2 *  addInscribedNonagon(  QPen Pen, QBrush Brush, qreal Angle = 0.0);
    void        addInscribedEnneagram(QPen Pen, QBrush Brush, qreal Angle = 0.0);
    void        add9ptStar(           QPen pen, QBrush brush, qreal Angle = 0.0);

    // painter path
    QPainterPath & getPPath() { return ppath;}
    void           setPPath(QPen pen = QPen(Qt::NoPen), QBrush brush = QBrush(Qt::NoBrush))
                            {ppPen = pen; ppBrush = brush;}

    static QPointF   rotatePoint(QPointF & p, qreal angle);
    static QPolygonF getMidPoints(QPolygonF & p);

protected:

    // triangles
    void calcInscribedTrianglePoints(Polygon2 * p);
    void calcCircumscribedTrianglePoints(Polygon2 * p);

    // squares
    void calcInscribedSquarePoints(Polygon2 * p);
    void calcCircumscribedSquarePoints(Polygon2 * p);

    // pentagons
    void calcInscribedPentagonPoints(Polygon2 * p);

    // hexagons
    void calcInscribedHexagonPoints(Polygon2 * p);
    void calcCircumscribedHexagonPoints(Polygon2 * p);
    void calcHexPackingPoints(Polygon2 * p);
    void calcExtendedHexPackingPoints(Polygon2 *P);

    // septagons
    void calcInscribedHeptagonPoints(Polygon2 * p);

    // octagons
    void calcInscribedOctagonPoints(Polygon2 * p);
    void calcCircumscribedOctagonPoints(Polygon2 * p);
    void calcOctPackingPoints(Polygon2 * p);
    void calcInscribedRectangleInOctagon(Polygon2* p);
    void calcInscribedSquareInOcatgon(Polygon2 * p);

    // nonagons
    void calcInscribedNonagonPoints(Polygon2 * p);

private:
    class ViewControl * view;

    qreal  _diameter;
    qreal  _radius;
};


#endif // SHAPEFACTORY_H
