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

#ifndef THICK_H
#define THICK_H

#include "style/colored.h"

////////////////////////////////////////////////////////////////////////////
//
// Thick.java
//
// A style that has a thickness and can have its outline drawn.

class Thick : public Colored
{
public:
    Thick(PrototypePtr proto);
    Thick(StylePtr other);
    ~Thick() override;

    virtual eStyleType getStyleType() const  override { return STYLE_THICK; }
    QString getStyleDesc() const override {return("Thick Lines");}

    void  setLineWidth(qreal width );
    qreal getLineWidth() { return width; }

    void  setOutlineWidth(qreal width );


    bool getDrawOutline() {return draw_outline;}
    void setDrawOutline(bool draw_outline) { this->draw_outline = draw_outline; }

    void resetStyleRepresentation() override;
    void createStyleRepresentation() override;
    void draw(GeoGraphics *gg) override;

protected:
    // Parameters of the rendering.
    qreal      width;
    bool       draw_outline;
    qreal      outline_width;

private:
};
#endif

