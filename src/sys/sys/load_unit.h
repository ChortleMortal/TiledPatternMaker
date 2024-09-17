#pragma once
#ifndef LOAD_UNIT_H
#define LOAD_UNIT_H

#include <QElapsedTimer>
#include "sys/sys/versioning.h"

enum eLoadState
{
    LOADING_NONE,
    LOADING_TILING,
    LOADING_MOSAIC,
    LOADING_LEGACY
};

class LoadUnit
{
public:
    LoadUnit();

    eLoadState      getLoadState()      { return loadState; }
    VersionedFile   getLoadFile()       { return loadFile; }
    eLoadState      getLastLoadState()  { return lastState; }
    VersionedFile   getLastLoadFile()   { return lastFile; }

    void            setLoadState(eLoadState state, VersionedFile name);
    void            resetLoadState() { loadState = LOADING_NONE; }

    QElapsedTimer   loadTimer;

private:
    eLoadState      loadState;
    VersionedFile   loadFile;
    VersionedFile   lastFile;
    eLoadState      lastState;
};

#endif // LOAD_UNIT_H
