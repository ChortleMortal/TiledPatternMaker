#pragma once
#ifndef TILINGMANAGER_H
#define TILINGMANAGER_H

#include <QString>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <memory>
#endif
#include "enums/estatemachineevent.h"

typedef std::shared_ptr<class Tiling>           TilingPtr;

class TilingManager
{
public:
    TilingManager();

    TilingPtr loadTiling(QString name, eTILM_Event event);
    bool      saveTiling(QString name, TilingPtr tiling);
    bool      verifyNameFiles();

private:
    class ViewController  * viewController;
    class Configuration   * config;
    class TilingMaker     * tilingMaker;
    class PrototypeMaker  * prototypeMaker;
};

#endif // TILINGMANAGER_H
