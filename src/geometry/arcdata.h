#ifndef ARCDATA_H
#define ARCDATA_H

#include <memory>
#include <QPointF>
#include <QLineF>
#include <QRectF>
#include <QDebug>
#include <QtMath>

typedef std::shared_ptr<class Edge> EdgePtr;

class ArcData
{
public:
    ArcData(QPointF p1, QPointF p2, QPointF c, bool convex);
    ArcData(Edge *edge);

    qreal   getRadius() { return rect.width() /2.0; }

    void    dump();
    void    testCenters(QPointF p1, QPointF p2, QPointF c);
    void    testEnds(QPointF p1, QPointF p2, QPointF c);

    QRectF  rect;
    qreal   start;
    qreal   end;    // used internally
    qreal   span;
    bool    convex;

    static QPointF getCenterOldConcave(QPointF p1, QPointF p2, QPointF center);

protected:
    QRectF  getRect(QPointF center, qreal radius);
    QPointF getCenterNew(QPointF p1, QPointF p2, qreal radius, bool convex);

    void calcConvex(QPointF p1, QPointF p2, QPointF c);
    void calcConcave(QPointF p1, QPointF p2, QPointF c);

    static bool equals(qreal a, qreal b, qreal tolerance)
    {
        return qAbs(a - b) < tolerance;
    }

private:
    void test1();
    void test2();
    int  trace;
};

#endif // ARCDATA_H
