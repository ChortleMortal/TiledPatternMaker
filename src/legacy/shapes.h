#pragma once
#ifndef SHAPES_H
#define SHAPES_H

#include <QPolygonF>
#include <QPen>

#define DEGREES_360     (360 * 16)

enum eForm
{
    POLYGON2,
    POLYLINE2,
    CIRCLE2
};

enum eArcType
{
    ARC,
    CHORD,
    PIE
};

class Polyform : public QPolygonF
{
protected:
    Polyform(QPen Pen = QPen(Qt::NoPen), QBrush Brush = QBrush(Qt::NoBrush));
    Polyform(const QPolygonF &polygon, QPen Pen = QPen(Qt::NoPen), QBrush Brush = QBrush(Qt::NoBrush));

public:
    void setPen(QPen Pen)           {pen      = Pen;}
    void setBrush(QBrush Brush)     {brush    = Brush;}
    void setDiameter(qreal Diameter){diameter = Diameter; radius = Diameter/2.0;}
    void setArc(eArcType ArcType, int start, int span){arcType = ArcType; arcStart = start; arcSpan = span;}
    void setInnerPen(QPen pen)      {innerPen = pen;}
    void setRadials(bool set)       {radials  = set;}

    QPen   getPen()                 {return pen;}
    QBrush getBrush()               {return brush;}

    QRectF boundingRect();

    eForm       polytype;
    QPen        pen;
    QPen        innerPen;

    QBrush      brush;
    bool        radials;

    qreal       diameter;   // circle
    qreal       radius;     // circle

    int         arcStart;
    int         arcSpan;
    eArcType    arcType;
};

class Polygon2 : public Polyform
{
public:
    Polygon2(QPen Pen = QPen(Qt::NoPen), QBrush Brush = QBrush(Qt::NoBrush));
    Polygon2(const QPolygonF &polygon, QPen Pen = QPen(Qt::NoPen), QBrush Brush = QBrush(Qt::NoBrush));
    Polygon2(const Polygon2 & poly);

    Polygon2 & operator=(const Polygon2 & other);

    void rotate(qreal delta);
};

class Polyline2 : public Polyform
{
public:
    Polyline2(QPen Pen = QPen(Qt::NoPen));
    Polyline2(const QPolygonF &polygon, QPen pen = QPen(Qt::NoPen));
    Polyline2(const Polyline2 & poly);

    Polyline2 & operator=(const Polyline2 & other);

};

class Circle2 : public Polyform
{
public:
    Circle2(qreal Diameter, QPen Pen = QPen(Qt::NoPen), QBrush Brush = QBrush(Qt::NoBrush), int ArcStart = 0, int ArcSpan = 0, eArcType ArcType = ARC);
    Circle2(const Circle2 & circle);

    Circle2 & operator=(const Circle2 & other);

};

#endif // SHAPES_H
