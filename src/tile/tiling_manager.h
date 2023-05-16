#pragma once
#ifndef TILINGMANAGER_H
#define TILINGMANAGER_H

#include <memory>
#include <QString>
#include "enums/estatemachineevent.h"

typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class TilingMaker>      TilingMakerPtr;

class TilingManager
{
public:
    TilingManager();

    TilingPtr loadTiling(QString name, eTILM_Event event);
    bool      saveTiling(QString name, TilingPtr tiling);
    bool      verifyNameFiles();

protected:
    void      setVCFillData(TilingPtr tiling);

private:
    class ViewControl     * view;
    class Configuration   * config;
    TilingMakerPtr          tilingMaker;
    class PrototypeMaker  * prototypeMaker;
};

#endif // TILINGMANAGER_H
