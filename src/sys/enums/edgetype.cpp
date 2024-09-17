#include "sys/enums/edgetype.h"

#define E2STR(x) #x

const QString sEdgeType[] =
{
    E2STR(EDGETYPE_NULL),
    E2STR(EDGETYPE_POINT),     // an incomplete edge
    E2STR(EDGETYPE_LINE),
    E2STR(EDGETYPE_CURVE),
    E2STR(EDGETYPE_CHORD)
};
