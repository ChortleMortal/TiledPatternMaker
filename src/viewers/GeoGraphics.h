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

////////////////////////////////////////////////////////////////////////////
//
// GeoGraphics.java
//
// A GeoGraphics instance acts like the 2D geometry version of
// java.awt.Graphics (or, more recently, like java2d's Graphics2D class).
// It understands how to draw a bunch of ordinary geometric primitives
// by doing the appropriate coordinate transforms and drawing the AWT
// versions.
//
// Note that circles are drawn in screen space -- circles drawn
// from a sheared coordinate system won't show up as ellipses.  Darn.
//
// GeoGraphics maintains a stack of coordinate transforms, meant to
// behave like OpenGL's matrix stack.  Makes it easy to execute a
// bunch of primitives in some pushed graphics state.

#ifndef GEOGRAPHICS_H
#define GEOGRAPHICS_H

#include <QtCore>
#include <QPainter>
#include "base/shared.h"

class GeoGraphics
{
public:
    GeoGraphics(QPainter * painter, QTransform  & transform);

    // Drawing functions.
    void drawEdge(EdgePtr e, QPen pen);                         // no brush
    void drawThickEdge(EdgePtr e, qreal width,  QPen pen);      // no brush

    void drawLine( qreal x1, qreal y1, qreal x2, qreal y2, QPen pen);
    void drawLine( QPointF v1, QPointF v2, QPen pen);
    void drawLine( QLineF line, QPen pen);
    void drawLineDirect( QPointF v1, QPointF v2, QPen pen);

    void drawChord(QPointF V1, QPointF V2, QPointF ArcCenter, QPen pen, QBrush brush, bool Convex);
    void drawPie(QPointF V1, QPointF V2, QPointF ArcCenter, QPen pen, QBrush brush, bool Convex);

    // Draw a thick like as a rectangle.
    void drawThickLine(qreal x1, qreal y1, qreal x2, qreal y2, qreal width, QPen pen);
    void drawThickLine(QPointF v1, QPointF v2, qreal width, QPen pen);
    void drawThickLine(QLineF line, qreal width, QPen pen);

    void drawThickChord(QPointF V1, QPointF V2, QPointF ArcCenter, QPen pen, QBrush brush, bool Convex, qreal width);


    void drawRect( QPointF topleft, qreal width, qreal height, QPen pen, QBrush brush);
    void drawRect( QRectF, QPen pen, QBrush brush);

    void drawPolygon(const QVector<QPointF> & pts, QPen pen, QBrush brush);
    void drawPolygon(const QVector<QPointF> & pts, int start, int end, QPen pen, QBrush brush);
    void drawPolygon(const QPolygonF & pgon, QPen pen, QBrush brush);

    void drawArrow( QPointF from, QPointF to, qreal length, qreal half_width, QPen pen, QBrush brush);

    void drawCircle(QPointF origin, int diameter, QPen pen, QBrush brush);

    void drawText(QPointF pos, QString txt);    // no pen

    void   save()    { painter->save(); }
    void   restore() { painter->restore(); }

    // Transform functions.
    QTransform  getTransform();
    void        push(QTransform T );
    void        pushAndCompose(QTransform T );
    QTransform  pop();

private:

    QPainter        * painter;
    QTransform      &  transform;   // reference to layer transform which can change
    QStack<QTransform> pushed;
};
#endif

