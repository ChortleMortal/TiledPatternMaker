/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tile/tiling_manager.h"
#include "tile/tiling.h"
#include "tile/tiling_loader.h"
#include "tile/tiling_writer.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "makers/motif_maker/motif_maker.h"
#include "makers/decoration_maker/decoration_maker.h"
#include "geometry/point.h"
#include "base/shared.h"
#include "base/fileservices.h"
#include "viewers/view.h"
#include "viewers/viewcontrol.h"
#include "settings/model_settings.h"

TilingManager::TilingManager()
{
    view         = View::getInstance();
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

    TilingLoader tm;
    loadedTiling  = tm.readTilingXML(filename);
    if (!loadedTiling)
    {
        qWarning().noquote() << "Error loading" << filename;
        return loadedTiling;
    }

    qDebug().noquote() << "Loaded tiling:" << filename << loadedTiling->getName();
    loadedTiling->setState(Tiling::LOADED);
    view->frameSettings.setModelAlignment(M_ALIGN_TILING);

    // tiling is loaded, now use it
    QSize size  = loadedTiling->getSettings().getSize();
    QSize zsize = loadedTiling->getSettings().getZSize();
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
    case SM_FEATURE_CHANGED:
    case SM_FIGURE_CHANGED:
    case SM_TILING_CHANGED:
        break;
    }

    return loadedTiling;
}

void  TilingManager::setVCFillData(TilingPtr tiling)
{
    ModelSettings & settings = tiling->getSettings();
    ViewControl * vcontrol = ViewControl::getInstance();
    vcontrol->setFillData(settings.getFillData());
}

bool TilingManager::saveTiling(QString name, TilingPtr tiling)
{
    if (tiling->getName() != name)
    {
        tiling->setName(name);
    }

    // match size to current view
    QSize size  = view->frameSettings.getCropSize(config->getViewerType());
    tiling->getSettings().setSize(size);
    QSize zsize = view->frameSettings.getZoomSize(config->getViewerType());
    tiling->getSettings().setZSize(zsize);

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
    QStringList files = FileServices::getTilingNames();
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

