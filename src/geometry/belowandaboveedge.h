#ifndef BELOWANDABOVEEDGE_H
#define BELOWANDABOVEEDGE_H

#include <QPainterPath>
#include "enums/edgetype.h"

struct BelowAndAbove
{
    QPointF below;
    QPointF above;
};

class BelowAndAbovePoint
{
public:
    QPointF below;
    QPointF v;
    QPointF above;

    bool validate();
    inline bool isReal(qreal x);
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
