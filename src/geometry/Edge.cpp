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
// Edge.java
//
#include "geometry/Edge.h"
#include "geometry/Map.h"
#include "style/Interlace.h"

int Edge::refs = 0;

Edge::Edge()
{
    refs++;
    interlaceData = nullptr;
    tmpIndex = -1;
}


Edge::Edge(VertexPtr v1, VertexPtr v2 )
{
    refs++;
    this->v1 = v1;
    this->v2 = v2;
    interlaceData     = nullptr;
    tmpIndex = -1;
}

Edge::~Edge()
{
    refs--;
    if (interlaceData)
        delete interlaceData;
    //qDebug() << "Edge destructor";
    //v1.reset();
    //v2.reset();
}

bool Edge::sameAs(EdgePtr other)
{
    if (v1 == other->getV1())
    {
        if (v2 == other->getV2())
        {
            return true;
        }
    }
    if (v2 == other->getV1())
    {
        if (v1 == other->getV2())
        {
            return true;
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////
//
// Data access.

VertexPtr Edge::getV1()
{
    return v1;
}

VertexPtr Edge::getV2()
{
    return v2;
}

QLineF Edge::getLine()
{
    return QLineF(v1->getPosition(), v2->getPosition());
}


////////////////////////////////////////////////////////////////////////////
//
// Helpers.

VertexPtr Edge::getOther(QPointF pos ) const
{
    if( pos  == v1->getPosition() )
    {
        return v2;
    }
    else
    {
        return v1;
    }
}

// Used to sort the edges in the map.
qreal Edge:: getMinX()
{
    return qMin( v1->getPosition().x(), v2->getPosition().x() );
}

interlaceInfo * Edge::getInterlaceInfo()
{
    return interlaceData;
}

void Edge::setInterlaceInfo(interlaceInfo * info )
{
    if (interlaceData)
        delete interlaceData;
    interlaceData = info;
}


