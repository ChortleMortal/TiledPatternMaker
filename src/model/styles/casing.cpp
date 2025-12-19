#include "gui/viewers/debug_view.h"
#include "gui/viewers/geo_graphics.h"
#include "model/styles/casing.h"
#include "model/styles/casing_neighbours.h"
#include "sys/geometry/debug_map.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/transform.h"

Casing::Casing()
{}

void Casing::createCurved()
{
    auto edge = wedge.lock();
    Q_ASSERT(edge);

    s1->mid     = edge->v1->pt;
    s2->mid     = edge->v2->pt;

    QPointF c   = edge->getArcCenter();
    if (edge->isConvex())
    {
        outerCircle = Circle(c,edge->getRadius() + width);
        innerCircle = Circle(c,edge->getRadius() - width);
    }
    else
    {
        innerCircle = Circle(c,edge->getRadius() + width);
        outerCircle = Circle(c,edge->getRadius() - width);
    }

    qreal ro    = outerCircle.radius;
    qreal ri    = innerCircle.radius;

    QLineF line;

    line = QLineF(c,s1->mid);
    line.setLength(ro);
    s1->outer   = line.p2();
    line.setLength(ri);
    s1->inner   = line.p2();

    line = QLineF(c,s2->mid);
    line.setLength(ro);
    s2->outer   = line.p2();
    line.setLength(ri);
    s2->inner   = line.p2();

    s1->created =  true;
    s2->created =  true;
}

void Casing::alignCurvedEdgeSide1(CasingSet &casings)
{
    auto edge = wedge.lock();
    if (!edge)
        return;

    //qDebug() << "aligning curved edge" << edgeIndex << "side 1";
    Q_ASSERT(edge->isCurve());;

    CasingPtr other;
    QPointF        * otherPoint;

    // side1
    CNeighboursPtr n1  = s1->cneighbours;
    BeforeAndAfter ba1 = n1->getBeforeAndAfter(edge);
    EdgePtr before1    = ba1.before;
    EdgePtr after1     = ba1.after;
    //qDebug() << "side 1 before" << before1->casingIndex << "after" << after1->casingIndex;

    // side1 - inner
    other  = casings.find(after1);
    QPointF p;
    bool rv = false;
    if (s1->mid == other->s2->mid)
    {
        otherPoint = &other->s2->inner;
        rv = getCircleIsect(innerCircle,*other,true,s1->mid,p);
    }
    else if (s1->mid == other->s1->mid)
    {
        otherPoint = &other->s1->outer;
        rv = getCircleIsect(innerCircle,*other,false,s1->mid,p);
    }

    if (rv)
    {
        s1->inner   = p;
        *otherPoint = p;
    }
    else
        qWarning() << "s1 inner - no isect";

    // side 1 outer
    other  = casings.find(before1);
    rv = false;
    if (s1->mid == other->s2->mid)
    {
        otherPoint = &other->s2->outer;
        rv = getCircleIsect(outerCircle,*other,false,s1->mid,p);
    }
    else if (s1->mid == other->s1->mid)
    {
        otherPoint = &other->s1->inner;
        rv = getCircleIsect(outerCircle,*other,true,s1->mid,p);
    }

    if (rv)
    {
        s1->outer   = p;
        *otherPoint = p;
    }
    else
        qWarning() << "s1 outer - no isect";
}

