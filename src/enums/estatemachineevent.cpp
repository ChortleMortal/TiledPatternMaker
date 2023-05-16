#include "estatemachineevent.h"

#define E2STR(x) #x

const QString sTILM_Events[] =
{
    E2STR(TILM_LOAD_EMPTY),
    E2STR(TILM_LOAD_FROM_MOSAIC),
    E2STR(TILM_LOAD_SINGLE),
    E2STR(TILM_LOAD_MULTI),
    E2STR(TILM_RELOAD)
};

const QString sPROM_Events[] =
{
    E2STR(PROM_LOAD_EMPTY),
    E2STR(PROM_LOAD_SINGLE),
    E2STR(PROM_RELOAD_SINGLE),
    E2STR(PROM_LOAD_MULTI),
    E2STR(PROM_RELOAD_MULTI),
    E2STR(PROM_TILE_NUM_SIDES_CHANGED),
    E2STR(PROM_TILE_EDGES_CHANGED),
    E2STR(PROM_TILE_SCALE_CHANGED),
    E2STR(PROM_TILE_ROTATION_CHANGED),
    E2STR(PROM_TILE_REGULARITY_CHANGED),
    E2STR(PROM_TILING_ADDED),
    E2STR(PROM_TILING_DELETED),
    E2STR(PROM_TILING_CHANGED),
    E2STR(PROM_MOTIF_CHANGED),
    E2STR(PROM_RENDER)
};

const QString sMOSM_Events[] =
{
    E2STR(MOSM_LOAD_EMPTY),
    E2STR(MOSM_LOAD_SINGLE),
    E2STR(MOSM_RELOAD_SINGLE),
    E2STR(MOSM_LOAD_MULTI),
    E2STR(MOSM_RELOAD_MULTI),
    E2STR(MOSM_TILE_CHANGED),
    E2STR(MOSM_TILING_CHANGED),
    E2STR(MOSM_MOTIF_CHANGED),
    E2STR(MOSM_RENDER)
};
