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
#include "geometry/edge.h"
#include "geometry/map.h"
#include "geometry/loose.h"
#include "style/interlace.h"

int Edge::refs = 0;

Edge::Edge()
{
    type          = EDGETYPE_NULL;
    isSwapped     = false;
    tmpEdgeIndex  = -1;
    refs++;
}

Edge::Edge(VertexPtr v1)
{
    type          = EDGETYPE_POINT;
    isSwapped     = false;
    this->v1      = v1;
    this->v2      = v1; // same
    tmpEdgeIndex  = -1;
    refs++;
}

Edge::Edge(VertexPtr v1, VertexPtr v2 )
{
    type          = EDGETYPE_LINE;
    isSwapped     = false;
    this->v1      = v1;
    this->v2      = v2;
    tmpEdgeIndex  = -1;
    refs++;
}

Edge::Edge(VertexPtr v1, VertexPtr v2, QPointF arcCenter, bool convex)
{
    type             = EDGETYPE_CURVE;
    isSwapped        = false;
    this->v1         = v1;
    this->v2         = v2;
    setArcCenter(arcCenter,convex);
    tmpEdgeIndex     = -1;
    refs++;
}

Edge::Edge(EdgePtr other, QTransform T)
{
    type = other->type;
    v1 = make_shared<Vertex>(T.map(other->getV1()->getPosition()));
    v2 = make_shared<Vertex>(T.map(other->getV2()->getPosition()));
    if (type == EDGETYPE_CURVE)
    {
        arcCenter    = T.map(other->arcCenter);
        convex       = other->convex;
        arcMagnitude = other->arcMagnitude;
    }

    isSwapped      = false;
    tmpEdgeIndex   = -1;
    refs++;
}

Edge::~Edge()
{
    refs--;
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

    if (type == EDGETYPE_CURVE)
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
    type         = EDGETYPE_CURVE;

    calcMagnitude();
}

void Edge::calcArcCenter(bool convex)
{
    // calculates a default line center
    this->convex = convex;
    type         = EDGETYPE_CURVE;

    QLineF line = getLine();
    QPointF mid = getMidPoint();
    // half-line
    // normal vector a point
    QPointF pt;
    if (convex)
    {
        pt = mid + QPointF(-line.dy(), line.dx());
    }
    else
    {
        pt = mid - QPointF(line.dy(), -line.dx());
    }
    arcCenter = pt;

    calcMagnitude();
}

void Edge::calcMagnitude()
{
    // calcs magnitude from arcCenter
    QLineF line  = getLine();
    QPointF mid  = getMidPoint();
    qreal dist   = Point::dist(mid,arcCenter);
    arcMagnitude = (dist/line.length());
    //qDebug() << "magnitude=" << arcMagnitude;
}

void Edge::setArcMagnitude(qreal magnitude)
{
    // calcs arcCenter from magntiude
    Q_ASSERT(type == EDGETYPE_CURVE);   // this means arcCenter is already set
    QLineF line = getLine();
    QPointF mid = getMidPoint();
    QPointF pt;
    if (convex)
    {
        pt = mid + QPointF(-line.dy(), line.dx());
    }
    else
    {
        pt = mid - QPointF(line.dy(), -line.dx());
    }
    QLineF perp(mid,pt);

    qreal  arcCenterLen = magnitude * line.length();
    perp.setLength(arcCenterLen);

    arcCenter = perp.p2();
}

arcData Edge::calcArcData()
{
    if (!isSwapped)
    {
        return calcArcData(v1->getPosition(),v2->getPosition(),arcCenter,convex);
    }
    else
    {
        arcData adc = calcArcData(v1->getPosition(),v2->getPosition(),arcCenter,convex);

        //qDebug() << "swapped start" << adc.start << "end" <<adc.end << "span" << adc.span;
        adc.span = -adc.span;
        if (convex)
        {
            if (adc.span > 180.0)
            {
                adc.span  = 360.0 - adc.span;
            }
        }
        return adc;
    }
}

