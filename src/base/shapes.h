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

#ifndef SHAPES_H
#define SHAPES_H

#include <QtWidgets>
#include "layer.h"

#define DEGREES_360     (360 * 16)


enum eForm
{
    POLYGON2,
    POLYLINE2,
    CIRCLE2
};

enum eArcType
{
    ARC,
    CHORD,
    PIE
};

class Polyform : public QPolygonF
{
protected:
    Polyform(QPen Pen = QPen(Qt::NoPen), QBrush Brush = QBrush(Qt::NoBrush));
    Polyform(const QPolygonF &polygon, QPen Pen = QPen(Qt::NoPen), QBrush Brush = QBrush(Qt::NoBrush));

public:
    void setPen(QPen Pen)           {pen      = Pen;}
    void setBrush(QBrush Brush)     {brush    = Brush;}
    void setDiameter(qreal Diameter){diameter = Diameter; radius = Diameter/2.0;}
    void setArc(eArcType ArcType, int start, int span){arcType = ArcType, arcStart = start; arcSpan = span;}
    void setInnerPen(QPen pen)      {innerPen = pen;}
    void setRadials(bool set)       {radials  = set;}

    QPen   getPen()                 {return pen;}
    QBrush getBrush()               {return brush;}

    QRectF boundingRect();

    eForm       polytype;
    QPen        pen;
    QPen        innerPen;

    QBrush      brush;
    bool        radials;

    qreal       diameter;   // circle
    qreal       radius;     // circle

    int         arcStart;
    int         arcSpan;
    eArcType    arcType;
};

class Polygon2 : public Polyform
{
public:
    Polygon2(QPen Pen = QPen(Qt::NoPen), QBrush Brush = QBrush(Qt::NoBrush));
    Polygon2(const QPolygonF &polygon, QPen Pen = QPen(Qt::NoPen), QBrush Brush = QBrush(Qt::NoBrush));
    Polygon2(const Polygon2 & poly);

    void rotate(qreal delta);
};

class Polyline2 : public Polyform
{
public:
    Polyline2(QPen Pen = QPen(Qt::NoPen));
    Polyline2(const QPolygonF &polygon, QPen pen = QPen(Qt::NoPen));
    Polyline2(const Polyline2 & poly);

};

class Circle2 : public Polyform
{
public:
    Circle2(qreal Diameter, QPen Pen = QPen(Qt::NoPen), QBrush Brush = QBrush(Qt::NoBrush), int ArcStart = 0, int ArcSpan = 0, eArcType ArcType = ARC);
    Circle2(const Circle2 & circle);
};

#endif // SHAPES_H
