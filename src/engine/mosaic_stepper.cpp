#include "engine/mosaic_stepper.h"
#include "misc/fileservices.h"
#include "misc/sys.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "panels/controlpanel.h"
#include "settings/configuration.h"

////////////////////////////////////////////
///
///  MosaicStepper - runs in GUI thread
///
////////////////////////////////////////////

MosaicStepper::MosaicStepper(ImageEngine * parent) : SteppingEngine(parent) {}

bool MosaicStepper::begin()
{
    if (isStarted())
    {
        end();
        return false;
    }

    qDebug() << "Starting mosaic view cycle";

    cydata.files  = FileServices::getMosaicNames(config->viewFileFilter);
    cydata.cIndex = -1;
    cydata.cCount = config->cycleInterval;     // start now

    setPaused (false);
    start(true);

    return next();
}

bool MosaicStepper::tick()
{
    return next();
}

bool MosaicStepper::next()
{
    if (!isStarted())
        return false;

    if (isPaused())
        return false;

    if (cydata.cCount++ <= config->cycleInterval)
    {
        qDebug() << "Tick";
        return false;
    }

    cydata.cCount = 0;
    if (++cydata.cIndex < cydata.files.size())
    {
        QString name = cydata.files.at(cydata.cIndex);
        Sys::mosaicMaker->loadMosaic(name);
    }
    else
    {
        end();
    }

    return true;
}

bool MosaicStepper::prev()
{
    return false;
}

bool MosaicStepper::end()
{
    qDebug() << "slot_stopCycle";

    setPaused(false);

    if (isStarted())
    {
        start(false);
        finish("Mosaic viewing");
        panel->clearStatus();
        return true;
    }
    else
    {
        return false;
    }
}
