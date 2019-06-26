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
// Feature.java
//
// A Feature is an element of a tiling, i.e. a tile.  It's really
// just an array of points.  I could just use a geometry.Polygon,
// but I keep adding (and removing) per-point information, which makes it
// useful to store the array explicitly -- sometimes I switch Point
// for a FancyPoint of some kind.  Also, we don't expect the number
// of points to change once the feature is created, so the array
// is fine.
//
// We also store whether the Feature was created as a regular polygon.
// This helps later when deciding what Features can have Rosettes
// in them.

#include "tile/Feature.h"
#include "geometry/Point.h"
#include "base/shared.h"
#include "base/utilities.h"
#include <QColor>

Feature::Feature()
{
    regular = false;
}

// Create an n-sided regular polygon with a vertex at (1,0).
Feature::Feature( int n )
{
    regular = true;

    for( int idx = 0; idx < n; ++idx )
    {
        qreal angle = (M_PI / static_cast<qreal>(n)) * static_cast<qreal>(2 * idx + 1);
        qreal sc = 1.0 / qCos( M_PI / static_cast<qreal>(n) );
        points << QPointF(sc * qCos( angle ), sc * qSin( angle ) );
    }
}

// Create a Feature2 explicitly from a polygon.
Feature::Feature(QPolygonF &pgon )
{
    regular = false;
    points  = pgon;
}

Feature::Feature(PolyPtr pgon )
{
    regular = false;
    points  = *pgon;
}

Feature::Feature(const FeaturePtr other )
{
    regular = other->regular;
    points  = other->points;
}

void Feature::reset()
{
    points.clear();
    bkgdColors.clear();
}

bool Feature::equals(FeaturePtr other ) const
{
    if ( other->regular != regular )
        return false;
    if (other->points != points)
        return false;
    return true;
}

QString Feature::toString() const
{
    QString text;
    QTextStream str(&text);
    str << "{ ";
    for ( int i = 0; i < points.size(); ++i )
    {
        str << i+1 << ":"  << points[i].x() << " " << points[i].y() << " , ";
    }
    str << "}";
    return text;
}

QVector<QLineF> Feature::getEdges()
{
    QVector<QLineF> qvlf = Utils::polyToLines(points);
    return qvlf;
}

QPointF Feature::getCenter()
{
    return Point::center(points);
}
