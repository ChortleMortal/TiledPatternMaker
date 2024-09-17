#include "model/styles/interlace.h"
#include "sys/geometry/arcdata.h"
#include "sys/geometry/map.h"
#include "sys/geometry/neighbours.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/transform.h"
#include "sys/geometry/vertex.h"
#include "gui/viewers/geo_graphics.h"
#include "model/styles/outline.h"


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

Interlace::Interlace(ProtoPtr proto) : Thick(proto)
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

    QPen pen(Qt::black, 1, Qt::SolidLine, cap_style, join_style);

    for (const Segment & seg : std::as_const(segments))
    {
        seg.draw(gg,pen);
    }

    if ( shadow > 0.0)
    {
        for (Segment seg : std::as_const(segments))
        {
            QPen & spen = seg.getShadowPen();
            spen.setJoinStyle(join_style);
            spen.setCapStyle(cap_style);
            seg.drawShadows(gg,shadow);
        }
    }

    if (drawOutline != OUTLINE_NONE)
    {
        QPen pen(Qt::black,1, Qt::SolidLine, cap_style, join_style);      // OUTLINE_DEFAULT;
        if (drawOutline == OUTLINE_SET)
        {
            pen = QPen(outline_color,Transform::scalex(gg->getTransform() * outline_width * 0.5));
        }
        pen.setJoinStyle(join_style);
        pen.setCapStyle(cap_style);

        for (const Segment & seg : segments)
        {
            seg.drawOutline(gg,pen);
        }
    }
}

void Interlace::resetStyleRepresentation()
{
    Thick::resetStyleRepresentation();
    segments.clear();
}

void Interlace::createStyleRepresentation()
{
    if (segments.size())
    {
        return;
    }

    MapPtr map = prototype->getProtoMap();

    if (colors.size() > 1)
    {
        threads.createThreads(map.get());
        threads.assignColors(colors);
    }

    assignInterlacing(map.get());

    // Given the interlacing assignment created above, we can
    // use the beefy getPoints routine to extract the graphics
    // of the interlacing.

    for (const auto & edge  : std::as_const(map->getEdges()))
    {
        ThreadPtr thread;
        QColor color;
        if ((colors.size() > 1) && (thread = edge->thread.lock()))
        {
            color = thread->color;
        }
        else
        {
            color = colors.getFirstTPColor().color;
        }

        Segment seg(edge->getType(),color);
        if (edge->isCurve())
        {
            seg.setCurve(edge->isConvex(),edge->getArcCenter());
        }
        seg.v1.getPoints(edge, edge->v1, edge->v2, width, gap, map,  edge->v1_under);
        seg.v2.getPoints(edge, edge->v2, edge->v1, width, gap, map, !edge->v1_under);
        seg.setPainterPath();
        seg.setShadowColor();
        segments.push_back(seg);
    }

    annotateEdges(map);

    map->verify();
}

void Interlace::assignInterlacing(Map * map)
{
    for (const auto & edge :std::as_const(map->getEdges()))
    {
        edge->visited = false;
    }

    for (const auto & vert : std::as_const(map->getVertices()))
    {
        vert->visited = false;
    }

    // Stack of edge to be processed.
    todo.clear();

    for (const auto & edge : std::as_const(map->getEdges()))
    {
        if (!edge->visited )
        {
            edge->v1_under = !interlace_start_under;
            todo.push(edge);
            buildFrom(map);
            //map->dumpMap(false);
        }
    }
}

// Propagate the over-under relation from an edge to its incident vertices.
void Interlace::buildFrom(Map * map)
{
    //qDebug() << "Interlace::buildFrom";

    while (!todo.empty())
    {
        EdgePtr edge = todo.pop();

        if (!edge->v1->visited)
        {
            propagate(map, edge->v1, edge, edge->v1_under);
        }
        if (!edge->v2->visited)
        {
            propagate(map, edge->v2, edge, !edge->v1_under);
        }
    }
}

// Propagate the over-under relationship from a vertices to its
// adjacent edges.  The relationship is encapsulated in the
// "edge_under_at_vert" variable, which says whether the
// edge passed in is in the under state at this vertex.
// The whole trick is to manage how neighbours receive modifications
// of edge_under_at_vert.

void Interlace::propagate(Map * map, VertexPtr & vertex, EdgePtr & edge, bool edge_under_at_vert)
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
    else if (nn == 2 || nn == 3)
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
        QVector<EdgePtr> ns;
        int index    = 0;
        int edge_idx = -1;  // index of edge in ns

        for (auto & wedge : std::as_const(*neighbours))
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

