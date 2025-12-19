#ifdef __linux__
#include <cfloat>
#endif

#include "gui/viewers/geo_graphics.h"
#include "sys/geometry/arcdata.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/neighbour_map.h"
#include "sys/geometry/threads.h"
#include "model/styles/casing_neighbours.h"
#include "model/styles/interlace.h"

///////////////////////////////////////////////////////////////////
///
///  InterlaceSide
///
////////////////////////////////////////////////////////////////////

InterlaceSide::InterlaceSide(Casing * parent, eSide side, CNeighboursPtr np, VertexPtr vertex) : CasingSide(parent,side,np,vertex)
{
    shadow       = false;
    created      = false;
}

void InterlaceSide::createSide1_ilace(const EdgePtr & edge, qreal width)
{
    QLineF  aline;
    QPointF omid;

    mid  = edge->v1->pt;
    omid = edge->v2->pt;
    aline = Geo::normalVectorP1(edge->getLine());
    //bool from_under = (edge->v1 == from ) == edge->v1_under;  // methinks ugly code

    //QPointF perp = calc_perp(mid - omid,width);
    aline.setLength(width);
    QPointF perp = aline.p2() - mid;

    BeforeAndAfter ba = cneighbours->getBeforeAndAfter(edge);

    // Four cases
    //  - cap
    //  - bend
    //  - interlace over
    //  - interlace under
    int nn = cneighbours->numNeighbours();
    if (nn == 1)
    {
        // this is an end (methinks a flat end)
        outer = mid - perp;
        inner = mid + perp;
    }
    else if (nn == 2)
    {
        outer = getJoinPoint(ba.before, mid, edge, width, EDGE_OUTER);
        if (outer.isNull())
        {
            outer = mid + perp;
            inner = mid - perp;
        }
        else
        {
            inner = Geo::convexSum(outer, mid, 2.0);
        }
    }
    else if (nn == 3)
    {
        outer = getJoinPoint(ba.before, mid, edge,  width, EDGE_OUTER);
        if (outer.isNull())
        {
            outer = mid + perp;
        }

        inner = getJoinPoint(ba.after, mid, edge,  width, EDGE_INNER);
        if (inner.isNull())
        {
            inner = mid - perp;
        }
    }
    else
    {
        // assumes is 'over' and 'under' is fixed on the next pass
        QVector<EdgePtr> ns;
        int index    = 0;
        int edge_idx = -1;
        for (auto & wedge : *cneighbours)
        {
            EdgePtr nedge = wedge.lock();
            ns << nedge;
            if (nedge == edge)
                edge_idx = index;
            index++;
        }
        int nidx = (edge_idx + 2) % nn;
        EdgePtr oedge = ns[nidx];

        outer = getJoinPoint(oedge, mid, edge, width, EDGE_OUTER);   // changed
        if (outer.isNull())
        {
            QPointF perp = calc_perp(mid - omid,width);
            outer = mid + perp;
        }
        inner = Geo::convexSum(outer, mid, 2.0);
    }
    created = true;
}

