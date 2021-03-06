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

#ifndef BORDER_H
#define BORDER_H

#include <QtCore>
#include <QtWidgets>
#include "base/layer.h"

typedef std::shared_ptr<class Crop>         CropPtr;
typedef std::shared_ptr<class ShapeFactory> ShapeFPtr;

#define LENGTH1        60.0
#define BORDER_ZLEVEL  10

// these enum numbers are use in xml writer/reader - do not change
enum eBorderType
{
    BORDER_NONE         = 0,
    BORDER_PLAIN        = 1,
    BORDER_TWO_COLOR    = 2,
    BORDER_BLOCKS       = 3,
    BORDER_INTEGRATED   = 4
};

class Border : public Layer
{
    Q_OBJECT

public:
    virtual void  construct() = 0;

    eBorderType getType()  { return type; }

public slots:
    virtual void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
    virtual void slot_mouseDragged(QPointF spt)       override;
    virtual void slot_mouseTranslate(QPointF pt)      override;
    virtual void slot_mouseMoved(QPointF spt)         override;
    virtual void slot_mouseReleased(QPointF spt)      override;
    virtual void slot_mouseDoublePressed(QPointF spt) override;

    virtual void slot_wheel_scale(qreal delta)  override;
    virtual void slot_wheel_rotate(qreal delta) override;

    virtual void slot_scale(int amount)  override;
    virtual void slot_rotate(int amount) override;
    virtual void slot_moveX(int amount)  override;
    virtual void slot_moveY(int amount)  override;

protected:
    Border();

    eBorderType type;
};

class InnerBorder : public Border
{
public:
    InnerBorder();

    void    construct() {}

    void    setInnerBoundary(CropPtr crect);
    CropPtr getInnerBoundary() { return innerBoundary; }

protected:
    CropPtr innerBoundary;
};

class OuterBorder : public Border
{
    Q_OBJECT

public:
    OuterBorder(QSize sz);

    void    setOuterBoundary(QRectF rect);
    QRectF  getOuterBoundary()  { return outerBoundary; }

    void    setWidth(qreal width) { this->width = width; construct(); }
    void    setColor(QColor color){ this->color = color; construct(); }

    qreal   getWidth() { return width; }
    QColor  getColor() { return color; }

public slots:
    void    resize(QSize sz);

protected:
    QRectF      outerBoundary;
    ShapeFPtr   sp;
    qreal       width;
    QColor      color;

};


class BorderPlain : public OuterBorder
{
public:
    BorderPlain(QSize sz, qreal width, QColor color);
    void construct();

    void get(qreal & width, QColor & color);
};

class BorderTwoColor : public OuterBorder
{
public:
    BorderTwoColor(QSize sz, QColor color1, QColor color2, qreal width);
    void    construct();

    void    get(QColor & color1, QColor & color2, qreal & width);
    QColor  getColor2() { return color2; }

    void    setColor2(QColor color) { color2 = color;  construct(); }

protected:
    QPen    nextBorderPen();
    QBrush  nextBorderBrush();

private:
    QColor      color2;
};

class BorderBlocks : public OuterBorder
{
public:
    BorderBlocks(QSize sz, QColor color, qreal diameter, int rows, int cols);
    void    construct();

    void    get(QColor & color1, qreal & diameter, int & rows, int & cols);
    void    setRows(int rows)   { this->rows = rows; construct(); }
    void    setCols(int cols)   { this->cols = cols; construct(); }

protected:

private:
    int         rows;
    int         cols;
};

#endif // BORDER_H
