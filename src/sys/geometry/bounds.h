#pragma once
#ifndef BOUNDS_H
#define BOUNDS_H

#include <QtCore>

#undef VARIABLE_BOUNDS

class Bounds
{
public:
    Bounds();

#ifdef VARIABLE_BOUNDS
    Bounds(const Bounds & other);
    Bounds(qreal left, qreal top, qreal width);

    Bounds & operator=(const Bounds & other);
    Bounds   operator+(const Bounds & other);
#endif

    qreal left;
    qreal top;
    qreal width;
};

#endif // BOUNDS_H
