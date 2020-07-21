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

#include "viewers/geo_graphics.h"
#include "geometry/transform.h"
#include "geometry/point.h"
#include "geometry/edge.h"
#include "geometry/edgepoly.h"
#include "geometry/vertex.h"
#include <QPainterPath>

GeoGraphics::GeoGraphics(QPainter * painter, QTransform & transform) : transform(transform)
{
    this->painter   = painter;
}


void GeoGraphics::drawLine( qreal x1, qreal y1, qreal x2, qreal y2, QPen pen )
{
#if 0
    QPen pen = painter->pen();
    pen.setJoinStyle(Qt::MiterJoin);
    pen.setCapStyle(Qt::RoundCap);
    painter->setPen(pen);
#endif
    QPointF a = transform.map(QPointF(x1, y1));
    QPointF b = transform.map(QPointF(x2, y2));

    painter->setPen(pen);
    painter->drawLine(a,b); // DAC - taprats uses int not qreal - why???
    //qDebug() << "GeoGraphics::drawLine2" << a << b << Transform::toInfoString(transform);
}

void GeoGraphics::drawLine( QPointF v1, QPointF v2, QPen pen)
{
    drawLine( v1.x(), v1.y(), v2.x(), v2.y(), pen);
}


// Draw a thick like as a rectangle.
void GeoGraphics::drawThickLine(qreal x1, qreal y1, qreal x2, qreal y2, qreal width, QPen pen)
{
    drawThickLine( QPointF( x1, y1 ), QPointF( x2, y2 ), width, pen);
}

void GeoGraphics::drawThickLine(QPointF v1, QPointF v2, qreal width, QPen pen)
{
    qreal widthF = Transform::distFromZero(transform,width);

    pen.setWidthF(widthF);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setCapStyle(Qt::RoundCap);

    drawLine(v1, v2, pen);
}

void GeoGraphics::drawRect(QRectF rect, QPen pen, QBrush brush)
{
#if 0
    drawRect(rect.topLeft(), rect.width(), rect.height(), brush);
#else
    //QBrush br = painter->brush();   // save
    painter->setPen(pen);
    painter->setBrush(brush);
    painter->drawRect(rect);
    //painter->setBrush(br);
#endif
}

void GeoGraphics::drawRect(QPointF topleft, qreal width, qreal height, QPen pen, QBrush brush)
{
#if 0
    qreal x = topleft.x();
    qreal y = topleft.y();

    QPolygonF pts;
    pts << topleft << QPointF( x + width, y ) << QPointF( x + width, y + height ) << QPointF( x, y + height );

    drawPolygon(pts, brush);
#else
    QRectF rect(topleft.x(),topleft.y(),width,height);
    drawRect(rect,pen,brush);
#endif
}

void GeoGraphics::drawPolygon(const QPolygonF & pgon, QColor color, int width)
{
    // don't fill
    QPolygonF p = transform.map(pgon);
    painter->setPen(QPen(color,width));
    painter->setBrush(Qt::NoBrush);
    painter->drawPolygon(p);
}

