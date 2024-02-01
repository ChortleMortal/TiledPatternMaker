#pragma once
#ifndef MOSAIC_MANAGER_H
#define MOSAIC_MANAGER_H

#include <QString>

typedef std::shared_ptr<class Mosaic> MosaicPtr;

class MosaicManager
{
    friend class MosaicMaker;

public:
    MosaicPtr loadMosaic(QString name);
    bool      saveMosaic(QString name, QString &savedName, bool forceOverwrite);

protected:
    MosaicManager();

private:
    class View            * view;
    class ViewController  * viewControl;
    class Configuration   * config;
    class MosaicMaker     * mosaicMaker;
};

#endif
