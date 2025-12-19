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
#include "gui/viewers/geo_graphics.h"
#include "gui/viewers/debug_view.h"
#include "sys/geometry/arcdata.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/edge_poly.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/transform.h"
#include "sys/geometry/vertex.h"
#include "sys/sys/debugflags.h"

GeoGraphics::GeoGraphics(QPainter * painter, QTransform  transform)
{
    this->painter   = painter;
    this->transform = transform;
}

void GeoGraphics::drawEdge(const EdgePtr & e, const QPen & pen)
{
    if (e->isLine())
    {
        drawLine(e->getLine(), pen);
    }
    else
    {
        Q_ASSERT(e->isCurve());
        //drawArc(e->v1->pt,e->v2->pt,e->getArcCenter(),e->getCurveType(),pen,true);
        drawArc(e->getArcData(),pen,true);
    }
}

void GeoGraphics::drawLine(const QLineF &line, const QPen &pen)
{
    drawLine(line.p1(),line.p2(),pen);
}

void GeoGraphics::drawLine(const QPointF &p1, const QPointF &p2, const QPen &pen)
{
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
    painter->setPen(pen);
    painter->setBrush(brush);
    painter->drawRect(rect);
}

void GeoGraphics::drawRect(QPointF topleft, qreal width, qreal height, QPen pen, QBrush brush)
{
    QRectF rect(topleft.x(),topleft.y(),width,height);
    drawRect(rect,pen,brush);
}

void GeoGraphics::drawPolygon(const QPolygonF & pgon, QPen &pen)
{
    // draw only, don't fill
    QPolygonF p = transform.map(pgon);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPolygon(p);
}

void GeoGraphics::fillPolygon(const QPolygonF & pgon, const QPen & pen)
{
    // daraw and fill
    QPolygonF p = transform.map(pgon);
    painter->setPen(pen);
    painter->setBrush(QBrush(pen.color()));
    painter->drawPolygon(p);
}

void GeoGraphics::drawEdgePoly(const EdgePoly & epoly, const QPen &pen)
{
    QPainterPath path  = epoly.getPainterPath();
    QPainterPath path2 = transform.map(path);

    painter->setPen(pen);
    //painter->setBrush(Qt::NoBrush);
    painter->drawPath(path2);
}

void GeoGraphics::fillEdgePoly(const EdgePoly & epoly, const QPen &pen)
{
    QPainterPath path  = epoly.getPainterPath();
    QPainterPath path2 = transform.map(path);

    painter->setPen(pen);
    painter->drawPath(path2);
    painter->fillPath(path2,QBrush(pen.color()));
}

void GeoGraphics::fillPath(QPainterPath path, QPen & pen) const
{
    for (int i = 0; i < path.elementCount(); i++)
    {
        QPainterPath::Element element = path.elementAt(i);
        QPointF pt = transform.map(QPointF(element.x,element.y));
        path.setElementPositionAt(i,pt.x(),pt.y());
    }

    if (!Sys::flags->flagged(NO_FILL))
    {
        painter->fillPath(path,QBrush(pen.color()));
    }
    if (!Sys::flags->flagged(NO_FILL_STROKE))
    {
        painter->strokePath(path,pen);  // always outline the fill
    }
}

void GeoGraphics::fillStrokedPath(QPainterPath path, QPen & pen, QPainterPathStroker &ps) const
{
    for (int i = 0; i < path.elementCount(); i++)
    {
        QPainterPath::Element element = path.elementAt(i);
        QPointF pt = transform.map(QPointF(element.x,element.y));
        path.setElementPositionAt(i,pt.x(),pt.y());
    }

    //path = path.simplified();

    if (Sys::flags->flagged(NO_FILL))
    {
        path = ps.createStroke(path);
        painter->strokePath(path,pen);
    }
    else
    {
        QPen nopen(Qt::NoPen);
        painter->setPen(nopen);
        painter->fillPath(path,QBrush(pen.color()));

        //painter->setPen(pen);
        auto path2 = ps.createStroke(path);
        painter->strokePath(path2,pen);
    }
}

