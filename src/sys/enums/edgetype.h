#pragma once
#ifndef EDGETYPE_H
#define EDGETYPE_H

#include <QString>

extern const QString sEdgeType[4];
extern const QString sSide[2];
extern const QString sLSide[2];
extern const QString sCurveType[3];

enum eEdgeType
{
    EDGETYPE_NULL,
    EDGETYPE_POINT,     // an incomplete edge
    EDGETYPE_LINE,
    EDGETYPE_CURVE
};

enum eCurveType
{
    CURVE_UNDEFINED,
    CURVE_CONCAVE,
    CURVE_CONVEX
};

enum eSide
{
    SIDE_1 = 0,
    SIDE_2 = 1
};

enum eLSide
{
    // assumes clockwise polygons
    EDGE_OUTER = 0,
    EDGE_INNER = 1
};

#endif // EDGETYPE_H
