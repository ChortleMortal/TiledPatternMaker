#include "base/border.h"
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

#include "base/canvas.h"
#include "base/shapefactory.h"

Border::Border()
{
    type = -1;  // undefined
    setZValue(BORDER_ZLEVEL);
}

Border::~Border()
{
    //qDebug() << "Border destructor";
}

void Border::clear()
{
}

void Border::addBorder0(QPen pen, QSizeF size)
{
    type  = 0;
    pen0  = pen;
    size1 = size;

    sp = make_shared<ShapeFactory>(2.0);
    addToGroup(sp.get());

    QPolygonF poly;
    poly << QPointF(0,0) << QPointF(size.width(),0) << QPointF(size.width(),size.height()) << QPointF(0,size.height());
    sp->addPolygon(pen,QBrush(Qt::NoBrush),poly);
}

void Border::getBorder0(QPen & pen, QSizeF & size)
{
    pen =  pen0;
    size = size1;
}


// An outer border with alternating tiles
void Border::addBorder1(QColor color1, QColor color2, QSizeF size)
{
    type    = 1;
    border1 = color1;
    border2 = color2;
    size1   = size;

    sp = make_shared<ShapeFactory>(2.0);
    addToGroup(sp.get());

    qreal width   = size.width();
    qreal height  = size.height();

    border1 = color1;
    border2 = color2;

    qreal x = 0.0;
    qreal y = 0.0;

    qreal bw, bh;
    QPen pen(Qt::green,1);


    // top
    while (x < width)
    {
        bh = THICKNESS1;
        if (x + LENGTH1 > width)
        {
            bw = width-x;
        }
        else
        {
            bw = LENGTH1;
        }
        QRectF rect(x,y,bw,bh);
        QPolygonF poly;
        poly << rect.topLeft() << rect.topRight() << rect.bottomRight() << rect.bottomLeft();
        sp->addPolygon(nextBorderPen(), nextBorderBrush(), poly);
        x+= LENGTH1;
    }

    //right
    y = THICKNESS1;
    x = width - THICKNESS1;
    while (y < height)
    {

        bw = THICKNESS1;
        if (y + LENGTH1 > height)
        {
            bh = height-y;
        }
        else
        {
            bh = LENGTH1;
        }
        QRectF rect(x,y,bw,bh);
        QPolygonF poly;
        poly << rect.topLeft() << rect.topRight() << rect.bottomRight() << rect.bottomLeft();
        sp->addPolygon(nextBorderPen(), nextBorderBrush(), poly);
        y += LENGTH1;
    }

    // bottom
    y = height - THICKNESS1;
    x = width - THICKNESS1 - LENGTH1 - 1;
    while (x >= 0.0)
    {
        bh = THICKNESS1;
        bw = LENGTH1;
        QRectF rect(x,y,bw,bh);
        QPolygonF poly;
        poly << rect.topLeft() << rect.topRight() << rect.bottomRight() << rect.bottomLeft();
        sp->addPolygon(nextBorderPen(), nextBorderBrush(), poly);
        if (x - LENGTH1 < 0.0)
        {
            bw = x;
            QRectF rect(0.0,y,bw,bh);
            QPolygonF poly;
            poly << rect.topLeft() << rect.topRight() << rect.bottomRight() << rect.bottomLeft();
            sp->addPolygon(nextBorderPen(), nextBorderBrush(), poly);
        }
        x -= LENGTH1;
    }

    // left
    x = 0.0;
    y = height - THICKNESS1 - LENGTH1 -1;
    while (y >= 0.0)
    {
        bw = THICKNESS1;
        bh = LENGTH1;
        QRectF rect(x,y,bw,bh);
        QPolygonF poly;
        poly << rect.topLeft() << rect.topRight() << rect.bottomRight() << rect.bottomLeft();
        sp->addPolygon(nextBorderPen(), nextBorderBrush(), poly);
        if (y - LENGTH1 < THICKNESS1)
        {
            bh = y - THICKNESS1;
            QRectF rect(0.0,THICKNESS1,bw,bh);
            QPolygonF poly;
            poly << rect.topLeft() << rect.topRight() << rect.bottomRight() << rect.bottomLeft();
            sp->addPolygon(nextBorderPen(), nextBorderBrush(), poly);
        }
        y -= LENGTH1;
    }
}

void Border::getBorder1(QColor & color1, QColor & color2, QSizeF & size)
{
    color1 = border1;
    color2 = border2;
    size   = size1;
}

void Border::addBorder2(QColor color1, qreal diameter, int rows, int cols)
{
    type      = 2;
    border1   = color1;
    diameter2 = diameter;
    rows2     = rows;
    cols2     = cols;

    sp = make_shared<ShapeFactory>(2.0);
    addToGroup(sp.get());

    qreal side  = diameter * qTan(M_PI/8.0);
    qreal piece = sqrt(side*side*0.5);

    QBrush brush(color1);
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

        start += QPointF(150.0,0.0);
    }

    // bottom row
    start.setX(0.0);
    start.setY(fabs(piece + (diameter*rows)) + 0.5);
    for (int i=0; i < cols; i++)
    {

        // trapezium
        QPolygonF tt;
        tt << (start + QPointF(piece,             0.0));
        tt << (start + QPointF(piece+side,        0.0));
        tt << (start + QPointF(side + (piece*2.0),piece));
        tt << (start + QPointF(0.0,               piece));

        sp->addPolygon(pen,brush,tt);

        start += QPointF(150.0,0.0);
    }
}

void Border::getBorder2(QColor & color1, qreal & diameter, int & rows, int & cols)
{
    color1      = border1;
    diameter    = diameter2;
    rows        = rows2;
    cols        = cols2;
}

QBrush Border::nextBorderBrush()
{
    static int i = 0;
    if (i==0)
    {
        i = 1;
        return QBrush(border1);
    }
    else
    {
        i = 0;
        return QBrush(border2);
    }
}

QPen Border::nextBorderPen()
{
    static int i = 0;
    if (i==0)
    {
        i = 1;
        return QPen(border1,1);
    }
    else
    {
        i = 0;
        return QPen(border2,1);
    }

}
