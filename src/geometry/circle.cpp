#include "circle.h"

Circle::Circle()
{
    centre = QPointF(0,0);
    radius = 0.25;
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

bool Circle::operator==(const Circle & other)
{
    if ((centre == other.centre) && Loose::equals(radius,other.radius))
        return true;
    else
        return false;
}

void Circle::dump()
{
    qDebug() << "Circle" << centre << "radius=" << radius;
}
