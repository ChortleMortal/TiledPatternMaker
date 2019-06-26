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

#include "style/Thick.h"
#include "style/Colored.h"
#include "style/Interlace.h"
#include "style/Outline.h"
#include "style/Plain.h"
#include "style/Emboss.h"
#include "base/canvas.h"
#include <QPainter>

////////////////////////////////////////////////////////////////////////////
//
// Thick.java
//
// A style that has a thickness and can have its outline drawn.

////////////////////////////////////////////////////////////////////////////
//
// Creation.

Thick::Thick(PrototypePtr proto, PolyPtr bounds ):
    Colored(proto,bounds)
{
    width = 0.05;
    draw_outline = false;           // DAC added
}

Thick::Thick(const Style *other ) : Colored(other)
{
    const Thick * thick  = dynamic_cast<const Thick*>(other);
    if (thick)
    {
        width        = thick->width;
        draw_outline = thick->draw_outline;
    }
    else
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
    dpts2.clear();
}

void Thick::createStyleRepresentation()
{
    if (dpts2.size() )
    {
        return;
    }

    setupStyleMap();

    qDebug().noquote() << getReadOnlyMap()->getInfo();

    resetStyleRepresentation();
    for (auto e = getReadOnlyMap()->getEdges()->begin(); e != getReadOnlyMap()->getEdges()->end(); e++)
    {
        EdgePtr edge = *e;
        QPointF v1 = edge->getV1()->getPosition();
        QPointF v2 = edge->getV2()->getPosition();

        dpts2 << v1;
        dpts2 << v2;
    }
    //qDebug() << "Thick::dpts2" << dpts2.count();
}


void Thick::draw(GeoGraphics * gg )
{
    qDebug() << "Thick::draw";

    if (!isVisible())
    {
        return;
    }

    // Note: we multiply the width by two because all other styles using
    //       the width actully widen the drawing in both perpendicular
    //       directions by that width.
    if( dpts2.size() != 0 )
    {
        gg->pushAndCompose(*getLayerTransform());

        if ( draw_outline )
        {
            gg->setColor(Qt::black);
            for( int idx = 0; idx < dpts2.size(); idx += 2 )
            {
                gg->drawThickLine(dpts2[idx], dpts2[idx+1],width * 2 + 0.05);
            }
        }

        gg->setColor(colors.getNextColor().color);
        for( int idx = 0; idx < dpts2.size(); idx += 2 )
        {
            gg->drawThickLine(dpts2[idx], dpts2[idx+1], width * 2);
        }

        gg->pop();
    }
}

