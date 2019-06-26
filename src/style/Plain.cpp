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

#include "style/Plain.h"
#include <QPainter>

////////////////////////////////////////////////////////////////////////////
//
// Plain.java
//
// The trivial rendering style.  Render the map as a collection of
// line segments.  Not very useful considering that DesignPreview does
// this better.  But there needs to be a default style for the RenderView.
// Who knows -- maybe some diagnostic information could be added later.


// Creation.

Plain::Plain(PrototypePtr proto, PolyPtr  bounds ) : Colored(proto,bounds)
{
}

Plain::Plain(const Style *other ) : Colored(other)
{
}

Plain::~Plain()
{
#ifdef EXPLICIT_DESTRUCTOR
    qDebug() << "deleting plain";
    pts2.clear();
#endif
}

// Data.

void Plain::resetStyleRepresentation()
{
    pts2.erase(pts2.begin(),pts2.end());
}

void Plain::createStyleRepresentation()
{
    if (pts2.size())
    {
        return;
    }

    setupStyleMap();

    for (auto e = getReadOnlyMap()->getEdges()->begin(); e != getReadOnlyMap()->getEdges()->end(); e++)
    {
        EdgePtr edge = *e;
        QPointF v1 = edge->getV1()->getPosition();
        QPointF v2 = edge->getV2()->getPosition();

        pts2 << v1;
        pts2 << v2;
    }
}

void Plain::draw(GeoGraphics *gg)
{
    qDebug() << "Plain::draw";

    if (!isVisible())
    {
        return;
    }

    if( pts2.size() != 0 )
    {
        gg->pushAndCompose(*getLayerTransform());
        gg->setColor(colors.getNextColor().color);
        for( int idx = 0; idx < pts2.size(); idx += 2 )
        {
            gg->drawLine( pts2[idx], pts2[idx+1]);
        }
        gg->pop();
    }
}

