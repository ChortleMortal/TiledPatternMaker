#pragma once
#ifndef GEOGRAPHICS_H
#define GEOGRAPHICS_H

///////////////////////////////////////////////////////////////////////////
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

#include <QStack>
#include <QTransform>
#include <QPainter>
#include <QPainterPathStroker>
#include "sys/enums/edgetype.h"
#include "sys/geometry/arcdata.h"

class EdgePoly;

typedef std::shared_ptr<class Edge> EdgePtr;

class GeoGraphics
{
public:
    GeoGraphics(QPainter * painter, QTransform  transform);

    // Drawing functions.
    void drawEdge(const EdgePtr & e, const QPen & pen);                         // no brush
    void drawThickEdge(const EdgePtr & e, qreal width, const QPen & pen);       // no brush

    void drawLine(const QPointF & p1, const QPointF & p2, const QPen & pen);
    void drawLine(const QLineF  & line, const QPen & pen);
    void drawThickCorner(const QPointF & from, const QPointF & mid, const QPointF & to, QPen &pen, qreal width, QPainterPathStroker &ps);
    void drawThickCorner2(const EdgePtr &e1, const EdgePtr & e2, QPen & pen, qreal width);

    void drawLineDirect( const QPointF & v1, const QPointF & v2, const QPen & pen);

    void drawArc(ArcData & ad, QPen pen, bool outer);
    void drawArc(QPointF p1, QPointF p2, QPointF center, eCurveType ctype, QPen pen, bool outer);

    void drawRect( QPointF topleft, qreal width, qreal height, QPen pen, QBrush brush);
    void drawRect( QRectF, QPen pen, QBrush brush);

    void fillPolygon(const QPolygonF & pgon, const QPen &pen);
    void drawPolygon(const QPolygonF & pgon, QPen & pen);

    void fillEdgePoly(const EdgePoly & epoly, const QPen & pen);
    void drawEdgePoly(const EdgePoly & epoly, const QPen & pen);

    void fillPath(QPainterPath pp, QPen &pen) const;      // not a reference, not const
    void fillStrokedPath(QPainterPath pp, QPen &pen, QPainterPathStroker & ps) const;      // not a reference, not const

    void drawArrow( QPointF from, QPointF to, qreal length, qreal half_width, QColor color);
    void drawLineArrow(QLineF line, QPen pen);
    void drawLineArrowDirect(QLineF line, QPen pen);
    static void drawLineArrowDirect(QLineF line, QPen pen, QPainter * painter);

    void drawCircle(QPointF origin, int diameter, QPen pen, QBrush brush);

    void drawText(QPointF pos, QString txt);    // no pen

    QPainter *  getPainter() { return painter; }

    // Transform functions.
    QTransform  getTransform();
    void        push(QTransform T );
    void        pushAndCompose(QTransform T );
    QTransform  pop();

protected:

private:
    void _drawThickLine( const QPointF & v1, const QPointF & v2, const QPen & pen);
    void _drawThickArc(  const QPointF & V1, const QPointF & V2, const QPointF && ArcCenter, eCurveType ctype, const QPen & pen);

    QPainter         * painter;
    QTransform         transform;
    QStack<QTransform> pushed;
};
#endif

