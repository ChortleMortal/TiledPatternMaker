#ifndef BOUNDS_H
#define BOUNDS_H

#include <QtCore>

class Bounds
{
public:
    Bounds();
    Bounds(qreal left, qreal top, qreal width);
    Bounds(qreal left, qreal top, qreal width, qreal theta);

    Bounds   operator+(const Bounds & other);

    void reset();

    qreal left;
    qreal top;
    qreal width;
    qreal theta;
};

#endif // BOUNDS_H
