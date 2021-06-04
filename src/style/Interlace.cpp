/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "style/interlace.h"
#include "geometry/map.h"
#include "geometry/point.h"
#include "style/outline.h"
#include <QPainter>

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
    gap    = 0.0;
    shadow = 0.05;
    includeTipVertices = false;
}

Interlace::Interlace(StylePtr other) : Thick(other)
{
    shared_ptr<Interlace> intl = std::dynamic_pointer_cast<Interlace>(other);
    if (intl)
    {
        gap     = intl->gap;
        shadow  = intl->shadow;
        includeTipVertices = intl->includeTipVertices;
    }
    else
    {
        gap     = 0.0;
        shadow  = 0.05;
        includeTipVertices = false;
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


// Style overrides.

void Interlace::resetStyleRepresentation()
{
    Thick::resetStyleRepresentation();
    pts.clear();
}

void Interlace::createStyleRepresentation()
{
    if (pts.size())
    {
        return;
    }

    MapPtr map = getMap();

    if (colors.size() > 1)
    {
        threads.findThreads(map);
        threads.assignColors(colors);
    }

    assignInterlacing();

    // Given the interlacing assignment created above, we can
    // use the beefy getPoints routine to extract the graphics
    // of the interlacing.

    for (auto edge  : map->getEdges())
    {
        segment seg;
        if (colors.size() > 1)
        {
            seg.c = edge->thread->color;
        }
        else
        {
            seg.c = colors.getFirstColor().color;
        }

        getPoints(edge, edge->v1, edge->v2, &seg.A);
        getPoints(edge, edge->v2, edge->v1, &seg.B);
        pts.push_back(seg);
    }

    annotateEdges(map);

    map->verify();
}

// Private magic to make it all happen.

void Interlace::getPoints(EdgePtr  edge, VertexPtr from, VertexPtr to, piece * p)
{
    MapPtr map = getMap();

    bool from_under = (edge->v1 == from ) == edge->start_under;  // methinks ugly code

    QPointF pfrom = from->pt;
    QPointF pto   = to->pt;

    // Four cases:
    //  - cap
    //  - bend
    //  - interlace over
    //  - interlace under

    NeighboursPtr nto = map->getBuiltNeighbours(to);

    int nn = nto->numNeighbours();
    if (nn == 1)
    {
        // cap
        QPointF dir = pto - pfrom;
        Point::normalizeD(dir);
        dir *= width;
        QPointF perp = Point::perp(dir);

        p->below  = pto - perp;
        p->below += dir;
        p->cen    = pto + dir;
        p->above  = pto + perp;
        p->above += dir;
    }
    else if (nn == 2)
    {
        // bend
        BelowAndAbove jps = Outline::getPoints(map, edge, from, to, width);
        p->below = jps.below;
        p->cen   = pto;
        p->above = jps.above;
    }
    else
    {
        if( from_under )
        {
            // interlace over
            QVector<EdgePtr> ns;
            int index    = 0;
            int edge_idx = -1;
            std::vector<WeakEdgePtr> * wedges = dynamic_cast<std::vector<WeakEdgePtr>*>(nto.get());
            for (auto pos = wedges->begin(); pos != wedges->end(); pos++)
            {
                WeakEdgePtr wedge = *pos;
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

            p->below = Outline::getJoinPoint( pto, pfrom, op, width );
            if (p-> below.isNull() )
            {
                QPointF perp = pto - pfrom;
                Point::perpD(perp);
                Point::normalizeD(perp);
                perp *= width;
                p->below = pto - perp ;
            }

            p->cen = pto;
            p->above = Point::convexSum(p->below, pto, 2.0);
        }
        else
        {
            // interlace under

            // This is the hard case, fraught with pitfalls for
            // the imprudent (i.e., me).  I think what I've got
            // now does a reasonable job on well-behaved maps
            // and doesn't dump core on badly-behaved ones.

            BeforeAndAfter ba = nto->getBeforeAndAfter(edge);
            QPointF before_pt = ba.before->getOtherP(to);
            QPointF after_pt  = ba.after->getOtherP(to);

            p->below = Outline::getJoinPoint(pto, pfrom,     after_pt, width );
            p->above = Outline::getJoinPoint(pto, before_pt, pfrom,    width );
            p->cen   = Outline::getJoinPoint(pto, before_pt, after_pt, width );

            QPointF dir = pto - pfrom ;
            Point::normalizeD(dir);
            QPointF perp = Point::perp(dir);
            perp        *= width;

            if (p->below.isNull())
            {
                p->below = pto - perp;
            }
            if (p->above.isNull())
            {
                p->above = pto + perp;
            }
            if (p->cen.isNull())
            {
                QPointF ab = after_pt - before_pt ;
                Point::normalizeD(ab);
                Point::perpD(ab);
                ab *= width;
                p->cen = pto - ab ;
            }

            // FIXMECSK -- The gap size isn't consistent since
            // it's based on subtracting gap scaled unit vectors.
            // Scale gap by 1/sin(theta) to compensate for
            // angle with neighbours and get a _perpendicular_ gap.

            if ( gap > 0.0 )
            {
                p->below -= (dir * capGap(p->below, pfrom, gap));
                p->above -= (dir * capGap(p->above, pfrom, gap));
                p->cen   -= (dir * capGap(p->cen,   pfrom, gap));
            }

            p->shadow = true;
        }
    }
}

void Interlace::initializeMap()
{
    MapPtr map = getMap();
    for (auto edge : map->getEdges())
    {
        edge->visited     = false;
        edge->start_under = false;
    }

    for (auto vert : map->getVertices())
    {
        vert->visited = false;
    }
}

void Interlace::assignInterlacing()
{
    initializeMap();

    // Stack of edge to be processed.
    todo.clear();

    MapPtr map = getMap();
    for(auto edge : map->getEdges())
    {
        if (!edge->visited )
        {
            edge->start_under = true;
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
            propagate(v1, edge, edge->start_under);
        }
        if (!v2->visited)
        {
            propagate(v2, edge, !edge->start_under);
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

    MapPtr map = getMap();

    NeighboursPtr n = map->getBuiltNeighbours(vertex);

    int nn = n->numNeighbours();
    if (nn == 2)
    {
        BeforeAndAfter  ba  = n->getBeforeAndAfter(edge);
        EdgePtr oe          = ba.before;

        if( !oe->visited )
        {
            // With a bend, we don't want to change the underness
            // of the edge we're propagating to.
            if( oe->v1 == vertex)
            {
                // The new edge starts at the current vertex.
                oe->start_under = !edge_under_at_vert;
            }
            else
            {
                // The new edge ends at the current vertex.
                oe->start_under = edge_under_at_vert;
            }
            oe->visited = true;
            todo.push( oe );
        }
    }
    else if (nn == 1 && includeTipVertices)
    {
        EdgePtr oe = n->getNeighbour(0);

        if( !oe->visited )
        {
            // With a bend, we don't want to change the underness
            // of the edge we're propagating to.
            if( oe->v1 == vertex)
            {
                // The new edge starts at the current vertex.
                oe->start_under = !edge_under_at_vert;
            }
            else
            {
                // The new edge ends at the current vertex.
                oe->start_under = edge_under_at_vert;
            }
            oe->visited = true;
            todo.push( oe );
        }
    }
    else if (nn > 2)
    {
        //Q_ASSERT(nn == 4);
        QVector<EdgePtr> ns;
        int index = 0;
        int edge_idx = -1;
        std::vector<WeakEdgePtr> * wedges = dynamic_cast<std::vector<WeakEdgePtr>*>(n.get());
        for (auto pos = wedges->begin(); pos != wedges->end(); pos++)
        {
            WeakEdgePtr wedge = *pos;
            EdgePtr edge2 = wedge.lock();
            ns << edge2;
            if (edge2 == edge)
            {
                edge_idx = index;
            }
            index++;
        }

        bool cur_under = edge_under_at_vert;

        for( int idx = 1; idx < nn; ++idx )
        {
            int cur = (edge_idx + idx) % nn;
            EdgePtr oe = ns[cur];
            Q_ASSERT(oe);

            if (!oe->visited)
            {
                if( oe->v1 == vertex)
                {
                    oe->start_under = !cur_under;
                }
                else
                {
                    oe->start_under = cur_under;
                }
                oe->visited = true;
                todo.push(oe);
            }

            cur_under = !cur_under;
        }
    }
}



void Interlace::draw(GeoGraphics * gg)
{
    if (!isVisible())
    {
        return;
    }

    if (pts.size() == 0)
    {
        return;
    }

    for (auto seg : qAsConst(pts))
    {
        QColor c = seg.c;

        QPolygonF poly = seg.toPoly();
        gg->fillPolygon(poly,c);
    }

    if ( shadow > 0.0)
    {
        for (auto& seg : qAsConst(pts))
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

            if (seg.A.shadow)
            {
                QPolygonF shadowPts1;
                shadowPts1 << (seg.A.above + getShadowVector(seg.A.above, seg.B.below));
                shadowPts1 <<  seg.A.above;
                shadowPts1 <<  seg.A.below;
                shadowPts1 << (seg.A.below + getShadowVector(seg.A.below, seg.B.above));
                gg->fillPolygon(shadowPts1,c);
            }
            if (seg.B.shadow)
            {
                QPolygonF shadowPts2;
                shadowPts2 << (seg.B.below + getShadowVector(seg.B.below, seg.A.above));
                shadowPts2 <<  seg.B.below;
                shadowPts2 <<  seg.B.above;
                shadowPts2 << (seg.B.above + getShadowVector(seg.B.above, seg.A.below));
                gg->fillPolygon(shadowPts2,c);
            }
        }
    }

    if ( draw_outline )
    {
        QPen pen(Qt::black);
        for(auto& seg : qAsConst(pts))
        {
            gg->drawLine(seg.A.above, seg.B.below, pen);
            gg->drawLine(seg.B.above, seg.A.below, pen);
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

qreal Interlace::capGap( QPointF p, QPointF base, qreal gap )
{
    qreal max_gap = Point::dist(p, base );
    return (gap < max_gap) ? gap : max_gap;
}

QPolygonF segment::toPoly()
{
    QPolygonF p;
    p <<  A.below <<  A.cen <<  A.above <<  B.below <<  B.cen <<  B.above;
    return p;
}


