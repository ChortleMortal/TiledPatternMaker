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
// Rosette.java
//
// The rosette is a classic feature of Islamic art.  It's a star
// shape surrounded by hexagons.
//
// This class implements the rosette as a RadialFigure using the
// geometric construction given in Lee [1].
//
// [1] A.J. Lee, _Islamic Star Patterns_.  Muqarnas 4.

#ifndef ROSETTE_H
#define ROSETTE_H

#include "tapp/radial_figure.h"

class Rosette : public RadialFigure
{
public:
    // n = points, q = tip angle, s=sides intersections
    Rosette(const Figure & fig,  int n, qreal q, int s, qreal k=0.0, qreal figureRotate=0.0);
    Rosette( int n, qreal q, int s, qreal k=0.0, qreal figureRotate=0.0);

    virtual MapPtr   buildUnit() override;

    qreal   getQ() {return q;}
    qreal   getK() {return k;}
    int     getS() {return s;}

    void    setQ( qreal q );
    void    setK( qreal k );
    void    setS( int s );
    void    setN(int n) override;

    virtual QString getFigureDesc() override { return "Rosette";}

    bool equals(const FigurePtr other) override;

private:
    int     count;
    qreal   q;
    qreal   k;
    int     s;
};

#endif

