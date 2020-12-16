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

#include "tile/feature.h"
#include "geometry/point.h"
#include "geometry/edge.h"
#include "geometry/loose.h"
#include "base/utilities.h"

int Feature::refs = 0;

Feature::Feature(EdgePoly ep, qreal rotate, qreal scale)
{
    // createas irregular feature
    base        = ep;
    n           = ep.size();
    rotation    = rotate;
    this->scale = scale;
    regular     = false;
    refs++;

    create();
}

Feature::Feature(int n, qreal rotate, qreal scale)
{
    // Create an n-sided regular polygon with a vertex at (1,0).
    this->n   = n;
    rotation  = rotate;
    this->scale = scale;
    regular   = true;
    refs++;

    int idx  = 0;
    qreal angle = (M_PI / static_cast<qreal>(n)) * (static_cast<qreal>(2.0 * idx) + 1.0);
    qreal sc = 1.0 / qCos( M_PI / static_cast<qreal>(n) );

    VertexPtr v  = make_shared<Vertex>(QPointF(sc * qCos(angle), sc * qSin(angle)));
    VertexPtr v1 = v;
    for( int idx = 1; idx < n; ++idx )
    {
        qreal angle2 = (M_PI / static_cast<qreal>(n)) * (static_cast<qreal>(2.0 * idx) + 1);
        VertexPtr v2 = make_shared<Vertex>(QPointF(sc * qCos(angle2), sc * qSin(angle2)));
        base.push_back(make_shared<Edge>(v1,v2));
        v1 = v2;
    }
    base.push_back(make_shared<Edge>(v1,v));

    create();
}

void Feature::create()
{
    epoly.clear();
    epoly = base.recreate();

    if (!Loose::zero(rotation))
    {
        epoly.rotate(rotation);
    }
    if (!Loose::equals(scale,1.0))
    {
        epoly.scale(scale);
    }
}

void Feature::decompose()
{
    // the base is really the created
    epoly.clear();
    epoly = base.recreate();

    if (!Loose::zero(rotation))
    {
        base.rotate(-rotation);
    }
    if (!Loose::equals(scale,1.0))
    {
        base.scale(1.0/scale);
    }
}

Feature::Feature(const FeaturePtr other )
{
    n           = other->n;
    rotation    = other->rotation;
    scale       = other->scale;
    regular     = other->regular;
    base        = other->base;
    epoly       = other->epoly;

    refs++;
}

Feature::~Feature()
{
    refs--;
}

void Feature::setRotation(qreal rotate)
{
    rotation = rotate;
    create();
}

qreal Feature::getRotation()
{
    return rotation;
}

void Feature::deltaRotation(qreal delta)
{
    setRotation(rotation + delta);
}

void Feature::setScale(qreal scale)
{
    this->scale = scale;
    create();
}

qreal Feature::getScale()
{
    return scale;
}

void Feature::deltaScale(qreal delta)
{
    setScale(scale + delta);
}

void Feature::setRegular(bool enb)
{
    if (enb && !regular)
    {
        regular =  true;
        create();
    }
    else if (!enb && regular)
    {
        // convert to irregular
        regular = false;
        create();
    }
}


FeaturePtr Feature::recreate()
{
    FeaturePtr f;
    if (regular)
    {
        f = make_shared<Feature>(n,rotation,scale);
    }
    else
    {
        f = make_shared<Feature>(base,rotation,scale);
    }
    return  f;
}


bool Feature::equals(const FeaturePtr other)
{
    if (regular != other->regular)
        return false;
    if (!base.equals(other->base))
        return false;
    if (!Loose::equals(rotation, other->rotation))
        return false;
    if (!Loose::equals(scale, other->scale))
        return false;
    return true;
}

bool Feature::isSimilar(const FeaturePtr other)
{
    if (regular != other->regular)
        return false;
    if (numSides() != other->numSides())
        return false;
    if (!Loose::equals(rotation, other->rotation))
        return false;
    if (!Loose::equals(scale, other->scale))
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

    //str << "(" <<Utils::addr(this) << ") ";
    str << "sides:" << epoly.size();
    if (regular)
        str << " regular";
    else
        str << " irregular";

    return text;
}

QString Feature::summary()
{
    return QString("%1%2 ").arg(numSides()).arg((regular) ? 'r' : 'i' );
}

QPointF Feature::getCenter()
{
    return Point::center(epoly);
}



