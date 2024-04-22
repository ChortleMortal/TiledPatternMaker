////////////////////////////////////////////////////////////////////////////
//
// Edge.java
//

#include <QTransform>
#include "geometry/edge.h"
#include "geometry/arcdata.h"
#include "geometry/geo.h"
#include "geometry//loose.h"
#include "geometry/vertex.h"

using std::make_shared;

int Edge::refs = 0;

Edge::Edge()
{
    type        = EDGETYPE_NULL;
    dvisited    = false;
    refs++;
}

Edge::Edge(const VertexPtr &v1)
{
    type        = EDGETYPE_POINT;
    dvisited    = false;
    this->v1    = v1;
    this->v2    = v1; // same
    refs++;
}

Edge::Edge(const VertexPtr & v1, const VertexPtr & v2 )
{
    type        = EDGETYPE_LINE;
    dvisited    = false;
    this->v1    = v1;
    this->v2    = v2;
    refs++;
}

Edge::Edge(const VertexPtr & v1, const VertexPtr & v2, const QPointF & arcCenter, bool convex, bool chord)
{
    if (chord)
        type    = EDGETYPE_CHORD;
    else
        type    = EDGETYPE_CURVE;
    dvisited    = false;
    this->v1    = v1;
    this->v2    = v2;
    setCurvedEdge(arcCenter,convex,chord);   // this creates arcData too
    refs++;
}

Edge::Edge(const Edge & other)
{
    type        = other.type;
    dvisited    = other.dvisited;
    v1          = other.v1;
    v2          = other.v2;
    if (type == EDGETYPE_CURVE || type == EDGETYPE_CHORD)
    {
        //setCurvedEdge(other.getArcCenter(),other.isConvex(),(type==EDGETYPE_CHORD));
        arcData = other.arcData;
    }
    refs++;
}

EdgePtr Edge::createTwin()
{
    // has same edge index as original
    EdgePtr ep        = make_shared<Edge>(*this);
    ep->dvisited      = dvisited;
    ep->v1            = v2;
    ep->v2            = v1;
    if (type == EDGETYPE_CURVE || type == EDGETYPE_CHORD)
    {
        ep->arcData       = arcData;
        ep->arcData.create(ep->v1->pt,ep->v2->pt,arcData.getCenter(),!arcData.convex());
    }
    return ep;
}

Edge::Edge(const EdgePtr &other)
{
    type           = other->type;
    dvisited       = false;
    v1             = other->v1;
    v2             = other->v2;
    if (type == EDGETYPE_CURVE || type == EDGETYPE_CHORD)
    {
        ArcData & ad = getArcData();
        ad = other->getArcData();
    }
    refs++;
}

