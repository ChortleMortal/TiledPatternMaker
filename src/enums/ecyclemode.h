#pragma once
#ifndef ECYCLEMODE_H
#define ECYCLEMODE_H

#include <QString>

extern const  QString sCycleMode[];

enum eCycleMode
{
    CYCLE_NONE,
    CYCLE_MOSAICS,
    CYCLE_TILINGS,
    CYCLE_ORIGINAL_PNGS,
    CYCLE_SAVE_MOSAIC_BMPS,
    CYCLE_SAVE_TILING_BMPS,
    CYCLE_COMPARE_ALL_BMPS,
    CYCLE_COMPARE_WORKLIST_BMPS
};

#endif // ECYCLEMODE_H
