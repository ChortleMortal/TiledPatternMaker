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
// Emboss.java
//
// A rendering style for maps that pretends that the map is carves out
// of wood with a diamond cross section and shines a directional light
// on the wood from some angle.  The map is drawn as a collection of
// trapezoids, and the darkness of each trapezoid is controlled by the
// angle the trapezoid makes with the light source.  The result is a
// simple but highly effective 3D effect, similar to flat-shaded 3D
// rendering.
//
// In practice, we can make this a subclass of RenderOutline -- it uses the
// same pre-computed point array, and just add one parameter and overloads
// the draw function.

#ifndef EMBOSS_H
#define EMBOSS_H

#include "style/Outline.h"

class Emboss : public Outline
{
public:
    Emboss(PrototypePtr proto, PolyPtr bounds);
    Emboss(const Style &  other);
    virtual ~Emboss() override;

    virtual eStyleType getStyleType() const override { return STYLE_EMBOSSED; }
    QString  getStyleDesc() const override {return "Embossed";}

    qreal   getAngle();
    void    setAngle(qreal angle );
    void    setColor( QColor color );

   void     draw(class GeoGraphics * gg) override;

protected:
   void     setGreys();
   void     drawTrap(class GeoGraphics * gg, QPointF a, QPointF b, QPointF c, QPointF d );

    // Parameters of the rendering.
    qreal 		angle;

    // Internal data derived from the angle.
    qreal 		light_x;
    qreal 		light_y;

private:
    QVector<QColor> greys;
};
#endif



