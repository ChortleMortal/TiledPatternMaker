#include <QPainter>

#include "gui/model_editors/border_edit/mouse_edit_border.h"
#include "gui/viewers/geo_graphics.h"
#include "model/borders/border_blocks.h"
#include "model/motifs/tile_color_defs.h"

////////////////////////////////////////////////
///
/// BorderBlocks
///
////////////////////////////////////////////////

BorderBlocks::BorderBlocks(Mosaic * parent, QSizeF sz, QColor color, int rows, int cols, qreal width) : Border(parent)
{
    borderType  = BORDER_BLOCKS;
    this->color = color;
    this->rows  = rows;
    this->cols  = cols;
    borderWidth = width;

    QRectF rect(QPointF(),sz);
    setRect(rect);
}

BorderBlocks::BorderBlocks(Mosaic *parent, QRectF rect, QColor color, int rows, int cols, qreal width) : Border(parent)
{
    borderType  = BORDER_BLOCKS;
    this->color = color;
    this->rows  = rows;
    this->cols  = cols;
    borderWidth = width;
    setRect(rect);
}

void BorderBlocks::createStyleRepresentation()
{
    qDebug() << "BorderBlocks::construct";

    if (styled)
        return;

    if (getUseViewSize())
    {
        setBorderSize(viewControl()->getCanvas().getViewSize());
    }

    faces.clear();

    qreal side  = borderWidth * qTan(M_PI/8.0);
    qreal piece = sqrt(side*side*0.5);

    // top row
    QPointF start = getRect().topLeft();
    for (int i=0; i < cols; i++)
    {
        // trapezium
        QPolygonF tt;
        tt << (start + QPointF(0.0,               0.0));
        tt << (start + QPointF(side + (piece*2.0),0.0));
        tt << (start + QPointF(piece+side,        piece));
        tt << (start + QPointF(piece,             piece));

        EdgePoly ep(tt);
        auto f = std::make_shared<Face>(ep);
        f->color = color;
        faces.push_back(f);

        start += QPointF(borderWidth,0.0);
    }

    // bottom row
    start = getRect().bottomLeft();
    start -= QPointF(0,piece);
    for (int i=0; i < cols; i++)
    {
        // trapezium
        QPolygonF tt;
        tt << (start + QPointF(piece,             0.0));
        tt << (start + QPointF(piece+side,        0.0));
        tt << (start + QPointF(side + (piece*2.0),piece));
        tt << (start + QPointF(0.0,               piece));

        EdgePoly ep(tt);
        auto f = std::make_shared<Face>(ep);
        f->color = color;
        faces.push_back(f);

        start += QPointF(borderWidth,0.0);
    }

    side    = borderWidth * qTan(M_PI/8.0);
    piece   = sqrt(side*side*0.5);

    // left col
    start = getRect().topLeft();
    for (int i=0; i < rows; i++)
    {
        // trapezium
        QPolygonF tt;
        tt << (start + QPointF(0.0,               0.0));
        tt << (start + QPointF(piece,             piece));
        tt << (start + QPointF(piece,             side + piece));
        tt << (start + QPointF(0,                 (piece*2) + side));

        EdgePoly ep(tt);
        auto f = std::make_shared<Face>(ep);
        f->color = color;
        faces.push_back(f);

        start += QPointF(0.0,borderWidth);
    }

    // right col
    start = getRect().topRight();
    for (int i=0; i < rows; i++)
    {
        // trapezium
        QPolygonF tt;
        tt << (start + QPointF(0.0,               0.0));
        tt << (start + QPointF(0,                 (piece*2) + side));
        tt << (start + QPointF(-piece,            side +  piece));
        tt << (start + QPointF(-piece,            piece));

        EdgePoly ep(tt);
        auto f = std::make_shared<Face>(ep);
        f->color = color;
        faces.push_back(f);

        start += QPointF(0.0,borderWidth);
    }

    styled = true;
}

void  BorderBlocks::draw(GeoGraphics * gg)
{
    for (const auto & face : std::as_const(faces))
    {
        QPen pen(face->color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        QPen pen2(QColor(TileBlack), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

        gg->fillEdgePoly(*face,pen);
        gg->drawEdgePoly(*face,pen2);
    }
}

void BorderBlocks::get(QColor & color, int & rows, int & cols, qreal & width)
{
    color       = this->color;
    rows        = this->rows;
    cols        = this->cols;
    width       = borderWidth;
}

void BorderBlocks::legacy_convertToModelUnits()
{
    borderWidth = screenToModel(borderWidth);

    convertCropToModelUnits();

    resetStyleRepresentation();
}
