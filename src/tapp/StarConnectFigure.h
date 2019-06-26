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

#ifndef STAR_CONNECTFIGURE_H
#define STAR_CONNECTFIGURE_H

#include "base/shared.h"
#include "tapp/Star.h"
#include "tapp/figureconnector.h"

class StarConnectFigure : public Star, FigureConnector
{
public:
    StarConnectFigure(int n, qreal d, int s, qreal r= 0.0);
    StarConnectFigure(const Figure & fig, int n, qreal d, int s, qreal r= 0.0);

    MapPtr buildUnit() override;

    virtual QString getFigureDesc() override { return "Star Connect figure";}

    qreal computeConnectScale();
};
#endif

