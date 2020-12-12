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

#include "designs/shapes.h"
#include "designs/shapefactory.h"
#include "designs/patterns.h"
#include "base/configuration.h"
#include "viewers/view.h"
#include <QDebug>

ShapeFactory::ShapeFactory(qreal diameter, QPointF loc)
{
    view = View::getInstance();

    setLoc(loc);
    _diameter       = diameter;
    _radius         = diameter/2.0;

}

ShapeFactory::~ShapeFactory()
{
}

///////////////////////////////////////////////////////////////////////
//
// public adders
//
///////////////////////////////////////////////////////////////////////

Polyline2 * ShapeFactory::addLine(QPen Pen, QLineF line)
{
    Polyline2 * p = new Polyline2(Pen);
    *p << line.p1() << line.p2();
    addPolyform(p);
    //prepareGeometryChange();
    view->update();
    return p;
}

Polyline2 * ShapeFactory::addLine(QPen Pen, QPointF a, QPointF b)
{
    Polyline2 * p = new Polyline2(Pen);
    *p << a << b;
    addPolyform(p);
    //prepareGeometryChange();
    view->update();
    return p;
}

Circle2 * ShapeFactory::addCircle(qreal diameter, QPen pen, QBrush brush)
{
    Circle2 * c = new Circle2(diameter,pen,brush);
    addPolyform(c);
    //prepareGeometryChange();
    view->update();
    return c;
}

Circle2 * ShapeFactory::addCircle(const Circle2 & Circle)
{
    Circle2 *c = new Circle2(Circle);
    *c = Circle;
    addPolyform(c);
    //prepareGeometryChange();
    view->update();
    return c;
}

Polygon2 * ShapeFactory::addPolygon(QPen Pen, QBrush Brush, QPolygonF points)
{
    Polygon2 * p = new Polygon2(Pen,Brush);
    *p << points;
    addPolyform(p);
    //prepareGeometryChange();
    view->update();
    return p;
}

Polygon2 * ShapeFactory::addPolygon(const Polygon2 & Poly)
{
    Polygon2 * p = new Polygon2();
    *p = Poly;
    addPolyform(p);
    //prepareGeometryChange();
    view->update();
    return p;
}

Polyline2 * ShapeFactory::addPolyline(QPen Pen, QPolygonF points)
{
    Polyline2 * p = new Polyline2(Pen);
    *p << points;
    addPolyform(p);
    //prepareGeometryChange();
    view->update();
    return p;
}

Polyline2 * ShapeFactory::addPolyline(const Polyline2 & Poly)
{
    Polyline2 * p = new Polyline2();
    *p = Poly;
    addPolyform(p);
    //prepareGeometryChange();
    view->update();
    return p;
}

Polygon2 * ShapeFactory::addInscribedTriangle(QPen Pen, QBrush Brush,qreal Angle)
{
    Polygon2 * p = new Polygon2(Pen,Brush);
    calcInscribedTrianglePoints(p);
    p->rotate(Angle);
    // try rotation of QGraphicsLineItem to find (rotated) points and then connect them in different orders
    addPolyform(p);
    //prepareGeometryChange();
    view->update();
    return p;
}

Polygon2 * ShapeFactory::addCircumscribedTriangle(QPen Pen, QBrush Brush, qreal Angle)
{
    Polygon2 * p = new Polygon2(Pen,Brush);
    calcCircumscribedTrianglePoints(p);
    p->rotate(Angle);
    addPolyform(p);
    //prepareGeometryChange();
    view->update();
    return p;
}

Polygon2 * ShapeFactory::addInscribedSquare(QPen Pen, QBrush Brush, qreal Angle)
{
    Polygon2 * p = new Polygon2(Pen,Brush);
    calcInscribedSquarePoints(p);
    p->rotate(Angle);
    addPolyform(p);
    //prepareGeometryChange();
    view->update();
    return p;
}

Polygon2 * ShapeFactory::addCircumscribedSquare(QPen Pen, QBrush Brush, qreal Angle)
{
    Polygon2 * p = new Polygon2(Pen,Brush);
    calcCircumscribedSquarePoints(p);
    p->rotate(Angle);
    addPolyform(p);
    //prepareGeometryChange();
    view->update();
    return p;
}

