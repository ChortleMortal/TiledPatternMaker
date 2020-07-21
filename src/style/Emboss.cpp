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

#include "style/emboss.h"
#include "geometry/point.h"
#include <QPainter>

////////////////////////////////////////////////////////////////////////////
//
// Emboss.java
//
// A rendering style for maps that pretends that the map is carves out
// of wood with a diamond cross section and shines a directional light
// on the wood from some angle.  The map is drawn as a collection of
// trapezoids, and the darkness of each trapezoid is controlled by the
// angle the trapezoid makes with the light source.  The result is a
// simple but highly effective 3D effect, similar to flat-shaded 3D
// rendering.
//
// In practice, we can make this a subclass of RenderOutline -- it uses the
// same pre-computed point array, and just add one parameter and overloads
// the draw function.


// Creation.

Emboss::Emboss(PrototypePtr proto, PolyPtr bounds) : Outline(proto,bounds)
{
    setAngle(M_PI * 0.25 );
}

Emboss::Emboss(const Style & other) : Outline(other)
{
    try
    {
        const Emboss & emb = dynamic_cast<const Emboss&>(other);
        angle   = emb.angle;
        light_x = emb.light_x;
        light_y = emb.light_y;
        greys   = emb.greys;
    }
    catch(std::bad_cast exp)
    {
        setAngle(M_PI * 0.25 );
        setGreys();
    }
}

Emboss::~Emboss()
{
#ifdef EXPLICIT_DESTRUCTOR
    qDebug() << "deleting emboss";
#endif
}

// Style overrides.

void Emboss::draw(GeoGraphics * gg)
{
    if (!isVisible())
    {
        return;
    }

    if (pts4.size() != 0)
    {
        for( int idx = 0; idx < pts4.size(); idx++)
        {
            BelowAndAboveEdge bae = pts4[idx];
            QPolygonF poly        = bae.getPoly();
            drawTrap(gg, bae.v2.v, bae.v2.above, bae.v1.below, bae.v1.v);
            drawTrap(gg, bae.v1.v, bae.v1.above, bae.v2.below, bae.v2.v);

            if ( draw_outline )
            {
                gg->drawPolygon(poly,Qt::black,1);
                gg->drawLine(bae.v2.v, bae.v1.v,QPen(Qt::black));
            }
        }
    }
}

void Emboss::drawTrap(GeoGraphics * gg, QPointF a, QPointF b, QPointF c, QPointF d )
{
    QPointF N = a - d;
    Point::perpD(N);
    Point::normalizeD(N);

    // dd is a normalized floating point value corresponding to
    // the brightness to use.
    qreal dd = 0.5 * ( N.x() * light_x + N.y() * light_y + 1.0 );

    // Quantize to sixteen grey values.
    int bb = static_cast<int>(16.0 * dd);
    QColor color = greys[bb];

    QPolygonF trap_pts;
    trap_pts << a << b << c << d;
    gg->fillPolygon(trap_pts,color);
}

// Data.

qreal Emboss::getAngle()
{
    return angle;
}

void Emboss::setAngle(qreal angle )
{
    this->angle = angle;
    light_x = qCos( angle );
    light_y = qSin( angle );
    //redraw();
}

void Emboss::setColorSet(ColorSet & cset)
{
    Outline::setColorSet(cset);
    setGreys();
}

void Emboss::setGreys()
{
    qreal h,s,b;
    getColorSet().getFirstColor().color.getHsvF(&h,&s,&b);

    greys.erase(greys.begin(),greys.end());

    for( int idx = 0; idx < 17; ++idx )
    {
        float t  = (float)idx / 16.0f;
        float s1 = (1.0f-t)*0.7f + t*0.99f;
        float b1 = (1.0f-t)*0.4f + t*0.99f;
        QColor c;
        c.setHsvF(h, s * s1, b * b1 );
        greys << c;
    }
}


