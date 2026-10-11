#pragma once

#include <QtGlobal>

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
