////////////////////////////////////////////////////////////////////////////
//
// Edge.java
//

#include <QTransform>
#include "sys/geometry/edge.h"
#include "sys/geometry/arcdata.h"
#include "sys/geometry/debug_map.h"
#include "sys/geometry/geo.h"
#include "sys/geometry//loose.h"
#include "sys/geometry/vertex.h"
#ifdef QT_DEBUG
#include "sys/sys/debugflags.h"
#endif

using std::make_shared;

int  Edge::refs          = 0;
bool Edge::curvesAsLines = false;

Edge::Edge()
{
    _type       = EDGETYPE_NULL;
    dvisited    = false;
    refs++;
}

Edge::Edge(const VertexPtr &v1)
{
    _type       = EDGETYPE_POINT;
    dvisited    = false;
    this->v1    = v1;
    this->v2    = v1; // same
    refs++;
}

Edge::Edge(const VertexPtr & v1, const VertexPtr & v2 )
{
    _type       = EDGETYPE_LINE;
    dvisited    = false;
    this->v1    = v1;
    this->v2    = v2;
    refs++;
}

Edge::Edge(const VertexPtr & v1, const VertexPtr & v2, const QPointF & arcCenter, eCurveType ctype)
{
    _type       = EDGETYPE_CURVE;
    dvisited    = false;
    this->v1    = v1;
    this->v2    = v2;
    chgangeToCurvedEdge(arcCenter,ctype);   // this creates arcData too
    refs++;
}

Edge::Edge(const Edge & other)
{
    _type       = other._type;
    dvisited    = other.dvisited;
    v1          = other.v1;
    v2          = other.v2;
    if (_type == EDGETYPE_CURVE)
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
    ep->v1            = v2;
    ep->v2            = v1;
    if (_type == EDGETYPE_CURVE)
    {
        ep->arcData       = arcData;
        ep->arcData.create(ep->getLine(),arcData.getCenter(),(arcData.getCurveType() == CURVE_CONVEX) ? CURVE_CONCAVE : CURVE_CONVEX);
    }
    return ep;
}

Edge::Edge(const EdgePtr &other)
{
    _type          = other->_type;
    dvisited       = false;
    v1             = other->v1;
    v2             = other->v2;
    if (_type == EDGETYPE_CURVE)
    {
        arcData = other->getArcData();
    }
    refs++;
}

Edge::Edge(const EdgePtr &other, QTransform T)
{
    _type          = other->_type;
    dvisited       = false;
    QPointF p1     = T.map(other->v1->pt);
    QPointF p2     = T.map(other->v2->pt);
    v1 = make_shared<Vertex>(p1);
    v2 = make_shared<Vertex>(p2);
    if (_type == EDGETYPE_CURVE)
    {
        arcData = other->getArcData();
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

eEdgeType Edge::getType()
{
    if (curvesAsLines)
        return EDGETYPE_LINE;
    else
        return _type;
}

bool Edge::isLine()
{
     if (curvesAsLines)
        return true;
     else
        return (_type == EDGETYPE_LINE);
}

bool Edge::isCurve()
{
    if (curvesAsLines)
        return false;
    else
        return (_type == EDGETYPE_CURVE);
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

    if (_type != other->_type)
        return false;

    if (_type == EDGETYPE_CURVE)
    {
        if (getArcCenter() != other->getArcCenter())
            return false;

        if (getCurveType() != other->getCurveType())
            return false;
    }

    return true;
}

void  Edge::setV1(const VertexPtr & v)
{
    this->v1 = v;
    if (isCurve())
    {
        arcData.create(QLineF(v1->pt,v2->pt),arcData.getCenter(),arcData.getCurveType());
    }
}

void  Edge::setV2(const VertexPtr & v)
{
    this->v2 = v;
    if ((_type == EDGETYPE_NULL) || (_type == EDGETYPE_POINT))
    {
        _type = EDGETYPE_LINE;
    }
    else if (isCurve())
    {
        arcData.create(QLineF(v1->pt,v2->pt),arcData.getCenter(),arcData.getCurveType());
    }
}

void Edge::resetCurveToLine()
{
    if (_type == EDGETYPE_CURVE)
    {
        _type = EDGETYPE_LINE;
    }
}

void Edge::changeCurveType(eCurveType ctype)
{
    arcData.changeCurveType(ctype);
}

void Edge::chgangeToCurvedEdge(QPointF arcCenter, eCurveType ctype)
{
    _type = EDGETYPE_CURVE;
    arcData.create(QLineF(v1->pt,v2->pt),arcCenter,ctype);
}

// saying convex makes it concave

// only used to create
void Edge::convertToCurve(eCurveType ctype)
{
    // calculates a default line center
    QLineF line     = getLine();
    QPointF mid     = getMidPoint();

    if (ctype == CURVE_CONVEX)
    {
        QPointF center  = mid - QPointF(line.dy(), -line.dx());
        chgangeToCurvedEdge(center,CURVE_CONVEX);
        Sys::debugMapCreate->insertDebugMark(center,"convex");
    }
    else
    {
        QPointF center   = mid + QPointF(line.dy(), -line.dx());
        chgangeToCurvedEdge(center,CURVE_CONCAVE);
        Sys::debugMapCreate->insertDebugMark(center,"concave");
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

eSide  Edge::side(VertexPtr v)
{
    Q_ASSERT(contains(v));
    return (v1 == v) ? SIDE_1 : SIDE_2;
}

eSide  Edge::side(QPointF p)
{
    Q_ASSERT(contains(p));
    return (v1->pt == p) ? SIDE_1 : SIDE_2;
}

bool Edge::contains(const VertexPtr & v)
{
    if ((v == v1) || (v == v2))
        return true;
    return false;
}

bool Edge::contains(const QPointF & p)
{
    if ((p == v1->pt) || (p == v2->pt))
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
    qreal r1 =  QLineF(getArcCenter(),v1->pt).length();
#if 0
    qreal r2 =  QLineF(getArcCenter(),v2->pt).length();
    Q_ASSERT(Loose::equals(r1,r2));
#endif
    return r1;
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

    switch (_type)
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
            << "arcCenter:" << getArcCenter()  << sCurveType[getCurveType()];
        break;
    case EDGETYPE_POINT:
        deb << "Edge POINT"
            << "v1" << v1->pt;
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

    switch (_type)
    {
    case EDGETYPE_LINE:
        deb << "LINE";
        break;
    case EDGETYPE_CURVE:
        deb << "CURVE"
            << "arcCenter:" << getArcCenter()  << sCurveType[getCurveType()];
        break;
    case EDGETYPE_POINT:
        deb << "POINT"
            << "v1" << v1->pt;
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
