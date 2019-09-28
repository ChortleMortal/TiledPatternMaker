/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SHAPEFACTORY_H
#define SHAPEFACTORY_H

#include "designs/shapes.h"
#include "viewers/shapeviewer.h"

#include <QGraphicsItem>
#include <QPainter>

class Canvas;


class ShapeFactory : public ShapeViewer
{
public:
    ShapeFactory(qreal diameter, QPointF loc = QPointF());
    ~ShapeFactory();

    void reset() { polyforms.clear(); }

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

    void enableAntialiasing(bool enable) {_antiAliasPolys = enable;}

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
    Canvas * canvas;
};


#endif // SHAPEFACTORY_H
