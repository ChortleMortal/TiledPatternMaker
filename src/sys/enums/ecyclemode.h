#pragma once

#include <QString>

extern const  QString sCycleMode[];


enum eImageType
{
    IMAGE_MOSAICS,
    IMAGE_TILINGS
};

enum eActionType
{
    ACT_GEN_MOSAIC_PNG,
    ACT_GEN_TILING_PNG,
    ACT_GEN_COMPARE_WLIST,
    ACT_REGEN_MOSAIC_XML,
    ACT_CONVERT_BMPS
};
