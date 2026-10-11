#pragma once

#include <QString>

extern const QString sFillType[];

enum eFillType
{
    FILL_ORIGINAL                = 0,
    FILL_TWO_FACE                = 1,
    FILL_MULTI_FACE              = 2,
    FILL_MULTI_FACE_MULTI_COLORS = 3,
    DEPRECATED_FILL_FACE_DIRECT  = 4,    // DEPRECATED
    FILL_FACE_DIRECT             = 5
};
