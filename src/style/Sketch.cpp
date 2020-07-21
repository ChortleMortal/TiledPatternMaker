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

#include "style/sketch.h"
#include <QPainter>
#include <QtGlobal>

////////////////////////////////////////////////////////////////////////////
//
// Sketch.java
//
// One day, it occured to me that I might be able to get a sketchy
// hand-drawn effect by drawing an edge as a set of line segments whose
// endpoints are jittered relative to the original edge.  And it worked!
// Also, since the map is fixed, we can just reset the random seed every
// time we draw the map to get coherence.  Note that coherence might not
// be a good thing -- some animations work well precisely because the
// random lines that make up some object change from frame to frame (c.f.
// Bill Plympton).  It's just a design decision, and easy to reverse
// (or provide a UI for).
//
// I haven't tried it yet, but I doubt this looks any good as postscript.
// the resolution is too high and it would probably look like, well,
// a bunch of lines.



// Creation.

Sketch::Sketch(PrototypePtr proto, PolyPtr bounds ) : Plain(proto,bounds)
{
    qsrand(279401L);
}

Sketch::Sketch(const Style & other ) : Plain(other)
{
    qsrand(279401L);
}

Sketch::~Sketch()
{
#ifdef EXPLICIT_DESTRUCTOR
    qDebug() << "deleting sketch";
#endif
}

// Style overrrides.

void Sketch::draw(GeoGraphics * gg)
{
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

    qreal jitter = Transform::distFromInvertedZero(gg->getTransform(),5.0);
    qreal halfjit = jitter / 2.0;
    for (auto edge : map->getEdges())
    {
        QPointF a = edge->getV1()->getPosition() - QPointF(halfjit,halfjit);
        QPointF b = edge->getV2()->getPosition() - QPointF(halfjit,halfjit);

        for( int c = 0; c < 8; ++c )
        {
            volatile qreal r1 = (qrand()/RAND_MAX) * jitter;
            volatile qreal r2 = (qrand()/RAND_MAX) * jitter;
            volatile qreal r3 = (qrand()/RAND_MAX) * jitter;
            volatile qreal r4 = (qrand()/RAND_MAX) * jitter;
            VertexPtr v1 = make_shared<Vertex>(a + QPointF(r1,r2));
            VertexPtr v2 = make_shared<Vertex>(b + QPointF(r3,r4));
            EdgePtr edge2;
            if (edge->getType() == EDGETYPE_LINE)
            {
                edge2 = make_shared<Edge>(v1,v2);
            }
            else if (edge->getType() == EDGETYPE_CURVE)
            {
                edge2 = make_shared<Edge>(v1,v2, edge->getArcCenter(), edge->isConvex());
            }
            gg->drawEdge(edge2,pen);
        }
    }
}


