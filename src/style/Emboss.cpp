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

#include "style/Emboss.h"
#include "geometry/Point.h"
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

Emboss::Emboss(PrototypePtr proto, PolyPtr bounds ) : Outline(proto,bounds)
{
    setAngle(M_PI * 0.25 );
}

Emboss::Emboss(const Style *other ) : Outline(other)
{
    const Emboss * emb = dynamic_cast<const Emboss*>(other);
    if (emb)
    {
        angle   = emb->angle;
        light_x = emb->light_x;
        light_y = emb->light_y;
        greys   = emb->greys;
    }
    else
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

    if (pts3.size() != 0)
    {
        gg->pushAndCompose(*getLayerTransform());
        for( int idx = 0; idx < pts3.size(); idx++)
        {
            QPolygonF poly = pts3[idx];
            drawTrap(gg, poly[1], poly[2], poly[3], poly[4] );
            drawTrap(gg, poly[4], poly[5], poly[0], poly[1] );

            if ( draw_outline )
            {
                gg->setColor(Qt::black);
                gg->drawPolygon(poly, false );
                gg->drawLine(poly[1], poly[4] );
            }
        }
        gg->pop();
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
    int bb = (int)( 16.0 * dd );
    gg->setColor( greys[ bb ] );

    QPolygonF trap_pts;
    trap_pts << a << b << c << d;
    gg->drawPolygon( trap_pts,true);

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

void Emboss::setColor( QColor color )
{
    Outline::setColor( color );
    setGreys();
}

void Emboss::setGreys()
{
    qreal h,s,b;
    getColor().getHsvF(&h,&s,&b);

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

