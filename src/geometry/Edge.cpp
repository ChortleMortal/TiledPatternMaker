////////////////////////////////////////////////////////////////////////////
//
// Edge.java
//

#include <QTransform>
#include "geometry/arcdata.h"
#include "geometry/edge.h"
#include "geometry/vertex.h"
#include "misc/utilities.h"

using std::make_shared;

int Edge::refs = 0;

Edge::Edge()
{
    type          = EDGETYPE_NULL;
    isSwapped     = false;
    dvisited      = false;
    refs++;
}

Edge::Edge(const VertexPtr &v1)
{
    type          = EDGETYPE_POINT;
    isSwapped     = false;
    dvisited      = false;
    this->v1      = v1;
    this->v2      = v1; // same
    refs++;
}

Edge::Edge(const VertexPtr & v1, const VertexPtr & v2 )
{
    type          = EDGETYPE_LINE;
    isSwapped     = false;
    dvisited      = false;
    this->v1      = v1;
    this->v2      = v2;
    refs++;
}

Edge::Edge(const VertexPtr & v1, const VertexPtr & v2, const QPointF & arcCenter, bool convex, bool chord)
{
    if (chord)
        type      = EDGETYPE_CHORD;
    else
        type      = EDGETYPE_CURVE;
    isSwapped     = false;
    dvisited      = false;
    this->v1      = v1;
    this->v2      = v2;
    setArcCenter(arcCenter,convex,chord);   // this creates arcData too
    refs++;
}

Edge::Edge(const Edge & other)
{
    type          = other.type;
    isSwapped     = other.isSwapped;
    dvisited      = other.dvisited;
    v1            = other.v1;
    v2            = other.v2;
    if (type == EDGETYPE_CURVE || type == EDGETYPE_CHORD)
    {
        setArcCenter(other.arcCenter,other.convex,(type==EDGETYPE_CHORD));
    }
    refs++;
}

EdgePtr Edge::getCopy()
{
    EdgePtr ep = make_shared<Edge>();
    ep->v1     = v1;
    ep->v2     = v2;
    ep->type   = type;
    if (type == EDGETYPE_CURVE || type == EDGETYPE_CHORD)
    {
        ep->setArcCenter(arcCenter,convex,(type==EDGETYPE_CHORD));

    }
    return ep;
}

EdgePtr Edge::createTwin()
{
    // has same edge index as original
    EdgePtr ep        = make_shared<Edge>();
    ep->type          = type;
    ep->isSwapped     = true;
    ep->dvisited      = dvisited;
    ep->v1            = v2;
    ep->v2            = v1;
    if (type == EDGETYPE_CURVE || type == EDGETYPE_CHORD)
    {
        Q_ASSERT(arcData);
        ep->setArcCenter(arcData->getCenterOldConcave(v1->pt,v2->pt,arcCenter),!convex,(type==EDGETYPE_CHORD));
    }
    return ep;
}

