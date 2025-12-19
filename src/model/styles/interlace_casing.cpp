#ifdef __linux__
#include <cfloat>
#endif
#include <QLineF>
#include "gui/viewers/debug_view.h"
#include "gui/viewers/geo_graphics.h"
#include "model/styles/casing_neighbours.h"
#include "model/styles/interlace.h"
#include "model/styles/interlace_casing.h"
#include "sys/geometry/arcdata.h"
#include "sys/geometry/geo.h"
//#include "sys/geometry/intersect.h"
#include "sys/geometry/neighbour_map.h"
#include "sys/geometry/threads.h"
#include "sys/sys/debugflags.h"

//////////////////////////////////////////////////////////////////////
///
/// InterlaceCasingSet
///
//////////////////////////////////////////////////////////////////////

void InterlaceCasingSet::weave()
{
    static uint max = 0;
    qDebug() << "InterlaceCasingSet::createUnders" << "size" << weavings.size();

    Q_ASSERT(!Sys::flags->flagged(APPROACH_6));

    for (auto it = weavings.begin(); it!= weavings.end(); it++)
    {
        CNeighboursPtr cnp = it.value();

        if (cnp->numNeighbours() < 4)
            continue;

        // do this only once for each intersection of edges
        if (!Sys::flags->flagged(NO_UNDER))
        {
            cnp->doUnders();
        }

        if (!Sys::flags->flagged(NO_OVER))
        {
            cnp->doOvers();
        }

        if (Sys::flags->flagged(USE_TRIGGER_1) && max >= Interlace::iTrigger)
            return;

        max++;
    }
}

void InterlaceCasingSet::useMap()
{
    for (auto & casing : *this)
    {
        InterlaceCasingPtr icp = std::static_pointer_cast<InterlaceCasing>(casing);
        icp->useMap();
    }
}


void InterlaceCasingSet::dumpWeaveStatus()
{
    QStringList qsl;

    for (auto it = weavings.begin(); it!= weavings.end(); it++)
    {
        CNeighboursPtr cnp = it.value();
        qsl << cnp->infoWeave();
    }

    qsl.sort();

    qInfo() << "over status begin";
    for (auto  & str : qsl)
    {
        qDebug().noquote() << str;
    }
    qInfo() << "over status end";
}

///////////////////////////////////////////////////////////////////
///
///  InterlaceCasing
///
////////////////////////////////////////////////////////////////////

InterlaceCasing::InterlaceCasing(CasingSet *owner, EdgePtr edge, qreal width)
{
    this->owner = owner;
    s1 = nullptr;
    s2 = nullptr;

    wedge       = edge;
    this->width = width;
}

void InterlaceCasing::init()
{
    auto edge = wedge.lock();

    auto n1 = owner->getNeighbouringCasings(edge->v1);
    auto n2 = owner->getNeighbouringCasings(edge->v2);

    s1 = new InterlaceSide(this,SIDE_1,n1,edge->v1);
    s2 = new InterlaceSide(this,SIDE_2,n2,edge->v2);

    if (Sys::flags->flagged(USE_OUTLINE_INIT))
    {
        if (edge->isLine())
        {
            s1->createSide1(edge,width);
            s2->createSide2(edge,width);
        }
        else
        {
            createCurved();
        }
    }
    else
    {
        if (edge->isLine())
        {
            auto is1 = static_cast<InterlaceSide*>(s1);
            auto is2 = static_cast<InterlaceSide*>(s2);

            is1->createSide1_ilace(edge, width);
            is2->createSide2_ilace(edge, width);
        }
        else
        {
            createCurved();
        }
    }
}

InterlaceCasing::~InterlaceCasing()
{
    if (s1)
    {
        delete s1;
        s1 = nullptr;
    }
    if (s2)
    {
        delete s2;
        s2 = nullptr;
    }
}

void InterlaceCasing::createColors(QColor defaultColor)
{
    EdgePtr edge = wedge.lock();

    ThreadPtr thread = wthread.lock();
    if (thread)
        color = thread->color;
    else
        color = defaultColor;

    setShadowColor();
}

void InterlaceCasing::createBasic()
{
    EdgePtr edge = wedge.lock();
    qDebug() << "edge" << edge->casingIndex << edgeIndex;
    QLineF line  = edge->getLine();

    QLineF outer = Geo::calculateOuterParallelLine(line,width);
    s1->outer = outer.p1();
    s2->outer = outer.p2();

    QLineF inner = Geo::calculateInnerParallelLine(line,width);
    s1->inner = inner.p1();
    s2->inner = inner.p2();

    s1->mid = edge->v1->pt;
    s2->mid = edge->v2->pt;

    s1->created = true;
    s2->created = true;
}

