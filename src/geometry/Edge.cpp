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
    tmpIndex      = -1;
    type          = EDGE_NULL;
}

Edge::Edge(VertexPtr v1)
{
    refs++;
    this->v1      = v1;
    this->v2      = v1; // same
    interlaceData = nullptr;
    tmpIndex      = -1;
    type          = EDGE_POINT;
}

Edge::Edge(VertexPtr v1, VertexPtr v2 )
{
    refs++;
    type          = EDGE_LINE;
    this->v1      = v1;
    this->v2      = v2;

    interlaceData = nullptr;
    tmpIndex      = -1;
}

Edge::Edge(VertexPtr v1, VertexPtr v2, QPointF arcCenter, bool convex)
{
    refs++;
    type             = EDGE_CURVE;
    this->v1         = v1;
    this->v2         = v2;
    setArcCenter(arcCenter,convex);

    interlaceData    = nullptr;
    tmpIndex         = -1;
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

bool Edge::equals(EdgePtr other)
{
    if (v1->getPosition() != other->v1->getPosition())
        return false;

    if (v2->getPosition() != other->v2->getPosition())
        return false;

    if (type != other->type)
        return false;

    if (type == EDGE_CURVE)
    {
        if (arcCenter != other->arcCenter)
            return false;

        if (convex != other->convex)
            return false;
    }

    return true;
}

void Edge::setArcCenter(QPointF ac, bool convex)
{
    arcCenter    = ac;
    this->convex = convex;
    type         = EDGE_CURVE;
}

arcData Edge::calcArcData(QPointF V1, QPointF V2, QPointF ArcCenter, bool Convex)
{
    arcData ad;

    qreal x0 = ArcCenter.x();
    qreal y0 = ArcCenter.y();
    qreal x1 = V1.x();
    qreal y1 = V1.y();
    qreal x2 = V2.x();
    qreal y2 = V2.y();

    qreal radius = QLineF(V1,ArcCenter).length();
    ad.rect      = QRectF(x0 - radius, y0 - radius, radius*2, radius*2);

    ad.start     = -(180/M_PI) * qAtan2(y1-y0, x1-x0);
    qreal arcEnd = -(180/M_PI) * qAtan2(y2-y0, x2-x0);
    ad.span      = qAbs(ad.start - arcEnd);
    if (ad.span > 180.0)
        ad.span  = 360.0 - ad.span;

    if (Convex)
    {
        ad.start -= ad.span;
    }

    //qDebug() << "start angle" << ad.start << "end angle" << arcEnd;

    return ad;
}


void Edge::resetCurve()
{
    if (type == EDGE_CURVE)
    {
        type = EDGE_LINE;
        arcCenter = QPointF();
    }
}

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


// Helpers.

VertexPtr Edge::getOtherV(VertexPtr vert) const
{
    if (vert  == v1)
        return v2;
    else
        return v1;
}

VertexPtr Edge::getOtherV(QPointF pos) const
{
    if (pos  == v1->getPosition())
        return v2;
    else
        return v1;
}

QPointF Edge::getOtherP(VertexPtr vert) const
{
    if (vert  == v1)
        return v2->getPosition();
    else
        return v1->getPosition();
}

QPointF Edge::getOtherP(QPointF pos) const
{
    if (pos  == v1->getPosition())
        return v2->getPosition();
    else
        return v1->getPosition();
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
    {
        delete interlaceData;
    }
    interlaceData = info;
}

QPointF Edge::calcDefaultArcCenter(bool convex)
{
    // calculates a default line center
    QLineF edge = getLine();
    QPointF mid = getMidPoint();
    // half-line
    // normal vector a point
    QPointF pt;
    if (convex)
    {
        pt = mid + QPointF(-edge.dy(), edge.dx());
    }
    else
    {
        pt = mid - QPointF(-edge.dy(), edge.dx());
    }
    return pt;
}


