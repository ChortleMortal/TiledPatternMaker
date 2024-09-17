#include "sys/sys/load_unit.h"
#include "sys/sys.h"
#include "model/settings/configuration.h"

LoadUnit::LoadUnit()
{
    loadState = LOADING_NONE;
    lastState = LOADING_NONE;
}

void LoadUnit::setLoadState(eLoadState state, VersionedFile file)
{
    loadState = state;
    loadFile  = file;

    lastState = state;
    lastFile  = file;

    switch (state)
    {
    case LOADING_MOSAIC:
        Sys::config->lastLoadedMosaic = file.getVersionedName();
        break;

    case LOADING_TILING:
        Sys::config->lastLoadedTiling = file.getVersionedName();
        break;

    case LOADING_LEGACY:
        Sys::config->lastLoadedLegacy = file.getVersionedName();
        break;

    default:
        break;
    }
};
