#pragma once
#ifndef TILINGMANAGER_H
#define TILINGMANAGER_H

#include <QString>

#include "sys/enums/estatemachineevent.h"
#include "sys/sys/versioning.h"

typedef std::shared_ptr<class Tiling> TilingPtr;

class TilingManager
{
public:
    TilingManager() {}

    TilingPtr loadTiling(VersionedFile &vfile, eTILM_Event event);
    bool      saveTiling(TilingPtr tiling, VersionedFile &retSavedFile, bool forceOverwrite);
    bool      verifyTilingFiles();
};

#endif // TILINGMANAGER_H
