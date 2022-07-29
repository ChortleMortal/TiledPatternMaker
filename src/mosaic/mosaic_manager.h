#ifndef MOSAIC_MANAGER_H
#define MOSAIC_MANAGER_H

#include <QString>

class MosaicManager
{
public:
    MosaicManager();

    bool loadMosaic(QString name);
    bool saveMosaic(QString name, QString &savedName, bool forceOverwrite, bool forceTest);

private:
    class ViewControl     * view;
    class Configuration   * config;
    class MosaicMaker     * mosaicMaker;
};

#endif