void Casing::alignCurvedEdgeSide2(CasingSet &casings)
{
    auto edge = wedge.lock();
    if (!edge)
        return;

    //qDebug() << "aligning curved edge" << edgeIndex << "side 2";
    Q_ASSERT(edge->isCurve());;

    CasingPtr other;
    QPointF        * otherPoint;

    // side2
    auto n2        = s2->cneighbours;
    auto ba2       = n2->getBeforeAndAfter(edge);
    auto before2   = ba2.before;
    auto after2    = ba2.after;

    //qDebug() << "side 2 before" << before2->casingIndex << "after" << after2->casingIndex;

    // side 2 inner
    other  = casings.find(before2);
    QPointF p;
    bool rv = false;
    if (s2->mid == other->s1->mid)
    {
        otherPoint = &other->s1->inner;
        rv= getCircleIsect(innerCircle,*other, true,s2->mid,p);

    }
    else if (s2->mid == other->s2->mid)
    {
        otherPoint = &other->s2->outer;
        rv = getCircleIsect(innerCircle,*other, false,s2->mid,p);
    }

    if (rv)
    {
        s2->inner   = p;
        *otherPoint = p;
    }
    else
        qWarning() << "s2 inner - no isect";

    // side 2 outer
    other  = casings.find(after2);
    rv = false;
    if (s2->mid == other->s1->mid)
    {
        otherPoint = &other->s1->outer;
        rv = getCircleIsect(outerCircle,*other,false,s2->mid,p);
    }
    else if (s2->mid == other->s2->mid)
    {
        otherPoint = &other->s2->inner;
        rv = getCircleIsect(outerCircle,*other,true,s2->mid,p);
    }

    if (rv)
    {
        s2->outer   = p;
        *otherPoint = p;
    }
    else
        qWarning() << "s2 inner - no isect";
}

void Casing::fillCasing(GeoGraphics * gg, QPen & pen) const
{
    gg->fillPath(path,pen);
}

void Casing::drawOutline(GeoGraphics *gg, QPen & pen) const
{
    auto edge = wedge.lock();
    if (!edge) return;

    if (!Sys::flags->flagged(NO_OUTLINE))
    {
        if (edge->getType() == EDGETYPE_LINE)
        {
            gg->drawLine(s1->inner, s2->inner, pen);
            gg->drawLine(s1->outer, s2->outer, pen);
        }
        else if (edge->getType() == EDGETYPE_CURVE)
        {
            gg->drawArc(s1->inner, s2->inner, edge->getArcCenter(), edge->getCurveType(), pen, false);    // inner
            gg->drawArc(s1->outer, s2->outer, edge->getArcCenter(), edge->getCurveType(), pen, true);    // outer
        }
    }
}

void Casing::addToMap(MapPtr map)
{
    EdgePtr edge = wedge.lock();
    if (!edge) return;

    VertexPtr vs1above = map->insertVertex(s1->outer);
    VertexPtr vs1mid   = map->insertVertex(s1->mid);
    VertexPtr vs1below = map->insertVertex(s1->inner);
    VertexPtr vs2above = map->insertVertex(s2->outer);
    VertexPtr vs2mid   = map->insertVertex(s2->mid);
    VertexPtr vs2below = map->insertVertex(s2->inner);

    map->insertEdge(vs2above,vs2mid);
    map->insertEdge(vs2mid,vs2below);
    if (edge->isCurve())
        map->insertEdge(vs1below,vs2below,edge->getArcCenter(),edge->getCurveType());
    else
        map->insertEdge(vs1below,vs2below);

    map->insertEdge(vs1below,vs1mid);
    map->insertEdge(vs1mid,vs1above);
    if (edge->isCurve())
        map->insertEdge(vs1above,vs2above,edge->getArcCenter(),edge->getCurveType());
    else
        map->insertEdge(vs1above,vs2above);
}

CNeighboursPtr Casing::getNeighbours(VertexPtr v)
{
    if (s1->wv.lock() == v)
    {
        return s1->cneighbours;
    }
    else
    {
        Q_ASSERT(s2->wv.lock() == v);
        return s2->cneighbours;
    }
}

bool Casing::getCircleIsect(const Circle & circle,  Casing &other, bool otherInner, const QPointF & oldPt, QPointF & newPt)
{
    //qDebug() << "intersect circle" << edgeIndex << "with" << other.edgeIndex << ( otherInner ? "inner" : "outer");
    bool otherCurve = other.getEdge()->isCurve();
    if (otherCurve)
    {
        if (otherInner)
            return Geo::getCircleCircleIsect(circle, other.innerCircle, oldPt, newPt);
        else
            return Geo::getCircleCircleIsect(circle, other.outerCircle, oldPt, newPt);
    }
    else
    {
        Q_ASSERT(other.getEdge()->isLine());
        if (otherInner)
            return Geo::getCircleLineIsect( circle, other.innerLine(), oldPt, newPt);
        else
            return Geo::getCircleLineIsect(circle, other.outerLine(), oldPt, newPt);
    }
}

