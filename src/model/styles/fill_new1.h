#pragma once
#ifndef FILL_NEW1
#define GFILL_NEW1

#include "model/styles/fill_color_maker.h"
#include "model/styles/colorset.h"
#include "sys/enums/efilltype.h"
#include "sys/geometry/dcel.h"
#include "sys/geometry/faces.h"

class New1Coloring : public ColorMaker
{
public:
    New1Coloring(Filled * filled);
    New1Coloring(const New1Coloring & other);

    virtual QString sizeInfo() override;

protected:
    virtual void initFrom(eFillType algorithm) override;
    virtual void resetStyleRepresentation() override;
    virtual void createStyleRepresentation(DCELPtr dcel) override;
    virtual void draw(GeoGraphics * gg) override;

    void assignColorsNew1();

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

