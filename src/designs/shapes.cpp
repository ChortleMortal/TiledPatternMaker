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
#include "designs//shapefactory.h"


Polyform::Polyform(QPen Pen, QBrush Brush) : QPolygonF()
{
    pen        = Pen;
    innerPen   = QPen(Qt::NoPen);
    brush      = Brush;
    radials    = false;
}


Polyform::Polyform(const QPolygonF & polygon, QPen Pen, QBrush Brush) : QPolygonF(polygon)
{
    pen        = Pen;
    innerPen   = QPen(Qt::NoPen);
    brush      = Brush;
    radials    = false;
}

QRectF Polyform::boundingRect()
{
    if (polytype == CIRCLE2)
    {
        return QRectF(-diameter/2.0,-diameter/2.0,diameter,diameter);
    }
    else
    {
        return QPolygonF::boundingRect();
    }
}

Polygon2::Polygon2(QPen Pen, QBrush Brush)
    : Polyform(Pen,Brush)
{
    polytype     = POLYGON2;
}

Polygon2::Polygon2(const QPolygonF & polygon, QPen Pen, QBrush Brush)
    : Polyform(polygon, Pen,Brush)
{
    polytype     = POLYGON2;
}

void Polygon2::rotate(qreal delta)
{
    if (delta == 0.0) return;

    QVector<QPointF>::iterator it;
    for (it = begin(); it != end(); it++)
    {
        QPointF pt = *it;
        pt = ShapeFactory::rotatePoint(pt,delta);
        *it = pt;
    }
}

Polyline2::Polyline2(QPen Pen)
    : Polyform(Pen)
{
    polytype = POLYLINE2;
}


Polyline2::Polyline2(const QPolygonF & polygon, QPen pen)
    : Polyform(polygon,pen)
{
    polytype = POLYLINE2;
}

Circle2::Circle2(qreal Diameter, QPen Pen, QBrush Brush, int ArcStart, int ArcSpan, eArcType ArcType)
    : Polyform(Pen,Brush)
{
    polytype    = CIRCLE2;
    diameter    = Diameter;
    radius      = Diameter/2.0;
    arcStart    = ArcStart;
    arcSpan     = ArcSpan;
    arcType     = ArcType;
}

Circle2::Circle2(const Circle2 & circle)
    : Polyform(circle.pen, circle.brush)
{
    polytype    = CIRCLE2;
    diameter    = circle.diameter;
    radius      = circle.radius;
    arcStart    = circle.arcStart;
    arcSpan     = circle.arcSpan;
    arcType     = circle.arcType;
}