QPolygonF Casing::getPoly() const
{
    QPolygonF p;
    p << s2->outer << s2->mid << s2->inner << s1->inner << s1->mid << s1->outer;
    if (!Geo::isClockwise(p)) qWarning() << "Poly is CCW";
    return p;
}

void Casing::debugDraw(QColor color, qreal width)
{
    EdgePtr edge = wedge.lock();
    if (!edge)
        return;

    //qDebug() << "edge" << edgeIndex;
    QString ei = QString::number(edgeIndex);

    if (Sys::flags->getIndexEnable())
    {
        int index = Sys::flags->getDbgIndex();
        if (index != edgeIndex)
            return;
    }

    if (Sys::flags->flagged(EXCLUDE_ODDS))
    {
        if (edgeIndex & 1)
            return;
    }

    if (Sys::flags->flagged(EXCLUDE_EVENS))
    {
        if (!(edgeIndex & 1))
            return;
    }

    QColor outerColor = color;
    QColor innerColor = color;

    if (Sys::flags->flagged(DRAW_INNER_OUTER))
    {
        outerColor = Qt::red;
        innerColor = Qt::blue;
    }

    if (Sys::flags->flagged(DRAW_MID_LINE))
    {
        if (edge->isCurve())
        {
            Sys::debugMapPaint->insertDebugCurve(edge->v1->pt,edge->v2->pt,edge->getCurveType(),edge->getArcCenter(),Qt::blue,true);
        }
        else
        {
            Sys::debugMapPaint->insertDebugLine(edge->v1->pt,edge->v2->pt,Qt::blue,2);
        }
    }

    if (edge->isLine())
    {
        if (Sys::flags->flagged(DRAW_LINE_EDGES))
        {
            Sys::debugMapPaint->insertDebugLine(s2->inner,  s2->mid,    color);
            Sys::debugMapPaint->insertDebugLine(s2->mid,    s2->outer,  color);
            Sys::debugMapPaint->insertDebugLine(s2->outer,  s1->outer,  outerColor);
            Sys::debugMapPaint->insertDebugLine(s1->outer,  s1->mid,    color);
            Sys::debugMapPaint->insertDebugLine(s1->mid,    s1->inner,  color);
            Sys::debugMapPaint->insertDebugLine(s1->inner,  s2->inner,  innerColor);
        }
    }
    else
    {
        if (Sys::flags->flagged(DRAW_CURVE_EDGES))
        {
            Sys::debugMapPaint->insertDebugLine(s1->inner,  s1->mid,    color);
            Sys::debugMapPaint->insertDebugLine(s1->mid,    s1->outer,  color);
            Sys::debugMapPaint->insertDebugCurve(s1->outer, s2->outer,  edge->getCurveType(),edge->getArcCenter(),outerColor,true);
            Sys::debugMapPaint->insertDebugLine(s2->outer,  s2->mid,    color);
            Sys::debugMapPaint->insertDebugLine(s2->mid,    s2->inner,  color);
            Sys::debugMapPaint->insertDebugCurve(s1->inner, s2->inner,  edge->getCurveType(),edge->getArcCenter(),innerColor,false);
        }
    }

    if (Sys::flags->flagged(SIDE_1_PTS))
    {
        s1->debugPoints();
    }

    if (Sys::flags->flagged(MARK_1_PTS))
    {
        Sys::debugMapPaint->insertDebugMark(s1->outer,ei+"s1outer",color);
        Sys::debugMapPaint->insertDebugMark(s1->inner,ei+"s1inner",color);
    }

    if (Sys::flags->flagged(SIDE_2_PTS))
    {
        s2->debugPoints();
    }

    if (Sys::flags->flagged(MARK_2_PTS))
    {
        Sys::debugMapPaint->insertDebugMark(s2->outer,ei+"s2outer",color);
        Sys::debugMapPaint->insertDebugMark(s2->inner,ei+"s2inner",color);
    }

    if (edge->isCurve() && Sys::flags->flagged(DRAW_RADII))
    {
        qreal radius = edge->getRadius();
        Sys::debugMapPaint->insertDebugCircle(edge->getArcCenter(), Qt::blue,  radius - width);
        Sys::debugMapPaint->insertDebugCircle(edge->getArcCenter(), Qt::green, radius);
        Sys::debugMapPaint->insertDebugCircle(edge->getArcCenter(), Qt::red,   radius + width);
        Sys::debugMapPaint->insertDebugMark(edge->getArcCenter(),"");

        QPointF pt = Geo::calculateEndPoint(edge->getArcCenter(),0,radius-width);
        Sys::debugMapPaint->insertDebugMark(pt,"zero",Qt::blue);

        QPointF pt1 = Geo::calculateEndPoint(edge->getArcCenter(),0,radius+width);
        QPointF pt2 = Geo::calculateEndPoint(edge->getArcCenter(),90.0,radius+width);
        QPointF pt3 = Geo::calculateEndPoint(edge->getArcCenter(),180.0,radius+width);
        QPointF pt4 = Geo::calculateEndPoint(edge->getArcCenter(),270.0,radius+width);
        Sys::debugMapPaint->insertDebugMark(pt1,"zero",Qt::red);
        Sys::debugMapPaint->insertDebugMark(pt2,"90",Qt::red);
        Sys::debugMapPaint->insertDebugMark(pt3,"180",Qt::red);
        Sys::debugMapPaint->insertDebugMark(pt4,"270",Qt::red);
    }
}

