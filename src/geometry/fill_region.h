#pragma once
#ifndef FILLREGION_H
#define FILLREGION_H

////////////////////////////////////////////////////////////////////////////
//
// FillRegion.java
//
// When working with periodic geometry, it often becomes necessary to
// fill some region with copies of a motif.  This class encapsulates
// a system for filling quadrilateral regions with the integer linear
// combinations of two translation vectors where each translate places
// the origin inside the quad.  It's sort of a modified polygon
// filling algorithm, where everything is first transformed into the
// coordinate system of the translation vectors.  The the quad is
// filled in the usual way.
//
// The algorithm isn't perfect.  It can leave gaps around the edge of
// the region to fill.  This is usually worked around by the client --
// the region is simply expanded before filling.
//
// To make the algorithm general, the output is provided through a
// callback that gets a sequence of calls, one for each translate.

// casper 20FEB19 - removed the original fill algorithm

#include <QTransform>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <memory>
#endif
#include "misc/unique_qvector.h"
#include "settings/filldata.h"
#include "enums/efillmode.h"

typedef QVector<QTransform> Placements;

typedef std::shared_ptr<class Tiling> TilingPtr;

class FillRegion
{
public:
    FillRegion(Tiling * tiling, const FillData & fd);

    Placements getPlacements(eRepeatType mode);

protected:
    QTransform calcTransform(int h, int v);

private:
    Tiling *    tiling;
    FillData    fillData;

    UniqueQVector<QTransform>   transforms;
};

#endif
