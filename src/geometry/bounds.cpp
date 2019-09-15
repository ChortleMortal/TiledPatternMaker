#include "bounds.h"

Bounds::Bounds()
{
    left  = 0.0;
    top   = 0.0;
    width = 0.0;
}

Bounds::Bounds(const Bounds & other)
{
    left  = other.left;
    top   = other.top;
    width = other.width;
}

Bounds::Bounds(qreal left, qreal top, qreal width)
{
    this->left  = left;
    this->top   = top;
    this->width = width;
}

Bounds  Bounds::operator+(const Bounds & other)
{
    Bounds b = *this;
    b.left  += other.left;
    b.top   += other.top;
    b.width += other.width;
    return b;
}

void Bounds::reset()
{
    left  = 0.0;
    top   = 0.0;
    width = 0.0;
}
