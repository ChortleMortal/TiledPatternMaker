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

#include <QPainterPath>
#include "misc/geo_graphics.h"
#include "geometry/arcdata.h"
#include "geometry/edge.h"
#include "geometry/edgepoly.h"
#include "geometry/point.h"
#include "geometry/transform.h"
#include "geometry/vertex.h"

GeoGraphics::GeoGraphics(QPainter * painter, QTransform  transform)
{
    this->painter   = painter;
    this->transform = transform;
}

void GeoGraphics::drawEdge(const EdgePtr & e, const QPen & pen)
{
    if (e->getType() == EDGETYPE_LINE)
    {
        drawLine(e->getLine(), pen);
    }
    else if (e->getType() == EDGETYPE_CURVE)
    {
        drawArc(e->v1->pt,e->v2->pt,e->getArcCenter(),e->isConvex(),pen);
    }
    else if (e->getType() == EDGETYPE_CHORD)
    {
        drawChord(e->v1->pt,e->v2->pt,e->getArcCenter(),e->isConvex(),pen);
    }
}

void GeoGraphics::drawLine(const QLineF &line, const QPen &pen)
{
    drawLine(line.p1(),line.p2(),pen);
}

void GeoGraphics::drawLine(const QPointF &p1, const QPointF &p2, const QPen &pen)
{
#if 0
    QPen pen = painter->pen();
    pen.setJoinStyle(Qt::MiterJoin);
    pen.setCapStyle(Qt::RoundCap);
    painter->setPen(pen);
#endif

    QLineF line(transform.map(p1),transform.map(p2));

    painter->setPen(pen);
    painter->drawLine(line);
}

