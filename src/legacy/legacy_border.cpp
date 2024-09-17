#include <QtMath>
#include "legacy/legacy_border.h"
#include "legacy/shapefactory.h"
#include "sys/geometry/map.h"
#include "model/motifs/tile_color_defs.h"

/*
 * A border (defined in model units) is both a a layer, and a crop
 * The crop defines its dimensions and type
 *
 * ViewControl adds a border as a layer
 */

LegacyBorder::LegacyBorder() : Layer("Border",false)
{
    borderType  = BORDER_NONE;

    setZValue(BORDER_ZLEVEL);
    sp = std::make_shared<ShapeFactory>(2.0);
    addSubLayer(sp);
}

void LegacyBorder::setModelXform(const Xform & xf, bool update)
{
    Q_ASSERT(!_unique);
    if (debug & DEBUG_XFORM) qInfo().noquote() << "SET" << getLayerName() << xf.info() << (isUnique() ? "unique" : "common");
    viewControl->setCurrentModelXform(xf,update);
}

const Xform & LegacyBorder::getModelXform()
{
    Q_ASSERT(!_unique);
    if (debug & DEBUG_XFORM) qInfo().noquote() << "SET" << getLayerName() << viewControl->getCurrentModelXform().info() << (isUnique() ? "unique" : "common");
    return viewControl->getCurrentModelXform();
}

////////////////////////////////////////////////
///
/// BorderTwoColor
/// An outer border with alternating tiles
///
////////////////////////////////////////////////

LegacyBorderTwoColor::LegacyBorderTwoColor(QSizeF sz, QColor color1, QColor color2, qreal width) : LegacyBorder()
{
    borderType   = BORDER_TWO_COLOR;
    color        = color1;
    this->color2 = color2;
    this->width  = width;

    QRectF rect(QPointF(),sz);
    setRect(rect);
}

LegacyBorderTwoColor::LegacyBorderTwoColor(QRectF rect, QColor color1, QColor color2, qreal width) : LegacyBorder()
{
    borderType   = BORDER_TWO_COLOR;
    color        = color1;
    this->color2 = color2;
    this->width  = width;
    setRect(rect);
}

void LegacyBorderTwoColor::construct()
{
    sp->reset();

    qreal w  = _rect.width();
    qreal h  = _rect.height();

    qreal x = 0.0;
    qreal y = 0.0;

    qreal bw, bh;

    // top
    while (x < w)
    {
        bh = width;
        if (x + LENGTH1 > w)
        {
            bw = w-x;
        }
        else
        {
            bw = LENGTH1;
        }
        QRectF rect(x,y,bw,bh);
        QPolygonF poly(rect);
        sp->addPolygon(nextBorderPen(), nextBorderBrush(), poly);
        x+= LENGTH1;
    }

    //right
    y = width;
    x = w - width;
    while (y < h)
    {

        bw = width;
        if (y + LENGTH1 > h)
        {
            bh = h-y;
        }
        else
        {
            bh = LENGTH1;
        }
        QRectF rect(x,y,bw,bh);
        QPolygonF poly(rect);
        sp->addPolygon(nextBorderPen(), nextBorderBrush(), poly);
        y += LENGTH1;
    }

    // bottom
    y = h - width;
    x = w - width - LENGTH1 - 1;
    while (x >= 0.0)
    {
        bh = width;
        bw = LENGTH1;
        QRectF rect(x,y,bw,bh);
        QPolygonF poly(rect);
        sp->addPolygon(nextBorderPen(), nextBorderBrush(), poly);
        if (x - LENGTH1 < 0.0)
        {
            bw = x;
            QRectF rect(0.0,y,bw,bh);
            QPolygonF poly(rect);
            sp->addPolygon(nextBorderPen(), nextBorderBrush(), poly);
        }
        x -= LENGTH1;
    }

    // left
    x = 0.0;
    y = h - width - LENGTH1 -1;
    while (y >= 0.0)
    {
        bw = width;
        bh = LENGTH1;
        QRectF rect(x,y,bw,bh);
        QPolygonF poly(rect);
        sp->addPolygon(nextBorderPen(), nextBorderBrush(), poly);
        if (y - LENGTH1 < width)
        {
            bh = y - width;
            QRectF rect(0.0,width,bw,bh);
            QPolygonF poly(rect);
            sp->addPolygon(nextBorderPen(), nextBorderBrush(), poly);
        }
        y -= LENGTH1;
    }
}

