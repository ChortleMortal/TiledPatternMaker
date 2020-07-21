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

#include "base/cycler.h"
#include "base/configuration.h"
#include "base/canvas.h"
#include "base/view.h"
#include "base/tiledpatternmaker.h"
#include "viewers/workspace_viewer.h"
#include "base/fileservices.h"

Cycler * Cycler::mpThis = nullptr;

Cycler * Cycler::getInstance()
{
    if (mpThis == nullptr)
    {
        mpThis = new Cycler();
    }
    return mpThis;
}


Cycler::Cycler() : QObject()
{
    busy        = false;
    cycleMode   = CYCLE_NONE;

    config      = Configuration::getInstance();
    canvas      = Canvas::getInstance();
}

void Cycler::init(QThread * thread)
{
    moveToThread(thread);

    QTimer * timer = new QTimer();

    Workspace * ws = Workspace::getInstance();
    connect(this,   &Cycler::sig_clearCanvas, ws,   &Workspace::slot_clearCanvas);
    connect(timer,  &QTimer::timeout,         this, &Cycler::slot_timeout);

    timer->start(1000);
}

Cycler::~Cycler()
{
    qDebug() << "Cycler destructor";
}


void Cycler::slot_startCycle(eCycleMode mode)
{
    cycleMode = mode;

    qDebug() << "slot_cycle" << sCycleMode[cycleMode];

    cyclePause = false;
    busy       = false;

    switch(cycleMode)
    {
    case CYCLE_STYLES:
    case CYCLE_SAVE_STYLE_BMPS:
        startCycleStyles();
        break;
    case CYCLE_TILINGS:
    case CYCLE_SAVE_TILING_BMPS:
        startCycleTilings();
        break;
    case CYCLE_ORIGINAL_PNGS:
        startCycleOriginalDesignPngs();
        break;
    case CYCLE_COMPARE_IMAGES:
        startCycleCompareImages();
        break;
    case CYCLE_NONE:
        qWarning() << "CYCLE_NONE sent to start";
        break;
    }
}

void Cycler::slot_stopCycle()
{
    eCycleMode oldMode = cycleMode;
    qDebug() << "slot_stopCycle";
    cycleMode = CYCLE_NONE;
    if (oldMode != CYCLE_NONE)
        sig_finished();
    busy = false;
}

void Cycler::slot_ready()
{
    busy = false;
    slot_timeout(); //force immediate
}

void Cycler::slot_timeout()
{
    if (cycleMode == CYCLE_NONE)
    {
        return;
    }

    if (cyclePause || busy)
    {
        return;
    }

    switch (cycleMode)
    {
    case CYCLE_TILINGS:
        if (++cCount < config->cycleInterval)
            break;

        cCount = 0;
        if (++cIndex < files.size())
        {
            QString name = files.at(cIndex);
            sig_loadTiling(name);
        }
        else
        {
            slot_stopCycle();
        }
        break;

    case CYCLE_STYLES:
        if (++cCount < config->cycleInterval)
            break;

        cCount = 0;
        busy   = true;
        if (++cIndex < files.size())
        {
            QString name = files.at(cIndex);
            sig_loadXML(name);
        }
        else
        {
            slot_stopCycle();
        }
        break;

    case  CYCLE_SAVE_STYLE_BMPS:
        if (++cCount < config->cycleInterval)
           break;

        cCount = 0;
        busy   = true;
        if (++cIndex < files.size())
        {
            // this saves a copy for testing
            QString name  = files.at(cIndex);
            emit sig_saveAsBMP(name);
        }
        else
        {
            slot_stopCycle();
        }
        break;

    case  CYCLE_SAVE_TILING_BMPS:
        if (++cCount < config->cycleInterval)
           break;

        cCount = 0;
        busy   = true;
        if (++cIndex < files.size())
        {
            // this saves a copy for testing
            QString name  = files.at(cIndex);
            emit sig_saveTilingAsBMP(name);
        }
        else
        {
            slot_stopCycle();
        }
        break;

    case CYCLE_COMPARE_IMAGES:
        if (map_it == mapa.end())
        {
            slot_stopCycle();
        }
        else
        {
            busy = true;
            QString name = map_it.key();
            map_it++;
            emit sig_compare(name,name);
        }
        break;

    case CYCLE_ORIGINAL_PNGS:
    case CYCLE_NONE:
        break;
    }
}

void Cycler::slot_psuedoKey(int key )
{
    qDebug() << "key=" << key;

    if (key ==  Qt::Key_Space)
    {
        if (cycleMode == CYCLE_ORIGINAL_PNGS)
        {
            nextCyclePng();
        }
        else
        {
            cyclePause = !cyclePause;
            cCount     = config->cycleInterval;
            qDebug() << "pause="  << cyclePause;
        }
    }
}

void Cycler::startCycleStyles()
{
    if (config->designFilterCheck &&  !config->designFilter.isEmpty())
    {
        files = FileServices::getFilteredDesignNames(config->designFilter);
    }
    else
    {
        files = FileServices::getDesignNames();
    }

    cIndex = -1;
    cCount = config->cycleInterval;     // start now
}

void Cycler::startCycleTilings()
{
    if (config->tileFilterCheck && !config->tileFilter.isEmpty())
    {
        files = FileServices::getFilteredTilingNames(config->tileFilter);
    }
    else
    {
        files = FileServices::getTilingNames();
    }

    cIndex = -1;
    cCount = config->cycleInterval;     // start now
}

void Cycler::startCycleCompareImages()
{
    mapa = FileServices::getDirFiles(config->compareDir0);
    mapb = FileServices::getDirFiles(config->compareDir1);

    map_it = mapa.begin();
}

void Cycler::slot_view_images()
{
    map_it--;
    QString name   = map_it.key();
    QString file1  = map_it.value();
    QString file2  = mapb.value(name);
    map_it++;

    emit sig_viewImage(file1);
    emit sig_viewImage(file2);
}

void Cycler::startCycleOriginalDesignPngs()
{
    pngRow   = 0;
    pngCol   = 0;
    pngIndex = 0;

    fileFilter << "*.png";

    QString path = config->examplesDir;
    QDir adir(path);
    files = adir.entryList(fileFilter);
    qDebug() << "num pngs =" << files.size();

    nextCyclePng();
}

void Cycler::nextCyclePng()
{
    qDebug() << "page starting: " << pngIndex;

    emit sig_clearCanvas();
    while (pngIndex < files.size())
    {
        emit sig_show_png(files.at(pngIndex),pngRow,pngCol);
        pngIndex++;

        if (++pngCol > 4)
        {
            pngCol = 0;
            if (++pngRow > 4)
            {
                // pause
                pngRow = 0;
                return;
            }
        }
    }
    pngIndex   = 0;
    pngRow = 0;
    pngCol = 0;
}

