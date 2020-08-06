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

#include "style/thick.h"
#include "style/colored.h"
#include "style/interlace.h"
#include "style/outline.h"
#include "style/plain.h"
#include "style/emboss.h"
#include "base/canvas.h"
#include <QPainter>

////////////////////////////////////////////////////////////////////////////
//
// Thick.java
//
// A style that has a thickness and can have its outline drawn.
//
////////////////////////////////////////////////////////////////////////////
//
// Creation.

Thick::Thick(PrototypePtr proto, PolyPtr bounds ):
    Colored(proto,bounds)
{
    width = 0.05;
    draw_outline = false;           // DAC added
}

Thick::Thick(const Style &other ) : Colored(other)
{
    try
    {
        const Thick & thick  = dynamic_cast<const Thick &>(other);
        width        = thick.width;
        draw_outline = thick.draw_outline;
    }
    catch(std::bad_cast exp)
    {
        width = 0.05;
        draw_outline = false;           // DAC added
    }
}

Thick::~Thick()
{
#ifdef EXPLICIT_DESTRUCTOR
    qDebug() << "deleting thick";
    dpts2.clear();
#endif
}

void Thick::setLineWidth(qreal width )
{
    this->width = width;
    resetStyleRepresentation();
}

void Thick::resetStyleRepresentation()
{
    resetStyleMap();
}

void Thick::createStyleRepresentation()
{
    getMap();
}


void Thick::draw(GeoGraphics * gg )
{
    qDebug() << "Thick::draw";

    if (!isVisible())
    {
        return;
    }

    MapPtr map = getMap();
    if (!map)
    {
        qDebug() << "Thick::draw EMPTY Map";
        return;
    }

    // Note: we multiply the width by two because all other styles using
    //       the width actully widen the drawing in both perpendicular
    //       directions by that width.

    if ( draw_outline )
    {
        QPen pen(Qt::black);
        for (auto edge : map->getEdges())
        {
            gg->drawThickEdge(edge,width * 2 + 0.05, pen);
        }
    }

    QPen pen(colors.getNextColor().color);
    for (auto edge : map->getEdges())
    {
        gg->drawThickEdge(edge, width * 2, pen);
    }
}

