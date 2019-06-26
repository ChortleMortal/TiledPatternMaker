#include "bounds.h"

Bounds::Bounds()
{
    left  = 0.0;
    top   = 0.0;
    width = 0.0;
    theta = 0.0;
}

Bounds::Bounds(qreal left, qreal top, qreal width)
{
    this->left  = left;
    this->top   = top;
    this->width = width;
    theta       = 0.0;
}

Bounds::Bounds(qreal left, qreal top, qreal width, qreal theta)
{
    this->left  = left;
    this->top   = top;
    this->width = width;
    this->theta = theta;
}


Bounds  Bounds::operator+(const Bounds & other)
{
    Bounds b = *this;
    b.left  += other.left;
    b.top   += other.top;
    b.width += other.width;
    b.theta += other.theta;
    return b;
}

void Bounds::reset()
{
    left  = 0.0;
    top   = 0.0;
    width = 0.0;
    theta = 0.0;
}
