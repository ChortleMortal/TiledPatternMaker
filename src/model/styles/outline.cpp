#include <QDebug>
#include "model/styles/outline.h"
#include "sys/geometry/map.h"
#include "sys/geometry/neighbours.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/transform.h"
#include "sys/geometry/vertex.h"
#include "gui/viewers/geo_graphics.h"

//#define DEBUG_NO_CURVES
//#define DEBUG_BAE
//#define DEBUG_BAE2

////////////////////////////////////////////////////////////////////////////
//
// Outline.java
//
// The simplest non-trivial rendering style.  Outline just uses
// some trig to fatten up the map's edges, also drawing a line-based
// outline for the resulting fat figure.
//
// The same code that computes the draw elements for Outline can
// be used by other "fat" styles, such as Emboss.

Outline::Outline(const ProtoPtr &proto) : Thick (proto)
{
    outline_width   = 0.03;
    join_style      = Qt::BevelJoin;
    cap_style       = Qt::SquareCap;
}

Outline::Outline(const StylePtr & other) : Thick(other)
{
    std::shared_ptr<Thick> thick  = std::dynamic_pointer_cast<Thick>(other);
    if (!thick)
    {
        outline_width   = 0.03;
        join_style      = Qt::BevelJoin;
        cap_style       = Qt::SquareCap;
    }
}

Outline::~Outline()
{
#ifdef EXPLICIT_DESTRUCTOR
    qDebug() << "deleting outline";
    pts4.clear();
#endif
}

