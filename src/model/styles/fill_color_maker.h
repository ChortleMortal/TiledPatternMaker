#pragma once
#ifndef COLORMAKER_H
#define COLORMAKER_H

#include "sys/enums/efilltype.h"
#include "sys/geometry/dcel.h"
#include "sys/geometry/faces.h"

typedef std::shared_ptr<class DCEL> DCELPtr;

class Filled;
class GeoGraphics;

// The ColorMaker is used to create the data to be drawn

class ColorMaker
{
    friend class Filled;

public:
    ColorMaker(Filled * filled);
    ~ColorMaker();

    virtual QString sizeInfo()                           = 0;

protected:
    virtual void initFrom(eFillType algorithm)           = 0;
    virtual void resetStyleRepresentation()              = 0;
    virtual void createStyleRepresentation(DCELPtr dcel) = 0;
    virtual void draw(GeoGraphics * gg)                  = 0;

    Filled *    filled;
    bool        initialised;
};

#endif // COLORMAKER_H

