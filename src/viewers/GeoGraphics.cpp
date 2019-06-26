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

#include "viewers/GeoGraphics.h"


GeoGraphics::GeoGraphics(QPainter * painter, Transform & transform)
{
    this->painter   = painter;
    this->transform = transform;
}

void GeoGraphics::drawLine( qreal x1, qreal y1, qreal x2, qreal y2 )
{
    qreal v1x = transform.applyX( x1, y1 );
    qreal v1y = transform.applyY( x1, y1 );
    qreal v2x = transform.applyX( x2, y2 );
    qreal v2y = transform.applyY( x2, y2 );

    QPointF a(v1x,v1y);
    QPointF b(v2x,v2y);
    painter->drawLine(a,b); // DAC - taprats uses int not qreal - why???
    //qDebug() << "GeoGraphics::drawLine" << a << b;
}

void GeoGraphics::drawLine( QPointF v1, QPointF v2 )
{
    drawLine( v1.x(), v1.y(), v2.x(), v2.y());
}

void GeoGraphics::drawLine( QLineF line)
{
    drawLine(line.p1(),line.p2());
}

void GeoGraphics::drawLineS( QPointF v1, QPointF v2)
{
    painter->drawLine(v1,v2);
}

// Draw a thick like as a rectangle.

void GeoGraphics::drawThickLine(QLineF line, qreal width)
{
    drawThickLine(line.p1(),line.p2(),width);
}

void GeoGraphics::drawThickLine(qreal x1, qreal y1, qreal x2, qreal y2, qreal width)
{
    drawThickLine( QPointF( x1, y1 ), QPointF( x2, y2 ), width);
}

void GeoGraphics::drawThickLine(QPointF v1, QPointF v2, qreal width )
{
    qreal widthF = transform.distFromZero(width);

    painter->save();

    QPen pen = painter->pen();
    //pen.setColor(_color);
    pen.setWidthF(widthF);
    //pen.setJoinStyle(Qt::MiterJoin);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setCapStyle(Qt::RoundCap);
    painter->setPen(pen);
    drawLine( v1, v2 );

    painter->restore();
}

void GeoGraphics::drawRect(QRectF rect, bool filled)
{
    drawRect(rect.topLeft(), rect.width(), rect.height(), filled);
}

void GeoGraphics::drawRect(QPointF topleft, qreal width, qreal height, bool filled)
{
    qreal x = topleft.x();
    qreal y = topleft.y();

    QPolygonF pts;
    pts << topleft << QPointF( x + width, y ) << QPointF( x + width, y + height ) << QPointF( x, y + height );

    drawPolygon( pts, filled );
}

void GeoGraphics::drawPolygon(const QVector<QPointF> & pts, bool filled )
{
    drawPolygon( pts, 0, pts.size(), filled );
}

void GeoGraphics::drawPolygon(const QVector<QPointF> &pts, int start, int end, bool filled )
{
    int len = end - start;
    QPolygonF poly;
    for (int i=0; i < len; i++ )
    {
        poly << pts[start + i];
    }
    drawPolygon(poly,filled);
}

void GeoGraphics::drawPolygon(const QPolygonF & pgon, bool filled )
{
    QPolygonF p;
    int len = pgon.size();
    for( int i = 0; i < len; ++i )
    {
        QPointF v = pgon.at( i );
        qreal x = v.x();
        qreal y = v.y();

        p << QPointF(transform.applyX(x,y),transform.applyY(x,y));
    }

    //qDebug() << "GeoGraphics::drawPolygon" << p;

    if( filled )
    {
        QPainterPath pp;
        pp.addPolygon(p);
        painter->fillPath(pp,_color);
    }
    else
    {
        //painter->setPen(QPen(_color,3));
        painter->drawPolygon(p);
    }
}

void GeoGraphics::drawArrow( QPointF from, QPointF to, qreal length, qreal half_width, bool filled )
{
    QPointF dir = to - from;
    Point::normalizeD(dir);
    QPointF perp = Point::perp(dir);
    perp *= half_width ;
    dir  *= length;
    QPolygonF poly;
    poly << to <<  (to - dir + perp) << (to - dir - perp)  << to ;
    drawPolygon( poly, filled );
}


// DAC - this issue here is whether it is being passed an origin or a center
void GeoGraphics::drawCircle( QPointF origin, qreal radius, bool filled )
{
    QPointF rad  = transform.apply(QPointF( radius, 0.0 ));
    QPointF orig = transform.apply(QPointF( 0.0, 0.0 ));
    qreal true_radius = Point::dist(rad,orig);

    QPointF new_origin = transform.apply( origin );
    //QPointF new_topleft = new_origin - QPointF(true_radius,true_radius);

    qreal r2 = (int)( true_radius * 2.0 );

    if( filled )
    {
        QPainterPath pp;
        //pp.addEllipse(new_topleft + QPointF(r2,r2), r2, r2 );
        // DAC methinks use new_origin
        pp.addEllipse(new_origin, r2, r2 );
        painter->fillPath(pp,QBrush(Qt::red));
    }
    else
    {
        //painter->drawEllipse(new_topleft + QPointF(r2,r2), r2, r2);
        painter->drawEllipse(new_origin, r2, r2);
    }
}

// DAC - this issue here is whether it is being passed an origin or a center
void GeoGraphics::drawCircle( QPointF origin, qreal radius )
{
    drawCircle( origin, radius, false );
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

QColor GeoGraphics::getColor()
{
    return _color;
}

void GeoGraphics::setColor( QColor c )
{
    _color = c;
    QPen apen = painter->pen();
    apen.setColor(c);
    painter->setPen(apen);
}

Transform GeoGraphics::getTransform()
{
    return transform;
}

void GeoGraphics::push(Transform T )
{
    pushed.push(transform);
    transform = T;
    //qDebug().noquote() << "GeoLayer pushed Transform:\n" << transform.toString();
}

void GeoGraphics::pushAndCompose( Transform T )
{
    push(transform.compose( T ) );
}

Transform GeoGraphics::pop()
{
    Transform it = transform;
    transform = pushed.pop();
    return it;
}

