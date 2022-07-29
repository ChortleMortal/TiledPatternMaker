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

#include <QStack>
#include <QTransform>
#include <QPainter>

class EdgePoly;

typedef std::shared_ptr<class Edge> EdgePtr;

class GeoGraphics
{
public:
    GeoGraphics(QPainter * painter, QTransform  transform);
    void set(QTransform t) {transform = t;}

    // Drawing functions.
    void drawEdge(const EdgePtr & e, const QPen & pen);                         // no brush
    void drawThickEdge(const EdgePtr & e, qreal width, const QPen & pen);       // no brush

    void drawLine(const QPointF & p1, const QPointF & p2, const QPen & pen);
    void drawLine(const QLineF  & line, const QPen & pen);
    void drawLineDirect( const QPointF & v1, const QPointF & v2, const QPen & pen);

    void drawArc(  QPointF V1, QPointF V2, QPointF ArcCenter, bool Convex, QPen pen);
    void drawChord(QPointF V1, QPointF V2, QPointF ArcCenter, bool Convex, QPen pen);
    void drawPie(  QPointF V1, QPointF V2, QPointF ArcCenter, bool Convex, QPen pen, QBrush brush);

    void drawRect( QPointF topleft, qreal width, qreal height, QPen pen, QBrush brush);
    void drawRect( QRectF, QPen pen, QBrush brush);

    void fillPolygon(const QPolygonF & pgon, QColor color);
    void drawPolygon(const QPolygonF & pgon, QPen pen);

    void fillEdgePoly(const EdgePoly & epoly, QColor color);
    void drawEdgePoly(const EdgePoly & epoly, QColor color, int width);

    void fillPath(QPainterPath & pp, QColor color);

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
    void _drawThickArc(  const QPointF & V1, const QPointF & V2, const QPointF && ArcCenter, bool Conves, const QPen & pen);
    void _drawThickChord(const QPointF & V1, const QPointF & V2, const QPointF && ArcCenter, bool Convex, const QPen & pen);

    QPainter         * painter;
    QTransform         transform;
    QStack<QTransform> pushed;
};
#endif

