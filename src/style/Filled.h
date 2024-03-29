#pragma once
#ifndef FILLED_H
#define FILLED_H

#include "style/style.h"
#include "misc/colorset.h"
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

class Filled : public Style
{
    friend class FilledEditor;
    friend class page_system_info;

public:
    Filled(const ProtoPtr &  proto, int algorithm);
    Filled(const StylePtr &other);
   virtual ~Filled();

    void resetStyleRepresentation()  override;
    void createStyleRepresentation() override;
    void updateStyleRepresentation();

    void draw(GeoGraphics *gg) override;

    ColorSet   * getWhiteColorSet() { return &colorMaker.whiteColorSet; }
    ColorSet   * getBlackColorSet() { return &colorMaker.blackColorSet; }
    ColorGroup * getColorGroup()    { return &colorMaker.colorGroup; }

    bool getDrawOutsideWhites() { return draw_outside_whites; }
    bool getDrawInsideBlacks()  { return draw_inside_blacks; }
    void setDrawOutsideWhites(bool draw) { draw_outside_whites = draw; }
    void setDrawInsideBlacks(bool  draw) { draw_inside_blacks  = draw; }

    ColorMaker & getColorMaker() { return colorMaker; };

    void    setAlgorithm(int val);
    int     getAlgorithm() { return algorithm; }

    virtual eStyleType getStyleType() const override { return STYLE_FILLED; }
    QString            getStyleDesc() const override { return "Filled";}
    virtual void       report()       const override { qDebug().noquote() << getStyleDesc(); }

protected:


private:
    ColorMaker  colorMaker;

    void drawDCEL(GeoGraphics *gg);
    void drawDCELNew2(GeoGraphics *gg);
    void drawDCELNew3(GeoGraphics *gg);

    // Parameters of the rendering. Control what gets drawn.
    bool        draw_outside_whites;
    bool        draw_inside_blacks;
    int         algorithm;
};
#endif

