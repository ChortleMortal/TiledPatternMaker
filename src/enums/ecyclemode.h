#pragma once
#ifndef ECYCLEMODE_H
#define ECYCLEMODE_H

#include <QString>

extern const  QString sCycleMode[];

enum eCycleMode
{
    CYCLE_VIEW_MOSAICS,
    CYCLE_VIEW_TILINGS,
    CYCLE_VIEW_ORIGINAL_PNGS,
};

#endif // ECYCLEMODE_H
