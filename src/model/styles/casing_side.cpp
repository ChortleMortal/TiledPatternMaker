#include "model/styles/casing_neighbours.h"
#include "model/styles/casing_side.h"
#include "sys/geometry/debug_map.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/vertex.h"
#include "sys/sys/debugflags.h"

CasingSide::CasingSide(Casing * parent, eSide side, CNeighboursPtr np, VertexPtr vertex)
{
    this->parent = parent;
    this->side   = side;
    cneighbours  = np;
    wv           = vertex;
    created      = false;
}

void CasingSide::createSide1(const EdgePtr & edge,  qreal qwidth)
{
    //qDebug() << "CasingSide1::create 1" << "edge=" << map->edgeIndex(edge);

    mid  = edge->v1->pt;

    QLineF aline = Geo::normalVectorP1(edge->getLine());
    aline.setLength(qwidth);
    QPointF norp = aline.p2() - mid;
    //qDebug() << "norp" << norp;

    BeforeAndAfter ba = cneighbours->getBeforeAndAfter(edge);
    int n = cneighbours->numNeighbours();
    //qDebug() << "CasingSide::create" << "edge=" << map->edgeIndex(edge) << "n=" << n;
    switch(n)
    {
    case 1:
        // this is an end (a flat end)
        outer = mid - norp                                                                                                                               ;
        inner = mid + norp;
        break;

    case 2:
        outer = getJoinPoint(ba.before, mid, edge, qwidth, EDGE_OUTER);
        if (outer.isNull())
        {
            outer = mid - norp;
            inner = mid + norp;
        }
        else
        {
            inner = Geo::convexSum(outer, mid, 2.0);
        }
        break;

    default:
        outer = getJoinPoint(ba.before, mid, edge, qwidth, EDGE_OUTER);
        if (outer.isNull())
            outer = mid + norp;

        inner = getJoinPoint(ba.after, mid, edge, qwidth, EDGE_INNER);
        if (inner.isNull())
            inner = mid - norp;
    }

    created = true;
}

void CasingSide::createSide2(const EdgePtr & edge,  qreal qwidth)
{
    //qDebug() << "CasingSide2::create" << "edge=" << map->edgeIndex(edge);

    mid  = edge->v2->pt;

    QLineF          aline;
    aline = Geo::normalVectorP2(edge->getLine());
    aline.setLength(qwidth);
    QPointF norp = aline.p2() - mid;
    //qDebug() << "norp" << norp;

    BeforeAndAfter ba = cneighbours->getBeforeAndAfter(edge);
    int n = cneighbours->numNeighbours();
    //qDebug() << "EdgeCasingSide::create" << "edge=" << map->edgeIndex(edge) << "n=" << n;
    switch(n)
    {
    case 1:
        // this is an end (a flat end)
        outer = mid - norp                                                                                                                               ;
        inner = mid + norp;
        break;

    case 2:
        outer = getJoinPoint(edge, mid, ba.before, qwidth, EDGE_OUTER);

        if (outer.isNull())
        {
            outer = mid - norp;
            inner = mid + norp;
        }
        else
        {
            inner = Geo::convexSum(outer, mid, 2.0);
        }
        break;

    default:
        outer = getJoinPoint(edge, mid, ba.after, qwidth, EDGE_OUTER);
        if (outer.isNull())
            outer = mid - norp;

        inner = getJoinPoint(edge, mid, ba.before, qwidth, EDGE_INNER);
        if (inner.isNull())
            inner = mid + norp;
        break;
    }
    created = true;
}

CasingSide & CasingSide::operator=(const CasingSide & other)
{
    side        = other.side;
    cneighbours = other.cneighbours;
    wv          = other.wv;
    created     = other.created;

    outer       = other.outer;
    mid         = other.mid;
    inner       = other.inner;

    return *this;
}

QPointF CasingSide::getJoinPoint(const EdgePtr&  from, QPointF mid,  const EdgePtr &  to, qreal qwidth, eLSide lside)
{
    // NOTE:  this assumes the otherP and its mid are identicqal.
    // Howwever, this is not true when othe other is 'under'
    return _getJoinPoint(from->getOtherP(mid), mid, to->getOtherP(mid), qwidth, lside);
}

QPointF CasingSide::_getJoinPoint(QPointF from, QPointF joint, QPointF to, qreal qwidth, eLSide lside)
{
    // Do a mitered join of the two fat lines (a la postscript, for example).
    // The join point on the other side of the joint can be computed by
    // reflecting the point returned by this function through the joint.

    //qDebug().noquote() << "EdgeCasingSide::getJoinPoint" << cSide[side] << cLSide[lside]  << from << joint << to << qwidth;

    QPointF d1 = joint - from;
    Geo::normalizeD(d1);
    QPointF d2 = joint - to;
    Geo::normalizeD(d2);
#if 0
    qreal theta = Geo::sweep(joint, from, to);  // radians
    if (Loose::zero(theta) ||qAbs(theta - M_PI) < 1e-7)
    {
        return QPointF(0,0);
    }
    qreal l   = qwidth / qSin(theta);
#else
    qreal  sth = d2.y() * d1.x() - d2.x() * d1.y();       // sin dot as per Ballargeon
    if (Loose::zero(sth,0.01))
    {
        return QPointF(0,0);
    }
    const qreal l = qwidth / sth;
#endif
    qreal dx = (d1.x() + d2.x()) * l;
    qreal dy = (d1.y() + d2.y()) * l;
    QPointF d(dx,dy);
    if (lside == EDGE_INNER)
    {
        joint += d;
        mark(MARK_JOIN,joint,"I");
    }
    else
    {
        joint -= d;
        mark(MARK_JOIN,joint,"O");
    }
    return joint;
}

void CasingSide::mark(eDbgFlag flag, QPointF & pt, const QString & txt, QColor color)
{
    if (Sys::flags->flagged(flag))
    {
        Sys::debugMapCreate->insertDebugMark(pt, txt, color);
    }
}

bool CasingSide::validate()
{
    if (outer.isNull() || mid.isNull() || inner.isNull())
        return false;
    return true;
}

void CasingSide::debugPoints()
{
    if (side == SIDE_1)
    {
        Sys::debugMapPaint->insertDebugPoint(inner,Qt::blue);
        Sys::debugMapPaint->insertDebugPoint(mid,  Qt::green);
        Sys::debugMapPaint->insertDebugPoint(outer,Qt::red);
    }
    else
    {
        Sys::debugMapPaint->insertDebugCircle(inner,Qt::blue);
        Sys::debugMapPaint->insertDebugCircle(mid,  Qt::green);
        Sys::debugMapPaint->insertDebugCircle(outer,Qt::red);
    }
}
