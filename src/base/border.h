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
#include "base/tile.h"
#include "base/shared.h"
#include "base/layer.h"
#include "viewers/view.h"
#include "viewers/viewcontrol.h"

#define LENGTH1        60.0
#define BORDER_ZLEVEL  10

enum eBorderType
{
    BORDER_NONE,
    BORDER_PLAIN,
    BORDER_TWO_COLOR,
    BORDER_BLOCKS
};

class Border : public Layer
{
    Q_OBJECT

public:
    virtual void  construct() = 0;

    eBorderType getType()  { return type; }

    void    setWidth(qreal width) { this->width = width; construct(); }
    void    setColor(QColor color){ this->color = color; construct(); }

    qreal   getWidth() { return width; }
    QColor  getColor() { return color; }

public slots:
    void    resize(QSize sz);

protected:
    Border(QSize sz);
    ~Border();

    eBorderType type;
    QSizeF      size;
    QColor      color;
    qreal       width;
    ShapeFPtr   sp;
};

class BorderPlain : public Border
{
public:
    BorderPlain(QSize sz, qreal width, QColor color);
    void construct();

    void get(qreal & width, QColor & color);
};

class BorderTwoColor : public Border
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

class BorderBlocks : public Border
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
