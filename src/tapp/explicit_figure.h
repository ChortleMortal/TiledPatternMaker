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

#ifndef EXPLICIT_FIGURE_H
#define EXPLICIT_FIGURE_H

#include "tapp/figure.h"

////////////////////////////////////////////////////////////////////////////
//
// ExplicitFigure.java
//
// A variety of Figure which contains an explicit map, which is
// simple returned when the figure is asked for its map.

class ExplicitFigure : public Figure
{
public:

    ExplicitFigure(MapPtr map, eFigType figType, int sides);
    ExplicitFigure(const Figure & fig, MapPtr map, eFigType figType, int sides);
    ExplicitFigure(const Figure & fig, eFigType figType, int sides);

    virtual ~ExplicitFigure() override;

    virtual void buildMaps() override;

    void   setExplicitMap(MapPtr map) { figureMap = map; }
    MapPtr getFigureMap() override;

    virtual QString getFigureDesc()  override { return "ExplicitFigure"; }

    bool equals(const FigurePtr other) override;

    // a miscellany (hodge-podge)
    qreal   skip;           // girih
    qreal   d;              // hourglass + intersect + star
    int     s;              // hourglass + intersect + star
    qreal   q;              // rosette
    qreal   r_flexPt;       // rosette
    bool    progressive;    // intersect

protected:
    void    init(int sides);
};

#endif