void GeoGraphics::fillPolygon(const QPolygonF & pgon, QColor color)
{
    // only fill
    QPolygonF p = transform.map(pgon);
    //painter->setPen(Qt::NoPen);
    QPen pen(color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(pen);
    painter->setBrush(QBrush(color));
    painter->drawPolygon(p);
}

void GeoGraphics::fillEdgePoly(const EdgePoly &epoly, QColor color)
{
    EdgePoly ep = epoly.map(transform);

    QPainterPath path;

    auto e = ep.first();
    path.moveTo(e->getV1()->getPosition());
    for (auto edge : ep)
    {
        if (edge->getType() == EDGETYPE_LINE)
        {
            path.lineTo(edge->getV2()->getPosition());
        }
        else if (edge->getType() == EDGETYPE_CURVE)
        {
            arcData ad = edge->calcArcData();
            path.arcTo(ad.rect, ad.start, ad.span);
        }
    }

    QPen pen(color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(pen);
    painter->drawPath(path);
    painter->fillPath(path,QBrush(color));
}


void GeoGraphics::drawEdgePoly(const EdgePoly &epoly, QColor color, int width)
{
    EdgePoly ep = epoly.map(transform);
    QPainterPath path;

    auto e = ep.first();
    path.moveTo(e->getV1()->getPosition());
    for (auto edge : ep)
    {
        if (edge->getType() == EDGETYPE_LINE)
        {
            path.lineTo(edge->getV2()->getPosition());
        }
        else if (edge->getType() == EDGETYPE_CURVE)
        {
            arcData ad = edge->calcArcData();
            path.arcTo(ad.rect, ad.start, ad.span);
        }
    }

    painter->setPen(QPen(color,width));
    painter->drawPath(path);
}


void GeoGraphics::drawArrow(QPointF from, QPointF to, qreal length, qreal half_width, QColor color)
{
    QPointF dir = to - from;
    Point::normalizeD(dir);
    QPointF perp = Point::perp(dir);
    perp *= half_width ;
    dir  *= length;
    QPolygonF poly;
    poly << to <<  (to - dir + perp) << (to - dir - perp)  << to ;
    fillPolygon(poly, color);
}


// DAC - this issue here is whether it is being passed an origin or a center
void GeoGraphics::drawCircle( QPointF origin, int diameter, QPen pen, QBrush brush)
{
    QPointF new_origin = transform.map(origin);
    painter->setBrush(brush);
    painter->setPen(pen);
    painter->drawEllipse(new_origin, diameter, diameter);
}

QTransform GeoGraphics::getTransform()
{
    return transform;
}

void GeoGraphics::push(QTransform T )
{
    pushed.push(transform);
    transform = T;
}

void GeoGraphics::pushAndCompose( QTransform T )
{
    push(T * transform);
}

QTransform GeoGraphics::pop()
{
    QTransform it = transform;
    transform = pushed.pop();
    return it;
}

/////////////////////////////////////////////////////////////////////////////
///
///
/////////////////////////////////////////////////////////////////////////////


void GeoGraphics::drawLine(QLineF line, QPen pen)
{
    drawLine(line.p1(),line.p2(),pen);
}

void GeoGraphics::drawLineDirect( QPointF v1, QPointF v2, QPen pen)
{
    painter->setPen(pen);
    painter->drawLine(v1,v2);
}

void GeoGraphics::drawThickLine(QLineF line, qreal width, QPen pen)
{
    drawThickLine(line.p1(),line.p2(),width, pen);
}

void GeoGraphics::drawEdge(EdgePtr e, QPen pen)
{
    if (e->getType() == EDGETYPE_LINE)
    {
        drawLine(e->getLine(), pen);
    }
    else if (e->getType() == EDGETYPE_CURVE)
    {
        drawChord(e->getV1()->getPosition(),e->getV2()->getPosition(),e->getArcCenter(),pen, QBrush(),e->isConvex());
    }
}

void GeoGraphics::drawThickEdge(EdgePtr e, qreal width, QPen pen)
{
    if (e->getType() == EDGETYPE_LINE)
    {
        drawThickLine(e->getV1()->getPosition(),e->getV2()->getPosition(),width, pen);
    }
    else if (e->getType() == EDGETYPE_CURVE)
    {
        drawThickChord(e->getV1()->getPosition(),e->getV2()->getPosition(),e->getArcCenter(),pen, QBrush(),e->isConvex(), width);
    }
}

void GeoGraphics::drawText(QPointF pos, QString txt)
{
    painter->save();
    QFont serifFont("Times", 18, QFont::Normal);
    painter->setFont(serifFont);
    painter->setPen(QPen(Qt::black,3));
    painter->drawText(pos,txt);
    painter->restore();
}

void GeoGraphics::drawChord(QPointF V1, QPointF V2, QPointF ArcCenter, QPen pen, QBrush brush, bool Convex)
{
#if 0
    QPen pen = painter->pen();
    pen.setJoinStyle(Qt::MiterJoin);
    pen.setCapStyle(Qt::RoundCap);
    painter->setPen(pen);
#endif
    //qDebug() << "draw Chord" << Transform::toInfoString(transform);

    V1 = transform.map(V1);
    V2 = transform.map(V2);
    ArcCenter = transform.map(ArcCenter);

    arcData ad = Edge::calcArcData(V1,V2,ArcCenter,Convex);

    int start = qRound(ad.start * 16.0);
    int span  = qRound(ad.span  * 16.0);

    if (brush.style() != Qt::NoBrush)
    {
        QPen pen = painter->pen();
        QBrush br = painter->brush();
        painter->setPen(Qt::transparent);   // prevents edges from being painted
        painter->setBrush(brush);
        painter->drawChord(ad.rect, start, span);
        painter->setBrush(br);
        painter->setPen(pen);
    }
    painter->setPen(pen);
    painter->drawArc(ad.rect, start, span);
}

void GeoGraphics::drawPie(QPointF V1, QPointF V2, QPointF ArcCenter, QPen pen, QBrush brush, bool Convex)
{
#if 0
    QPen pen = painter->pen();
    pen.setJoinStyle(Qt::MiterJoin);
    pen.setCapStyle(Qt::RoundCap);
    painter->setPen(pen);
#endif
    //qDebug() << "draw Chord" << Transform::toInfoString(transform);

    V1 = transform.map(V1);
    V2 = transform.map(V2);
    ArcCenter = transform.map(ArcCenter);

    arcData ad = Edge::calcArcData(V1,V2,ArcCenter,Convex);

    int start = qRound(ad.start * 16.0);
    int span  = qRound(ad.span  * 16.0);

    if (brush.style() != Qt::NoBrush)
    {
        QPen pen = painter->pen();
        QBrush br = painter->brush();
        painter->setPen(Qt::transparent);   // prevents edges from being painted
        painter->setBrush(brush);
        painter->drawPie(ad.rect, start, span);
        painter->setBrush(br);
        painter->setPen(pen);
    }
    painter->setPen(pen);
    painter->drawArc(ad.rect, start, span);
}


void GeoGraphics::drawThickChord(QPointF V1, QPointF V2, QPointF ArcCenter, QPen pen, QBrush brush, bool Convex, qreal width)
{
    qreal widthF = Transform::distFromZero(transform,width);

    painter->save();

    pen.setWidthF(widthF);
    //pen.setJoinStyle(Qt::MiterJoin);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setCapStyle(Qt::RoundCap);
    painter->setPen(pen);

    //qDebug() << "draw Chord" << Transform::toInfoString(transform);

    V1 = transform.map(V1);
    V2 = transform.map(V2);
    ArcCenter = transform.map(ArcCenter);

    arcData ad = Edge::calcArcData(V1,V2,ArcCenter,Convex);

    int start = qRound(ad.start * 16.0);
    int span  = qRound(ad.span  * 16.0);

    if (brush.style() != Qt::NoBrush)
    {
        QPen apen = painter->pen();
        QBrush br = painter->brush();
        painter->setPen(Qt::transparent);   // prevents inner stright edge of chord being painted
        painter->setBrush(brush);
        painter->drawChord(ad.rect, start, span);
        painter->setBrush(br);
        painter->setPen(apen);
    }
    painter->setPen(pen);
    painter->drawArc(ad.rect, start, span);

    painter->restore();
}
