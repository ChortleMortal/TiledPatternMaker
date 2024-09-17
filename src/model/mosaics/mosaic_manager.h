#pragma once
#ifndef MOSAIC_MANAGER_H
#define MOSAIC_MANAGER_H

#include "sys/sys/versioning.h"
#include <QString>

typedef std::shared_ptr<class Mosaic> MosaicPtr;

class MosaicManager
{
    friend class MosaicMaker;

public:
    MosaicPtr loadMosaic(VersionedFile vfile);
    bool      saveMosaic(MosaicPtr mosaic, VersionedFile &rvSavedFile, bool forceOverwrite);

protected:
    MosaicManager() {}

private:
};

#endif
