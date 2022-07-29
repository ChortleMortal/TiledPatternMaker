#include "legacy/shapes.h"
#include "legacy//shapefactory.h"


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

Polygon2 & Polygon2::operator=(const Polygon2 & other)
{
    Polyform::operator=(other);
    return *this;
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

Polyline2 & Polyline2::operator=(const Polyline2 & other)
{
    Polyform::operator=(other);
    return *this;
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

Circle2 & Circle2::operator=(const Circle2 & other)
{
    Polyform::operator=(other);
    return *this;
}
