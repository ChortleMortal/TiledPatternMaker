////////////////////////////////////////////////////////////////////////////
//
// Loose.java
//
// A bunch of fuzzy comparisons that have a margin of error.
// Useful whenever you're doing computational geometry.

#include "sys/geometry/loose.h"
#include "sys/geometry/geo.h"

bool Loose::equals(qreal a, qreal b, qreal tolerance)
{
    return qAbs(a - b) < tolerance;
}

bool Loose::lessThan(qreal a, qreal b)
{
    return a < (b + Sys::TOL);
}

bool Loose::greaterThan(qreal a, qreal b)
{
    return a > (b - Sys::TOL);
}

bool Loose::equalsPt( QPointF a, QPointF b )
{
    return Geo::dist2(a, b ) < Sys::TOL;   // DAC - was TOL2
}

bool Loose::Near( qreal a, qreal tolerance )
{
    return qAbs( a ) < tolerance;
}

bool Loose::Near( QPointF a, QPointF b, qreal tolerance )
{
    return Geo::dist2(a, b ) < tolerance;
}

bool Loose::zero( QPointF a )
{
    return Geo::mag2(a) < Sys::TOL2;
}

bool Loose::zero( qreal a, qreal tolerance)
{
    return qAbs( a ) < tolerance;
}

