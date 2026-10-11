#pragma once

#include "sys/geometry/crop.h"
#include "sys/geometry/faces.h"
#include "model/borders/border.h"

class BorderTwoColor : public Border
{
public:
    BorderTwoColor(Mosaic * parent, QSizeF sz,   QColor color1, QColor color2, qreal width, qreal len);
    BorderTwoColor(Mosaic * parent, QRectF rect, QColor color1, QColor color2, qreal width, qreal len);

    virtual void createStyleRepresentation() override;
    virtual void draw(GeoGraphics * gg) override;

    qreal   getLength()             { return segmentLen; }
    void    setLength(qreal length) { segmentLen = length;  resetStyleRepresentation(); }

    void    get(QColor & color1, QColor & color2, qreal & width, qreal &length);

    void    legacy_convertToModelUnits() override;

protected:
    int     segmentCount(qreal pathLength, qreal hint) const;
    QColor  nextBorderColor();
    void    addSegment(qreal x, qreal y, qreal width, qreal height);

    qreal   segmentLen;

private:
    FaceSet faces;
};

