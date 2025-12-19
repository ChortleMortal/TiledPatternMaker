#include "sys/enums/edgetype.h"

#define E2STR(x) #x

const QString sEdgeType[] =
{
    E2STR(EDGETYPE_NULL),
    E2STR(EDGETYPE_POINT),     // an incomplete edge
    E2STR(EDGETYPE_LINE),
    E2STR(EDGETYPE_CURVE)
};

const QString sCurveType[] =
{
    E2STR(CURVE_UNDEFINED),
    E2STR(CURVE_CONCAVE),
    E2STR(CURVE_CONVEX)
};

const QString  sSide[2]  = {
    "side-1",
    "side-2"
};

const QString  sLSide[2]  = {
    E2STR(EDGE_BOT),
    E2STR(EDGE_TOP)
};