void InterlaceCasing::setUnder(bool set)
{
    InterlaceSide * is1 = nullptr;
    is1 = static_cast<InterlaceSide*>(s1);
    Q_ASSERT(is1);
    InterlaceSide * is2 = nullptr;
    is2 = static_cast<InterlaceSide*>(s2);
    Q_ASSERT(is2);

    if (set)
    {
        is1->_under.set(Tristate::True);
        is2->_under.set(Tristate::False);
    }
    else
    {
        is1->_under.set(Tristate::False);
        is2->_under.set(Tristate::True);
    }
}

void InterlaceCasing::setPainterPath()
{
    path.clear();

    EdgePtr edge = wedge.lock();
    if (!edge) return;

    if (edge->getType() == EDGETYPE_LINE)
    {
        path.moveTo(s1->inner);
        path.lineTo(s1->mid);
        path.lineTo(s1->outer);
        path.lineTo(s2->outer);
        path.lineTo(s2->mid);
        path.lineTo(s2->inner);
        path.lineTo(s1->inner);
    }
    else if (edge->getType() == EDGETYPE_CURVE)
    {
        path.moveTo(s1->inner);
        path.lineTo(s1->mid);
        path.lineTo(s1->outer);

        ArcData ad1(QLineF(s1->outer,s2->outer),edge->getArcCenter(),edge->getCurveType());
        path.arcTo(ad1.rect(),ad1.start(),ad1.span());

        path.lineTo(s2->mid);
        path.lineTo(s2->inner);

        ArcData ad2(QLineF(s2->inner,s1->inner),edge->getArcCenter(),edge->getCurveType());
        path.arcTo(ad2.rect(),ad2.start(),-ad2.span());
    }
}

// The shadow is produces from the over-edge(s) and placed onto
// the under-edge and must fit onto the under-edge
// The taprats implemenation assumed bot oover and under were both straight
// lines.  Here over or under (or both) could be curved lines.
// But the simpliciication is that the shadow must fir into the the
// thick line (curved or stright)
void  InterlaceCasing::drawShadows(GeoGraphics * gg, qreal shadow) const
{
    auto edge = wedge.lock();

    // assumes only one side can have shadow
    InterlaceSide * is1 = nullptr;
    is1 = static_cast<InterlaceSide*>(s1);
    Q_ASSERT(is1);
    InterlaceSide * is2 = nullptr;
    is2 = static_cast<InterlaceSide*>(s2);
    Q_ASSERT(is2);

    if (is1->shadow)
    {
        if (edge->isLine())
        {
            QPolygonF shadowPts;
            shadowPts << (s1->inner + getShadowVector(s1->inner, s2->inner, shadow));
            shadowPts <<  s1->inner;
            shadowPts <<  s1->outer;
            shadowPts << (s1->outer + getShadowVector(s1->outer, s2->outer, shadow));
            gg->fillPolygon(shadowPts,shadowPen);
        }
        else
        {
            QPolygonF shadowPts;
            shadowPts <<  curveAlign(s1->inner + getShadowVector(s1->inner, s2->inner,shadow),EDGE_OUTER);
            shadowPts <<  s1->inner;
            shadowPts <<  s1->outer;
            shadowPts <<  curveAlign(s1->outer + getShadowVector(s1->outer, s2->outer, shadow),EDGE_INNER);

            EdgePoly ep(shadowPts);
            EdgeSet & base = ep.getBaseRW();
            EdgePtr e = base[0];
            e->chgangeToCurvedEdge(edge->getArcCenter(),CURVE_CONCAVE);
            e = base[2];
            e->chgangeToCurvedEdge(edge->getArcCenter(),CURVE_CONVEX);
            ep.compose();
            gg->fillEdgePoly(ep,shadowPen);
        }
    }
    else if (is2->shadow)
    {
        if (edge->isLine())
        {
            QPolygonF shadowPts;
            shadowPts << (s2->inner + getShadowVector(s2->inner, s1->inner, shadow));
            shadowPts <<  s2->inner;
            shadowPts <<  s2->outer;
            shadowPts << (s2->outer + getShadowVector(s2->outer, s1->outer, shadow));
            gg->fillPolygon(shadowPts,shadowPen);
        }
        else
        {
            QPolygonF shadowPts;
            shadowPts <<  curveAlign(s2->inner + getShadowVector(s2->inner, s1->inner, shadow),EDGE_OUTER);
            shadowPts <<  curveAlign(s2->outer + getShadowVector(s2->outer, s1->outer, shadow),EDGE_INNER);
            shadowPts <<  s2->outer;
            shadowPts <<  s2->inner;

            EdgePoly ep(shadowPts);
            EdgeSet & base = ep.getBaseRW();
            EdgePtr e = base[1];
            e->chgangeToCurvedEdge(edge->getArcCenter(),CURVE_CONVEX);
            e = base[3];
            e->chgangeToCurvedEdge(edge->getArcCenter(),CURVE_CONCAVE);
            ep.compose();
            gg->fillEdgePoly(ep,shadowPen);
        }
    }
}