void Casing::dump()
{
    qDebug() << edgeIndex << "s1" << s1->outer << s1->mid << s1->inner;
    qDebug() << edgeIndex << "s2" << s2->outer << s2->mid << s2->inner;
}

CasingData Casing::getCasingData()
{
    CasingData cd;
    cd.outer[0] = s1->outer;
    cd.mid[0]   = s1->mid;
    cd.inner[0] = s1->inner;

    cd.outer[1] = s2->outer;
    cd.mid[1]   = s2->mid;
    cd.inner[1] = s2->inner;

    cd.edgeIndex = edgeIndex;

    return cd;
}

bool Casing::validateCurves()
{
    bool rv = true;

    QPointF c = getEdge()->getArcCenter();

    qreal m1 = Geo::dist(c,s1->mid);
    qreal m2 = Geo::dist(c,s2->mid);
    if (!Loose::equals(m2,m2))
    {
        qWarning("unequal mids");
        rv = false;
    }

    qreal ri  = innerCircle.radius;
    qreal ro  = outerCircle.radius;

    if (getEdge()->isConvex())
    {
        if (!Loose::equals(m1-width,ri))
        {
            qWarning("inner radius error");
            rv = false;
        }

        qreal ro = outerCircle.radius;
        if (!Loose::equals(m1+width,ro))
        {
            qWarning("outer radius error");
            rv = false;
        }
    }
    else
    {
        if (!Loose::equals(m1+width,ri))
        {
            qWarning("inner radius error");
            rv = false;
        }

        qreal ro = outerCircle.radius;
        if (!Loose::equals(m1-width,ro))
        {
            qWarning("outer radius error");
            rv = false;

        }
    }

    qreal  l1 = Geo::dist(c, s1->inner);
    if (!Loose::equals(ri,l1))
    {
        qWarning("s1 inner error");
        return false;
    }

    qreal  l2 = Geo::dist(c, s2->inner);
    if (!Loose::equals(ri,l2))
    {
        qWarning("s2 inner error");
        return false;
    }

    qreal l3 = Geo::dist(c, s1->outer);
    if (!Loose::equals(ro,l3))
    {
        qWarning("s1 outer error");
        return false;
    }

    qreal l4 = Geo::dist(c, s2->outer);
    if (!Loose::equals(ro,l4))
    {
        qWarning("s2 outer error");
        return false;
    }

    return rv;
}
