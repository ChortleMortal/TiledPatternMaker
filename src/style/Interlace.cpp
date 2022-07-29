#include "style/interlace.h"
#include "geometry/arcdata.h"
#include "geometry/map.h"
#include "geometry/neighbours.h"
#include "geometry/point.h"
#include "geometry/transform.h"
#include "geometry/vertex.h"
#include "misc/geo_graphics.h"
#include "misc/utilities.h"
#include "style/outline.h"


using std::make_shared;

////////////////////////////////////////////////////////////////////////////
//
// Interlace.java
//
// Probably the most important rendering style from an historical point
// of view.  RenderInterlace assigns an over-under rule to the edges
// of the map and renders a weave the follows that assignment.  Getting
// the over-under rule is conceptually simple but difficult in practice,
// especially since we want to have some robust against degenerate maps
// being produced by other parts of the program (*sigh*).
//
// Basically, if a diagram can be interlaced, you can just choose an
// over-under relationship at one vertex and propagate it to all other
// vertices using a depth-first search.
//
// Drawing the interlacing takes a bit of trig, but it's doable.  It's
// just a pain when crossing edges don't cross in a perfect X.  I
// might get this wrong.

Interlace::Interlace(PrototypePtr proto) : Thick(proto)
{
    outline_width         = 0.03;
    join_style            = Qt::BevelJoin;
    cap_style             = Qt::SquareCap;
    gap                   = 0.0;
    shadow                = 0.05;
    includeTipVertices    = false;
    interlace_start_under = false;
}

Interlace::Interlace(StylePtr other) : Thick(other)
{
    std::shared_ptr<Interlace> intl = std::dynamic_pointer_cast<Interlace>(other);
    if (intl)
    {
        gap                   = intl->gap;
        shadow                = intl->shadow;
        includeTipVertices    = intl->includeTipVertices;
        interlace_start_under = intl->interlace_start_under;
    }
    else
    {
        std::shared_ptr<Thick> thick  = std::dynamic_pointer_cast<Thick>(other);
        if (!thick)
        {
            outline_width  = 0.03;
            join_style     = Qt::BevelJoin;
            cap_style      = Qt::SquareCap;
        }
        gap                   = 0.0;
        shadow                = 0.05;
        includeTipVertices    = false;
        interlace_start_under = false;
    }
}

Interlace:: ~Interlace()
{
#ifdef EXPLICIT_DESTRUCTOR
    qDebug() << "deleting interlace";
    pts.clear();
    todo.clear();
    threads.clear();
#endif
}
void Interlace::draw(GeoGraphics * gg)
{
    if (!isVisible())
    {
        return;
    }

    if (segments.size() == 0)
    {
        return;
    }

    for (auto & seg : segments)
    {
        QColor color = seg.c;
#if 0
        QPolygonF poly = seg.getPoly();
        gg->fillPolygon(poly,color);
#else
        QPainterPath path = seg.getPainterPath();
        gg->fillPath(path,color);
#endif
    }

    if ( shadow > 0.0)
    {
        for (auto& seg : qAsConst(segments))
        {
            QColor color = seg.c;
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
            float h;
            float s;
            float b;
#else
            qreal h;
            qreal s;
            qreal b;
#endif
            color.getHsvF(&h,&s,&b);
            QColor c;
            c.setHsvF(h, s * 0.9, b * 0.8 );

            if (seg.v1.shadow)
            {
                QPolygonF shadowPts1;
                shadowPts1 << (seg.v1.above + getShadowVector(seg.v1.above, seg.v2.below));
                shadowPts1 <<  seg.v1.above;
                shadowPts1 <<  seg.v1.below;
                shadowPts1 << (seg.v1.below + getShadowVector(seg.v1.below, seg.v2.above));
                gg->fillPolygon(shadowPts1,c);
            }
            if (seg.v2.shadow)
            {
                QPolygonF shadowPts2;
                shadowPts2 << (seg.v2.below + getShadowVector(seg.v2.below, seg.v1.above));
                shadowPts2 <<  seg.v2.below;
                shadowPts2 <<  seg.v2.above;
                shadowPts2 << (seg.v2.above + getShadowVector(seg.v2.above, seg.v1.below));
                gg->fillPolygon(shadowPts2,c);
            }
        }
    }

    if (drawOutline != OUTLINE_NONE)
    {
        QPen pen;
        if (drawOutline == OUTLINE_SET)
        {
            pen = QPen(outline_color,Transform::scalex(gg->getTransform() * outline_width * 0.5));
        }
        else
        {
            pen = QPen(Qt::black,1);
        }
        pen.setJoinStyle(join_style);
        pen.setCapStyle(cap_style);

        for(auto & seg : qAsConst(segments))
        {
            if (seg.type == EDGETYPE_LINE)
            {
                gg->drawLine(seg.v2.above, seg.v1.below, pen);
                gg->drawLine(seg.v2.below, seg.v1.above, pen);
            }
            else if (seg.type == EDGETYPE_CURVE)
            {
                gg->drawArc(seg.v2.above, seg.v1.below,  seg.arcCenter, seg.convex, pen);    // inside
                gg->drawArc(seg.v2.below, seg.v1.above,  seg.arcCenter, seg.convex, pen);    // outside
            }
            else if (seg.type == EDGETYPE_CHORD)
            {
                gg->drawChord(seg.v2.above, seg.v1.below, seg.arcCenter, seg.convex, pen);  // inside
                gg->drawChord(seg.v2.below, seg.v1.above, seg.arcCenter, seg.convex, pen);  // outside
            }
        }
    }
}

