#include <QDebug>
#include <QtMath>
#include "geometry/circle.h"

QString sQuad[] =
{
    "Q1",
    "Q2",
    "Q3",
    "Q4"
};

Circle::Circle()
{
    init();
}
Circle::Circle(QPointF centre, qreal radius)
{
    this->centre = centre;
    this->radius = radius;
}

Circle::Circle(const Circle &c)
{
    centre = c.centre;
    radius = c.radius;
}

QRectF  Circle::boundingRect()
{
    QRectF rect(centre.x() - radius, centre.y()-radius,radius*2,radius*2);
    return rect;
}

Circle & Circle::operator=(const Circle &other)
{
    centre = other.centre;
    radius = other.radius;
    return *this;
}

#if 0
bool Circle::operator==(const Circle & other)
{
    if ((centre == other.centre) && Loose::equals(radius,other.radius))
        return true;
    else
        return false;
}

bool Circle::operator==(const Circle other)
{
    if ((centre == other.centre) && Loose::equals(radius,other.radius))
        return true;
    else
        return false;
}
#endif

eQuad Circle::getQuadrant(qreal radians)
{
    Q_ASSERT(radians >= 0);
    if (radians < M_PI_2)
        return QUAD_1;
    if (radians < M_PI)
        return QUAD_2;
    if (radians < (M_PI_2 * 3.0))
        return QUAD_3;
    return QUAD_4;
}

QString Circle::getQuadrantString(qreal radians)
{
    eQuad quadrant = getQuadrant(radians);
    return sQuad[quadrant];
}

void Circle::dump()
{
    qDebug() << "Circle" << centre << "radius=" << radius;
}
