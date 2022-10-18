#include "tile/tiling_manager.h"
#include "misc/fileservices.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "makers/motif_maker/motif_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "settings/model_settings.h"
#include "settings/configuration.h"
#include "misc/backgroundimage.h"
#include "tile/tiling.h"
#include "tile/tiling_loader.h"
#include "tile/tiling_writer.h"
#include "viewers/viewcontrol.h"

TilingManager::TilingManager()
{
    view         = ViewControl::getInstance();
    config       = Configuration::getInstance();
    tilingMaker  = TilingMaker::getSharedInstance();
    motifMaker   = MotifMaker::getInstance();
}

TilingPtr TilingManager::loadTiling(QString name, eSM_Event mode)
{
    TilingPtr loadedTiling;

    QString filename = FileServices::getTilingFile(name);
    if (filename.isEmpty())
    {
        qWarning() << "No tiling found with name" << name;
        return loadedTiling;
    }

    BkgdImgPtr bip = BackgroundImage::getSharedInstance();
    bip->unload();

    qInfo().noquote() << "TilingManager::loadTiling" << filename << sSM_Events[mode];

    TilingLoader tm;
    loadedTiling  = tm.readTilingXML(filename);
    if (!loadedTiling)
    {
        qWarning().noquote() << "Error loading" << filename;
        return loadedTiling;
    }

    qInfo().noquote() << "Loaded  tiling:" << filename << loadedTiling->getName();

    loadedTiling->setState(Tiling::LOADED);
    view->frameSettings.setModelAlignment(M_ALIGN_TILING);

    // tiling is loaded, now use it
    QSize size  = loadedTiling->getData().getSettings().getSize();
    QSize zsize = loadedTiling->getData().getSettings().getZSize();
    switch(mode)
    {
    case SM_LOAD_SINGLE:
    case SM_RELOAD_SINGLE:
    case SM_LOAD_MULTI:
    case SM_RELOAD_MULTI:
        view->frameSettings.initialise(VIEW_TILING_MAKER,size,zsize);
        view->frameSettings.initialiseCommon(size,zsize);

        setVCFillData(loadedTiling);
        tilingMaker->sm_take(loadedTiling, mode);
        break;

    case SM_LOAD_FROM_MOSAIC:
        view->frameSettings.initialise(VIEW_TILING_MAKER,size,zsize);

        setVCFillData(loadedTiling);
        break;

    case SM_LOAD_EMPTY:
    case SM_RENDER:
    case SM_TILE_CHANGED:
    case SM_MOTIF_CHANGED:
    case SM_TILING_CHANGED:
        break;
    }

    return loadedTiling;
}

void  TilingManager::setVCFillData(TilingPtr tiling)
{
    const FillData & fd = tiling->getData().getFillData();
    ViewControl * vcontrol = ViewControl::getInstance();
    vcontrol->setFillData(fd);
}

bool TilingManager::saveTiling(QString name, TilingPtr tiling)
{
    if (tiling->getName() != name)
    {
        tiling->setName(name);
    }

    // match size to current view
    QSize size  = view->frameSettings.getCropSize(config->getViewerType());
    QSize zsize = view->frameSettings.getZoomSize(config->getViewerType());

    const ModelSettings & ms = tiling->getData().getSettings();
    if (ms.getSize() != size || ms.getZSize() != zsize)
    {
        ModelSettings & settings = tiling->getDataAccess().getSettingsAccess();
        settings.setSize(size);
        settings.setZSize(zsize);
    }

    if (tilingMaker->getSelected() == tiling)
    {
        Xform xf = tilingMaker->getCanvasXform();
        tiling->setCanvasXform(xf);
    }

    // write
    TilingWriter writer(tiling);
    bool rv = writer.writeTilingXML();   // uses the name in the tiling
    if (rv)
    {
        tiling->setState(Tiling::LOADED);
    }
    return rv;
}

bool TilingManager::verifyNameFiles()
{
    bool rv = true;
    QStringList files = FileServices::getTilingNames(LOAD_ALL);
    for (int i=0; i < files.size(); i++)
    {
        QString name = files[i];
        TilingPtr tiling = loadTiling(name,SM_LOAD_SINGLE);
        if (tiling->getName() != name)
        {
            qWarning() << "Error: name does not match filename =" << name <<"internal name= " << tiling->getName();
            rv = false;
        }
        if (!FileServices::verifyTilingName(name))
        {
            qWarning() << "Error: name does not match filename =" << name;
            rv = false;
        }
    }
    return rv;
}