Polygon2 * ShapeFactory::addInscribedPentagon(QPen Pen, QBrush Brush, qreal Angle)
{
    Polygon2 * p = new Polygon2(Pen,Brush);
    calcInscribedPentagonPoints(p);
    p->rotate(Angle);
    addPolyform(p);
    //prepareGeometryChange();
    view->update();
    return p;
}

Polygon2 * ShapeFactory::addInscribedHexagon(QPen Pen, QBrush Brush, qreal Angle)
{
    Polygon2 * p = new Polygon2(Pen,Brush);
    calcInscribedHexagonPoints(p);
    p->rotate(Angle);
    addPolyform(p);
    //prepareGeometryChange();
    view->update();
    return p;
}

Polygon2 * ShapeFactory::addCircumscribedHexagon(QPen Pen, QBrush Brush, qreal Angle)
{
    Polygon2 * p = new Polygon2(Pen,Brush);
    calcCircumscribedHexagonPoints(p);
    p->rotate(Angle);
    addPolyform(p);
    //prepareGeometryChange();
    view->update();
    return p;
}

Polygon2 * ShapeFactory::addExternalHexagon(QPen Pen, QBrush Brush, qreal Angle)
{
    Polygon2 * p = new Polygon2(Pen,Brush);
    calcHexPackingPoints(p);
    p->rotate(Angle);
    addPolyform(p);
    //prepareGeometryChange();
    view->update();
    return p;
}

Polygon2 * ShapeFactory::addStretchedExternalHexagon(QPen Pen, QBrush Brush, qreal Angle)
{
    Polygon2 * p = new Polygon2(Pen,Brush);
    calcExtendedHexPackingPoints(p);
    p->rotate(Angle);
    addPolyform(p);
    //prepareGeometryChange();
    view->update();
    return p;
}


Polygon2 *  ShapeFactory::addInscribedHepatgon(QPen Pen, QBrush Brush, qreal Angle)
{
    Polygon2 * p = new Polygon2(Pen,Brush);
    calcInscribedHeptagonPoints(p);
    p->rotate(Angle);
    addPolyform(p);
    //prepareGeometryChange();
    view->update();
    return p;
}

Polygon2 * ShapeFactory::addInscribedOctagon(QPen Pen, QBrush Brush, qreal Angle)
{
    Polygon2 * p = new Polygon2(Pen,Brush);
    calcInscribedOctagonPoints(p);
    p->rotate(Angle);
    addPolyform(p);
    //prepareGeometryChange();
    view->update();
    return p;
}

Polygon2 * ShapeFactory::addCircumscribedOctagon(QPen Pen, QBrush Brush, qreal Angle)
{
    Polygon2 * p = new Polygon2(Pen,Brush);
    calcCircumscribedOctagonPoints(p);
    p->rotate(Angle);
    addPolyform(p);
    //prepareGeometryChange();
    view->update();
    return p;
}

Polygon2 *  ShapeFactory::addInscribedNonagon(  QPen Pen, QBrush Brush, qreal Angle)
{
    Polygon2 * p = new Polygon2(Pen,Brush);
    calcInscribedNonagonPoints(p);
    p->rotate(Angle);
    addPolyform(p);
    //prepareGeometryChange();
    view->update();
    return p;
}

void ShapeFactory::addInscribedEnneagram(QPen Pen, QBrush Brush, qreal Angle)
{
    Polygon2  p(Pen,Brush);
    calcInscribedNonagonPoints(&p);
    p.rotate(Angle);

    Pen.setColor(QColor(Qt::red));
    addCircle(_diameter,Pen);

    Pen.setCapStyle(Qt::RoundCap);
    Pen.setJoinStyle(Qt::BevelJoin);

    // connect 142857 and 963
    Pen.setColor(QColor(Qt::blue));
    Polygon2 * p3 = new Polygon2(Pen,Brush);
    *p3 << p.at(0) << p.at(6) << p.at(3);
    addPolyform(p3);

    Pen.setColor(QColor(Qt::green));
    Polygon2 * p2 = new Polygon2(Pen,Brush);
    *p2 << p.at(1) << p.at(4) << p.at(2) << p.at(8) << p.at(5) << p.at(7);
    addPolyform(p2);

    //prepareGeometryChange();
    view->update();
}

