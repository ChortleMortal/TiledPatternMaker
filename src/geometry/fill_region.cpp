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

#include "geometry/fill_region.h"
#include "tile/tiling.h"


FillRegion::FillRegion(TilingPtr tiling, const FillData &fd)
{
    this->tiling = tiling;
    fillData     = fd;
}

Placements FillRegion::getPlacements(eRepeatType mode)
{
    int minX,minY,maxX,maxY;
    bool singleton;
    fillData.get(singleton,minX,maxX,minY,maxY);

    if (singleton)
    {
        transforms.push_back(calcTransform(0,0));
        return transforms;
    }

    switch(mode)
    {
    case REPEAT_SINGLE:
        //qDebug() << "REPEAT_SINGLE";
        transforms.push_back(calcTransform(0,0));
        break;

    case REPEAT_PACK:
        //qDebug() << "REPEAT_PACK";
        for (int h = -1; h <= 1; h++)
        {
            for (int v = 0; v <= 1; v++)
            {
                 transforms.push_back(calcTransform(h,v));
            }
        }
        break;

    case REPEAT_DEFINED:
        //qDebug().noquote() << "REPEAT_DEFINED"  << minX << maxX << minY << maxY;
        for (int h = minX; h <= maxX; h++)
        {
            for (int v = minY; v <= maxY; v++)
            {
                transforms.push_back(calcTransform(h,v));
            }
        }
        break;
    }

    return transforms;
}

QTransform FillRegion::calcTransform(int h, int v)
{
    //qDebug() << "FillRegion::calcTransform:"  << h << v;
    QPointF pt   = (tiling->getData().getTrans1() * static_cast<qreal>(h)) + (tiling->getData().getTrans2() * static_cast<qreal>(v));
    QTransform T = QTransform::fromTranslate(pt.x(),pt.y());
    return T;
}
