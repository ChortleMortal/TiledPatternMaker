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

#ifndef FEATURE
#define FEATURE

#include <QtCore>
#include <QPolygonF>
#include "base/shared.h"
#include "base/colorset.h"
#include "geometry/edge.h"
#include "geometry/edgepoly.h"

class Feature
{
public:
    Feature(int n, qreal rotate = 0.0, qreal scale = 1.0);           // Create an n-sided regular polygon with a vertex at (1,0).
    Feature(EdgePoly epoly, qreal rotate = 0.0, qreal scale = 1.0);
    Feature(const FeaturePtr other);
    ~Feature();

    void        setRegular(bool enb);
    void        setN(int n);
    void        setRotation(qreal rot);
    void        setScale(qreal rot);

    void        deltaRotation(qreal delta);
    void        deltaScale(qreal delta);

    qreal       getRotation();
    qreal       getScale();

    void        create();          // makes epoly from base
    void        decompose();       // makes base from epoly (should not be needed except to fix historical files)
    FeaturePtr  recreate();

    bool        isRegular() { return regular; }
    bool        isClockwise() { return  epoly.isClockwise(); }
    bool        equals(const FeaturePtr other);
    bool        isSimilar(const FeaturePtr other);

    EdgePoly &  getEdgePoly()       { return epoly;}             // must be ref so can change
    EdgePoly &  getBase()           { return base; }

    QPolygonF   getPolygon()        { return epoly.getPoly(); }
    QPolygonF   getPoints()         { return epoly.getPoly(); }
    int         numPoints()         { return epoly.size(); }
    int         numSides()          { return epoly.size(); }
    ColorSet &  getBkgdColors()     { return bkgdColors; }

    QPointF     getCenter();
    qreal       edgeLen(int side = 0);

    QString     toString() const;
    QString     info();
    QString     summary();

    static int refs;

protected:
    int         n;             // sides
    bool        regular;
    EdgePoly    base;

    qreal       rotation;
    qreal       scale;

    EdgePoly    epoly;
    ColorSet    bkgdColors;    // backgrounds
};

#endif
