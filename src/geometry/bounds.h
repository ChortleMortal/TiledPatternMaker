#ifndef BOUNDS_H
#define BOUNDS_H

#include <QtCore>

class Bounds
{
public:
    Bounds();
    Bounds(const Bounds & other);
    Bounds(qreal left, qreal top, qreal width);

    Bounds   operator+(const Bounds & other);

    void reset();

    qreal left;
    qreal top;
    qreal width;
};

#endif // BOUNDS_H
