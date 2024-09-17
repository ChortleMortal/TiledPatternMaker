#pragma once
#ifndef ARCDATA_H
#define ARCDATA_H

#include <QPointF>
#include <QLineF>
#include <QRectF>
#include <QDebug>
#include <QtMath>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <memory>
#endif

class Edge;

class ArcData
{
public:
    ArcData();
    ArcData & operator=(const ArcData & other);

    void    create(QPointF p1, QPointF p2, QPointF center, bool convex);

    void    calcSpan (QPointF p1, QPointF p2);
    void    calcSpan (Edge * edge);

    inline qreal   span()  const            { return _span; }
    inline void    setSpan(qreal span)      {_span = span; }

    inline bool    convex()  const          { Q_ASSERT(_arcType != AT_UNDEFINED); return (_arcType == AT_CONVEX); }
           void    setConvex(bool convex, QPointF p1, QPointF p2);

           QPointF getCenter() const;
           void    setCenter(QPointF mpt);

    void    dump();
    QString info();


    QRectF  rect;

    qreal   magnitude;  // range 0 to 1
    qreal   start;
    qreal   end;        // used internally

    static QPointF reflectCentreOverEdge(QPointF p1, QPointF p2, QPointF arcCenter);

    bool    pointWithinArc(QPointF pt);

protected:
    QRectF  getRect(QPointF center, qreal radius);

    void    calcConvexSpan( QPointF p1, QPointF p2);
    void    calcConcaveSpan(QPointF p1, QPointF p2);

private:
    enum eArcType
    {
        AT_UNDEFINED,
        AT_CONVEX,
        AT_CONCAVE
    };

    eArcType _arcType;
    QPointF  _convexCenter;       // inside the polygon
    QPointF  _concaveCenter;      // outside the polygon
    qreal    _span;

    int     trace;
};

#endif // ARCDATA_H
