#pragma once
#ifndef FILLED_H
#define FILLED_H

#include "sys/enums/efilltype.h"
#include "model/styles/colormaker.h"
#include "model/styles/style.h"
#include "model/styles/colorset.h"

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
    Filled(const ProtoPtr &  proto, eFillType algorithm);
    Filled(const StylePtr &other);
    virtual ~Filled();

    void resetStyleRepresentation()  override;
    void createStyleRepresentation() override;

    void draw(GeoGraphics *gg) override;

    ColorSet   * getWhiteColorSet()         { return &whiteColorSet; }
    ColorSet   * getBlackColorSet()         { return &blackColorSet; }
    ColorGroup * getColorGroup()            { return &colorGroup; }

    bool getDrawOutsideWhites()             { return draw_outside_whites; }
    bool getDrawInsideBlacks()              { return draw_inside_blacks; }

    void setDrawOutsideWhites(bool draw)    { draw_outside_whites = draw; }
    void setDrawInsideBlacks(bool  draw)    { draw_inside_blacks  = draw; }

    void resetColorIndices() { faceColorIndices.fill(-1);}
    int  getColorIndex(int faceIndex);
    void setColorIndex(int faceIndex,int colorIndex);
    int  numColorIndices() { return faceColorIndices.size(); }

    void            setPaletteIndices(QVector<int> & indices) { faceColorIndices = indices; }
    QVector<int> &  getPaletteIndices()                       { return faceColorIndices; }
    QColor          getColorFromPalette(int index)            { return whiteColorSet.getQColor(index); }

    void            setAlgorithm(eFillType val);
    eFillType       getAlgorithm()          { return algorithm; }

    ColorMaker *    getColorMaker()         { return colorMaker; };

    virtual eStyleType getStyleType() const override { return STYLE_FILLED; }
    QString            getStyleDesc() const override { return "Filled";}
    void               dump()         const override { qDebug().noquote() << getStyleDesc(); }

protected:
    void drawDCEL(GeoGraphics *gg);
    void drawDCELNew2(GeoGraphics *gg);
    void drawDCELNew3(GeoGraphics *gg);
    void drawDCELDirectColor(GeoGraphics *gg);

private:
    ColorMaker *    colorMaker;

    // Parameters of the rendering. Control what gets drawn.
    bool            draw_outside_whites;
    bool            draw_inside_blacks;
    eFillType       algorithm;

    ColorSet        whiteColorSet;
    ColorSet        blackColorSet;
    ColorGroup      colorGroup;

    QVector<int>    faceColorIndices;
};
#endif

