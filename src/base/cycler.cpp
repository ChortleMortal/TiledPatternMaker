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
#include "base/tiledpatternmaker.h"
#include "base/fileservices.h"
#include "panels/versioned_list_widget.h"
#include "viewers/viewcontrol.h"

Q_DECLARE_METATYPE(QTextCharFormat)
Q_DECLARE_METATYPE(QTextCursor)

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
}

void Cycler::init(QThread * thread)
{
    moveToThread(thread);

    timer = new QTimer();

    qRegisterMetaType<eCycleMode>();
    qRegisterMetaType<QTextEdit*>();
    qRegisterMetaType<QTextCharFormat>();
    qRegisterMetaType<QTextCursor>();

    ViewControl * vc = ViewControl::getInstance();
    connect(this,   &Cycler::sig_clearView, vc,   &ViewControl::slot_clearView);
    connect(timer,  &QTimer::timeout,       this, &Cycler::slot_timeout);

    timer->start(1000);
}

Cycler::~Cycler()
{
    qDebug() << "Cycler destructor";
}

void Cycler::slot_startCycle(eCycleMode mode)
{
    if (cycleMode != CYCLE_NONE)
    {
        qDebug() << "Stopping the cycler";
        slot_stopCycle();
        return;
    }

    cycleMode = mode;
    qDebug() << "slot_cycle" << sCycleMode[cycleMode];

    cyclePause = false;
    busy       = false;

    switch(cycleMode)
    {
    case CYCLE_STYLES:
        startCycleStyles();
        break;
    case CYCLE_TILINGS:
        startCycleTilings();
        break;
    case CYCLE_ORIGINAL_PNGS:
        startCycleOriginalDesignPngs();
        break;
    case CYCLE_COMPARE_IMAGES:
        startCycleCompareImages();
        break;
    case RE_CYCLE_COMPARE_IMAGES:
        startReCycleCompareImages();
        break;
    case CYCLE_NONE:
    case CYCLE_SAVE_TILING_BMPS:
    case CYCLE_SAVE_STYLE_BMPS:
        qWarning() << "slot_startCycle: unexpeced mode";
    }
}

void Cycler::slot_stopCycle()
{
    eCycleMode oldMode = cycleMode;
    qDebug() << "slot_stopCycle";
    cycleMode = CYCLE_NONE;
    if (oldMode != CYCLE_NONE)
    {
        imgList.clear();
        emit sig_finished();
    }
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
            emit sig_cycleLoadTiling(name);
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
            emit sig_cycleLoadMosaic(name);
        }
        else
        {
            slot_stopCycle();
        }
        break;


    case CYCLE_COMPARE_IMAGES:
    case RE_CYCLE_COMPARE_IMAGES:
        if (imgList_it == imgList.end())
        {
            slot_stopCycle();
        }
        else
        {
            busy = true;
            QString name = *imgList_it;
            imgList_it++;
            emit sig_compare(name,name,true);
        }
        break;

    case CYCLE_NONE:
    case CYCLE_ORIGINAL_PNGS:
    case CYCLE_SAVE_STYLE_BMPS:
    case CYCLE_SAVE_TILING_BMPS:
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
    if (config->mosaicFilterCheck &&  !config->mosaicFilter.isEmpty())
    {
        files = FileServices::getFilteredDesignNames(config->mosaicFilter);
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
    mapa = FileServices::getDirBMPFiles(config->compareDir0);
    mapb = FileServices::getDirBMPFiles(config->compareDir1);

    QStringList names = mapa.keys();

    VersionList vlist;
    vlist.create(names);

    imgList    = vlist.recompose();
    imgList_it = imgList.begin();
}

void Cycler::startReCycleCompareImages()
{
    mapa = FileServices::getDirBMPFiles(config->compareDir0);
    mapb = FileServices::getDirBMPFiles(config->compareDir1);

    QStringList names = mapa.keys();

    imgList    = config->badImages;
    imgList_it = imgList.begin();
}

void Cycler::slot_view_images()
{
    if (!imgList.isEmpty())
    {
        imgList_it--;
        QString name   = *imgList_it;
        QString file1  = mapa.value(name);
        QString file2  = mapb.value(name);
        imgList_it++;

        emit sig_viewImage(file1);
        emit sig_viewImage(file2);
    }
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

    emit sig_clearView();
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


