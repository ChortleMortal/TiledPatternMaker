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
#include "settings/configuration.h"
#include "tile/tiling.h"
#include <QPolygonF>

FillRegion::FillRegion(TilingPtr tiling, FillData *fd)
{
    this->tiling = tiling;
    fillData     = fd;
    config       = Configuration::getInstance();
}

QVector<QTransform> FillRegion::getTransforms()
{
    int minX,minY,maxX,maxY;
    bool singleton;
    fillData->get(singleton,minX,maxX,minY,maxY);

    if (singleton)
    {
        transforms.push_back(receive(0,0));
        return transforms;
    }

    switch(config->repeatMode)
    {
    case REPEAT_SINGLE:
        //qDebug() << "REPEAT_SINGLE";
        transforms.push_back(receive(0,0));
        break;

    case REPEAT_PACK:
        //qDebug() << "REPEAT_PACK";
      //for (int h = -1; h <= 1; h++)
        for (int h = 0; h <= 1; h++)
        {
            for (int v = 0; v <= 1; v++)
            {
                 transforms.push_back(receive(h,v));
            }
        }
        break;

    case REPEAT_DEFINED:
        //qDebug().noquote() << "REPEAT_DEFINED"  << minX << maxX << minY << maxY;
        for (int h = minX; h <= maxX; h++)
        {
            for (int v = minY; v <= maxY; v++)
            {
                transforms.push_back(receive(h,v));
            }
        }
        break;
    }

    return transforms;
}




QTransform FillRegion::receive(int h, int v)
{
    //qDebug() << "Prototype::receive:"  << h << v;
    QPointF pt   = (tiling->getTrans1() * static_cast<qreal>(h)) + (tiling->getTrans2() * static_cast<qreal>(v));
    QTransform T = QTransform::fromTranslate(pt.x(),pt.y());
    return T;
}
