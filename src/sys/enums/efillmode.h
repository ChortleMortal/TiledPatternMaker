#pragma once
#ifndef E_FILL_MODE_H
#define E_FILL_MODE_H

#include <QString>

extern const QString sRepeatType[];

enum eRepeatType
{
    REPEAT_SINGLE,
    REPEAT_PACK,
    REPEAT_DEFINED,
    REPEAT_MAX = REPEAT_DEFINED
};

#endif // EMOTIFTYPE_H
