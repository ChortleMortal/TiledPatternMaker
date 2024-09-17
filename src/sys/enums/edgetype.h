#ifndef EDGETYPE_H
#define EDGETYPE_H

#include <QString>

extern const QString sEdgeType[];

enum eEdgeType
{
    EDGETYPE_NULL,
    EDGETYPE_POINT,     // an incomplete edge
    EDGETYPE_LINE,
    EDGETYPE_CURVE,
    EDGETYPE_CHORD,
};

#endif // EDGETYPE_H
