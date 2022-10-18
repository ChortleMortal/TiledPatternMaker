#include "estatemachineevent.h"

#define E2STR(x) #x

const QString sSM_Events[] =
    {
        E2STR(SM_LOAD_EMPTY),
        E2STR(SM_LOAD_FROM_MOSAIC),
        E2STR(SM_LOAD_SINGLE),
        E2STR(SM_RELOAD_SINGLE),
        E2STR(SM_LOAD_MULTI),
        E2STR(SM_RELOAD_MULTI),
        E2STR(SM_TILE_CHANGED),
        E2STR(SM_TILING_CHANGED),
        E2STR(SM_MOTIF_CHANGED),
        E2STR(SM_RENDER)
};