void InterlaceSide::createSide2_ilace(const EdgePtr & edge, qreal width)
{
    QLineF  aline;
    QPointF omid;

    mid  = edge->v2->pt;
    omid = edge->v1->pt;
    aline = Geo::normalVectorP2(edge->getLine());
    //bool from_under = (edge->v1 == from ) == edge->v1_under;  // methinks ugly code

    //QPointF perp = calc_perp(mid - omid,width);
    aline.setLength(width);
    QPointF perp = aline.p2() - mid;

    BeforeAndAfter ba = cneighbours->getBeforeAndAfter(edge);

    // Four cases
    //  - cap
    //  - bend
    //  - interlace over
    //  - interlace under
    int nn = cneighbours->numNeighbours();
    if (nn == 1)
    {
        // this is an end (methinks a flat end)
        outer = mid - perp;
        inner = mid + perp;
    }
    else if (nn == 2)
    {
        outer = getJoinPoint(edge, mid, ba.before, width, EDGE_OUTER);
        if (outer.isNull())
        {
            outer = mid - perp;
            inner = mid + perp;
        }
        else
        {
            inner = Geo::convexSum(outer, mid, 2.0);
        }
    }
    else if (nn == 3)
    {
        outer = getJoinPoint(edge, mid, ba.after,  width, EDGE_OUTER);
        if (outer.isNull())
        {
            outer = mid - perp;
        }

        inner = getJoinPoint(edge, mid, ba.before, width, EDGE_INNER);
        if (inner.isNull())
        {
            inner = mid + perp;
        }
    }
    else
    {
        // assumes is 'over' and 'under' is fixed on the next pass
        EdgePtr oedge;
        for (int i = 0; i < nn; i++)
        {
            EdgePtr nedge = cneighbours->at(i).lock();
            if (nedge == edge)
            {
                oedge = cneighbours->at((i+2) % nn).lock();
                break;
            }
        }

        outer = getJoinPoint(edge, mid, oedge,  width, EDGE_OUTER);
        if (outer.isNull())
        {
            QPointF perp = calc_perp(mid - omid,width);
            outer = mid - perp;
        }
        inner = getJoinPoint(edge, mid, oedge,  width, EDGE_INNER);
        if (inner.isNull())
        {
            inner = Geo::convexSum(outer, mid, 2.0);
        }
    }
    created = true;
}

void InterlaceSide::underSide1(const EdgePtr & edge, qreal width)
{
    if (cneighbours->numNeighbours() < 4)
        return;

    if (Interlace::dbgDump2 & 0x100) qDebug() << "setting under for edge" << edge->casingIndex << "side 1";
    // interlace under
    // This is the hard case, fraught with pitfalls for
    // the imprudent (i.e., me).  I think what I've got
    // now does a reasonable job on well-behaved maps
    // and doesn't dump core on badly-behaved ones.

    QLineF aline = Geo::normalVectorP1(edge->getLine());
    aline.setLength(width);
    QPointF perp = aline.p2() - mid;

    BeforeAndAfter ba = cneighbours->getBeforeAndAfter(edge);
    if (Interlace::dbgDump2 & 0x100) qDebug() << "edge" << edge->casingIndex << "before" << ba.before->casingIndex << "after" << ba.after->casingIndex;

    if (edge->isLine())
    {
        QPointF oldMid = mid;

        if (ba.before->isLine())
        {
            outer = getJoinPoint(ba.before, oldMid, edge, width, EDGE_OUTER);
            if (outer.isNull())
                outer = oldMid + perp;
        }
        else
        {
            qWarning() << "curve TODO";
        }

        if (ba.after->isLine())
        {
            inner = getJoinPoint(ba.after, oldMid, edge, width, EDGE_INNER);
            if (inner.isNull())
                inner = oldMid - perp;
        }
        else
        {
            qWarning() << "curve TODO";
        }

        if (!Sys::flags->flagged(USE_MID_FIX))
        {
            mid   = getJoinPoint(ba.after, oldMid, ba.before, width, EDGE_INNER);
            if (mid.isNull())
            {
                QPointF after_pt  = ba.after->getOtherP(oldMid);
                QPointF before_pt = ba.before->getOtherP(oldMid);
                QPointF perp      = calc_perp(after_pt - before_pt,width);
                mid               = oldMid - perp ;
                mark(NULL_MARKS_1, mid, "nullmid");
            }
        }
        else
            mid = QLineF(inner,outer).center();
    }
    else
    {
        Q_ASSERT(edge->isCurve());
        mid = QLineF(inner, outer).center();
    }

    shadow = true;
}

