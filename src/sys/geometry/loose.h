#pragma once

////////////////////////////////////////////////////////////////////////////
//
// Loose.java
//
// A bunch of fuzzy comparisons that have a margin of error.
// Useful whenever you're doing computational geometry.

#include <QPointF>

class Loose
{
public:
    static constexpr QPointF ORIGIN         = QPointF(0,0);
    static constexpr qreal NEAR_TOL         = 1e-4;
    static constexpr qreal TOL              = 1e-7;
    static constexpr qreal TOL2             = 1e-10;

    static bool equals(qreal a, qreal b, qreal tolerance = TOL);
    static bool zero(qreal a, qreal tolerance = TOL);
    static bool lessThan(qreal a, qreal b);
    static bool greaterThan(qreal a, qreal b);
    static bool equalsPt( QPointF a, QPointF b );
    static bool Near( qreal a, qreal tolerance = TOL );
    static bool Near( QPointF a, QPointF b, qreal tolerance );
    static bool zero( QPointF a );

    static constexpr qreal TOLSQ            = TOL * TOL;
};
