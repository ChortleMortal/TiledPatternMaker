#pragma once
#ifndef CASING_H
#define CASING_H

#include <QDebug>
#include <QPainterPath>

#include "model/styles/casing_data.h"
#include "model/styles/casing_set.h"
#include "model/styles/casing_side.h"
#include "sys/geometry/circle.h"

class Casing
{
public:
    Casing();

    virtual void       setPainterPath() = 0;
    virtual bool       validate()       = 0;

    void        createCurved();
    void        alignCurvedEdgeSide1(CasingSet &casings);
    void        alignCurvedEdgeSide2(CasingSet &casings);

    void        fillCasing( GeoGraphics * gg, QPen & pen) const;
    void        drawOutline(GeoGraphics * gg, QPen & pen) const;
    void        debugDraw(QColor color, qreal width);

    EdgePtr     getEdge()           { return wedge.lock(); }
    qreal       getWidth()          { return width; }

    void        addToMap(MapPtr map);

    void        dump();

    CasingSide * side(eSide side)   { return (side == SIDE_1) ? s1 : s2; }
    CasingSide * side(VertexPtr v)  { if (s1->vertex() ==v) return s1; else {Q_ASSERT(s2->vertex()==v); return s2;} };
    bool        created()           { return (s1->created && s2->created); }

    QLineF      outerLine()         { return QLineF(s1->outer,s2->outer); }
    QLineF      innerLine()         { return QLineF(s1->inner,s2->inner); }
    QLineF      s1Line()            { return QLineF(s1->inner,s1->outer); }
    QLineF      s2Line()            { return QLineF(s2->inner,s2->outer); }
    QPolygonF   getPoly() const;

    bool        getCircleIsect(const Circle & circle, Casing &other, bool inner, const QPointF & oldPt, QPointF & newPt);

    CNeighboursPtr getNeighbours(VertexPtr v);

    CasingData  getCasingData();

    bool        validateCurves();

    Circle          innerCircle;
    Circle          outerCircle;
    WeakEdgePtr     wedge;
    uint            edgeIndex;

    CasingSide  * s1;
    CasingSide  * s2;

protected:
    QPainterPath    path;
    qreal           width;

    CasingSet   * owner;
};

#endif // CASING_H
