#include "sys/engine/mosaic_stepper.h"
#include "sys/sys/fileservices.h"
#include "sys/sys.h"
#include "model/makers/mosaic_maker.h"
#include "gui/top/controlpanel.h"
#include "model/settings/configuration.h"

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

    cydata.files  = FileServices::getMosaicFiles(config->viewFileFilter);
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
    if (++cydata.cIndex < cydata.files.count())
    {
        VersionedFile file = cydata.files.at(cydata.cIndex);
        Sys::mosaicMaker->loadMosaic(file);
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
