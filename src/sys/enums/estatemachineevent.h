#pragma once
#ifndef ESTATEMACHINEEVENT_H
#define ESTATEMACHINEEVENT_H

#include <QString>

extern const QString sTILM_Events[];
extern const QString sPROM_Events[];
extern const QString sMOSM_Events[];

// Tiling Maker state machine events
enum eTILM_Event
{
    TILM_LOAD_EMPTY,
    TILM_LOAD_FROM_MOSAIC,
    TILM_LOAD_SINGLE,
    TILM_LOAD_MULTI,
    TILM_RELOAD,
    TILM_LOAD_FROM_STEPPER
};

// Prototype Maker state machine events
enum ePROM_Event
{
    PROM_LOAD_EMPTY,
    PROM_LOAD_SINGLE,
    PROM_RELOAD_SINGLE,
    PROM_LOAD_MULTI,
    PROM_RELOAD_MULTI,
    PROM_TILE_NUM_SIDES_CHANGED,
    PROM_TILE_EDGES_CHANGED,
    PROM_TILE_SCALE_CHANGED,
    PROM_TILE_ROTATION_CHANGED,
    PROM_TILE_REGULARITY_CHANGED,
    PROM_TILES_ADDED,
    PROM_TILES_DELETED,
    PROM_TILING_DELETED,
    PROM_TILING_CHANGED,    // placements,transform,fill-vectors,repeats
    PROM_MOTIF_CHANGED,
    PROM_RENDER
};

// Mosaic Maker state machine events
enum eMOSM_Event
{
    MOSM_LOAD_EMPTY,
    MOSM_LOAD_SINGLE,
    MOSM_RELOAD_SINGLE,
    MOSM_LOAD_MULTI,
    MOSM_RELOAD_MULTI,
    MOSM_TILE_CHANGED,
    MOSM_TILING_CHANGED,
    MOSM_MOTIF_CHANGED,
    MOSM_TILING_DELETED,
    MOSM_RENDER
};

#endif // ESTATEMACHINEEVENT_H
