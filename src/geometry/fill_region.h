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

#ifndef FILLREGION_H
#define FILLREGION_H

#include <QTransform>
#include "misc/unique_qvector.h"
#include "settings/filldata.h"

class FillData;

typedef std::shared_ptr<class Tiling> TilingPtr;

class FillRegion
{
public:
    FillRegion(TilingPtr tiling, const FillData & fd);

    QVector<QTransform> getTransforms();

protected:
    QTransform calcTransform(int h, int v);

private:
    class Configuration * config;

    TilingPtr             tiling;
    FillData              fillData;

    UniqueQVector<QTransform>   transforms;
};

#endif