void GeoGraphics::drawArrow(QPointF from, QPointF to, qreal length, qreal half_width, QColor color)
{
    QPointF dir = to - from;
    Geo::normalizeD(dir);
    QPointF perp = Geo::perp(dir);
    perp *= half_width ;
    dir  *= length;
    QPolygonF poly;
    poly << to <<  (to - dir + perp) << (to - dir - perp)  << to ;

    QPen pen(color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    fillPolygon(poly, pen);
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
    line1.setLength(15);
    painter->drawLine(line1);

    line1.setP1(centreEdge);
    line1.setAngle(angle + 135.0);
    line1.setLength(15);
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

void GeoGraphics::drawArc(QPointF p1, QPointF p2, QPointF center, eCurveType ctype, QPen pen, bool outer)
{
    p1     = transform.map(p1);
    p2     = transform.map(p2);
    center = transform.map(center);

    ArcData ad;
    if (outer)
        ad.create(QLineF(p1,p2),center,ctype);
    else
        ad.create(QLineF(p2,p1),center,ctype);

    int start = qRound(ad.start() * 16.0);
    int span  = qRound(ad.span()  * 16.0);

    painter->setPen(pen);

    if (outer)
        painter->drawArc(ad.rect(), start, span);
    else
        painter->drawArc(ad.rect(), start, -span);
}

void GeoGraphics::drawArc(ArcData & ad, QPen pen, bool outer)
{
    ArcData ad2 = ad.transform(transform);

    int start = qRound(ad2.start() * 16.0);
    int span  = qRound(ad2.span()  * 16.0);

    painter->setPen(pen);

    if (outer)
        painter->drawArc(ad2.rect(), start, span);
    else
        painter->drawArc(ad2.rect(), start, -span);
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
        _drawThickArc(e->v1->pt,e->v2->pt,e->getArcCenter(),e->getCurveType(), apen);
    }

    painter->restore();
}

void GeoGraphics::_drawThickLine(const QPointF &v1, const QPointF &v2, const QPen &pen)
{
    drawLine(v1, v2, pen);
}

void GeoGraphics::_drawThickArc(const QPointF & V1, const QPointF & V2, const QPointF && ArcCenter, eCurveType ctype, const QPen & pen)
{
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    QPointF v1 = transform.map(V1);
    QPointF v2 = transform.map(V2);
    QPointF arcCenter = transform.map(ArcCenter);

    ArcData ad;
    ad.create(QLineF(v1,v2),arcCenter,ctype);

    int start = qRound(ad.start() * 16.0);
    int span  = qRound(ad.span()  * 16.0);

    painter->drawArc(ad.rect(), start, span);
}

void GeoGraphics::drawThickCorner(const QPointF & from, const QPointF & mid, const QPointF & to, QPen & pen, qreal width, QPainterPathStroker & ps)
{
    // taken from Pierre Ballargeon's Alhhambra
    qreal gwidth = Transform::scalex(transform) * width;
    pen.setWidth(gwidth);
    //pen.setWidth(width);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    ps.setWidth(width);

    QPointF tf = transform.map(from);
    QPointF tm = transform.map(mid);
    QPointF tt = transform.map(to);

#if 0
    QPointF qps[3] = {tf, tm, tt};
    painter->drawPolyline(qps, 3);
#else
    QPainterPath path;
    path.moveTo(tf);
    path.lineTo(tm);
    path.lineTo(tt);
    path = ps.createStroke(path);
    painter->drawPath(path);
#endif
}

void GeoGraphics::drawThickCorner2(const EdgePtr & e1, const EdgePtr &e2, QPen & pen, qreal width)
{
    // taken from Pierre Ballargeon's Alhhambra
    pen.setWidth(Transform::scalex(transform) * width);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    QLineF te1 = transform.map(e1->getLine());
    QLineF te2 = transform.map(e2->getLine());

#if 0
    QPointF qps[3] = {te1.p1(), te1.p2(), te2.p2()};
    painter->drawPolyline(qps, 3);
#else
    QPainterPath path;
    path.moveTo(te1.p1());
    path.lineTo(te1.p2());
    path.lineTo(te2.p2());
    painter->drawPath(path);
#endif
}