void Outline::draw(GeoGraphics *gg)
{
    if (!isVisible())
    {
        return;
    }

#ifdef DEBUG_NO_CURVES
    for (auto & bae : std::as_const(pts4))
    {
        QColor color  = colors.getNextColor().color;

        QPolygonF poly = bae.getPoly();
        gg->fillPolygon(poly,color);

        if (draw_outline )
        {
            QPen pen(outline_color,Transform::scalex(gg->getTransform() * outline_width * 0.5));
            pen.setJoinStyle(join_style);
            pen.setCapStyle(cap_style);
            gg->drawLine(bae.v2.above, bae.v1.below, pen);
            gg->drawLine(bae.v1.above, bae.v2.below, pen);
        }
    }
#else
    for (auto & bae : pts4)
    {
        QColor color  = colors.getNextTPColor().color;
        QPen pen(color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        QPainterPath path = bae.getPainterPath();
        gg->fillPath(path,pen);

        if (drawOutline != OUTLINE_NONE)
        {
            QPen pen;
            if (drawOutline == OUTLINE_SET)
            {
                pen = QPen(outline_color,Transform::scalex(gg->getTransform() * outline_width * 0.5));
            }
            else
            {
                Q_ASSERT(drawOutline == OUTLINE_DEFAULT);
                pen = QPen(Qt::black,1);
            }
            pen.setJoinStyle(join_style);
            pen.setCapStyle(cap_style);

            if (bae.type == EDGETYPE_LINE)
            {
                gg->drawLine(bae.v2.above, bae.v1.below, pen);
                gg->drawLine(bae.v1.above, bae.v2.below, pen);
            }
            else if (bae.type == EDGETYPE_CURVE)
            {
                gg->drawArc(bae.v1.below, bae.v2.above, bae.arcCenter, bae.convex, pen);    // inside
                gg->drawArc(bae.v1.above, bae.v2.below, bae.arcCenter, bae.convex, pen);    // outside
            }
            else if (bae.type == EDGETYPE_CHORD)
            {
                gg->drawChord(bae.v1.below, bae.v2.above, bae.arcCenter, bae.convex, pen);  // inside
                gg->drawChord(bae.v1.above, bae.v2.below, bae.arcCenter, bae.convex, pen);  // outside
            }
        }
    }
#endif
}

void Outline::resetStyleRepresentation()
{
    pts4.clear();
}

void Outline::createStyleRepresentation()
{
    qDebug() << __FUNCTION__;

    if (pts4.size())
    {
        return;
    }

    MapPtr map = prototype->getProtoMap();

#ifdef DEBUG_BAE
    qDebug() << "gen outline bae";
    uint iEdge = 0;
    uint iGood = 0;
    uint iBad  = 0;
#endif

    for (auto & edge : std::as_const(map->getEdges()))
    {
#ifdef DEBUG_BAE
        qDebug().noquote() << iEdge << edge->info();
#endif
        VertexPtr v1 = edge->v1;
        VertexPtr v2 = edge->v2;

        BelowAndAbove bae_to   = getPoints(map, edge, v1, v2, width);
        BelowAndAbove bae_from = getPoints(map, edge, v2, v1, width);

        BelowAndAboveEdge bae;

        bae.v2.below = bae_to.below;
        bae.v2.v     = v2->pt;
        bae.v2.above = bae_to.above;

        bae.v1.below = bae_from.below;
        bae.v1.v     = v1->pt;
        bae.v1.above = bae_from.above;

        bae.type     = edge->getType();
        if (bae.type == EDGETYPE_CURVE || bae.type == EDGETYPE_CHORD)
        {
            bae.convex    = edge->isConvex();
            bae.arcCenter = edge->getArcCenter();
        }

#ifdef DEBUG_BAE
        bae.dumpV(iEdge);
        if (bae.validate(iEdge++))
        {
            iGood++;
            pts4 << bae;
        }
        else
        {
            iBad++;
        }
#else
        pts4 << bae;
#endif
    }

#ifdef DEBUG_BAE
    qDebug() << __FUNCTION__ << "- end - good =" << iGood << "bad =" << iBad;
#endif
}

// Look at a given edge and construct a plausible set of points
// to draw at the edge's 'to' vertex.  Call this twice to get the
// complete outline of the hexagon to draw for this edge.
BelowAndAbove Outline::getPoints(const MapPtr & map, const EdgePtr & edge, const VertexPtr & fromV, const VertexPtr & toV, qreal qwidth)
{
    QPointF fromP = fromV->pt;
    QPointF toP   = toV->pt;

    QPointF dir   = toP - fromP;
    Geo::normalizeD(dir);
    QPointF perp = Geo::perp(dir);

    BelowAndAbove ret;

    NeighboursPtr nto = map->getNeighbours(toV);
    int nn = nto->numNeighbours();
#ifdef DEBUG_BAE2
    qDebug() << "nn = " << nn;
#endif
    if( nn == 1 )
    {
        ret.below = toP - (perp * qwidth);
        ret.above = toP + (perp * qwidth);
    }
    else if( nn == 2 )
    {
        BeforeAndAfter ba = nto->getBeforeAndAfter(edge);
        QPointF       pov = ba.before->getOtherP(toV);
        QPointF        jp = getJoinPoint(toP, fromP, pov, qwidth);

        if (jp.isNull())
        {
            ret.below = toP - (perp * qwidth);
            ret.above = toP + (perp * qwidth);
        }
        else
        {
            ret.below = jp;
            ret.above = Geo::convexSum(jp, toP, 2.0);
        }
    }
    else
    {
        BeforeAndAfter ba = nto->getBeforeAndAfter(edge);
        QPointF before_pt = ba.before->getOtherP(toV);
        QPointF after_pt  = ba.after->getOtherP(toV);

        ret.below = getJoinPoint(toP, fromP, after_pt, qwidth);
        if (ret.below.isNull())
        {
            ret.below = toP - (perp * qwidth);
        }
        ret.above = getJoinPoint(toP, before_pt, fromP, qwidth);
        if (ret.above.isNull())
        {
            ret.above = toP + (perp * qwidth);
        }
    }

#ifdef DEBUG_BAE2
    if (!ret.validate())
    {
        qWarning() << "Bad below and above" << ret.below << ret.above;
        //qCritical("Bad BelowAndAbove");
    }
#endif
    return ret;
}

// Do a mitered join of the two fat lines (a la postscript, for example).
// The join point on the other side of the joint can be computed by
// reflecting the point returned by this function through the joint.
QPointF  Outline::getJoinPoint(QPointF joint, QPointF from, QPointF to, qreal qwidth )
{
    qreal theta = Geo::sweep(joint, from, to);

    if (Loose::zero(theta) ||qAbs(theta - M_PI) < 1e-7)
    {
        return QPointF(0,0);
    }

    QPointF d1 = joint - from;
    Geo::normalizeD(d1);
    QPointF d2 = joint - to;
    Geo::normalizeD(d2);

    qreal l   = qwidth / qSin(theta);
    qreal isx = joint.x() - ((d1.x() + d2.x()) * l);
    qreal isy = joint.y() - ((d1.y() + d2.y()) * l);
    return QPointF(isx, isy);
}
