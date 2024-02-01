#include "engine/tiling_stepper.h"
#include "misc/fileservices.h"
#include "misc/sys.h"
#include "panels/controlpanel.h"
#include "settings/configuration.h"
#include "tile/tiling_manager.h"
#include "viewers/view_controller.h"

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

    cydata.files = FileServices::getTilingNames(config->viewFileFilter);

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
        QString name = cydata.files.at(cydata.cIndex);
        loadTiling(name);
    }
    else
    {
        end();
    }

    return true;
}

void TilingStepper::loadTiling(QString name)
{
    LoadUnit & loadUnit = Sys::view->getLoadUnit();
    loadUnit.setLoadState(LOADING_TILING,name);

    TilingManager tm;
    if (tm.loadTiling(name,TILM_LOAD_SINGLE))
    {
        Sys::viewController->removeAllImages();

        Sys::viewController->slot_reconstructView();
    }
    loadUnit.resetLoadState();
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
        panel->popPanelStatus();
        return true;
    }
    else
    {
        return false;
    }
}
