#pragma once
#ifndef BORDER_BLOCKS_H
#define BORDER_BLOCKS_H

#include "sys/geometry/crop.h"
#include "sys/geometry/faces.h"
#include "model/borders/border.h"

class BorderBlocks : public Border
{
public:
    BorderBlocks(Mosaic * parent, QSizeF sz,   QColor color, int rows, int cols, qreal width);
    BorderBlocks(Mosaic * parent, QRectF rect, QColor color, int rows, int cols, qreal width);

    virtual void createStyleRepresentation() override;
    virtual void draw(GeoGraphics * gg) override;

    void    get(QColor & color1, int & rows, int & cols, qreal & width);
    void    setRows(int rows)   { this->rows = rows; resetStyleRepresentation(); }
    void    setCols(int cols)   { this->cols = cols; resetStyleRepresentation(); }

    void    legacy_convertToModelUnits() override;

protected:

private:
    int         rows;
    int         cols;
    FaceSet     faces;
};

#endif

