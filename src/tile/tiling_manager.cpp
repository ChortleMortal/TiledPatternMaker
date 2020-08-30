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
#include "makers/tiling_maker/tiling_maker.h"
#include "geometry/point.h"
#include "base/shared.h"
#include "base/fileservices.h"
#include "base/view.h"


TilingManager::TilingManager()
{
    workspace = Workspace::getInstance();
}

TilingPtr TilingManager::loadTiling(QString name)
{
    TilingPtr tp = workspace->findTiling(name);
    if (tp)
    {
       workspace->removeTiling(tp);
    }

    QString filename = FileServices::getTilingFile(name);
    if (filename.isEmpty())
    {
        qWarning() << "No tiling found with name" << name;
        return tp;
    }

    TilingLoader tm;
    tp = tm.readTilingXML(filename);
    if (tp)
    {
        qDebug().noquote() << "Loaded tiling:" << filename << tp->getName();
        tp->setState(TILING_LOADED);
        workspace->setCurrentTiling(tp);        // also adds
    }
    else
    {
        qWarning().noquote() << "Error loading" << filename;
    }
    return tp;
}

bool TilingManager::saveTiling(QString name, TilingPtr tp)
{
    if (tp->getName() != name)
    {
        tp->setName(name);
    }

    View * view = View::getInstance();
    QSize size  = view->size();
    tp->setCanvasSize(size);

    TilingMaker * maker = TilingMaker::getInstance();
    if (maker->getTiling() == tp)
    {
        Xform xf = maker->getCanvasXform();
        tp->setCanvasXform(xf);
    }

    TilingWriter writer(tp);
    bool rv = writer.writeTilingXML();   // uses the name in the tiling
    if (rv)
    {
        tp->setState(TILING_LOADED);
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
        workspace->resetTilings();
        TilingPtr tp = loadTiling(name);    // adds to workspace
        if (tp->getName() != name)
        {
            qWarning() << "Error: name does not match filename =" << name <<"internal name= " << tp->getName();
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

