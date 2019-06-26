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
// ConnectFigure.java
//
// A ConnectFigure is a special kind of ScaleFigure.  It knows how to
// compute just the right scale factor so that scaled out edges will join
// up to create a fancier figure.  This is how we turn Rosettes into
// Extended Rosettes.  To make sure that the resulting figure still lines
// up with the feature that will eventually contain it, we need to do
// some fancy reshuffling of the basic unit to move the apex to (1,0).

#include "tapp/StarConnectFigure.h"
#include "geometry/Loose.h"

StarConnectFigure::StarConnectFigure(int n, qreal d, int s, qreal r)
    : Star(n,d,s,r), FigureConnector(this)
{
    setFigureScale(computeConnectScale());
    setFigType(FIG_TYPE_CONNECT_STAR);
}

StarConnectFigure::StarConnectFigure(const Figure & fig, int n, qreal d, int s, qreal r)
    : Star(fig, n,d,s,r), FigureConnector(this)
{
    setFigureScale(computeConnectScale());
    setFigType(FIG_TYPE_CONNECT_STAR);
}

qreal StarConnectFigure::computeConnectScale()
{
    // Find the vertex at (1,0), extend its incoming edges, intersect
    // with rotations of same, and use the location of the intersection
    // to compute a scale factor.

    setFigureScale(1.0);
    qreal rot = r;      //save
    r = 0.0;
    MapPtr cunit = Star::buildUnit();
    r = rot;            // restore
    qreal sc = computeScale(cunit);

    resetMaps();        // so unit can build

    return sc;
}

MapPtr StarConnectFigure::buildUnit()
{
    qDebug() << "StarConnectFigure::buildUnit";

    // save
    qreal scale = figureScale;
    qreal rot   = r;
    figureScale = 1.0;
    r           = 0.0;

    // build Rosette
    unitMap = Star::buildUnit();
    //unitMap->dump();

    // restore
    figureScale = scale;
    r           = rot;

    // extend Rosette
    connectFigure(unitMap);
    //unitMap->dump();

    // We want the tip of the new figure to still be at (1,0).
    // To accomplish this, move the top half of the figure around
    // to lie under the bottom half.  This rebuilds the tip
    // at the correct location.

    rotateHalf(unitMap);  // DAC methinks not needed

    unitMap->transformMap( Transform::rotate( M_PI * don ) );

    scaleToUnit(unitMap);

    unitMap->verify("StarConnectFigure",true,true,true);

    return unitMap;
}
