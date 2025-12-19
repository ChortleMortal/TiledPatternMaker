#pragma once
#ifndef CIRCLE_H
#define CIRCLE_H

#include <QDataStream>

#include "sys/geometry/loose.h"

enum eQuad
{
    QUAD_1,
    QUAD_2,
    QUAD_3,
    QUAD_4
};

class Circle
{
public:
    Circle();
    Circle(const Circle & c);
    Circle(QPointF centre, qreal radius);

    void init() { radius = 1.0; centre = QPointF(); }
    void set(Circle * other)  { radius = other->radius;  centre = other->centre; }

    void setRadius(qreal r) { radius = r; }
    void setCenter(QPointF p) { centre = p; }

    QRectF  boundingRect();
    qreal   x() { return centre.x(); }
    qreal   y() { return centre.y(); }
    qreal   r() { return radius; }

    friend bool operator==(const Circle & c1, const Circle & c2)
    {
        if ((c1.centre == c2.centre) && Loose::equals(c1.radius,c2.radius))
            return true;
        else
            return false;
    }

    friend bool operator!=(const Circle & c1, const Circle & c2)
    {
        return !(c1==c2);
    }

    Circle & operator=(const Circle &other);

    friend QDataStream & operator<< (QDataStream &out, const Circle & c)
    {
        out << c.radius;
        out << c.centre;
        return out;
    }

    friend QDataStream & operator>> (QDataStream &in, Circle & c)
    {
        in >> c.radius;
        in >> c.centre;
        return in;
    }

    static eQuad   getQuadrant(qreal radians);
    static QString getQuadrantString(qreal radians);

    void        dump();

    qreal       radius;
    QPointF     centre;
    qreal       tmpDist2;   // variable omly used in selection
};


#endif