void LegacyBorderTwoColor::get(QColor & color1, QColor & color2, qreal & width)
{
    color1 = this->color;
    color2 = this->color2;
    width  = this->width;
}

QBrush LegacyBorderTwoColor::nextBorderBrush()
{
    static int i = 0;
    if (i==0)
    {
        i = 1;
        return QBrush(color);
    }
    else
    {
        i = 0;
        return QBrush(color2);
    }
}

QPen LegacyBorderTwoColor::nextBorderPen()
{
    static int i = 0;
    if (i==0)
    {
        i = 1;
        return QPen(color,1);
    }
    else
    {
        i = 0;
        return QPen(color2,1);
    }
}

////////////////////////////////////////////////
///
/// BorderBlocks
///
////////////////////////////////////////////////

LegacyBorderBlocks::LegacyBorderBlocks(QSizeF sz, QColor color, qreal width, int rows, int cols) : LegacyBorder()
{
    borderType  = BORDER_BLOCKS;
    this->color = color;
    this->width = width;
    this->rows  = rows;
    this->cols  = cols;

    QRectF rect(QPointF(),sz);
    setRect(rect);
}

LegacyBorderBlocks::LegacyBorderBlocks(QRectF rect, QColor color, qreal width, int rows, int cols) : LegacyBorder()
{
    borderType  = BORDER_BLOCKS;
    this->color = color;
    this->width = width;
    this->rows  = rows;
    this->cols  = cols;
    setRect(rect);
}

void LegacyBorderBlocks::construct()
{
    sp->reset();

    qreal w  = _rect.width();
    qreal h  = _rect.height();

    width = w /cols;

    qreal side  = width * qTan(M_PI/8.0);
    qreal piece = sqrt(side*side*0.5);

    QBrush brush(color);
    QPen   pen(QColor(TileBlack),1.0);

    // top row
    QPointF start(0.0,0.0);
    for (int i=0; i < cols; i++)
    {
        // trapezium
        QPolygonF tt;
        tt << (start + QPointF(0.0,               0.0));
        tt << (start + QPointF(side + (piece*2.0),0.0));
        tt << (start + QPointF(piece+side,        piece));
        tt << (start + QPointF(piece,             piece));
        sp->addPolygon(pen,brush,tt);

        start += QPointF(width,0.0);
    }

    // bottom row
    start.setX(0.0);
    start.setY(h-piece);
    for (int i=0; i < cols; i++)
    {
        // trapezium
        QPolygonF tt;
        tt << (start + QPointF(piece,             0.0));
        tt << (start + QPointF(piece+side,        0.0));
        tt << (start + QPointF(side + (piece*2.0),piece));
        tt << (start + QPointF(0.0,               piece));

        sp->addPolygon(pen,brush,tt);

        start += QPointF(width,0.0);
    }

    width = h /rows;

    side  = width * qTan(M_PI/8.0);
    piece = sqrt(side*side*0.5);

    // left col
    start= QPointF(0.0,0.0);
    for (int i=0; i < rows; i++)
    {
        // trapezium
        QPolygonF tt;
        tt << (start + QPointF(0.0,               0.0));
        tt << (start + QPointF(piece,             piece));
        tt << (start + QPointF(piece,             side + piece));
        tt << (start + QPointF(0,                 (piece*2) + side));
        sp->addPolygon(pen,brush,tt);

        start += QPointF(0.0,width);
    }

    // right col
    start = QPointF(w,0);
    for (int i=0; i < rows; i++)
    {
        // trapezium
        QPolygonF tt;
        tt << (start + QPointF(0.0,               0.0));
        tt << (start + QPointF(0,                 (piece*2) + side));
        tt << (start + QPointF(-piece,            side +  piece));
        tt << (start + QPointF(-piece,            piece));
        sp->addPolygon(pen,brush,tt);

        start += QPointF(0.0,width);
    }
}

void LegacyBorderBlocks::get(QColor & color, qreal & diameter, int & rows, int & cols)
{
    color       = this->color;
    diameter    = this->width;
    rows        = this->rows;
    cols        = this->cols;
}