void ShapeFactory::add6ptStar(QPen pen, QBrush brush, qreal Angle)
{
    addInscribedTriangle(pen,brush,0.0  + Angle);
    addInscribedTriangle(pen,brush,60.0 + Angle);
}

void ShapeFactory::add9ptStar(QPen pen, QBrush brush, qreal Angle)
{
    addInscribedTriangle(pen,brush,0.0  + Angle);
    addInscribedTriangle(pen,brush,40.0 + Angle);  //40
    addInscribedTriangle(pen,brush,80.0 + Angle);  //80
}

Polygon2 * ShapeFactory::addInscribedRectangleInOctagon(QPen Pen, QBrush Brush, qreal Angle)
{
    Polygon2 * p = new Polygon2(Pen,Brush);
    calcInscribedRectangleInOctagon(p);
    p->rotate(Angle);
    addPolyform(p);
    //prepareGeometryChange();
    view->update();
    return p;
}

Polygon2 * ShapeFactory::addInscribedSquareInOcatgon(QPen Pen, QBrush Brush, qreal Angle)
{
    Polygon2 * p = new Polygon2(Pen,Brush);
    calcInscribedSquareInOcatgon(p);
    p->rotate(Angle);
    addPolyform(p);
    //prepareGeometryChange();
    view->update();
    return p;
}

///////////////////////////////////////////////////////////////////////
//
// getters
//
///////////////////////////////////////////////////////////////////////


void ShapeFactory::getHexPackingPoints(QPolygonF & pf)
{
    Polygon2 p;
    calcHexPackingPoints(&p);
    pf = p;
}

void ShapeFactory::getOctGridPoints(QPolygonF &pf)
{
    Polygon2 p;
    calcOctPackingPoints(&p);
    pf = p;
}

///////////////////////////////////////////////////////////////////////
//
// calc triangles
//
///////////////////////////////////////////////////////////////////////

void ShapeFactory::calcInscribedTrianglePoints(Polygon2 *p)
{
    // half triangle sides 1,2,sqrt 3
    p->clear();
    *p <<  QPointF(0,-_radius);
    *p <<  QPointF((0.75 *  _diameter) / sqrt(3), _diameter/4);
    *p <<  QPointF((0.75 * -_diameter) / sqrt(3), _diameter/4);
}

void ShapeFactory::calcCircumscribedTrianglePoints(Polygon2 * p)
{
    qreal sideLength = _radius * sqrt(3) * 2.0;
    qreal height  = sqrt((sideLength*sideLength) - ((sideLength/2.0)*(sideLength/2.0)));

    p->clear();
    *p <<  QPointF(0,-(height-_radius));
    *p <<  QPointF(sideLength /2.0,_radius);
    *p <<  QPointF(-sideLength/2.0, _radius);
}

///////////////////////////////////////////////////////////////////////
//
// calc squares/rects
//
///////////////////////////////////////////////////////////////////////

void ShapeFactory::calcInscribedSquarePoints(Polygon2 * p)
{
    qreal pos = sqrt((_diameter*_diameter)/2.0)/2.0;

    p->clear();
    *p <<   QPointF ( pos,  -pos);
    *p <<   QPointF ( pos,   pos);
    *p <<   QPointF (-pos,   pos);
    *p <<   QPointF (-pos,  -pos);
}

void ShapeFactory::calcCircumscribedSquarePoints(Polygon2 * p)
{
    p->clear();
    *p <<  QPointF ( _radius, -_radius);
    *p <<  QPointF ( _radius,  _radius);
    *p <<  QPointF (-_radius,  _radius);
    *p <<  QPointF (-_radius, -_radius);
}

