#pragma once
#ifndef LOOSE_H
#define LOOSE_H

////////////////////////////////////////////////////////////////////////////
//
// Loose.java
//
// A bunch of fuzzy comparisons that have a margin of error.
// Useful whenever you're doing computational geometry.

#include "geometry/point.h"
#include <QPointF>

class Loose
{
public:
    static qreal TOL;
    static qreal TOL2;
    static qreal NEAR_TOL;

    static bool equals(qreal a, qreal b, qreal tolerance = TOL)
    {
        return qAbs(a - b) < tolerance;
    }

    static bool zero(qreal a, qreal tolerance = TOL);

    static bool lessThan(qreal a, qreal b)
    {
        return a < (b + TOL);
    }

    static bool greaterThan(qreal a, qreal b)
    {
        return a > (b - TOL);
    }

    static bool equalsPt( QPointF a, QPointF b )
    {
        return Point::dist2(a, b ) < TOL;   // DAC - was TOL2
    }

    static bool Near( qreal a, qreal tolerance )
    {
        return qAbs( a ) < tolerance;
    }

    static bool Near( QPointF a, QPointF b, qreal tolerance )
    {
        return Point::dist2(a, b ) < tolerance;
    }

    static bool zero( QPointF a )
    {
        return Point::mag2(a) < TOL2;
    }
};

#endif
