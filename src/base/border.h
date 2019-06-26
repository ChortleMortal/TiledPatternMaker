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

#include "base/tile.h"
#include "base/shared.h"
#include <QtCore>
#include <QtWidgets>

#define LENGTH1        60.0
#define THICKNESS1     20.0
#define BORDER_ZLEVEL  10.0

class Border : public QGraphicsItemGroup
{
public:
    Border();
    ~Border();

    void clear();

    void addBorder0(QPen pen, QSizeF size);
    void addBorder1(QColor color1, QColor color2, QSizeF size);
    void addBorder2(QColor color1, qreal diameter, int rows, int cols);

    int  getType() { return type; }
    void getBorder0(QPen & pen, QSizeF & size);
    void getBorder1(QColor & color1, QColor & color2, QSizeF & size);
    void getBorder2(QColor & color1, qreal & diameter, int & rows, int & cols);

protected:

    QPen    nextBorderPen();
    QBrush  nextBorderBrush();

private:
    ShapeFPtr   sp;

    int         type;
    QColor      border1;
    QColor      border2;
    QSizeF      size1;
    QPen        pen0;
    QRectF      rect0;
    qreal       diameter2;
    int         rows2;
    int         cols2;
};

#endif // BORDER_H
