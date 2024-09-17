#include "sys/engine/image_engine.h"
#include "sys/engine/worklist_stepper.h"

////////////////////////////////////////////
///
///  WorklistBMPStepper
///
////////////////////////////////////////////

WorklistBMPStepper::WorklistBMPStepper(ImageEngine * parent) : SteppingEngine(parent) {}

bool WorklistBMPStepper::begin()
{
    if (isStarted())
    {
        end();
        return false;
    }

    start(true);
    setPaused (false);

    imgList_it = imgList.begin();

    VersionedName name = *imgList_it;
    parent->compareBMPs(name,name);

    return true;
}

bool WorklistBMPStepper::tick()
{
    return false;
}

bool WorklistBMPStepper::next()
{
    if (!isStarted()) return false;

    if (imgList_it == imgList.end())
    {
        end();
        return false;
    }

    imgList_it++;

    if (imgList_it == imgList.end())
    {
        end();
        return false;
    }

    VersionedName name = *imgList_it;
    parent->compareBMPs(name,name);
    return true;
}

bool WorklistBMPStepper::prev()
{
    if (!isStarted()) return false;

    if (imgList_it == imgList.begin())
    {
        end();
        return false;
    }

    imgList_it--;

    VersionedName name = *imgList_it;
    parent->compareBMPs(name,name);
    return true;
}

bool WorklistBMPStepper:: end()
{
    setPaused (false);

    if (isStarted())
    {
        start(false);
        finish("Worklist BMP comparisons");
        return true;
    }
    else
    {
        return false;
    }
}

void WorklistBMPStepper::resync(VersionedName name)
{
    if (!isStarted())
        return;

    imgList_it = imgList.begin();

    while (imgList_it != imgList.end())
    {
        VersionedName vn = *imgList_it;
        if (vn.get() == name.get())
        {
            return;  // we are resynced
        }
        imgList_it++;
    }

    // could not resyncB
    end();
}

void WorklistBMPStepper::setWorklist(VersionList &vlist)
{
    if (isStarted())
    {
        VersionedName current = *imgList_it;
        imgList    = vlist;
        // the worklist has been changed externally - resync to to current (or end)
        resync(current);
    }
    else
    {
        imgList    = vlist;
        imgList_it = imgList.begin();
    }
}
