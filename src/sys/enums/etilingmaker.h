#pragma once

#include <QString>

enum eTileMenuRefresh
{
    TMR_ALL_CLEAR,
    TMR_ALL,
    TMR_MAKER_TILINGS,
    TMR_TILING,
    TMR_TILING_HEADER,
    TMR_TILING_UNIT,
    TMR_PLACED_TILE,
};

enum eVectorState
{
    VEC_READY,
    VEC_SETTING_T1,
    VEC_SETTING_T2
};