void Interlace::resetStyleRepresentation()
{
    Thick::resetStyleRepresentation();
    segments.clear();
    map.reset();
}

void Interlace::createStyleRepresentation()
{
    if (segments.size())
    {
        return;
    }

    map = getMap();

    if (colors.size() > 1)
    {
        threads.createThreads(map);
        threads.assignColors(colors);
    }

    assignInterlacing();

    // Given the interlacing assignment created above, we can
    // use the beefy getPoints routine to extract the graphics
    // of the interlacing.

    for (auto & edge  : map->getEdges())
    {
        Segment seg;
        ThreadPtr thread;
        if ((colors.size() > 1) && (thread = edge->thread.lock()))
        {
            seg.c = thread->color;
        }
        else
        {
            seg.c = colors.getFirstColor().color;
        }

        seg.type = edge->getType();
        if (edge->isCurve())
        {
            seg.convex    = edge->isConvex();
            seg.arcCenter = edge->getArcCenter();
        }

        seg.v1.getPoints(edge, edge->v1, edge->v2, width, gap, map,  edge->v1_under);
        seg.v2.getPoints(edge, edge->v2, edge->v1, width, gap, map, !edge->v1_under);
        segments.push_back(seg);
    }

    annotateEdges(map);

    map->verify();
}

void Interlace::assignInterlacing()
{
    for (auto & edge : map->getEdges())
    {
        edge->visited = false;
    }

    for (auto & vert : map->getVertices())
    {
        vert->visited = false;
    }

    // Stack of edge to be processed.
    todo.clear();

    for(auto & edge : map->getEdges())
    {
        if (!edge->visited )
        {
            edge->v1_under = !interlace_start_under;
            todo.push(edge);
            buildFrom();
            //map->dumpMap(false);
        }
    }
}

// Propagate the over-under relation from an edge to its incident vertices.
void Interlace::buildFrom()
{
    //qDebug() << "Interlace::buildFrom";

    while (!todo.empty())
    {
        EdgePtr edge = todo.pop();

        VertexPtr v1 = edge->v1;
        VertexPtr v2 = edge->v2;

        if (!v1->visited)
        {
            propagate(v1, edge, edge->v1_under);
        }
        if (!v2->visited)
        {
            propagate(v2, edge, !edge->v1_under);
        }
    }
}

// Propagate the over-under relationship from a vertices to its
// adjacent edges.  The relationship is encapsulated in the
// "edge_under_at_vert" variable, which says whether the
// edge passed in is in the under state at this vertex.
// The whole trick is to manage how neighbours receive modifications
// of edge_under_at_vert.

