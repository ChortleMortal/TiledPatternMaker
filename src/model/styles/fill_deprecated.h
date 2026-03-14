#pragma once
#ifndef FILL_DEPRECATED_H
#define FILL_DEPRECATED_H

#include "model/styles/fill_color_maker.h"
#include "model/styles/colorset.h"
#include "sys/enums/efilltype.h"
#include "sys/geometry/dcel.h"
#include "sys/geometry/faces.h"

class DeprecatedDirectColoring : public ColorMaker
{
public:
    DeprecatedDirectColoring(Filled * filled);

    void setPaletteIndices(QVector<int> & indices) { faceColorIndices = indices; }

    virtual QString sizeInfo() override;

protected:
    virtual void initFrom(eFillType algorithm) override;
    virtual void resetStyleRepresentation() override;
    virtual void createStyleRepresentation(DCELPtr dcel) override;
    virtual void draw(GeoGraphics * gg) override;

    void removePaletteIndex(int index);

    void resetColorIndices() { faceColorIndices.fill(-1);}
    int  numColorIndices() { return faceColorIndices.size(); }

    int  getColorIndex(int faceIndex);
    void setColorIndex(int faceIndex,int colorIndex);

    QColor          getColorFromPalette(int index)            { return palette.getQColor(index); }
    QVector<int> &  getPaletteIndices()                       { return faceColorIndices; }

    void initDCELFaces(DCELPtr dcel);
    void assignPaletteColors(DCELPtr dcel);

public:
    ColorSet        palette;

private:
    QVector<int>    faceColorIndices;
    bool            converted;
};
#endif

