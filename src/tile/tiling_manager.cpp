#include "tile/tiling_manager.h"
#include "misc/fileservices.h"
#include "makers/prototype_maker/prototype_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "settings/model_settings.h"
#include "settings/configuration.h"
#include "tile/tiling.h"
#include "tile/tiling_loader.h"
#include "tile/tiling_writer.h"
#include "viewers/backgroundimageview.h"
#include "viewers/viewcontrol.h"

TilingManager::TilingManager()
{
    view            = ViewControl::getInstance();
    config          = Configuration::getInstance();
    tilingMaker     = TilingMaker::getInstance();
    prototypeMaker  = PrototypeMaker::getInstance();
}

TilingPtr TilingManager::loadTiling(QString name, eTILM_Event event)
{
    TilingPtr loadedTiling;
    
    QString filename = FileServices::getTilingXMLFile(name);
    if (filename.isEmpty())
    {
        qWarning() << "No tiling found with name" << name;
        return loadedTiling;
    }
    
    auto bip = BackgroundImageView::getInstance();
    bip->unload();

    qInfo().noquote() << "TilingManager::loadTiling" << filename << sTILM_Events[event];

    TilingLoader tm;
    loadedTiling  = tm.readTilingXML(filename);
    if (!loadedTiling)
    {
        qWarning().noquote() << "Error loading" << filename;
        return loadedTiling;
    }

    qInfo().noquote() << "Loaded  tiling:" << filename << loadedTiling->getName();

    loadedTiling->setState(Tiling::LOADED);
    
    auto & settings = view->getViewSettings();
    settings.setModelAlignment(M_ALIGN_TILING);

    // tiling is loaded, now use it
    QSize size  = loadedTiling->getData().getSettings().getSize();
    QSize zsize = loadedTiling->getData().getSettings().getZSize();
    switch(event)
    {
    case TILM_LOAD_SINGLE:
    case TILM_LOAD_MULTI:
    case TILM_RELOAD:
        settings.initialise(VIEW_TILING_MAKER,size,zsize);
        settings.initialiseCommon(size,zsize);

        setVCFillData(loadedTiling);
        tilingMaker->sm_takeUp(loadedTiling, event);
        break;

    case TILM_LOAD_FROM_MOSAIC:
        settings.initialise(VIEW_TILING_MAKER,size,zsize);

        setVCFillData(loadedTiling);
        break;

    case TILM_LOAD_EMPTY:
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
    auto & settings = view->getViewSettings();
    auto mostRecent = view->getMostRecent();
    QSize size      = settings.getCropSize(mostRecent);
    QSize zsize     = settings.getZoomSize(mostRecent);

    const ModelSettings & ms = tiling->getData().getSettings();
    if (ms.getSize() != size || ms.getZSize() != zsize)
    {
        ModelSettings & settings = tiling->getRWData(false).getSettingsAccess();
        settings.setSize(size);
        settings.setZSize(zsize);
    }

    if (tilingMaker->getSelected() == tiling)
    {
        Xform xf = TilingMakerView::getInstance()->getCanvasXform();
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
    QStringList files = FileServices::getTilingNames(ALL_TILINGS);
    for (int i=0; i < files.size(); i++)
    {
        QString name = files[i];
        TilingPtr tiling = loadTiling(name,TILM_LOAD_SINGLE);
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