void ShapeFactory::calcInscribedSquareInOcatgon(Polygon2 * p)
{
       qreal side  = _diameter * qTan(M_PI/8.0);

       p->clear();
       *p << QPointF(  (side*0.5),        -(side*0.5));
       *p << QPointF(  (side*0.5),         (side*0.5));
       *p << QPointF(- (side*0.5),         (side*0.5));
       *p << QPointF(- (side*0.5),        -(side*0.5));
}

void ShapeFactory::calcInscribedRectangleInOctagon(Polygon2 * p)
{
       qreal side  = _diameter * qTan(M_PI/8.0);
       qreal piece = sqrt(side*side*0.5);

       p->clear();
       *p << QPointF(  (side*0.5),        -(side*0.5) - piece);     //0
       *p << QPointF(  (side*0.5),         (side*0.5) + piece);     //3
       *p << QPointF(- (side*0.5),         (side*0.5) + piece);     //4
       *p << QPointF(- (side*0.5),        -(side*0.5) - piece);     //7
}

///////////////////////////////////////////////////////////////////////
//
// calc pentagons
//
///////////////////////////////////////////////////////////////////////

void ShapeFactory::calcInscribedPentagonPoints(Polygon2 * p)
{
    p->clear();

    int n = 5;
    for (int i = 0; i < n; i++)
    {
        // generic formula for polygon vertices
        QPointF pt = QPointF(_radius * qCos(2 * M_PI * i / n), _radius * qSin(2 * M_PI * i / n));
        //qDebug() << pt;
        *p << rotatePoint(pt,-(108.0 - 90.0));  // rotate 18 degrees
    }
}


///////////////////////////////////////////////////////////////////////
//
// calc hexagons
//
///////////////////////////////////////////////////////////////////////


void ShapeFactory::calcInscribedHexagonPoints(Polygon2 * p)
{
    qreal yPos  = sqrt((_radius*_radius) - ((_radius/2.0)*(_radius/2.0)));

    p->clear();
    *p << QPointF(  (_diameter * 0.25),     - yPos);
    *p << QPointF(  _radius,                  0.0);
    *p << QPointF(  (_diameter * 0.25),       yPos);
    *p << QPointF(- (_diameter * 0.25),       yPos);
    *p << QPointF(- _radius,                  0.0);
    *p << QPointF(- (_diameter * 0.25),     - yPos);
}

void ShapeFactory::calcCircumscribedHexagonPoints(Polygon2 * p)
{
    qreal side = _diameter * qTan(M_PI/6.0);

    p->clear();
    *p << QPointF(  side*0.5,   - _radius);
    *p << QPointF(  side,         0.0);
    *p << QPointF(  side*0.5,     _radius);
    *p << QPointF(- (side*0.5),   _radius);
    *p << QPointF(- (side),       0.0);
    *p << QPointF(- (side*0.5), - _radius);
}

void ShapeFactory::calcHexPackingPoints(Polygon2 * p)
{
    // packing of circles has equlateral triangle of centers
    p->clear();

    qreal ypos = sqrt((_diameter*_diameter) - (_radius*_radius));

    *p << QPointF(_radius,     -ypos);   // top right
    *p << QPointF(_diameter,   0);       // right
    *p << QPointF(_radius,     ypos);    // below right
    *p << QPointF(-_radius,    ypos);    // below left
    *p << QPointF(-_diameter,  0);       // left
    *p << QPointF(-_radius,    -ypos);   // top left
}

void ShapeFactory::calcExtendedHexPackingPoints(Polygon2 * p)
{
    // packing of circles has equlateral triangle of centers
    p->clear();

    qreal ypos = sqrt((_diameter*_diameter) - (_radius*_radius));
    //qDebug() << "ypos" << ypos << _diameter << _radius;
    *p << QPointF(_radius,     -_radius);   // top right
    *p << QPointF(ypos,   0);          // right
    *p << QPointF(_radius,     _radius);    // below right
    *p << QPointF(-_radius,    _radius);    // below left
    *p << QPointF(-ypos,  0);          // left
    *p << QPointF(-_radius,    -_radius);   // top left
}

///////////////////////////////////////////////////////////////////////
//
// calc heptagons (not septagons)
//
///////////////////////////////////////////////////////////////////////

