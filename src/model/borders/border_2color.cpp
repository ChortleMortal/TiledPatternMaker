#include <QPainter>

#include "gui/top/view_controller.h"
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
int BorderTwoColor::segmentCount(qreal pathLength, qreal hint) const
{
    if (pathLength <= 0 || hint <= 0)
        return 1;
    return qMax(1, qRound(pathLength / hint));
}

void BorderTwoColor::createStyleRepresentation()
{
    if (styled)
        return;

    if (getUseViewSize())
    {
        setBorderSize(viewControl()->viewSize());
    }

    faces.clear();

    QPointF tl = getRect().topLeft();
    QPointF tr = getRect().topRight();
    QPointF br = getRect().bottomRight();
    QPointF bl = getRect().bottomLeft();

    // top edge: tl.x() -> tr.x()
    {
        qreal edgeStart = tl.x();
        qreal edgeEnd   = tr.x();
        qreal pathLength = edgeEnd - edgeStart;
        int n = segmentCount(pathLength, segmentLen);

        for (int i = 0; i < n; ++i)
        {
            qreal x0 = edgeStart + pathLength * i / n;
            qreal x1 = (i == n - 1) ? edgeEnd : edgeStart + pathLength * (i + 1) / n;
            addSegment(x0, tl.y(), x1 - x0, borderWidth);
        }
    }

    // right edge: (tr.y()+borderWidth) -> br.y()
    {
        qreal edgeStart = tr.y() + borderWidth;
        qreal edgeEnd   = br.y();
        qreal pathLength = edgeEnd - edgeStart;
        int n = segmentCount(pathLength, segmentLen);

        for (int i = 0; i < n; ++i)
        {
            qreal y0 = edgeStart + pathLength * i / n;
            qreal y1 = (i == n - 1) ? edgeEnd : edgeStart + pathLength * (i + 1) / n;
            addSegment(tr.x() - borderWidth, y0, borderWidth, y1 - y0);
        }
    }

    // bottom edge: (br.x()-borderWidth) -> bl.x()   (decreasing x)
    {
        qreal edgeStart = br.x() - borderWidth;
        qreal edgeEnd   = bl.x();
        qreal pathLength = edgeStart - edgeEnd;
        int n = segmentCount(pathLength, segmentLen);
        qreal y = br.y() - borderWidth;

        for (int i = 0; i < n; ++i)
        {
            qreal x1 = edgeStart - pathLength * i / n;
            qreal x0 = (i == n - 1) ? edgeEnd : edgeStart - pathLength * (i + 1) / n;
            addSegment(x0, y, x1 - x0, borderWidth);
        }
    }

    // left edge: (bl.y()-borderWidth) -> (tl.y()+borderWidth)   (decreasing y)
    {
        qreal edgeStart = bl.y() - borderWidth;
        qreal edgeEnd   = tl.y() + borderWidth;
        qreal pathLength = edgeStart - edgeEnd;
        int n = segmentCount(pathLength, segmentLen);

        for (int i = 0; i < n; ++i)
        {
            qreal y1 = edgeStart - pathLength * i / n;
            qreal y0 = (i == n - 1) ? edgeEnd : edgeStart - pathLength * (i + 1) / n;
            addSegment(bl.x(), y0, borderWidth, y1 - y0);
        }
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
    for (const auto & face : std::as_const(faces) | std::views::reverse)
    {
        QPen pen(face->color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        gg->fillEdgePoly(*face,pen);
        //gg->drawEdgePoly(*face,pen);
    }

    QPen pen(color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    EdgePoly outerEp(getRect());
    auto outerFace = std::make_shared<Face>(outerEp);
    gg->drawEdgePoly(*outerFace, pen);
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
    return (faces.size() % 2 == 0) ? color : color2;
}

void BorderTwoColor::legacy_convertToModelUnits()
{
    borderWidth = screenToModel(borderWidth);
    segmentLen  = screenToModel(segmentLen);

    convertCropToModelUnits();

    resetStyleRepresentation();
}

