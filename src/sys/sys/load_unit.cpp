#include "sys/sys/load_unit.h"
#include "sys/sys.h"
#include "model/settings/configuration.h"

LoadUnit::LoadUnit(eLoadUnitType type)
{
    loadType  = type;
    loadState = LS_READY;
}

void LoadUnit::start(VersionedFile file)
{
    loadState = LS_LOADING;
    loadFile  = file;
    loadTimer.restart();
}

void LoadUnit::end(eLoadUnitState state)
{
    loadState = state;

    if (state == LS_LOADED)
    {
        switch (loadType)
        {
        case LT_MOSAIC:
            Sys::config->lastLoadedMosaic = loadFile.getVersionedName();
            break;

        case LT_TILING:
            Sys::config->lastLoadedTiling = loadFile.getVersionedName();
            break;

        case LT_LEGACY:
            Sys::config->lastLoadedLegacy = loadFile.getVersionedName();
            break;

        default:
            break;
        }
        Sys::config->lastLoadedType = loadType;
    }
}

void LoadUnit::ready()
{
    loadState = LS_READY;
}

void  LoadUnit::declareLoaded(VersionedFile name)
{
    start(name);
    end(LS_LOADED);
    ready();
}
