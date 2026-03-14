#pragma once
#ifndef BORDER_PLAIN_H
#define BORDER_PLAIN_H

#include "sys/geometry/crop.h"
#include "model/borders/border.h"

typedef std::shared_ptr<class MouseEditBorder> BorderMouseActionPtr;

typedef std::shared_ptr<class Map> MapPtr;

class BorderPlain : public Border
{
public:
    BorderPlain(Mosaic * parent, const Crop &crop, qreal width, QColor color);
    BorderPlain(Mosaic * parent, Circle c,         qreal width, QColor color);
    BorderPlain(Mosaic * parent, QRectF rect,      qreal width, QColor color);
    BorderPlain(Mosaic * parent, QPolygonF poly,   qreal width, QColor color);
    BorderPlain(Mosaic * parent, QSizeF sz,        qreal width, QColor color);
    BorderPlain(Mosaic * parent);

    virtual void createStyleRepresentation() override;
    virtual void draw(GeoGraphics * gg) override;

    void get(qreal & width, QColor & color);

    void legacy_convertToModelUnits() override;

    MapPtr  bmap;
};

#endif

