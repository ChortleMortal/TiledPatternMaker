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

#include "tapp/explicit_figure.h"

ExplicitFigure::ExplicitFigure(MapPtr map, eFigType figType, int sides)
    : Figure()
{
    figureMap = map;
    setFigType(figType);
    init(sides);
}

ExplicitFigure::ExplicitFigure(const Figure &fig, MapPtr map, eFigType figType, int sides)
    : Figure(fig)
{
    figureMap = map;
    setFigType(figType);
    init(sides);
}

ExplicitFigure::ExplicitFigure(const Figure & fig, eFigType figType, int sides)
    : Figure(fig)
{
    setFigType(figType);
    init(sides);
}

ExplicitFigure::~ExplicitFigure()
{}

void ExplicitFigure::init(int sides)
{
    this->n = sides;  // was sides = 10; // girih + intersect + rosette + star
    skip  = 3.0;         // girih
    d     = 2.0;         // hourglass + intersect + star
    s     = 1;           // hourglass + intersect + star
    q     = 0.0;         // rosette
    r_flexPt    = 0.5;   // rosette
    progressive = false; // intersect    
}


bool ExplicitFigure::equals(const FigurePtr other)
{
    ExplicitPtr otherp = std::dynamic_pointer_cast<ExplicitFigure>(other);
    if (!otherp)
        return  false;

    if (getFigType() != other->getFigType())
        return false;

    if (n != otherp->n)
        return false;

    if (d != otherp->d)
        return  false;

    if (s != otherp->s)
        return false;

    if (q != otherp->q)
        return  false;

    if (r_flexPt != otherp->r_flexPt)
        return  false;

    if (progressive != otherp->progressive)
        return  false;

    if (!Figure::equals(other))
        return false;

     return true;
}

MapPtr ExplicitFigure::getFigureMap()
{
    return figureMap;
}

void ExplicitFigure::buildMaps()
{
    qWarning() << "ExplicitFigure::buildMaps - does nothing";
}