void Interlace::propagate(VertexPtr vertex, EdgePtr edge, bool edge_under_at_vert)
{
    vertex->visited = true;

    NeighboursPtr neighbours = map->getNeighbours(vertex);

    int nn = neighbours->numNeighbours();
    //qInfo() << "Interlace Edge Count" << nn;

    if (nn == 1)
    {
        if ( includeTipVertices)
        {
            EdgePtr oe = neighbours->getNeighbour(0);

            if( !oe->visited )
            {
                // With a bend, we don't want to change the underness
                // of the edge we're propagating to.
                if( oe->v1 == vertex)
                {
                    // The new edge starts at the current vertex.
                    oe->v1_under = !edge_under_at_vert;
                }
                else
                {
                    // The new edge ends at the current vertex.
                    oe->v1_under = edge_under_at_vert;
                }
                oe->visited = true;
                todo.push( oe );
            }
        }
    }
    else if (nn == 2)
    {
        BeforeAndAfter  ba  = neighbours->getBeforeAndAfter(edge);
        EdgePtr oe          = ba.before;

        if( !oe->visited )
        {
            // With a bend, we don't want to change the underness
            // of the edge we're propagating to.
            if( oe->v1 == vertex)
            {
                // The new edge starts at the current vertex.
                oe->v1_under = !edge_under_at_vert;
            }
            else
            {
                // The new edge ends at the current vertex.
                oe->v1_under = edge_under_at_vert;
            }
            oe->visited = true;
            todo.push( oe );
        }
    }
    else
    {
        if (nn == 3 || nn > 4)
            qInfo() << "Interlace Edge Count" << nn;

        QVector<EdgePtr> ns;
        int index    = 0;
        int edge_idx = -1;  // index of edge in ns

        for (auto & wedge : *neighbours)
        {
            EdgePtr edge2 = wedge.lock();
            ns << edge2;
            if (edge2 == edge)
            {
                edge_idx = index;
            }
            index++;
        }

        // this assumes edges are sorted by angle, so the +1 edge has the reversed
        // v1_under, and the +2 edge is the colinear continuation of the first edge, etc.
        // Seems reasonable for nn=4.
        // I am wondering if when nn=3 this works if +1 is from left but +1 could be colinear
        for (int idx = 1; idx < nn; ++idx )
        {
            int cur = (edge_idx + idx) % nn;
            EdgePtr oe = ns[cur];
            Q_ASSERT(oe);

            if (!oe->visited)
            {
                if( oe->v1 == vertex)
                {
                    oe->v1_under = !edge_under_at_vert;
                }
                else
                {
                    oe->v1_under = edge_under_at_vert;
                }
                oe->visited = true;
                todo.push(oe);
            }
            edge_under_at_vert = !edge_under_at_vert;
        }
    }
}

QPointF Interlace::getShadowVector(QPointF from, QPointF to)
{
    QPointF dir = to - from;
    qreal magnitude = Point::mag(dir);
    if ( shadow < magnitude )
    {
        dir *= (shadow / magnitude );
    }
    return dir;
}