QPointF InterlaceCasing::getShadowVector(QPointF from, QPointF to, qreal shadow) const
{
    QPointF dir = to - from;
    qreal magnitude = Geo::mag(dir);
    if ( shadow < magnitude )
    {
        dir *= (shadow / magnitude );
    }
    return dir;
}

QPointF InterlaceCasing::getShadowPt(QPointF from, QPointF to, qreal shadow) const
{
    QPointF rv      = to; // maximum
    qreal magnitude = QLineF(from,to).length();
    if ( shadow < magnitude )
    {
        rv = QLineF(from,to).pointAt(shadow / magnitude);
    }
    return rv;
}

QPointF InterlaceCasing::curveAlign(QPointF pt, eLSide lside) const
{
    auto edge = wedge.lock();
    QLineF line(edge->getArcCenter(),pt);
    if (lside == EDGE_INNER)
        line.setLength(edge->getRadius()+width);
    else
        line.setLength(edge->getRadius()-width);
    return line.p2();
}

QPointF InterlaceCasing::getCurvedShadowPt(QPointF from, QPointF to, qreal shadow, eLSide lside) const
{
    QPointF pt = getShadowPt(from,to,shadow);

    auto edge = wedge.lock();
    QLineF line(edge->getArcCenter(),pt);
    if (lside == EDGE_INNER)
        line.setLength(edge->getRadius()+width);
    else
        line.setLength(edge->getRadius()-width);
    return line.p2();
}

void InterlaceCasing::setGap(qreal gap)
{
    Q_ASSERT(gap > 0.0);

    EdgePtr edge = getEdge();

    auto ss1 = static_cast<InterlaceSide*>(s1);
    auto ss2 = static_cast<InterlaceSide*>(s2);

    ss1->setGap(edge,gap);
    ss2->setGap(edge,gap);
}

void InterlaceCasing::setShadowColor()
{
    float h;
    float s;
    float b;
    color.getHsvF(&h,&s,&b);

    shadowColor.setHsvF(h, s * 0.9, b * 0.8 );
    shadowPen = QPen(shadowColor);
}

QString InterlaceCasing::casingInfo(NeighboursPtr n, InterlaceCasingSet & casings)
{
    VertexPtr v = n->getVertex();

    InterlaceSide * is1 = nullptr;
    is1 = static_cast<InterlaceSide*>(s1);
    Q_ASSERT(is1);
    InterlaceSide * is2 = nullptr;
    is2 = static_cast<InterlaceSide*>(s2);
    Q_ASSERT(is2);

    QString info;
    for (const WeakEdgePtr & wedge : std::as_const(*n))
    {
        EdgePtr edge = wedge.lock();
        info += QString::number(edge->casingIndex);
        info += "-";
        auto other = casings.find(edge);
        if (other->s1->mid == v->pt)
        {
            info += "s1-";
            info += (is1->under()) ? "U" : "O";
        }
        else
        {
            info += "s2-";
            info += (is2->under()) ? "U" : "O";
        }
        info += " ";
    }
    return info;
}

bool InterlaceCasing::valid()
{
    bool rv = true;
    if (QLineF(s1->outer,s1->mid).length() != QLineF(s1->mid,s1->inner).length())
        rv = false;
    if (QLineF(s2->outer,s2->mid).length() != QLineF(s2->mid,s2->inner).length())
        rv = false;
    return rv;
}

void InterlaceCasing::useMap()
{
    s1->outer  = vs1above->pt;
    s1->mid    = vs1mid->pt;
    s1->inner  = vs1below->pt;
    s2->outer  = vs2above->pt;
    s2->mid    = vs2mid->pt;
    s2->inner  = vs2below->pt;
}

