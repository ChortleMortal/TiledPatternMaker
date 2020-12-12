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

// The classic [n/d]s star construction.  See the paper for moredetails.

#ifndef STAR_H
#define STAR_H

#include "tapp/radial_figure.h"

class Star : public RadialFigure
{
public:
    // n = points, d = sides hops, s = sides intersections
    Star( int n, qreal d, int s, qreal figureRotate= 0.0);
    Star( const Figure & fig, int n, qreal d, int s, qreal figureRotate= 0.0);

    virtual MapPtr buildUnit() override;

    qreal   getD()  {return d;}
    int     getS()  {return s;}

    void    setD(qreal d);
    void    setS(int s);

    virtual QString getFigureDesc() override { return "Star"; }

    bool equals(const FigurePtr other) override;

private:
    qreal 	d;
    int		s;
};

#endif

