/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

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
#include "base/configuration.h"
#include "viewers/viewcontrol.h"
#include "tapp/prototype.h"
#include "viewers/tiling_view.h"
#include "viewers/prototype_view.h"
#include "tile/tiling.h"
#include <QPolygonF>

FillRegion::FillRegion()
{
    config   = Configuration::getInstance();
    vcontrol = ViewControl::getInstance();
}

void FillRegion::fill(GeoGraphics *gg)
{
    switch(config->repeatMode)
    {
    case REPEAT_SINGLE:
        qDebug() << "REPEAT_SINGLE";
        receive(gg,0,0);
        break;

    case REPEAT_PACK:
        qDebug() << "REPEAT_PACK";
      //for (int h = -1; h <= 1; h++)
        for (int h = 0; h <= 1; h++)
        {
            for (int v = 0; v <= 1; v++)
            {
                 receive(gg,h,v);
            }
        }
        break;

    case REPEAT_DEFINED:
    {
        FillData fd = vcontrol->getFillData();
        int minX,minY,maxX,maxY;
        fd.get(minX,maxX,minY,maxY);
        qDebug().noquote() << "REPEAT_DEFINED"  << minX << maxX << minY << maxY;
        for (int h = minX; h <= maxX; h++)
        {
            for (int v = minY; v <= maxY; v++)
            {
                receive(gg,h,v);
            }
        }
    }
    break;
    }
}
