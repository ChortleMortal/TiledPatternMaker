#include "engine/image_engine.h"
#include "engine/worklist_stepper.h"
#include "widgets/versioned_list_widget.h"

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

    QString name = *imgList_it;
    parent->compareBMPsByName(name,name,false);

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

    QString name = *imgList_it;
    parent->compareBMPsByName(name,name,false);
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

    QString name = *imgList_it;
    parent->compareBMPsByName(name,name,false);
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

void WorklistBMPStepper::resync(QString name)
{
    if (!isStarted())
        return;

    imgList_it = imgList.begin();

    while (imgList_it != imgList.end())
    {
        if (*imgList_it == name)
        {
            return;  // we are resynced
        }
        imgList_it++;
    }

    // could not resync
    end();
}

void WorklistBMPStepper::setWorklist(VersionList & vlist)
{
    if (isStarted())
    {
        QString current = *imgList_it;
        imgList    = vlist.recompose();
        // the worklist has been changed externally - resync to to current (or end)
        resync(current);
    }
    else
    {
        imgList    = vlist.recompose();
        imgList_it = imgList.begin();
    }
}
