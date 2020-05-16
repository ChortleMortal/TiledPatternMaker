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
#include "geometry/Edge.h"
#include "geometry/Loose.h"
#include "base/utilities.h"

int Feature::refs = 0;

Feature::Feature()
{
    regular  = false;
    rotation = 0.0;
    refs++;
}

Feature::Feature(EdgePoly ep, qreal rotate)
{
    epoly    = ep;
    regular  = false;
    rotation = rotate;
    refs++;
}

// Create an n-sided regular polygon with a vertex at (1,0).
Feature::Feature(int n, qreal rotate)
{
    regular  = true;
    rotation = rotate;

    int idx  = 0;
    qreal angle = (M_PI / static_cast<qreal>(n)) * (static_cast<qreal>(2.0 * idx) + 1.0);
    qreal sc = 1.0 / qCos( M_PI / static_cast<qreal>(n) );

    VertexPtr v  = make_shared<Vertex>(QPointF(sc * qCos(angle), sc * qSin(angle)));
    VertexPtr v1 = v;
    for( int idx = 1; idx < n; ++idx )
    {
        qreal angle2 = (M_PI / static_cast<qreal>(n)) * (static_cast<qreal>(2.0 * idx) + 1);
        VertexPtr v2 = make_shared<Vertex>(QPointF(sc * qCos(angle2), sc * qSin(angle2)));
        epoly.push_back(make_shared<Edge>(v1,v2));
        v1 = v2;
    }
    epoly.push_back(make_shared<Edge>(v1,v));

    if (!Loose::zero(rotate))
    {
        epoly.rotate(rotate);
    }
    refs++;
}

Feature::Feature(const FeaturePtr other )
{
    regular  = other->regular;
    epoly    = other->epoly;
    rotation = other->rotation;
    refs++;
}

Feature::~Feature()
{
    refs--;
}

void Feature::reset()
{
    epoly.clear();
    bkgdColors.clear();
    rotation = 0;
}

bool Feature::equals(const FeaturePtr other)
{
    if (regular != other->regular)
        return false;
    if (!epoly.equals(other->epoly))
        return false;
    if (!Loose::equals(rotation, other->rotation))
        return false;
    return true;
}

QString Feature::toString() const
{
    QString text;
    QTextStream str(&text);

    str << "{ ";
    for (int i = 0; i < epoly.size(); ++i )
    {
        str << i+1 << ":"  << epoly[i]->getV1()->getPosition().x() << " " << epoly[i]->getV1()->getPosition().y() << " , ";
    }
    str << "}";

    return text;
}

QString Feature::info()
{
    QString text;
    QTextStream str(&text);

    str << "(" <<Utils::addr(this) << ") ";
    if (regular)
        str << "regular";
    else
        str << "not-regular";
    str << " " << "points=" << epoly.size();

    return text;
}

QPointF Feature::getCenter()
{
    return Point::center(epoly);
}

void  Feature::setRotation(qreal rot)
{
    qreal diff = rot - rotation;
    epoly.rotate(diff);
    rotation = rot;
}

