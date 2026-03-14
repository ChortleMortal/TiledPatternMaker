#include "sys/enums/efilltype.h"

#define E2STR(x) #x

const QString sFillType[6] =
{
    "Original Inside/Outside Coloring",     //FILL_ORIGINAL                = 0,
    "Alternative Inside/Outside Coloring ", //FILL_TWO_FACE                = 1,
    "Single Color per Shape",               //FILL_MULTI_FACE              = 2,
    "Multi Colors per Shape",               //FILL_MULTI_FACE_MULTI_COLORS = 3,
    "Deprecated",                           //DEPRECATED_FILL_FACE_DIRECT  = 4,
    "Color per Face",                       //FILL_FACE_DIRECT             = 5
};
