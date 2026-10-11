#pragma once

#include <QString>

extern const QString sRepeatType[];

enum eRepeatType
{
    REPEAT_SINGLE,
    REPEAT_PACK,
    REPEAT_DEFINED,
    REPEAT_MAX = REPEAT_DEFINED
};
