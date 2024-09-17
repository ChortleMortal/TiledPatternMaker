#pragma once
#ifndef DEBUG_MAP_H
#define DEBUG_MAP_H

#include "sys/geometry/map_base.h"
#include "sys/geometry/map.h"

class DebugMap : public MapBase
{
public:
    DebugMap();
    DebugMap(const Map * regularMap);

    void        insertDebugMark(QPointF m, QString txt);
    void        insertDebugEdge(EdgePtr edge);
    void        insertDebugLine(QPointF p1, QPointF p2);
    void        insertDebugLine(QLineF l1);
    void        insertDebugPolygon(QPolygonF & poly);
    void        insertDebugPoints(QPolygonF & poly);

    QString     info();

protected:
    void        insertEdge(VertexPtr v1,VertexPtr v2);

private:
    QString     name;
};

#endif // DEBUG_MAP_H
