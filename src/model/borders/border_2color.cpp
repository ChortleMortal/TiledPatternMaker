#include <QPainter>

#include "gui/viewers/geo_graphics.h"
#include "model/borders/border_2color.h"

////////////////////////////////////////////////
///
/// BorderTwoColor
/// An outer border with alternating tiles
///
////////////////////////////////////////////////

BorderTwoColor::BorderTwoColor(Mosaic * parent, QSizeF sz, QColor color1, QColor color2, qreal width, qreal len) : Border(parent)
{
    borderType   = BORDER_TWO_COLOR;
    color        = color1;
    this->color2 = color2;
    borderWidth  = width;
    segmentLen   = len;
    QRectF rect(QPointF(),sz);
    setRect(rect);
}

BorderTwoColor::BorderTwoColor(Mosaic * parent, QRectF rect, QColor color1, QColor color2, qreal width, qreal len) : Border(parent)
{
    borderType   = BORDER_TWO_COLOR;
    color        = color1;
    this->color2 = color2;
    borderWidth  = width;
    segmentLen   = len;
    setRect(rect);
}

void BorderTwoColor::createStyleRepresentation()
{
    // TODO - the question is: where should the border be?
    //  a) inside the croop (a true border)
    //  b) outside the crop (a frame)
    //  c) aligned with moddle of the crop (straddling it)

    if (styled)
        return;

    if (getUseViewSize())
    {
        setBorderSize(viewControl()->getCanvas().getViewSize());
    }

    faces.clear();

    qreal x,y;
    qreal bw, bh;

    QPointF tl = getRect().topLeft();
    QPointF tr = getRect().topRight();
    QPointF br = getRect().bottomRight();
    QPointF bl = getRect().bottomLeft();

    // top edge
    x = tl.x();
    y = tl.y();
    while (x < tr.x())
    {
        bw = segmentLen;
        bh = borderWidth;
        if (x + segmentLen > tr.x())
        {
            bw = tr.x() - x;
        }

        addSegment(x,y,bw,bh);
        x += segmentLen;
    }

    // right edge
    x = tr.x();
    y = tr.y() + borderWidth;
    while (y < br.y())
    {
        bw = borderWidth;
        bh = segmentLen;
        if (y + segmentLen > br.y())
        {
            bh = br.y() - y;
        }

        addSegment(x-borderWidth,y,bw,bh);
        y += segmentLen;
    }

    // bottom edge
    x = br.x() - borderWidth;
    y = br.y();
    while (x > bl.x())
    {
        bw = segmentLen;
        bh = borderWidth;
        if (x - segmentLen < bl.x())
        {
            bw = x - bl.x();
        }

        addSegment(x-bw,y-borderWidth,bw,bh);
        x -= segmentLen;
    }

    // left edge - final piece should not overlap
    x = bl.x();
    y = bl.y() - borderWidth;
    while (y > (tl.y() - borderWidth))
    {
        bw = borderWidth;
        bh = segmentLen;
        if (y - segmentLen < tl.y())
        {
            bh = y - tl.y() - borderWidth;
        }

        addSegment(x,y-bh,bw,bh);
        y -= segmentLen;
    }

    styled = true;
}

void BorderTwoColor::addSegment(qreal x, qreal y, qreal width, qreal height)
{
    QRectF rect(x,y,width,height);

    EdgePoly ep(rect);
    auto f = std::make_shared<Face>(ep);
    f->color = nextBorderColor();
    faces.push_back(f);
}

void BorderTwoColor::draw(GeoGraphics * gg)
{
    for (const auto & face : std::as_const(faces))
    {
        QPen pen(face->color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        gg->fillEdgePoly(*face,pen);
        gg->drawEdgePoly(*face,pen);
    }
}

void BorderTwoColor::get(QColor & color1, QColor & color2, qreal & width, qreal & length)
{
    color1 = this->color;
    color2 = this->color2;
    width  = borderWidth;
    length = segmentLen;
}

QColor BorderTwoColor::nextBorderColor()
{
    static int i = 0;
    if (i==0)
    {
        i = 1;
        return color;
    }
    else
    {
        i = 0;
        return color2;
    }
}

void BorderTwoColor::legacy_convertToModelUnits()
{
    borderWidth = screenToModel(borderWidth);
    segmentLen  = screenToModel(segmentLen);

    convertCropToModelUnits();

    resetStyleRepresentation();
}

