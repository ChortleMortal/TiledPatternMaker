#pragma once
#ifndef TILINGMANAGER_H
#define TILINGMANAGER_H

#include <QString>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <memory>
#endif
#include "sys/enums/estatemachineevent.h"
#include "sys/sys/versioning.h"

typedef std::shared_ptr<class Tiling> TilingPtr;

class TilingManager
{
public:
    TilingManager() {}

    TilingPtr loadTiling(VersionedFile &vfile, eTILM_Event event);
    bool      saveTiling(TilingPtr tiling);
    bool      verifyTilingFiles();
};

#endif // TILINGMANAGER_H
