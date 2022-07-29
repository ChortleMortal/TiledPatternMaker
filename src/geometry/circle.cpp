#include <QDebug>
#include "geometry/circle.h"

Circle::Circle()
{
    init();
}
Circle::Circle(QPointF centre, qreal radius)
{
    this->centre = centre;
    this->radius = radius;
}

Circle::Circle(const Circle & c)
{
    centre = c.centre;
    radius = c.radius;
}

QRectF  Circle::boundingRect()
{
    QRectF rect(centre.x() - radius, centre.y()-radius,radius*2,radius*2);
    return rect;
}

Circle & Circle::operator=(const Circle & other)
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

void Circle::dump()
{
    qDebug() << "Circle" << centre << "radius=" << radius;
}
