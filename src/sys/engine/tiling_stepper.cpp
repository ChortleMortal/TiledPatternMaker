#include "sys/engine/tiling_stepper.h"
#include "model/makers/tiling_maker.h"
#include "sys/sys/fileservices.h"
#include "sys/sys.h"
#include "gui/top/controlpanel.h"
#include "model/settings/configuration.h"

////////////////////////////////////////////
///
///  TilingStepper runs in GUI thread
///
////////////////////////////////////////////

TilingStepper::TilingStepper(ImageEngine * parent) : SteppingEngine(parent)
{
}

bool TilingStepper::begin()
{
    if (isStarted())
    {
        end();
        return false;
    }

    qDebug() << "Starting tiling view cycle";

    cydata.files = FileServices::getTilingFiles(config->imageFileFilter);

    cydata.cIndex = -1;
    cydata.cCount = config->cycleInterval;     // start now

    setPaused(false);
    start(true);

    return next();
}

bool TilingStepper::tick()
{
    return next();
}

bool TilingStepper::next()
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
        VersionedFile file = cydata.files.at(cydata.cIndex);
        Sys::tilingMaker->loadTiling(file,TILM_LOAD_FROM_STEPPER);
    }
    else
    {
        end();
    }

    return true;
}

bool TilingStepper::prev()
{
    return false;
}

bool TilingStepper::end()
{
    setPaused(false);

    if (isStarted())
    {
        start(false);
        finish("Tiling viewing");
        panel->restorePageStatus();
        return true;
    }
    else
    {
        return false;
    }
}
