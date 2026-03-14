#include <QPainter>

#include "gui/model_editors/border_edit/mouse_edit_border.h"
#include "gui/viewers/geo_graphics.h"
#include "model/borders/border_plain.h"
#include "model/motifs/tile_color_defs.h"
#include "sys/geometry/crop.h"
#include "sys/geometry/map.h"

////////////////////////////////////////////////
///
/// BorderPlain
///
////////////////////////////////////////////////

BorderPlain::BorderPlain(Mosaic *parent, QSizeF sz, qreal width, QColor color) : Border(parent)
{
    borderType   = BORDER_PLAIN;
    borderWidth  = width;
    this->color  = color;

    QRectF rect(QPointF(),sz);
    setRect(rect);
}

BorderPlain::BorderPlain(Mosaic * parent, QRectF rect, qreal width, QColor color) : Border(parent)
{
    borderType   = BORDER_PLAIN;
    borderWidth  = width;
    this->color  = color;

    setRect(rect);
}

BorderPlain::BorderPlain(Mosaic * parent, Circle c, qreal width, QColor color) : Border(parent)
{
    borderType   = BORDER_PLAIN;
    borderWidth  = width;
    this->color  = color;

    setCircle(c);
}

BorderPlain::BorderPlain(Mosaic * parent, QPolygonF p, qreal width, QColor color) : Border(parent)
{
    borderType   = BORDER_PLAIN;
    borderWidth  = width;
    this->color  = color;

    setPolygon(p);
}

BorderPlain::BorderPlain(Mosaic *parent, const Crop & crop, qreal width, QColor color) : Border(parent, crop)
{
    // some defaults
    borderType   = BORDER_PLAIN;
    borderWidth  = width;
    this->color  = color;
}

BorderPlain::BorderPlain(Mosaic * parent) : Border(parent)
{
    borderType   = BORDER_PLAIN;
    borderWidth  = 20;
    color        = QColor(AlhambraBrown);
    QRectF rect(0,0,0,0);
    setRect(rect);
    setUseViewSize(true);
}

void BorderPlain::createStyleRepresentation()
{
    if (styled)
        return;

    if (getUseViewSize())
    {
        setBorderSize(viewControl()->getCanvas().getViewSize());
    }

    bmap = std::make_shared<Map>("Border");

    QPen pen(color,borderWidth);
    pen.setJoinStyle(Qt::MiterJoin);

    switch (_cropType)
    {
    case CROP_RECTANGLE:
        bmap = std::make_shared<Map>("Border",QPolygonF(getRect()));
        break;

    case CROP_POLYGON:
        bmap = std::make_shared<Map>("Border",getAPolygon().get());
        break;

    case CROP_CIRCLE:
    {
        EdgePoly ep(getCircle());
        bmap = std::make_shared<Map>("Border",ep);
    }   break;

    default:
        qWarning("Not implemented yet");
        break;
    }

    styled = true;
}

void  BorderPlain::draw(GeoGraphics *gg)
{
    QPen pen(color,borderWidth);
    for (const auto & edge : std::as_const(bmap->getEdges()))
    {
        gg->drawEdge(edge,pen);
    }
}

void BorderPlain::get(qreal & width, QColor & color)
{
    width = borderWidth;
    color = this->color;
}

void BorderPlain::legacy_convertToModelUnits()
{
    convertCropToModelUnits();

    resetStyleRepresentation();
}