QPointF InterlaceCasing::getIsect(QLineF l1, eSide tside, QLineF l2, eSide oside)
{

    if (Geo::isColinear(l1,l2,Sys::NEAR_TOL))
    {
        QPointF p1 = (tside == SIDE_1) ? l1.p1() :  l1.p2();
        QPointF p2 = (oside == SIDE_1) ? l2.p1() :  l2.p2();
        QLineF l(p1,p2);
        //qDebug() << l.length();
        return l.center();
    }
    else
    {
        QPointF p;
#if 1
        l1.intersects(l2,&p);
#else
        QLineF::IntersectionType rv = l1.intersects(l2,&p);
        switch (rv)
        {
        case QLineF::NoIntersection:
            qDebug() << "NO ISECT";
            break;
        case QLineF::BoundedIntersection:
            qDebug() << "BOUNDED";
            break;
        case QLineF::UnboundedIntersection:
            qDebug() << "UN-BOUNDED";
            break;
        }
#endif
        return p;
    }
}

void InterlaceCasing::setIsectsForOvers(CasingSide *thisSide, CasingSide *otherSide)
{

    QPointF & outer = thisSide->outer;
    //QPointF & mid   = thisSide->mid;
    QPointF & inner = thisSide->inner;

    auto edge        = getEdge();
    bool isLine      = edge->isLine();

    Casing * otherCasing = otherSide->getParent();
    auto oedge       = otherCasing->getEdge();
    bool otherIsLine = oedge->isLine();

    eSide tside = thisSide->side;
    eSide oside = otherSide->side;

    //qDebug().noquote() <<  "setIsectsForOvers:        edge" << edge->casingIndex << sSide[tside] << "continuation" << otherCasing->edgeIndex << sSide[oside];

    if (isLine && otherIsLine)
    {
        if (Interlace::dbgDump2 & 0x400) qDebug() << "line - line";

        if (tside != oside)
        {
            outer = getIsect(outerLine(), tside,  otherCasing->outerLine(), oside);
            otherSide->outer = outer;

            inner = getIsect(innerLine(), tside, otherCasing->innerLine(), oside);
            otherSide->inner = inner;
        }
        else
        {
            outer = getIsect(outerLine(), tside, otherCasing->innerLine(), oside);
            otherSide->inner = outer;

            inner = getIsect(innerLine(), tside, otherCasing->outerLine(), oside);
            otherSide->outer = inner;
        }
    }
    else if (isLine && !otherIsLine)
    {
        if (Interlace::dbgDump2 & 0x400) qDebug() << "line - circle";

        if (tside != oside)
        {
            QPointF oldPt = otherSide->inner;
            if (Geo::getCircleLineIsect(otherCasing->innerCircle, innerLine(), oldPt, inner))
            {
                if (Interlace::dbgDump2 & 0x400) qDebug() << "also setting edge" << otherCasing->edgeIndex << "side" << oside << "inner";
                otherSide->inner = inner;
            }
            else if (Interlace::dbgDump2 & 0x400) qDebug() << "no isect inner";

            oldPt = otherSide->outer;
            if (Geo::getCircleLineIsect(otherCasing->outerCircle, outerLine(), oldPt, outer))
            {
                if (Interlace::dbgDump2 & 0x400) qDebug() << "also setting edge" << otherCasing->edgeIndex << "side" << oside << "outer";
                otherSide->outer = outer;
            }
            else if (Interlace::dbgDump2 & 0x400) qDebug() << "no isect outer";
        }
        else
        {
            QPointF oldPt = otherSide->outer;
            if (Geo::getCircleLineIsect(otherCasing->outerCircle, innerLine(), oldPt, inner))
            {
                if (Interlace::dbgDump2 & 0x400) qDebug() << "also setting edge" << otherCasing->edgeIndex << "side" << oside << "outer";
                otherSide->outer = inner;
            }
            else if (Interlace::dbgDump2 & 0x400) qDebug() << "no isect inner";

            oldPt = otherSide->outer;
            if (Geo::getCircleLineIsect(otherCasing->innerCircle, outerLine(), oldPt, outer))
            {
                if (Interlace::dbgDump2 & 0x400) qDebug() << "also setting edge" << otherCasing->edgeIndex << "side" << oside << "inner";
                otherSide->inner = outer;
            }
            else if (Interlace::dbgDump2 & 0x400) qDebug() << "no isect outer";
        }
    }
    else if (otherIsLine)
    {
        Q_ASSERT(!isLine);
        if (Interlace::dbgDump2 & 0x400) qDebug() <<  "circle - line";

        if (tside != oside)
        {
            QPointF oldPt = otherSide->inner;
            if (Geo::getCircleLineIsect(innerCircle, otherCasing->innerLine(), oldPt, inner))
            {
                if (Interlace::dbgDump2 & 0x400) qDebug() << "also setting edge" << otherCasing->edgeIndex << "side" << oside << "inner";
                otherSide->inner = inner;
            }
            else if (Interlace::dbgDump2 & 0x400) qDebug() << "no isect inner";

            oldPt = otherSide->outer;
            if (Geo::getCircleLineIsect(outerCircle, otherCasing->outerLine(), oldPt, outer))
            {
                if (Interlace::dbgDump2 & 0x400) qDebug() << "also setting edge" << otherCasing->edgeIndex << "side" << oside << "outer";
                otherSide->outer = outer;
            }
            else if (Interlace::dbgDump2 & 0x400) qDebug() << "no isect outer";
        }
        else
        {
            QPointF oldPt = otherSide->outer;
            if (Geo::getCircleLineIsect(innerCircle, otherCasing->outerLine(), oldPt, inner))
            {
                if (Interlace::dbgDump2 & 0x400) qDebug() << "also setting edge" << otherCasing->edgeIndex << "side" << oside << "outer";
                otherSide->outer = inner;
            }
            else qDebug() << "no isect inner";

            oldPt = otherSide->inner;
            if (Geo::getCircleLineIsect(outerCircle, otherCasing->innerLine(), oldPt, outer))
            {
                if (Interlace::dbgDump2 & 0x400) qDebug() << "also setting edge" << otherCasing->edgeIndex << "side" << oside << "inner";
                otherSide->inner = outer;
            }
            else if (Interlace::dbgDump2 & 0x400)  qDebug() << "no isect outer";
        }
    }
    else
    {
        Q_ASSERT(!isLine);
        Q_ASSERT(!otherIsLine);
        if (Interlace::dbgDump2 & 0x400) qDebug() << "circle - circle";

        if (Loose::Near(thisSide->getParent()->outerCircle.centre,otherSide->getParent()->outerCircle.centre,Sys::TOL))
        {
            ArcData this_ad_o(QLineF(s1->outer,s2->outer),edge->getArcCenter(),edge->getCurveType());
            //qDebug() << this_ad_o.start << QLineF(edge->getArcCenter(),s1->outer).angle() << this_ad_o.end << QLineF(edge->getArcCenter(),s2->outer).angle();
            Sys::debugMapCreate->insertDebugMark(s1->outer,"END-S1");
            Sys::debugMapCreate->insertDebugMark(s2->outer,"START-S2");

            ArcData this_ad_i(QLineF(s1->inner,s2->inner),edge->getArcCenter(),edge->getCurveType());
            //qDebug() << this_ad_i.start << QLineF(edge->getArcCenter(),s1->inner).angle() << this_ad_i.end << QLineF(edge->getArcCenter(),s2->inner).angle();
            Sys::debugMapCreate->insertDebugMark(s1->inner,"START-S1");
            Sys::debugMapCreate->insertDebugMark(s2->inner,"END-S2");

            EdgePtr oedge = otherCasing->getEdge();
            ArcData other_ad_o(QLineF(otherCasing->s1->outer,otherCasing->s2->outer), oedge->getArcCenter(),oedge->getCurveType());
            //qDebug() << other_ad_o.start << QLineF(oedge->getArcCenter(),otherCasing->s1->outer).angle() << other_ad_o.end << QLineF(oedge->getArcCenter(),otherCasing->s2->outer).angle();
            Sys::debugMapCreate->insertDebugMark(otherCasing->s1->outer,"OEND-S1");
            Sys::debugMapCreate->insertDebugMark(otherCasing->s2->outer,"OSTART-S2");

            ArcData other_ad_i(QLineF(otherCasing->s1->inner,otherCasing->s2->inner), oedge->getArcCenter(),oedge->getCurveType());
            //qDebug() << other_ad_i.start << QLineF(oedge->getArcCenter(),otherCasing->s1->inner).angle() << other_ad_i.end << QLineF(edge->getArcCenter(),otherCasing->s2->inner).angle();
            Sys::debugMapCreate->insertDebugMark(otherCasing->s1->inner,"OSTART-S1");
            Sys::debugMapCreate->insertDebugMark(otherCasing->s2->inner,"OEND-S2");

            QPointF pt;
            // tiles are clockwise
            // casings are anticlockwise qwuch that outers go from s1 to s2 and innters from s1 to s2
            // angles increas in anticlockside  direction
            if (tside == SIDE_2  && oside == SIDE_1)
            {
                pt = connectArcs(this_ad_o.getCenter(),this_ad_o.radius(),this_ad_o.end(),other_ad_o.start());
                outer = pt;
                otherSide->outer = pt;

                pt = connectArcs(this_ad_i.getCenter(),this_ad_i.radius(),this_ad_i.end(),other_ad_i.start());
                inner = pt;
                otherSide->inner = pt;
            }
            else if (tside == SIDE_1 && oside == SIDE_2)
            {
                pt = connectArcs(this_ad_o.getCenter(),this_ad_o.radius(),this_ad_o.start(),other_ad_o.end());
                outer = pt;
                otherSide->outer = pt;

                pt = connectArcs(this_ad_i.getCenter(),this_ad_i.radius(),this_ad_i.start(),other_ad_i.end());
                inner = pt;
                otherSide->inner = pt;
            }
            else if (tside == SIDE_2 && oside == SIDE_2)
            {
                Sys::appDebugBreak();
            }
            else
            {
                Q_ASSERT(tside == SIDE_1 && oside == SIDE_1);
                Sys::appDebugBreak();
            }
        }
        else if (tside != oside)
        {
            if (Geo::getCircleCircleIsect(innerCircle, otherCasing->innerCircle, inner, inner))
            {
                if (Interlace::dbgDump2 & 0x400) qDebug() << "also setting edge" << otherCasing->edgeIndex << "side" << oside << "inner";
                otherSide->inner = inner;
            }
            else if (Interlace::dbgDump2 & 0x400) qDebug() << "no isect inner";

            if (Geo::getCircleCircleIsect(outerCircle, otherCasing->outerCircle, outer, outer))
            {
                if (Interlace::dbgDump2 & 0x400) qDebug() << "also setting edge" << otherCasing->edgeIndex << "side" << oside << "outer";
                otherSide->outer = outer;
            }
            else if (Interlace::dbgDump2 & 0x400) qDebug() << "no isect outer ";
        }
        else
        {
            Q_ASSERT(tside == oside);
            if (Geo::getCircleCircleIsect(innerCircle, otherCasing->outerCircle, inner, inner))
            {
                if (Interlace::dbgDump2 & 0x400) qDebug() << "also setting edge" << otherCasing->edgeIndex << "side" << oside << "outer";
                otherSide->outer = inner;
            }
            else if (Interlace::dbgDump2 & 0x400) qDebug() << "no isect inner";

            if (Geo::getCircleCircleIsect(outerCircle, otherCasing->innerCircle, outer, outer))
            {
                if (Interlace::dbgDump2 & 0x400) qDebug() << "also setting edge" << otherCasing->edgeIndex << "side" << oside << "inner";
                otherSide->inner = outer;
            }
            else if (Interlace::dbgDump2 & 0x400) qDebug() << "no isect outer ";
        }
    }

}

