/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "base/border.h"
#include "designs/shapefactory.h"
#include "geometry/crop.h"
#include "geometry/edgepoly.h"
#include "geometry/map.h"
#include "viewers/view.h"

////////////////////////////////////////////////
///
/// Border
///
////////////////////////////////////////////////

Border::Border() : Layer("Border")
{
    type = BORDER_NONE;
}

void Border::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{ Q_UNUSED(spt); Q_UNUSED(btn);}
void Border::slot_mouseDragged(QPointF spt)
{ Q_UNUSED(spt)}
void Border::slot_mouseTranslate(QPointF pt)
{ Q_UNUSED(pt)}
void Border::slot_mouseMoved(QPointF spt)
{ Q_UNUSED(spt)}
void Border::slot_mouseReleased(QPointF spt)
{ Q_UNUSED(spt)}
void Border::slot_mouseDoublePressed(QPointF spt)
{ Q_UNUSED(spt)}
void Border::slot_wheel_scale(qreal delta)
{ Q_UNUSED(delta);}
void Border::slot_wheel_rotate(qreal delta)
{ Q_UNUSED(delta);}
void Border::slot_scale(int amount)
{ Q_UNUSED(amount);}
void Border::slot_rotate(int amount)
{ Q_UNUSED(amount);}
void Border:: slot_moveX(int amount)
{ Q_UNUSED(amount);}
void Border::slot_moveY(int amount)
{ Q_UNUSED(amount);}


////////////////////////////////////////////////
///
///  InnerBorder
///
////////////////////////////////////////////////

InnerBorder::InnerBorder()
{
    type = BORDER_INTEGRATED;
    innerBoundary = std::make_shared<Crop>();
}

void InnerBorder::setInnerBoundary(CropPtr crect)
{
    innerBoundary = crect;
}

////////////////////////////////////////////////
///
/// OuterBorder
///
////////////////////////////////////////////////

OuterBorder::OuterBorder(QSize sz)
{
    outerBoundary  = QRectF(QPointF(),QSizeF(sz));

    setZValue(BORDER_ZLEVEL);
    sp = std::make_shared<ShapeFactory>(2.0);
    addSubLayer(sp);

    View * view = View::getInstance();
    connect(view, &View::sig_viewSizeChanged, this, &OuterBorder::resize);
}

void OuterBorder::resize(QSize sz)
{
    outerBoundary = QRectF(QPointF(),QSizeF(sz));
    construct();
}

void OuterBorder::setOuterBoundary(QRectF rect)
{
    outerBoundary = rect;
}

////////////////////////////////////////////////
///
/// BorderPlain
///
////////////////////////////////////////////////

BorderPlain::BorderPlain(QSize sz, qreal width, QColor color) : OuterBorder(sz)
{
    type         = BORDER_PLAIN;
    this->width  = width;
    this->color  = color;
}

void BorderPlain::construct()
{
    sp->reset();
    QPen pen(color,width);
    QPolygonF poly(outerBoundary);
    sp->addPolygon(pen,QBrush(Qt::NoBrush),poly);
}

void BorderPlain::get(qreal & width, QColor & color)
{
    width = this->width;
    color = this->color;
}

////////////////////////////////////////////////
///
/// BorderTwoColor
/// An outer border with alternating tiles
///
////////////////////////////////////////////////

BorderTwoColor::BorderTwoColor(QSize sz, QColor color1, QColor color2, qreal width) : OuterBorder(sz)
{
    type         = BORDER_TWO_COLOR;
    color        = color1;
    this->color2 = color2;
    this->width  = width;
}

void BorderTwoColor::construct()
{
    sp->reset();

    qreal w  = outerBoundary.width();
    qreal h  = outerBoundary.height();

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

void BorderTwoColor::get(QColor & color1, QColor & color2, qreal & width)
{
    color1 = color;
    color2 = this->color2;
    width  = this->width;
}

QBrush BorderTwoColor::nextBorderBrush()
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

QPen BorderTwoColor::nextBorderPen()
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

BorderBlocks::BorderBlocks(QSize sz, QColor color, qreal diameter, int rows, int cols) : OuterBorder(sz)
{
    type        = BORDER_BLOCKS;
    this->color = color;
    this->width = diameter;
    this->rows  = rows;
    this->cols  = cols;
}

void BorderBlocks::construct()
{
    sp->reset();

    qreal w  = outerBoundary.width();
    qreal h  = outerBoundary.height();

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

void BorderBlocks::get(QColor & color, qreal & diameter, int & rows, int & cols)
{
    color       = this->color;
    diameter    = this->width;
    rows        = this->rows;
    cols        = this->cols;
}