///////////////////////////////////////////////////////////////////
///
///  Segment
///
////////////////////////////////////////////////////////////////////

Segment::Segment(eEdgeType etype, QColor ecolor)
{
    type  = etype;
    color = ecolor;
}

void Segment::setCurve(bool isConvex, QPointF center)
{
    convex = isConvex;
    arcCenter = center;
}

QPolygonF Segment::getPoly()
{
    QPolygonF p;
    p << v1.below <<  v1.v << v1.above <<  v2.below <<  v2.v << v2.above; // same as interlace
    //p << v2.below << v2.v << v2.above << v1.below << v1.v << v1.above;  // same as for outline
    if (!Geo::isClockwise(p))
        qWarning() << "Poly is CCW";
    return p;
}

void Segment::setPainterPath()
{
    path.clear();

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

        ArcData ad1;
        ad1.create(v2.above,v1.below,arcCenter,convex);
        path.arcTo(ad1.rect,ad1.start,ad1.span());

        path.lineTo(v1.v);
        path.lineTo(v1.above);

        ArcData ad2;
        ad2.create(v1.above,v2.below,arcCenter,convex);
        path.arcTo(ad2.rect,ad2.start,-ad2.span());
    }
    else if (type == EDGETYPE_CHORD)
    {
        qWarning("BelowAndAboveEdge - unexpected EDGETYPE_CHORD");
    }
}

void Segment::draw(GeoGraphics * gg, QPen & pen) const
{
    pen.setColor(color);
    gg->fillPath(path,pen);
}

void  Segment::drawOutline(GeoGraphics * gg, QPen & pen) const
{
    if (type == EDGETYPE_LINE)
    {
        gg->drawLine(v2.above, v1.below, pen);
        gg->drawLine(v2.below, v1.above, pen);
    }
    else if (type == EDGETYPE_CURVE)
    {
        gg->drawArc(v2.above, v1.below,  arcCenter, convex, pen);    // inside
        gg->drawArc(v2.below, v1.above,  arcCenter, convex, pen);    // outside
    }
    else if (type == EDGETYPE_CHORD)
    {
        gg->drawChord(v2.above, v1.below, arcCenter, convex, pen);  // inside
        gg->drawChord(v2.below, v1.above, arcCenter, convex, pen);  // outside
    }
}

void  Segment::drawShadows(GeoGraphics * gg, qreal shadow) const
{
    if (v1.shadow)
    {
        QPolygonF shadowPts1;
        shadowPts1 << (v1.above + getShadowVector(v1.above, v2.below, shadow));
        shadowPts1 <<  v1.above;
        shadowPts1 <<  v1.below;
        shadowPts1 << (v1.below + getShadowVector(v1.below, v2.above, shadow));
        gg->fillPolygon(shadowPts1,shadowPen);
    }

    if (v2.shadow)
    {
        QPolygonF shadowPts2;
        shadowPts2 << (v2.below + getShadowVector(v2.below, v1.above, shadow));
        shadowPts2 <<  v2.below;
        shadowPts2 <<  v2.above;
        shadowPts2 << (v2.above + getShadowVector(v2.above, v1.below, shadow));
        gg->fillPolygon(shadowPts2,shadowPen);
    }
}

QPointF Segment::getShadowVector(QPointF from, QPointF to, qreal shadow) const
{
    QPointF dir = to - from;
    qreal magnitude = Geo::mag(dir);
    if ( shadow < magnitude )
    {
        dir *= (shadow / magnitude );
    }
    return dir;
}

void Segment::setShadowColor()
{
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
    shadowColor.setHsvF(h, s * 0.9, b * 0.8 );

    shadowPen = QPen(shadowColor);
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

///////////////////////////////////////////////////////////////////
///
///  Piece
///
////////////////////////////////////////////////////////////////////

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
        Geo::normalizeD(dir);
        dir *= width;
        QPointF perp = Geo::perp(dir);

        below  = pto - perp;
        below += dir;
        v      = pto + dir;
        above  = pto + perp;
        above += dir;
    }
    else if (nn == 2 || nn ==3)
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
            for (auto & wedge : std::as_const(*toNeighbours))
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
                Geo::perpD(perp);
                Geo::normalizeD(perp);
                perp *= width;
                below = pto - perp ;
            }

            v   = pto;
            above = Geo::convexSum(below, pto, 2.0);
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
            Geo::normalizeD(dir);
            QPointF perp = Geo::perp(dir);
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
                Geo::normalizeD(ab);
                Geo::perpD(ab);
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
    qreal max_gap = Geo::dist(p, base );
    return (gap < max_gap) ? gap : max_gap;
}