Edge::Edge(const EdgePtr &other, QTransform T)
{
    type           = other->type;
    dvisited       = false;
    QPointF p1     = T.map(other->v1->pt);
    QPointF p2     = T.map(other->v2->pt);
    v1 = make_shared<Vertex>(p1);
    v2 = make_shared<Vertex>(p2);
    if (type == EDGETYPE_CURVE || type == EDGETYPE_CHORD)
    {
        ArcData & ad = getArcData();
        ad = other->getArcData();
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

bool Edge::sameAs(const QPointF &op1, const QPointF &op2)
{
    if (v1->pt == op1 &&  v2->pt == op2)
    {
        return true;
    }
    if (v1->pt == op2 && v2->pt == op1)
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
        if (getArcCenter() != other->getArcCenter())
            return false;

        if (isConvex() != other->isConvex())
            return false;
    }

    return true;
}

void  Edge::setV1(const VertexPtr & v)
{
    this->v1 = v;
    if (isCurve())
    {
        arcData.create(v1->pt,v2->pt,arcData.getCenter(),arcData.convex());
    }
}

void  Edge::setV2(const VertexPtr & v)
{
    this->v2 = v;
    if ((type == EDGETYPE_NULL) || (type == EDGETYPE_POINT))
    {
        type = EDGETYPE_LINE;
    }
    else if (isCurve())
    {
        arcData.create(v1->pt,v2->pt,arcData.getCenter(),arcData.convex());
    }
}

void Edge::resetCurveToLine()
{
    if (type == EDGETYPE_CURVE || type == EDGETYPE_CHORD)
    {
        type = EDGETYPE_LINE;
    }
}

void Edge::setConvex(bool convex)
{
    ArcData & ad = getArcData();
    ad.setConvex(convex,v1->pt,v2->pt);
}

void Edge::setCurvedEdge(QPointF arcCenter, bool convex, bool chord)
{
    if (chord)
        type     = EDGETYPE_CHORD;
    else
        type     = EDGETYPE_CURVE;

    ArcData & ad = getArcData();
    ad.create(v1->pt,v2->pt,arcCenter,convex);
    calcMagnitude();
    ad.calcSpan(this);
}

// FIXME - only used in create
void Edge::convertToConvexCurve()
{
    // calculates a default line center
    type  = EDGETYPE_CURVE;

    QLineF line = getLine();
    QPointF mid = getMidPoint();
    QPointF pt  = mid + QPointF(-line.dy(), line.dx());

    setCurvedEdge(pt,true,false);
}

void Edge::calcMagnitude()
{
    // calcs magnitude from arcCenter
    ArcData & ad = getArcData();
    QLineF line  = getLine();
    QPointF mid  = getMidPoint();
    qreal dist   = Geo::dist(mid,getArcCenter());
    ad.magnitude = (dist/line.length());
    //qDebug() << "magnitude=" << arcMagnitude;
}

void Edge::setArcMagnitude(qreal magnitude)
{
    // calcs arcCenter from magntiude
    Q_ASSERT(isCurve());   // this means arcCenter is already set
    QLineF line = getLine();
    QPointF mid = getMidPoint();
    QPointF pt;
    if (isConvex())
    {
        pt = mid + QPointF(-line.dy(), line.dx());
    }
    else
    {
        pt = mid - QPointF(line.dy(), -line.dx());
    }

    QLineF perp(mid,pt);

    qreal arcCenterLen = magnitude * line.length();
    perp.setLength(arcCenterLen);

    ArcData & ad  = getArcData();
    ad.setCenter(perp.p2());
    ad.calcSpan(this);
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
    return QLineF(getArcCenter(),v1->pt).length();
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
    
    if (Geo::dist2(v1->pt,v2->pt) < tolerance)
    {
        //qWarning("Trivial edge - points");
        return true;
    }
    return false;

}

bool Edge::isColinearAndTouching(const EdgePtr & e, qreal tolerance)
{
    static bool debug = false;
    if (debug) qDebug().noquote() << "testing"  << info() << "and" << e->info();

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

    qreal angle = Geo::angle(getLine(),e->getLine());
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
    return qAtan2(v1->pt.x() - v2->pt.x(), v1->pt.y() - v2->pt.y());
}

QString Edge::info()
{
    QString astring;
    QDebug  deb(&astring);

    deb << this;

    switch (type)
    {
    case EDGETYPE_LINE:
        deb << "Edge LINE "
            << "v1" << v1->pt
            << "v2" << v2->pt;
        break;
    case EDGETYPE_CURVE:
        deb << "Edge CURVE"
            << "v1" << v1->pt
            << "v2" << v2->pt
            << "arcCenter:" << getArcCenter()  << (isConvex() ? "CONVEX" : "CONCAVE");
        break;
    case EDGETYPE_POINT:
        deb << "Edge POINT"
            << "v1" << v1->pt;
        break;
    case EDGETYPE_CHORD:
        deb << "Edge CHORD"
            << "v1" << v1->pt
            << "v2" << v2->pt
            << "arcCenter:" << getArcCenter()  << (isConvex() ? "CONVEX" : "CONCAVE");
        break;
    case EDGETYPE_NULL:
        deb << "Edge NULL";
        break;
    }
    if (isCurve())
    {
        deb << getArcData().info();
    }
    return astring;
}

QString Edge::summary()
{
    QString astring;
    QDebug  deb(&astring);

    switch (type)
    {
    case EDGETYPE_LINE:
        deb << "LINE";
        break;
    case EDGETYPE_CURVE:
        deb << "CURVE"
            << "arcCenter:" << getArcCenter()  << (isConvex() ? "CONVEX" : "CONCAVE");
        break;
    case EDGETYPE_POINT:
        deb << "POINT"
            << "v1" << v1->pt;
        break;
    case EDGETYPE_CHORD:
        deb << "CHORD"
            << "arcCenter:" << getArcCenter()  << (isConvex() ? "CONVEX" : "CONCAVE");
        break;
    case EDGETYPE_NULL:
        deb << "NULL";
        break;
    }
    return astring;
}

void Edge::dump()
{
    qDebug().noquote() << info();
}

// The less-than operator compares the first point, then the order.
// Thus, two overlapped edges with different second point will not be less than each other,
// yet they won't be equal. This cannot happen in a map anyway since edges never overlap.
bool Edge::operator < (const Edge & other) const
{
    QPointF point  = v1->pt;
    QPointF opoint = other.v1->pt;
    return (Geo::lessThan(point, opoint) || (point == opoint  &&  angle() < other.angle() - Sys::TOL));
}
