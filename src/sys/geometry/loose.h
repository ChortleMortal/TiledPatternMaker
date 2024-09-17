#pragma once
#ifndef LOOSE_H
#define LOOSE_H

////////////////////////////////////////////////////////////////////////////
//
// Loose.java
//
// A bunch of fuzzy comparisons that have a margin of error.
// Useful whenever you're doing computational geometry.

#include "sys/sys.h"
#include <QPointF>

class Loose
{
public:
    static bool equals(qreal a, qreal b, qreal tolerance = Sys::TOL);
    static bool zero(qreal a, qreal tolerance = Sys::TOL);
    static bool lessThan(qreal a, qreal b);
    static bool greaterThan(qreal a, qreal b);
    static bool equalsPt( QPointF a, QPointF b );
    static bool Near( qreal a, qreal tolerance );
    static bool Near( QPointF a, QPointF b, qreal tolerance );
    static bool zero( QPointF a );
};

#endif
