#pragma once
#ifndef FILL_GROUP_H
#define FILL_GROUP_H

#include "model/styles/fill_color_maker.h"
#include "model/styles/colorset.h"
#include "sys/enums/efilltype.h"
#include "sys/geometry/dcel.h"
#include "sys/geometry/faces.h"

class New3Coloring : public ColorMaker
{
public:
    New3Coloring(Filled * filled);

    void adjustColorGroupSizeIfNeeded();

    virtual QString sizeInfo() override;

protected:
    virtual void initFrom(eFillType algorithm) override;
    virtual void resetStyleRepresentation() override;
    virtual void createStyleRepresentation(DCELPtr dcel) override;
    virtual void draw(GeoGraphics * gg) override;

public:
    FaceGroup   faceGroup;
    ColorGroup  colorGroup;

private:
    FaceSet     facesToDo;
};

#endif

