#pragma once
#ifndef LOAD_UNIT_H
#define LOAD_UNIT_H

#include <QElapsedTimer>
#include "sys/sys/versioning.h"

enum eLoadUnitType
{
    LT_LEGACY,
    LT_MOSAIC,
    LT_TILING,
    LT_UNDEFINED
};

enum eLoadUnitState
{
    LS_READY,
    LS_LOADING,
    LS_LOADED,
    LS_FAILED
};

class LoadUnit
{
public:
    LoadUnit(eLoadUnitType type);

    eLoadUnitState  getLoadState()      { return loadState; }
    VersionedFile   getLoadFile()       { return loadFile; }

    void            start(VersionedFile name);
    void            end(eLoadUnitState state);
    void            declareLoaded(VersionedFile name);
    void            ready();

    QElapsedTimer   loadTimer;

private:
    eLoadUnitType   loadType;
    eLoadUnitState  loadState;
    VersionedFile   loadFile;
};

#endif // LOAD_UNIT_H
