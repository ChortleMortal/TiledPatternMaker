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

#include "tilingmanager.h"
#include "tile/Tiling.h"
#include "tile/tilemaker.h"
#include "geometry/Point.h"
#include "base/shared.h"
#include "base/fileservices.h"

TilingManager * TilingManager::mpThis = nullptr;

TilingManager * TilingManager::getInstance()
{
    if (mpThis == nullptr)
    {
        mpThis = new TilingManager();
    }
    return mpThis;
}

TilingManager::TilingManager()
{
}

TilingPtr TilingManager::loadTiling(QString name)
{
    TilingPtr tp;
    QString filename = FileServices::getTilingFile(name);
    if (filename.isEmpty())
    {
        qWarning() << "No tiling found with name" << name;
        return tp;
    }

    Tiling t;
    tp = t.readTilingXML(filename);
    if (tp)
    {
        qDebug().noquote() << "Loaded tiling:" << filename << tp->getName();
    }
    else
    {
        qWarning().noquote() << "Error loading" << filename;
    }
    return tp;
}

bool TilingManager::verifyNameFiles()
{
    QStringList files = FileServices::getTilingNames();
    for (int i=0; i < files.size(); i++)
    {
        QString name = files[i];
        TilingPtr tp = TilingManager::loadTiling(name);
        if (tp->getName() != name)
        {
            qWarning() << "Error: name does not match (1)" << name;
            return false;
        }
        if (!FileServices::verifyTilingName(name))
        {
            qWarning() << "Error: name does not match (3)" << name;
            return false;
        }
    }
    return true;
}

