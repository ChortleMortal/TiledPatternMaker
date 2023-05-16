#pragma once
#ifndef BELOWANDABOVEEDGE_H
#define BELOWANDABOVEEDGE_H

#include <QPainterPath>
#include "enums/edgetype.h"

class BelowAndAbove
{
public:
    QPointF below;
    QPointF above;

    bool validate();
};

class BelowAndAbovePoint
{
public:
    QPointF below;
    QPointF v;
    QPointF above;

    bool validate();
};

class BelowAndAboveEdge
{
public:
    QPolygonF    getPoly();
    QPainterPath getPainterPath();

    void         dump(int idx);
    void         dumpV(int idx);
    bool         validate(int idx);

    BelowAndAbovePoint v1;
    BelowAndAbovePoint v2;

    eEdgeType   type;
    QPointF     arcCenter;
    bool        convex;
};

#endif // BELOWANDABOVEEDGE_H
