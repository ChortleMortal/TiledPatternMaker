#pragma once
#ifndef ARCDATA_H
#define ARCDATA_H

#include <QPointF>
#include <QLineF>
#include <QRectF>
#include <QDebug>
#include <QtMath>
#include "sys/enums/edgetype.h"
#include "sys/geometry/circle.h"

class Edge;

class ArcData
{
    friend class DlgMagnitude;

public:
    ArcData();
    ArcData(QLineF line, QPointF center, eCurveType ctype);

    ArcData & operator=(const ArcData & other);

    void    create(QLineF line, QPointF center, eCurveType ctype);

    void    changeCurveType(eCurveType ctype);
    ArcData transform(QTransform t);

    inline  qreal        start()         const { return _start; }
    inline  qreal        end()           const { return _end; }
    inline  qreal        span()          const { return _span; }
    inline  qreal        magnitude()     const { return _magnitude; }
    inline  qreal        radius()        const { return QLineF(getCenter(),_line.p1()).length(); }
    inline  eCurveType   getCurveType()  const { return _curveType; }
    inline  QPointF      getCenter()     const { return (_curveType == CURVE_CONVEX) ? _convexCenter : _concaveCenter; }
    inline  Circle       getCircle()     const { return Circle(getCenter(),radius()); }
    inline  QRectF       rect()          const { return  _rect; }

    static  QPointF computeNewPoint(QPointF orig, QPointF c, qreal span, bool moveA);

    bool    pointWithinArc(QPointF pt);

    void    dump();
    QString info();

protected:
    void    create();
    void    create(QPointF center);

    void    setArcMagnitude(qreal magnitude);
    void    calcMagnitude();
    void    setConvex();
    void    setConcave();

    qreal   _start;
    qreal   _end;        // used internally
    qreal   _span;
    qreal   _magnitude;  // range 0 to 1
    QRectF  _rect;

private:
    QLineF     _line;
    eCurveType _curveType;
    QPointF    _convexCenter;       // inside the polygon
    QPointF    _concaveCenter;      // outside the polygon
};

#endif // ARCDATA_H
