#pragma once
#ifndef ESTATEMACHINEEVENT_H
#define ESTATEMACHINEEVENT_H

#include "model/tilings/tiling.h"
#include "model/prototypes/prototype.h"

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
    TILM_LOAD_REPLACE,
    TILM_REPLACE_ALL,
    TILM_RELOAD,
    TILM_LOAD_FROM_STEPPER,
    TILM_UNLOAD_TILING
};

class TilingEvent
{
public:
    TilingEvent() {};
    eTILM_Event   event;
    TilingPtr     tiling;
    TilingPtr     lastTiling;   // used by replace
};

// Prototype Maker state machine events
enum ePROM_Event
{
    PROM_LOAD_EMPTY,
    PROM_LOAD_SINGLE,
    PROM_LOAD_MULTI,
    PROM_RELOAD_SINGLE,
    PROM_RELOAD_MULTI,
    PROM_REPLACE,
    PROM_REPLACE_ALL,
    PROM_TILING_DELETED,
    PROM_TILING_CHANGED,                // singleton, repeats, vectors
    PROM_TILING_MODIFED,                // all placements
    PROM_TILE_UNIQUE_TRANS_CHANGED,     // scale,rot, mirror
    PROM_TILE_UNIQUE_NATURE_CHANGED,    // regularity, sides,
    PROM_TILE_UNIQUE_REGULARITY_CHANGED,// flip
    PROM_TILE_PLACED_TRANS_CHANGED,     // pos,scale,rot
    PROM_TILING_UNIT_CHANGED,
    PROM_MOTIF_CHANGED                  // from protomaker
};

class ProtoEvent
{
public:
    ProtoEvent() {}
    ePROM_Event   event;
    TilingPtr     tiling;
    PlacedTilePtr ptile;
    TilePtr       tile;
    QVector<TilingPtr> oldTilings;
};

// Mosaic Maker state machine events
enum eMOSM_Event
{
    MOSM_LOAD_PROTO_EMPTY,
    MOSM_LOAD_PROTO_SINGLE,
    MOSM_LOAD_PROTO_MULTI,
    MOSM_RELOAD_PROTO_SINGLE,
    MOSM_RELOAD_PROTO_MULTI,
    MOSM_PROTO_DELETED,
    MOSM_PROTO_CHANGED
};

class MosaicEvent
{
public:
    MosaicEvent() {}
    eMOSM_Event event;
    ProtoPtr prototype;
};

#endif // ESTATEMACHINEEVENT_H
