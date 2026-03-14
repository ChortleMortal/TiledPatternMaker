#pragma once
#ifndef FILLED_H
#define FILLED_H

#include "sys/enums/efilltype.h"
#include "model/styles/fill_color_maker.h"
#include "model/styles/fill_original.h"
#include "model/styles/fill_new1.h"
#include "model/styles/fill_color_set.h"
#include "model/styles/fill_color_group.h"
#include "model/styles/fill_faces.h"
#include "model/styles/fill_deprecated.h"
#include "model/styles/style.h"

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
public:
    Filled(const ProtoPtr &  proto, eFillType algorithm);
    Filled(const StylePtr &other);
    virtual ~Filled();

    void resetStyleRepresentation()  override;
    void createStyleRepresentation() override;

    void draw(GeoGraphics *gg) override;

    eFillType    getAlgorithm() const        { return _algorithm; }
    void         setAlgorithm(eFillType algo);
    void         initAlgorithmFrom(eFillType old);

    virtual eStyleType getStyleType() const override { return STYLE_FILLED; }
    QString            getStyleDesc() const override { return "Filled";}
    void               dump()         const override { qDebug().noquote() << getStyleDesc() << "algo =" << getAlgorithm(); }

protected:
    void drawDCEL(GeoGraphics *gg);
    void drawDCELNew2(GeoGraphics *gg);
    void drawDCELNew3(GeoGraphics *gg);
    void drawDCELDirectColor(GeoGraphics *gg);
    void drawDCELDirectColor2(GeoGraphics *gg);

public:
    ColorMaker  *       currentColoring;
    OriginalColoring    original;
    New1Coloring        new1;
    New2Coloring        new2;
    New3Coloring        new3;
    DirectColoring      direct;
    DeprecatedDirectColoring  deprecated;

    eFillType           _algorithm;
};

typedef std::shared_ptr<Filled> FilledPtr;

#endif