void InterlaceSide::underSide2(const EdgePtr & edge, qreal width)
{
    if (cneighbours->numNeighbours() < 4)
        return;

    if (Interlace::dbgDump2 & 0x100) qDebug() << "setting under for edge" << edge->casingIndex << "side 2";
    // interlace under
    // This is the hard case, fraught with pitfalls for
    // the imprudent (i.e., me).  I think what I've got
    // now does a reasonable job on well-behaved maps
    // and doesn't dump core on badly-behaved ones.

    QLineF aline = Geo::normalVectorP2(edge->getLine());
    aline.setLength(width);
    QPointF perp = aline.p2() - mid;

    BeforeAndAfter ba = cneighbours->getBeforeAndAfter(edge);
    if (Interlace::dbgDump2 & 0x100) qDebug() << "edge" << edge->casingIndex << "before" << ba.before->casingIndex << "after" << ba.after->casingIndex;

    if (edge->isLine())
    {
        QPointF oldMid = mid;

        if (ba.after->isLine())
        {
            outer = getJoinPoint(edge, oldMid, ba.after,  width, EDGE_OUTER);
            if (outer.isNull())
                outer = oldMid - perp;
        }
        else
        {
            qWarning() << "curve TODO";
        }

        if (ba.before->isLine())
        {
            inner = getJoinPoint(edge, oldMid, ba.before, width, EDGE_INNER);
            if (inner.isNull())
                inner = oldMid + perp;
        }
        else
        {
            qWarning() << "curve TODO";
        }

        if (!Sys::flags->flagged(USE_MID_FIX))
        {
            mid   = getJoinPoint(ba.before, oldMid, ba.after,  width, EDGE_OUTER);
            if (mid.isNull())
            {
                QPointF after_pt  = ba.after->getOtherP(oldMid);
                QPointF before_pt = ba.before->getOtherP(oldMid);
                QPointF perp  = calc_perp(before_pt -after_pt,width);
                mid           = oldMid + perp ;
                mark(NULL_MARKS_2, mid, "nullmid");
            }
        }
        else
            mid = QLineF(inner,outer).center();
    }
    else
    {
        Q_ASSERT(edge->isCurve());
        mid = QLineF(inner, outer).center();
    }

    shadow = true;
}

void InterlaceSide::setGap(const EdgePtr & edge, qreal gap)
{
    Q_ASSERT(gap > 0.0);

    QPointF from_pt;
    QPointF to_pt;

    if (side == SIDE_1)
    {
        from_pt    = edge->v2->pt;
        to_pt      = edge->v1->pt;
    }
    else
    {
        from_pt    = edge->v1->pt;
        to_pt      = edge->v2->pt;
    }

    int nn = cneighbours->numNeighbours();
    if (nn > 3 && under())
    {
        if (edge->isLine())
        {
            QPointF dir = to_pt - from_pt;
            Geo::normalizeD(dir);
            inner -= (dir * capGap(inner, from_pt, gap));
            outer -= (dir * capGap(outer, from_pt, gap));
            mid   -= (dir * capGap(mid  , from_pt, gap));
        }
        else
        {
            qreal delta = gap * 20.0;
            if (side == SIDE_1)
                delta = -delta;
            inner = ArcData::computeNewPoint(inner,edge->getArcCenter(),delta,true);
            outer = ArcData::computeNewPoint(outer,edge->getArcCenter(),delta,true);
            mid   = ArcData::computeNewPoint(mid  ,edge->getArcCenter(),delta,true);
        }
    }
}

qreal InterlaceSide::capGap( QPointF p, QPointF base, qreal gap )
{
    qreal max_gap = Geo::dist(p, base );
    return (gap < max_gap) ? gap : max_gap;
}

QPointF InterlaceSide::calc_perp(QPointF pt, qreal width)
{
    Geo::normalizeD(pt);
    QPointF perp = Geo::perp(pt);
    perp *= width;
    return perp;
}

InterlaceSide & InterlaceSide::operator=(const InterlaceSide & other)
{
    parent      = other.parent;
    side        = other.side;
    cneighbours  = other.cneighbours;
    wv          = other.wv;
    created     = other.created;

    shadow      = other.shadow;
    outer       = other.outer;
    mid         = other.mid;
    inner       = other.inner;

    return *this;
}

void InterlaceSide::dump2()
{
    qDebug() << "LOG2 edge" << parent->edgeIndex << thisSide() << outer << mid << inner;
}

QString InterlaceSide::thisSide()
{
    return sSide[side];
}
