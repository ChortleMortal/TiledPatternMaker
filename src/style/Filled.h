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

#ifndef FILLED_H
#define FILLED_H

#include "style/style.h"
#include "base/colorset.h"
#include "geometry/faces.h"
#include "geometry/colormaker.h"

////////////////////////////////////////////////////////////////////////////
//
// Filled.java
//
// A rendering style that converts the map to a collection of
// polygonal faces.  The faces are divided into two groups according to
// a two-colouring of the map (which is always possible for the
// kinds of Islamic designs we're building).
//
// The code to build the faces from the map is contained in
// geometry.Faces.

class Filled : public Style, public ColorMaker
{
    friend class FilledEditor;

public:
    Filled(PrototypePtr proto, int algorithm);
    Filled(StylePtr other);
    virtual ~Filled() override;

    virtual eStyleType getStyleType() const override { return STYLE_FILLED; }

    QString getStyleDesc() const override {return "Filled";}

    void resetStyleRepresentation()  override;
    void createStyleRepresentation() override;
    void updateStyleRepresentation();

    void draw(GeoGraphics *gg) override;

    bool getDrawOutsideWhites() { return draw_outside_whites; }
    bool getDrawInsideBlacks()  { return draw_inside_blacks; }
    void setDrawOutsideWhites(bool draw) { draw_outside_whites = draw; }
    void setDrawInsideBlacks(bool  draw) { draw_inside_blacks  = draw; }

    ColorSet   * getWhiteColorSet() { return &whiteColorSet; }
    ColorSet   * getBlackColorSet() { return &blackColorSet; }
    ColorGroup * getColorGroup()    { return &colorGroup; }

    void    setAlgorithm(int val);
    int     getAlgorithm() { return algorithm; }

    void    setCleanseLevel(int val);
    int     getCleanseLevel() { return cleanseLevel; }

protected:
    ColorSet    whiteColorSet;
    ColorSet    blackColorSet;
    ColorGroup  colorGroup;

private:
    void drawDCEL(GeoGraphics *gg);
    void drawDCELNew2(GeoGraphics *gg);
    void drawDCELNew3(GeoGraphics *gg);

    // Parameters of the rendering. Control what gets drawn.
    bool        draw_outside_whites;
    bool        draw_inside_blacks;
    int         algorithm;
    int         cleanseLevel;
};
#endif