arcData Edge::calcArcData(QPointF p1, QPointF p2, QPointF c, bool convex)
{
    arcData ad;
    if (convex)
    {
        //qDebug()  << "convex " << p1 << p2 << c;

        qreal cx = c.x();
        qreal cy = c.y();

        qreal   radius = QLineF(p1,c).length();
        QRectF  rect;
        qreal   start,end,span;

        rect = QRectF(cx - radius, cy - radius, radius*2, radius*2);

        start = QLineF(c,p1).angle();
        end   = QLineF(c,p2).angle();
        span  = end-start;

        if (span > 180.0)
        {
            span  = 360.0 - span;
        }
        if (span  > 0)
        {
            span = -span;
        }

        ad.start = start;
        ad.end   = end;
        ad.span  = span;
        ad.rect  = rect;
    }
    else
    {
        //qDebug() << "concave" << p1 << p2 << c;

        qreal   start,end,span;
        QRectF  rect;

        QPointF amid = (p1 + p2)/2;
        QLineF  mid  = QLineF(c,amid);
        int len = mid.length();
        mid.setLength(len*2.0);
        c = mid.p2();

        qreal   radius = QLineF(p1,c).length();
        rect = QRectF(QPointF(), radius * 2 * QSizeF(1, 1));
        rect.moveCenter(QPointF(c.x(), c.y()));

        start = QLineF(c,p1).angle();
        end   = QLineF(c,p2).angle();
        span  = end-start;

        if (span < 0)
        {
            span = -span;
        }
        if (span > 180.0)
        {
            span  = 360.0 - span;
        }

        ad.start = start;
        ad.end   = end;
        ad.span  = span;
        ad.rect  = rect;
    }

    return ad;
}

void Edge::resetCurve()
{
    if (type == EDGETYPE_CURVE)
    {
        type = EDGETYPE_LINE;
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
    if (vert == v1)
        return v2;
    else
    {
        Q_ASSERT (vert == v2);
        return v1;
    }
}

VertexPtr Edge::getOtherV(QPointF pos) const
{
    if (pos == v1->getPosition())
        return v2;
    else
    {
        Q_ASSERT (pos == v2->getPosition());
        return v1;
    }
}

QPointF Edge::getOtherP(VertexPtr vert) const
{
    if (vert == v1)
        return v2->getPosition();
    else
    {
        Q_ASSERT (vert == v2);
        return v1->getPosition();
    }
}

QPointF Edge::getOtherP(QPointF pos) const
{
    if (pos == v1->getPosition())
        return v2->getPosition();
    else
    {
        Q_ASSERT (pos == v2->getPosition());
        return v1->getPosition();
    }
}

bool Edge::contains(VertexPtr v)
{
    if ((v == v1) || (v == v2))
        return true;
    return false;
}

// Used to sort the edges in the map.
qreal Edge:: getMinX()
{
    return qMin(v1->getPosition().x(), v2->getPosition().x());
}

bool Edge::isColinearAndTouching(EdgePtr e)
{
    static bool debug = false;
    if (debug) qDebug() << "testing"  << getTmpEdgeIndex() << "and" << e->getTmpEdgeIndex();

    if ((e->contains(v1)) || (e->contains(v2)))
    {
        qreal angle = getLine().angle(e->getLine());
        if (debug) qDebug() << "    angle=" << angle;
        if ((qAbs(angle) < 1e-5) || (qAbs(angle-180.0) < 1e-5))
        {
            if (debug) qDebug() << "    colinear";
            return true;
        }
    }
    else if (debug) qDebug() << "    not touching";

    return false;
}

qreal Edge::getAngle()
{
    return qAtan2(v1->getPosition().x() - v2->getPosition().x(),v1->getPosition().y()-v2->getPosition().y());
}

EdgePtr Edge::getSwappedEdge()
{
    // has same edge index as original
    EdgePtr ep = make_shared<Edge>(*this);
    ep->setV1(v2);
    ep->setV2(v1);
    ep->isSwapped = true;
    return ep;
}

void Edge::dump()
{
    switch (type)
    {
    case EDGETYPE_LINE:
        qDebug().noquote() << "Edge LINE " << tmpEdgeIndex
                           << "v1" << v1->getTmpVertexIndex() << v1->getPosition()
                           << "v2" << v2->getTmpVertexIndex() << v2->getPosition()
                           << ((isSwapped) ? "swapped" : "");
        break;
    case EDGETYPE_CURVE:
        qDebug().noquote() << "Edge CURVE" << tmpEdgeIndex
                           << "\tv1" << v1->getTmpVertexIndex() << v1->getPosition()
                           << "\tv2" << v2->getTmpVertexIndex() << v2->getPosition()
                           << "\tarcCenter:" << arcCenter  << ((convex) ? "CONVEX" : "CONCAVE")
                           << ((isSwapped) ? "swapped" : "");
        break;
    case EDGETYPE_POINT:
        qDebug().noquote() << "Edge POINT" << tmpEdgeIndex
                           << "v1" << v1->getTmpVertexIndex() << v1->getPosition();
        break;
    case EDGETYPE_NULL:
        qDebug().noquote() << "Edge NULL" << tmpEdgeIndex;
    }
}
