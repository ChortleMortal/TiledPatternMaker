/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

////////////////////////////////////////////////////////////////////////////
//
// Loose.java
//
// A bunch of fuzzy comparisons that have a margin of error.
// Useful whenever you're doing computational geometry.

#ifndef LOOSE_H
#define LOOSE_H

#include <QtCore>
#include "geometry/point.h"
#include <QPointF>

class Loose
{
public:
    static qreal TOL;
    static qreal TOL2;
    static qreal NEAR_TOL;

    static bool equals( qreal a, qreal b )
    {
        return qAbs( a - b ) < TOL;
    }

    static bool zero( qreal a )
    {
        return qAbs( a ) < TOL;
    }

    static bool lessThan( qreal a, qreal b )
    {
        return a < (b + TOL);
    }

    static bool greaterThan( qreal a, qreal b )
    {
        return a > (b - TOL);
    }

    static bool equals( QPointF a, QPointF b )
    {
        return Point::dist2(a, b ) < TOL2;
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
