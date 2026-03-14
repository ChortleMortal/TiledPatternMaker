#pragma once
#ifndef FILL_ORIGINAL_H
#define FILL_ORIGINAL_H

#include "model/styles/fill_color_maker.h"
#include "model/styles/colorset.h"
#include "sys/enums/efilltype.h"
#include "sys/geometry/dcel.h"
#include "sys/geometry/faces.h"

class OriginalColoring : public ColorMaker
{
    friend class New1Coloring;
    friend class New2Coloring;

public:
    OriginalColoring(Filled * filled);
    OriginalColoring(const OriginalColoring & other);

    virtual QString sizeInfo() override;

protected:
    virtual void initFrom(eFillType algorithm) override;
    virtual void resetStyleRepresentation() override;
    virtual void createStyleRepresentation(DCELPtr dcel) override;
    virtual void draw(GeoGraphics * gg) override;

    static void createFacesToDo(DCELPtr dcel, FaceSet & facesToDo);
    static void assignColorsOriginal(FaceSet &facesToDo, FaceSet & whiteFaces, FaceSet & blackFaces);
    static void assignColorsToFaces(FaceSet & fset);
    static void addFaceResults(FaceSet & fset, FaceSet & whiteFaces, FaceSet & blackFaces);

public:
    FaceSet     whiteFaces;
    FaceSet     blackFaces;

    ColorSet    whiteColorSet;
    ColorSet    blackColorSet;

    bool        draw_outside_whites;
    bool        draw_inside_blacks;

private:
    FaceSet     facesToDo;
};

#endif

