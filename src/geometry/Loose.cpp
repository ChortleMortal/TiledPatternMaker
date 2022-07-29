////////////////////////////////////////////////////////////////////////////
//
// Loose.java
//
// A bunch of fuzzy comparisons that have a margin of error.
// Useful whenever you're doing computational geometry.

#include "geometry/loose.h"

qreal Loose::TOL      = 1e-7;
qreal Loose::TOL2     = 1e-10;
qreal Loose::NEAR_TOL = 1e-4;

bool Loose::zero( qreal a, qreal tolerance)
{
    return qAbs( a ) < tolerance;
}

