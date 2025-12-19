#pragma once
#ifndef ECYCLEMODE_H
#define ECYCLEMODE_H

#include <QString>

extern const  QString sCycleMode[];


enum eImageType
{
    IMAGE_MOSAICS,
    IMAGE_TILINGS
};

enum eActionType
{
    ACT_GEN_MOSAIC_BMP,
    ACT_GEN_TILING_BMP,
    ACT_GEN_COMPARE_WLIST
};

#endif // ECYCLEMODE_H