void Piece::getPoints(EdgePtr  edge, VertexPtr from, VertexPtr to, qreal width, qreal gap, MapPtr map, bool from_under)
{
    //bool from_under = (edge->v1 == from ) == edge->v1_under;  // methinks ugly code

    QPointF pfrom = from->pt;
    QPointF pto   = to->pt;

    // Four cases:
    //  - cap
    //  - bend
    //  - interlace over
    //  - interlace under

    NeighboursPtr toNeighbours = map->getNeighbours(to);

    int nn = toNeighbours->numNeighbours();
    if (nn == 1)
    {
        // cap
        QPointF dir = pto - pfrom;
        Point::normalizeD(dir);
        dir *= width;
        QPointF perp = Point::perp(dir);

        below  = pto - perp;
        below += dir;
        v      = pto + dir;
        above  = pto + perp;
        above += dir;
    }
    else if (nn == 2)
    {
        // bend
        BelowAndAbove jps = Outline::getPoints(map, edge, from, to, width);
        below = jps.below;
        v     = pto;
        above = jps.above;
    }
    else
    {
        if( from_under )
        {
            // interlace over
            QVector<EdgePtr> ns;
            int index    = 0;
            int edge_idx = -1;
            for (auto & wedge : *toNeighbours)
            {
                EdgePtr nedge = wedge.lock();
                ns << nedge;
                if (nedge == edge)
                {
                    edge_idx = index;
                }
                index++;
            }

            int nidx = (edge_idx + 2) % nn;

            QPointF op = ns[nidx]->getOtherP(to);

            below = Outline::getJoinPoint(pto, pfrom, op, width);
            if ( below.isNull() )
            {
                QPointF perp = pto - pfrom;
                Point::perpD(perp);
                Point::normalizeD(perp);
                perp *= width;
                below = pto - perp ;
            }

            v   = pto;
            above = Point::convexSum(below, pto, 2.0);
        }
        else
        {
            // interlace under

            // This is the hard case, fraught with pitfalls for
            // the imprudent (i.e., me).  I think what I've got
            // now does a reasonable job on well-behaved maps
            // and doesn't dump core on badly-behaved ones.

            BeforeAndAfter ba = toNeighbours->getBeforeAndAfter(edge);
            QPointF before_pt = ba.before->getOtherP(to);
            QPointF after_pt  = ba.after->getOtherP(to);

            below = Outline::getJoinPoint(pto, pfrom,     after_pt, width );
            above = Outline::getJoinPoint(pto, before_pt, pfrom,    width );
            v     = Outline::getJoinPoint(pto, before_pt, after_pt, width );

            QPointF dir = pto - pfrom ;
            Point::normalizeD(dir);
            QPointF perp = Point::perp(dir);
            perp        *= width;

            if (below.isNull())
            {
                below = pto - perp;
            }
            if (above.isNull())
            {
                above = pto + perp;
            }
            if (v.isNull())
            {
                QPointF ab = after_pt - before_pt ;
                Point::normalizeD(ab);
                Point::perpD(ab);
                ab *= width;
                v   = pto - ab ;
            }

            // FIXMECSK -- The gap size isn't consistent since
            // it's based on subtracting gap scaled unit vectors.
            // Scale gap by 1/sin(theta) to compensate for
            // angle with neighbours and get a _perpendicular_ gap.

            if ( gap > 0.0 )
            {
                below -= (dir * capGap(below, pfrom, gap));
                above -= (dir * capGap(above, pfrom, gap));
                v     -= (dir * capGap(v  ,   pfrom, gap));
            }

            shadow = true;
        }
    }
}

qreal Piece::capGap( QPointF p, QPointF base, qreal gap )
{
    qreal max_gap = Point::dist(p, base );
    return (gap < max_gap) ? gap : max_gap;
}

QPolygonF Segment::getPoly()
{
    QPolygonF p;
    p << v1.below <<  v1.v << v1.above <<  v2.below <<  v2.v << v2.above; // same as interlace
    //p << v2.below << v2.v << v2.above << v1.below << v1.v << v1.above;  // same as for outline
    if (!Utils::isClockwise(p))
        qWarning() << "Poly is CCW";
    return p;
}

QPainterPath Segment::getPainterPath()
{
    QPainterPath path;

    if (type == EDGETYPE_LINE)
    {
        path.moveTo(v2.below);
        path.lineTo(v2.v);
        path.lineTo(v2.above);
        path.lineTo(v1.below);
        path.lineTo(v1.v);
        path.lineTo(v1.above);
        path.lineTo(v2.below);
    }
    else if (type == EDGETYPE_CURVE)
    {
        path.moveTo(v2.below);
        path.lineTo(v2.v);
        path.lineTo(v2.above);

        ArcData ad1(v2.above,v1.below,arcCenter,convex);
        path.arcTo(ad1.rect,ad1.start,ad1.span);

        path.lineTo(v1.v);
        path.lineTo(v1.above);

        ArcData ad2(v1.above,v2.below,arcCenter,convex);
        path.arcTo(ad2.rect,ad2.start,-ad2.span);
    }
    else if (type == EDGETYPE_CHORD)
    {
        qWarning("BelowAndAboveEdge - unexpected EDGETYPE_CHORD");
    }
    return path;
}

bool Segment::valid()
{
    bool rv = true;
    if (QLineF(v1.above,v1.v).length() != QLineF(v1.v,v1.below).length())
        rv = false;
    if (QLineF(v2.above,v2.v).length() != QLineF(v2.v,v2.below).length())
        rv = false;
    return rv;
}
