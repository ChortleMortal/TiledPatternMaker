#pragma once
#ifndef FILL_SET_H
#define FILL_SET_H

#include "model/styles/fill_color_maker.h"
#include "model/styles/colorset.h"
#include "sys/enums/efilltype.h"
#include "sys/geometry/dcel.h"
#include "sys/geometry/faces.h"

class New2Coloring : public ColorMaker
{
    friend class New3Coloring;

public:
    New2Coloring(Filled * filled);

    virtual QString sizeInfo() override;

protected:
    virtual void initFrom(eFillType algorithm) override;
    virtual void resetStyleRepresentation() override;
    virtual void createStyleRepresentation(DCELPtr dcel) override;
    virtual void draw(GeoGraphics * gg) override;

    static void buildFaceGroup(DCELPtr dcel, FaceSet &facesToDo, FaceGroup &faceGroup);

    void adjustColorGroupSizeIfNeeded();

    void removeOverlappingFaces();

public:
    FaceSet     facesToDo;

    FaceGroup   faceGroup;

    ColorSet    colorSet;
};

#endif