Edge::Edge(const EdgePtr &other, QTransform T)
{
    type           = other->type;
    isSwapped      = false;
    dvisited       = false;
    v1 = make_shared<Vertex>(T.map(other->v1->pt));
    v2 = make_shared<Vertex>(T.map(other->v2->pt));
    if (type == EDGETYPE_CURVE || type == EDGETYPE_CHORD)
    {
        arcCenter    = T.map(other->arcCenter);
        convex       = other->convex;
        arcMagnitude = other->arcMagnitude;
    }
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

bool Edge::sameAs(const EdgePtr & other)
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

bool Edge::sameAs(const VertexPtr &ov1, const VertexPtr &ov2)
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

bool Edge::equals(const EdgePtr & other)
{
    if (!Loose::equalsPt(v1->pt,other->v1->pt))
        return false;

    if (!Loose::equalsPt(v2->pt,other->v2->pt))
        return false;

    if (type != other->type)
        return false;

    if (type == EDGETYPE_CURVE || type == EDGETYPE_CHORD)
    {
        if (arcCenter != other->arcCenter)
            return false;

        if (convex != other->convex)
            return false;
    }

    return true;
}

ArcDataPtr Edge::getArcData()
{
    Q_ASSERT(arcData);
    return arcData;
}

void Edge::setArcCenter(const QPointF & ac, bool convex, bool chord)
{
    arcCenter    = ac;
    this->convex = convex;
    if (chord)
        type     = EDGETYPE_CHORD;
    else
        type     = EDGETYPE_CURVE;

    calcMagnitude();
    arcData = make_shared<ArcData>(this);   // always overwrites
}

void Edge::calcArcCenter(bool convex, bool chord)
{
    // calculates a default line center

    Q_ASSERT(arcData);

    this->convex = convex;
    if (chord)
        type     = EDGETYPE_CHORD;
    else
        type     = EDGETYPE_CURVE;

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
        //QPointF pt2 = mid + QPointF(-line.dy(), line.dx());
        //QPointF pt3 = Point::reflectPoint(pt2,line);
        //Q_ASSERT(pt2 == pt3);
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
    Q_ASSERT(isCurve());   // this means arcCenter is already set
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

bool Edge::pointWithinSpan(const QPointF & pt, qreal originalSpan)
{
    static int debug = 0;

    Q_ASSERT(arcData);
    Q_ASSERT(isCurve());

    if (debug >=1) qDebug() << "v1:" << v1->pt << "v2:" << v2->pt << "pt:" << pt;


    if (Loose::equalsPt(pt,v1->pt) || Loose::equalsPt(pt,v2->pt))
        return false;   // not a true intersect

    ArcData ad(v1->pt,pt,arcCenter,convex);
    qreal aspan = qAbs(ad.span);
    qreal aorig = qAbs(originalSpan);

    if (aspan > aorig)
        return false;
    if (Loose::equals(aspan,aorig))
        return false;
    if (aspan < 1.0)
        return false;   // not a new intersection - points are at ends
    if (Loose::equals(aspan,180) || Loose::equals(aspan,360))
        return false;   // not what we want

    // spans have been 'corrected' for drawing purposes.  Here we need to be comparing the 'real' spans
    qreal arcspan  = arcData->end - arcData->start;
    qreal ad2span = ad.end - ad.start;

    // another case
    if (arcspan > 0 && ad2span < 0)
        return false;
    if (ad2span > 0 && arcspan < 0)
        return false;

    if (debug >=2) qDebug() << (arcData->convex ? "convex" : "concave") << arcData->start << arcData->end << arcspan << ad.start  << ad.end << ad2span;
    if (arcData->convex)
    {
        Q_ASSERT(arcData->span < 0);
        Q_ASSERT(ad.span < 0);
        if ((ad.end < arcData->start) &&  ad.end > arcData->end)
        {
            if (debug >=2) qDebug() << "valid span";
            return true;
        }
    }
    else
    {
        Q_ASSERT(!arcData->convex);
        Q_ASSERT(arcData->span > 0);
        Q_ASSERT(ad.span > 0);
        if ((ad.end > arcData->start) &&  ad.end < arcData->end)
        {
            if (debug >= 2) qDebug() << "valid span";
            return true;
        }
    }
    if (debug >= 1) qDebug() << "rejected span";
    return false;
}

void Edge::resetCurve()
{
    if (type == EDGETYPE_CURVE || type == EDGETYPE_CHORD)
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

VertexPtr Edge::getOtherV(const VertexPtr & vert) const
{
    if (vert == v1)
        return v2;
    else
    {
        Q_ASSERT (vert == v2);
        return v1;
    }
}

VertexPtr Edge::getOtherV(const QPointF &pos) const
{
    if (pos == v1->pt)
        return v2;
    else
    {
        Q_ASSERT (pos == v2->pt);
        return v1;
    }
}

QPointF Edge::getOtherP(const VertexPtr & vert) const
{
    if (vert == v1)
        return v2->pt;
    else
    {
        Q_ASSERT (vert == v2);
        return v1->pt;
    }
}

QPointF Edge::getOtherP(const QPointF & pos) const
{
    if (pos == v1->pt)
        return v2->pt;
    else
    {
        Q_ASSERT (pos == v2->pt);
        return v1->pt;
    }
}

bool Edge::contains(const VertexPtr & v)
{
    if ((v == v1) || (v == v2))
        return true;
    return false;
}

// Used to sort the edges in the map.
qreal Edge::getMinX()
{
    return qMin(v1->pt.x(), v2->pt.x());
}

qreal Edge::getMaxX()
{
    return qMax(v1->pt.x(), v2->pt.x());
}

qreal Edge::getRadius()
{
    return QLineF(arcCenter,v1->pt).length();
}

qreal Edge::getArcSpan()
{
    Q_ASSERT(arcData);
    return arcData->span;
}

bool Edge::isTrivial(qreal tolerance)
{
    if (!v1 || !v2)
    {
        //qWarning("Edge missing vertex");
        return true;
    }

    if (v1 == v2)
    {
        //qWarning("Trivial edge - vertices");
        return true;
    }

    if (Point::dist2(v1->pt,v2->pt) < tolerance)
    {
        //qWarning("Trivial edge - points");
        return true;
    }
    return false;

}

bool Edge::isColinearAndTouching(const EdgePtr & e, qreal tolerance)
{
    static bool debug = false;
    if (debug) qDebug().noquote() << "testing"  << dump() << "and" << e->dump();

    if ((e->contains(v1)) || (e->contains(v2)))
    {
        return isColinear(e,tolerance);
    }

    if (debug) qDebug() << "    not touching";

    return false;
}

bool Edge::isColinear(const EdgePtr & e, qreal tolerance)
{
    static bool debug = false;

    qreal angle = Utils::angle(getLine(),e->getLine());
    if (debug) qDebug() << "    angle=" << angle;
    if ((qAbs(angle) < tolerance) || (qAbs(angle-180.0) < tolerance))
    {
        if (debug) qDebug() << "    colinear";
        return true;
    }
    else
    {
        if (debug) qDebug() << "NOT colinear";

        return false;
    }
}

qreal Edge::getAngle()
{
    return qAtan2(v1->pt.x() - v2->pt.x(),v1->pt.y()-v2->pt.y());
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
            << "v1" << v1->pt
            << "v2" << v2->pt
            << "arcCenter:" << arcCenter  << ((convex) ? "CONVEX" : "CONCAVE")
            << ((isSwapped) ? "swapped" : "");
        break;
    case EDGETYPE_POINT:
        deb << "Edge POINT"
            << "v1" << v1->pt;
        break;
    case EDGETYPE_CHORD:
        deb << "Edge CHORD"
            << "v1" << v1->pt
            << "v2" << v2->pt
            << "arcCenter:" << arcCenter  << ((convex) ? "CONVEX" : "CONCAVE")
            << ((isSwapped) ? "swapped" : "");
        break;
    case EDGETYPE_NULL:
        deb << "Edge NULL";
    }
    if (isCurve())
    {
        deb << "span" << arcData->span;
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