QPointF InterlaceCasing::connectArcs(QPointF center, qreal radius, qreal from, qreal to)
{
    QPointF pt1 = Geo::calculateEndPoint(center,from,radius);
    QPointF pt2 = Geo::calculateEndPoint(center,to,radius);
    Sys::debugMapCreate->insertDebugMark(pt1,"FROM");
    Sys::debugMapCreate->insertDebugMark(pt2,"TO");

    qreal angle = (from + to)/ 2.0;
    qDebug() << "angles" << from << to << angle;

    QPointF pt = Geo::calculateEndPoint(center,angle,radius);
    Sys::debugMapCreate->insertDebugMark(pt,"X");
    return pt;
}

bool InterlaceCasing::validate()
{
    QPointF isect;
    auto is = outerLine().intersects(innerLine(),&isect);
    if (is == QLineF::BoundedIntersection)
        return false;   // not valid

    if (!s1->validate())
        return false;

    if (!s2->validate())
        return false;

    if (getEdge()->isCurve())
    {
        if (!validateCurves())
            return  false;
    }

    return true;    // valid
}

void InterlaceCasing::dumpI(InterlaceCasingSet &casings)
{
    NeighboursPtr n = s1->cneighbours;
    qDebug().noquote() << "edge" << edgeIndex << "side1" << "edges:" << casingInfo(n,casings);

    n = s2->cneighbours;
    qDebug().noquote() << "edge" << edgeIndex << "side2" << "edges:" << casingInfo(n,casings);
}


