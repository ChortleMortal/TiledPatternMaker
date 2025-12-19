#include "sys/engine/image_engine.h"
#include "sys/engine/view_bmp_stepper.h"

////////////////////////////////////////////
///
///  ViewBMPStepper
///
////////////////////////////////////////////

ViewBMPStepper::ViewBMPStepper(ImageEngine * parent) : SteppingEngine(parent)
{
}

bool ViewBMPStepper::begin()
{
    if (isStarted())
    {
        end();
        return false;
    }

    parent->setCompareMode(false);
    start(true);
    setPaused (false);

    imgList_it = imgList.begin();

    VersionedName name = *imgList_it;
    parent->showBMP(name);

    return true;
}

bool ViewBMPStepper::tick()
{
    return false;
}

bool ViewBMPStepper::next()
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
    parent->showBMP(name);
    return true;
}

bool ViewBMPStepper::prev()
{
    if (!isStarted()) return false;

    if (imgList_it == imgList.begin())
    {
        end();
        return false;
    }

    imgList_it--;

    VersionedName name = *imgList_it;
    parent->showBMP(name);
    return true;
}

bool ViewBMPStepper:: end()
{
    setPaused (false);

    if (isStarted())
    {
        start(false);
        finish("BMP viewing");
        return true;
    }
    else
    {
        return false;
    }
}

void ViewBMPStepper::resync(VersionedName name)
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

void ViewBMPStepper::setWorklist(VersionList &vlist)
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
