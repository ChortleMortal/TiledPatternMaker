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
#include "geometry/vertex.h"
#include "geometry/map.h"
#include "geometry/loose.h"
#include "style/interlace.h"
#include "base/utilities.h"

int Edge::refs = 0;

Edge::Edge()
{
    type          = EDGETYPE_NULL;
    isSwapped     = false;
    visited       = false;
    dvisited      = false;
    start_under   = false;
    refs++;
}

Edge::Edge(VertexPtr v1)
{
    type          = EDGETYPE_POINT;
    isSwapped     = false;
    visited       = false;
    dvisited      = false;
    start_under   = false;
    this->v1      = v1;
    this->v2      = v1; // same
    refs++;
}

Edge::Edge(VertexPtr v1, VertexPtr v2 )
{
    type          = EDGETYPE_LINE;
    isSwapped     = false;
    visited       = false;
    dvisited      = false;
    start_under   = false;
    this->v1      = v1;
    this->v2      = v2;
    refs++;
}

Edge::Edge(VertexPtr v1, VertexPtr v2, QPointF arcCenter, bool convex)
{
    type          = EDGETYPE_CURVE;
    isSwapped     = false;
    visited       = false;
    dvisited      = false;
    start_under   = false;
    this->v1      = v1;
    this->v2      = v2;
    setArcCenter(arcCenter,convex);
    refs++;
}

Edge::Edge(EdgePtr other, QTransform T)
{
    type = other->type;
    v1 = std::make_shared<Vertex>(T.map(other->v1->pt));
    v2 = std::make_shared<Vertex>(T.map(other->v2->pt));
    if (type == EDGETYPE_CURVE)
    {
        arcCenter    = T.map(other->arcCenter);
        convex       = other->convex;
        arcMagnitude = other->arcMagnitude;
    }

    isSwapped      = false;
    visited        = false;
    dvisited       = false;
    start_under    = false;
    refs++;
}

Edge::~Edge()
{
    refs--;
    //qDebug() << "Edge destructor";
    //v1.reset();
    //v2.reset();
}

double Edge::angle() const
{
    return std::atan2(v2->pt.y() - v1->pt.y(), v2->pt.x() - v1->pt.x());
}

bool Edge::sameAs(EdgePtr other)
{
    if (v1 == other->v1 &&  v2 == other->v2)
    {
        return true;
    }
    if (v1 == other->v2 && v2 == other->v1)
    {
        return true;
    }
    return false;
}

bool Edge::sameAs(VertexPtr ov1, VertexPtr ov2)
{
    if (v1 == ov1 &&  v2 == ov2)
    {
        return true;
    }
    if (v1 == ov2 && v2 == ov1)
    {
        return true;
    }
    return false;
}

bool Edge::equals(EdgePtr other)
{
    if (!Loose::equalsPt(v1->pt,other->v1->pt))
        return false;

    if (!Loose::equalsPt(v2->pt,other->v2->pt))
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
        return calcArcData(v1->pt,v2->pt,arcCenter,convex);
    }
    else
    {
        arcData adc = calcArcData(v1->pt,v2->pt,arcCenter,convex);

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

QLineF Edge::getLine()
{
    return QLineF(v1->pt, v2->pt);
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
    if (pos == v1->pt)
        return v2;
    else
    {
        Q_ASSERT (pos == v2->pt);
        return v1;
    }
}

QPointF Edge::getOtherP(VertexPtr vert) const
{
    if (vert == v1)
        return v2->pt;
    else
    {
        Q_ASSERT (vert == v2);
        return v1->pt;
    }
}

QPointF Edge::getOtherP(QPointF pos) const
{
    if (pos == v1->pt)
        return v2->pt;
    else
    {
        Q_ASSERT (pos == v2->pt);
        return v1->pt;
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
    return qMin(v1->pt.x(), v2->pt.x());
}

bool Edge::isColinearAndTouching(EdgePtr e)
{
    static bool debug = false;
    if (debug) qDebug().noquote() << "testing"  << dump() << "and" << e->dump();

    if ((e->contains(v1)) || (e->contains(v2)))
    {
        return isColinear(e);
    }

    if (debug) qDebug() << "    not touching";

    return false;
}

bool Edge::isColinear(EdgePtr e)
{
    static bool debug = false;

        qreal angle = Utils::angle(getLine(),e->getLine());
        if (debug) qDebug() << "    angle=" << angle;
        if ((qAbs(angle) < 1e-5) || (qAbs(angle-180.0) < 1e-5))
        {
            if (debug) qDebug() << "    colinear";
            return true;
        }

    return false;
}

qreal Edge::getAngle()
{
    return qAtan2(v1->pt.x() - v2->pt.x(),v1->pt.y()-v2->pt.y());
}

EdgePtr Edge::getSwappedEdge()
{
    // has same edge index as original
    EdgePtr ep = std::make_shared<Edge>();
    ep->setV1(v2);
    ep->setV2(v1);
    ep->type          = type;
    ep->isSwapped     = true;
    ep->convex        = convex;
    ep->arcCenter     = arcCenter;
    ep->arcMagnitude  = arcMagnitude;
    ep->visited       = visited;
    ep->start_under   = start_under;

    return ep;
}

QString Edge::dump()
{
    QString astring;
    QDebug  deb(&astring);

    switch (type)
    {
    case EDGETYPE_LINE:
        deb << "Edge LINE "
            << "v1" << v1->pt
            << "v2" << v2->pt
            << ((isSwapped) ? "swapped" : "");
        break;
    case EDGETYPE_CURVE:
        deb << "Edge CURVE"
            << "\tv1" << v1->pt
            << "\tv2" << v2->pt
            << "\tarcCenter:" << arcCenter  << ((convex) ? "CONVEX" : "CONCAVE")
            << ((isSwapped) ? "swapped" : "");
        break;
    case EDGETYPE_POINT:
        deb << "Edge POINT"
            << "v1" << v1->pt;
        break;
    case EDGETYPE_NULL:
        deb << "Edge NULL";
    }
    return astring;
}

// The less-than operator compares the first point, then the order.
// Thus, two overlapped edges with different second point will not be less than each other,
// yet they won't be equal. This cannot happen in a map anyway since edges never overlap.
bool Edge::operator < (const Edge & other) const
{
    QPointF point  = v1->pt;
    QPointF opoint = other.v1->pt;
    return (Point::lessThan(point, opoint) || (point == opoint  &&  angle() < other.angle() - Point::TOLERANCE));
}
