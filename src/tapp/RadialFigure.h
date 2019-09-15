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
// RadialFigure.java
//
// A RadialFigure is a special kind of Figure that has d_n symmetry.  That
// means that it can be rotated by 360/n degrees and flipped across certain
// lines through the origin and it looks the same.
//
// We take advantage of this by only making subclasses produce a basic
// unit, i.e. a smaller map that generates the complete figure through the
// action of c_n (just the rotations; the reflections are factored in
// by subclasses).

#ifndef RADIAL_FIGURE_H
#define RADIAL_FIGURE_H

#include "tapp/Figure.h"

class RadialFigure : public Figure
{
public:
    virtual ~RadialFigure() {}

    virtual void buildMaps() override;
    virtual void resetMaps() override;

    // Get a complete map from unit.
    virtual MapPtr  getFigureMap() override;

    virtual void    setN(int n);
    int     getN();

    void    setR( qreal r );
    qreal   getR()    { return r;}

    qreal   get_dn()  { return dn; }
    qreal   get_don() { return don; }
    QTransform getTransform() {return Tr;}

    virtual MapPtr   buildUnit() = 0;
    virtual void     buildExtBoundary() override;
    virtual MapPtr   useBuiltMap() const { return figureMap; }
    virtual QString  getFigureDesc() override { return "Radial Figure"; }

    MapPtr           replicateUnit();

    // Get the point frac of the way around the unit circle.
    static QPointF getArc( qreal frac );

protected:
    RadialFigure( int n, qreal rotate);
    RadialFigure(const Figure & fig, int n, qreal rotate);

    int           n;
    qreal         dn;
    qreal         don;
    QTransform    Tr;
    qreal         r;  // rotate

    MapPtr        unitMap;
};

#endif