void ShapeFactory::calcInscribedHeptagonPoints(Polygon2 * p)
{
    p->clear();

    int n = 7;
    for (int i = 0; i < n; i++)
    {
        // generic formula for polygon vertice
        QPointF pt = QPointF(_radius * qCos(2 * M_PI * i / n), _radius * qSin(2 * M_PI * i / n));
        //qDebug() << pt;
        *p << rotatePoint(pt,-90.0);
    }
}

///////////////////////////////////////////////////////////////////////
//
// calc octagons
//
///////////////////////////////////////////////////////////////////////

void ShapeFactory::calcInscribedOctagonPoints(Polygon2 * p)
{
    qreal dist  = _radius * qCos(qDegreesToRadians(22.5));
    qreal hside = _radius * qSin(qDegreesToRadians(22.5));


    p->clear();
    *p << QPointF(  hside,     - dist);
    *p << QPointF(  dist,      - hside);
    *p << QPointF(  dist,        hside);
    *p << QPointF(  hside,       dist);
    *p << QPointF(- hside,       dist);
    *p << QPointF(- dist,        hside);
    *p << QPointF(- dist,      - hside);
    *p << QPointF(- hside,     - dist);

}

void ShapeFactory::calcCircumscribedOctagonPoints(Polygon2 *p)
{
    qreal side  = _diameter * qTan(M_PI/8.0);
    qreal piece = sqrt(side*side*0.5);

    p->clear();
    *p << QPointF(  (side*0.5),        -(side*0.5) - piece);     //0
    *p << QPointF(  (side*0.5)+piece,  -(side*0.5));             //1
    *p << QPointF(  (side*0.5)+piece,   (side*0.5));             //2
    *p << QPointF(  (side*0.5),         (side*0.5) + piece);     //3
    *p << QPointF(- (side*0.5),         (side*0.5) + piece);     //4
    *p << QPointF(- (side*0.5)-piece,   (side*0.5));             //5
    *p << QPointF(- (side*0.5)-piece,  -(side*0.5));             //6
    *p << QPointF(- (side*0.5),        -(side*0.5) - piece);     //7
}

void ShapeFactory::calcOctPackingPoints(Polygon2 *p)
{
    p->clear();
    *p << QPointF(  _diameter,   - _diameter);
    *p << QPointF(  _diameter,     0.0);
    *p << QPointF(  _diameter,     _diameter);
    *p << QPointF(  0.0,           _diameter);
    *p << QPointF(- _diameter,     _diameter);
    *p << QPointF(- _diameter,    0.0);
    *p << QPointF(- _diameter,   - _diameter);
    *p << QPointF(  0.0,         - _diameter);
}

///////////////////////////////////////////////////////////////////////
//
// calc nonagons
//
///////////////////////////////////////////////////////////////////////

void ShapeFactory::calcInscribedNonagonPoints(Polygon2 * p)
{
    p->clear();

    int n = 9;
    for (int i = 0; i < n; i++)
    {
        // generic formula for polygon vertice
        QPointF pt = QPointF(_radius * qCos(2 * M_PI * i / n), _radius * qSin(2 * M_PI * i / n));
        //qDebug() << pt;
        *p << rotatePoint(pt,-90.0);
    }
}

///////////////////////////////////////////////////////////////////////
//
// other
//
///////////////////////////////////////////////////////////////////////

QPointF ShapeFactory::rotatePoint(QPointF & p, qreal angle)
{
  qreal s = qSin(qDegreesToRadians(angle));
  qreal c = qCos(qDegreesToRadians(angle));


  qreal x = (p.x() * c) - (p.y() * s);
  qreal y = (p.x() * s) + (p.y() * c);

  return QPointF(x,y);
}

QPolygonF ShapeFactory::getMidPoints(QPolygonF & p)
{
    QPolygonF result;

    for (int i=0; i < p.count(); i++ )
    {
        int next = i+1;
        if (next == p.count()) next = 0;
        QLineF line(p[i],p[next]);
        result << line.pointAt(0.5);
    }
    return result;
}

///////////////////////////////////////////////////////////////////////
//
// paint
//
///////////////////////////////////////////////////////////////////////



