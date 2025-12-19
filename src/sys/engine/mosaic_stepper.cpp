#include "gui/top/controlpanel.h"
#include "gui/top/system_view.h"
#include "model/makers/mosaic_maker.h"
#include "model/settings/configuration.h"
#include "sys/engine/mosaic_stepper.h"
#include "sys/sys.h"
#include "sys/sys/fileservices.h"

////////////////////////////////////////////
///
///  MosaicStepper - runs in GUI thread
///
////////////////////////////////////////////

MosaicStepper::MosaicStepper(ImageEngine * parent) : SteppingEngine(parent)
{
    mImmediate = false;
    connect(Sys::sysview, &SystemView::sig_loadComplete, this, &MosaicStepper::do_immediate, Qt::QueuedConnection);
}

bool MosaicStepper::begin()
{
    if (isStarted())
    {
        end();
        return false;
    }

    qDebug() << "Starting mosaic view cycle";

    cydata.files  = FileServices::getMosaicFiles(config->imageFileFilter);
    cydata.cIndex = -1;
    cydata.cCount = config->cycleInterval;     // start now

    setPaused (false);
    start(true);

    if (mImmediate)
        return do_immediate();
    else
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

    if (mImmediate)
        return false;

    if (cydata.cCount++ <= config->cycleInterval)
    {
        qDebug() << "Tick";
        return false;
    }
    cydata.cCount = 0;

    if (++cydata.cIndex < cydata.files.size())
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

bool MosaicStepper::do_immediate()
{
    if (!isStarted())
        return false;

    if (isPaused())
        return false;

    if (!mImmediate)
        return false;

    if (++cydata.cIndex < cydata.files.size())
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
        panel->restorePageStatus();
        return true;
    }
    else
    {
        return false;
    }
}
