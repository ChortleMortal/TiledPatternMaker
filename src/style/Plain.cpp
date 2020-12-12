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

#include "style/plain.h"
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

Plain::Plain(PrototypePtr proto) : Colored(proto)
{
}

Plain::Plain(const Style & other ) : Colored(other)
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
    eraseStyleMap();
}

void Plain::createStyleRepresentation()
{
    getMap();
}

void Plain::draw(GeoGraphics *gg)
{
    qDebug() << "Plain::draw";

    if (!isVisible())
    {
        return;
    }

    MapPtr map = getMap();
    if (!map)
    {
        return;
    }

    QPen pen(colors.getNextColor().color);

    for (const auto &edge : map->getEdges())
    {
        gg->drawEdge(edge,pen);
    }
}