void GeoGraphics::drawLineDirect(const QPointF & v1, const QPointF & v2, const QPen & pen)
{
    painter->setPen(pen);
    painter->drawLine(v1,v2);
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

void GeoGraphics::drawPolygon(const QPolygonF & pgon, QPen pen)
{
    // don't fill
    QPolygonF p = transform.map(pgon);
    painter->setPen(pen);
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

void GeoGraphics::fillEdgePoly(const EdgePoly & epoly, QColor color)
{
    EdgePoly ep = epoly.map(transform);
    Q_ASSERT(ep.isCorrect());

    QPainterPath path;

    auto e = ep.first();
    path.moveTo(e->v1->pt);
    for (auto edge : ep)
    {
        if (edge->getType() == EDGETYPE_LINE)
        {
            path.lineTo(edge->v2->pt);
        }
        else if (edge->getType() == EDGETYPE_CURVE)
        {
            auto ad = edge->getArcData();
            Q_ASSERT(ad);
            path.arcTo(ad->rect, ad->start, ad->span);
        }
        else if (edge->getType() == EDGETYPE_CHORD)
        {
            // TODO - does this work
            path.moveTo(edge->v2->pt);
            path.lineTo(edge->v1->pt);
            auto ad = edge->getArcData();
            Q_ASSERT(ad);
            path.arcTo(ad->rect, ad->start, ad->span);
        }
    }

    QPen pen(color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(pen);
    painter->fillPath(path,QBrush(color));
    painter->drawPath(path);
}

void GeoGraphics::fillPath(QPainterPath & path, QColor color)
{
    for (int i = 0; i < path.elementCount(); i++)
    {
        QPainterPath::Element element = path.elementAt(i);
        QPointF pt = transform.map(QPointF(element.x,element.y));
        path.setElementPositionAt(i,pt.x(),pt.y());
    }

    painter->fillPath(path,QBrush(color));
    QPen pen(color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter->strokePath(path,pen);
}

// TODO - replace this with EdgePoly::draw  or vice versa
void GeoGraphics::drawEdgePoly(const EdgePoly & epoly, QColor color, int width)
{
    EdgePoly ep = epoly.map(transform);
    QPainterPath path;

    auto e = ep.first();
    path.moveTo(e->v1->pt);
    for (auto edge : ep)
    {
        if (edge->getType() == EDGETYPE_LINE)
        {
            path.lineTo(edge->v2->pt);
        }
        else if (edge->getType() == EDGETYPE_CURVE)
        {
            auto ad = edge->getArcData();
            Q_ASSERT(ad);
            path.arcTo(ad->rect, ad->start, ad->span);
        }
        else if (edge->getType() == EDGETYPE_CHORD)
        {
            // TODO - does this work
            path.moveTo(edge->v2->pt);
            path.lineTo(edge->v1->pt);
            auto ad = edge->getArcData();
            path.arcTo(ad->rect, ad->start, ad->span);
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

void GeoGraphics::drawLineArrow(QLineF line, QPen pen)
{
    drawLineArrowDirect(transform.map(line),pen);
}

void GeoGraphics::drawLineArrowDirect(QLineF line, QPen pen)
{
    painter->setPen(pen);

    // draw arrow
    qreal angle        = line.angle();
    QPointF centreEdge = line.pointAt(0.5);

    QLineF line1;
    line1.setP1(centreEdge);
    line1.setAngle(angle - 135.0);
    line1.setLength(25);
    painter->drawLine(line1);

    line1.setP1(centreEdge);
    line1.setAngle(angle + 135.0);
    line1.setLength(25);
    painter->drawLine(line1);
}

void GeoGraphics::drawLineArrowDirect(QLineF line, QPen pen, QPainter * painter)
{
    painter->setPen(pen);

    // draw arrow
    qreal angle        = line.angle();
    QPointF centreEdge = line.pointAt(0.5);

    QLineF line1;
    line1.setP1(centreEdge);
    line1.setAngle(angle - 135.0);
    line1.setLength(25);
    painter->drawLine(line1);

    line1.setP1(centreEdge);
    line1.setAngle(angle + 135.0);
    line1.setLength(25);
    painter->drawLine(line1);
}

// DAC - this issue here is whether it is being passed an origin or a center
void GeoGraphics::drawCircle( QPointF origin, int diameter, QPen pen, QBrush brush)
{
    painter->save();
    QPointF new_origin = transform.map(origin);
    painter->setBrush(brush);
    painter->setPen(pen);
    painter->drawEllipse(new_origin, diameter, diameter);
    painter->restore();
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

void GeoGraphics::drawText(QPointF pos, QString txt)
{
    painter->save();
    QFont serifFont("Times", 18, QFont::Normal);
    painter->setFont(serifFont);
    painter->setPen(QPen(Qt::black,3));
    painter->drawText(pos,txt);
    painter->restore();
}

void GeoGraphics::drawArc(QPointF V1, QPointF V2, QPointF ArcCenter, bool Convex, QPen pen)
{
#if 0
    QPen pen = painter->pen();
    pen.setJoinStyle(Qt::MiterJoin);
    pen.setCapStyle(Qt::RoundCap);
    painter->setPen(pen);
#endif

    V1 = transform.map(V1);
    V2 = transform.map(V2);
    ArcCenter = transform.map(ArcCenter);

    ArcData ad(V1,V2,ArcCenter,Convex);

    int start = qRound(ad.start * 16.0);
    int span  = qRound(ad.span  * 16.0);

    painter->setPen(pen);
    painter->drawArc(ad.rect, start, span);
}



void GeoGraphics::drawChord(QPointF V1, QPointF V2, QPointF ArcCenter, bool Convex, QPen pen)
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

    ArcData ad(V1,V2,ArcCenter,Convex);

    int start = qRound(ad.start * 16.0);
    int span  = qRound(ad.span  * 16.0);

    painter->setPen(pen);
    painter->drawChord(ad.rect, start, span);
}

void GeoGraphics::drawPie(QPointF V1, QPointF V2, QPointF ArcCenter, bool Convex, QPen pen, QBrush brush)
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

    ArcData ad(V1,V2,ArcCenter,Convex);

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

/////////////////////////////////////////////////////////////////////////////
///
///     Thick
///
/////////////////////////////////////////////////////////////////////////////

void GeoGraphics::drawThickEdge(const EdgePtr & e, qreal width, const QPen & pen)
{
    painter->save();

    //qreal widthF = Transform::distFromZero(transform,width);
    qreal widthF = Transform::scalex(transform) * width;
    QPen apen = pen;
    apen.setWidthF(widthF);

    if (e->getType() == EDGETYPE_LINE)
    {
        _drawThickLine(e->v1->pt,e->v2->pt,apen);
    }
    else if (e->getType() == EDGETYPE_CURVE)
    {
        _drawThickArc(e->v1->pt,e->v2->pt,e->getArcCenter(),e->isConvex(), apen);
    }
    else if (e->getType() == EDGETYPE_CURVE)
    {
        _drawThickChord(e->v1->pt,e->v2->pt,e->getArcCenter(), e->isConvex(), apen);
    }

    painter->restore();
}

void GeoGraphics::_drawThickLine(const QPointF &v1, const QPointF &v2, const QPen &pen)
{
    drawLine(v1, v2, pen);
}

void GeoGraphics::_drawThickArc(const QPointF & V1, const QPointF & V2, const QPointF && ArcCenter, bool Convex, const QPen & pen)
{
    QPen apen = pen;
    //pen.setJoinStyle(Qt::MiterJoin);
    apen.setJoinStyle(Qt::RoundJoin);
    apen.setCapStyle(Qt::RoundCap);
    painter->setPen(apen);
    painter->setBrush(Qt::NoBrush);

    QPointF v1 = transform.map(V1);
    QPointF v2 = transform.map(V2);
    QPointF arcCenter = transform.map(ArcCenter);

    ArcData ad(v1,v2,arcCenter,Convex);

    int start = qRound(ad.start * 16.0);
    int span  = qRound(ad.span  * 16.0);

    painter->drawArc(ad.rect, start, span);
}

void GeoGraphics::_drawThickChord(const QPointF & V1, const QPointF & V2, const QPointF && ArcCenter, bool Convex, const QPen & pen)
{
    QPen apen = pen;
    //pen.setJoinStyle(Qt::MiterJoin);
    apen.setJoinStyle(Qt::RoundJoin);
    apen.setCapStyle(Qt::RoundCap);
    painter->setPen(apen);
    painter->setBrush(Qt::NoBrush);

    //qDebug() << "draw Chord" << Transform::toInfoString(transform);

    QPointF v1 = transform.map(V1);
    QPointF v2 = transform.map(V2);
    QPointF arcCenter = transform.map(ArcCenter);

    ArcData ad(v1,v2,arcCenter,Convex);

    int start = qRound(ad.start * 16.0);
    int span  = qRound(ad.span  * 16.0);

    painter->drawChord(ad.rect, start, span);
}

