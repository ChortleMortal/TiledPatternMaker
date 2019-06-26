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

#include "style/Interlace.h"
#include "geometry/Point.h"
#include "style/Outline.h"
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

Interlace::Interlace(PrototypePtr proto, PolyPtr bounds ) : Thick(proto,bounds)
{
    gap    = 0.0;
    shadow = 0.05;
    includeTipVertices = false;
}

Interlace::Interlace(const Style *other ) : Thick(other)
{
    const Interlace * intl = dynamic_cast<const Interlace *>(other);
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
    shadows.clear();
#endif
}

// Style overrides.
void Interlace::resetStyleRepresentation()
{
    Thick::resetStyleRepresentation();
    pts.erase(pts.begin(),pts.end());
    shadows.erase(shadows.begin(),shadows.end());
}

void Interlace::createStyleRepresentation()
{
    if ( pts.size() || shadows.size())
    {
        return;
    }

    setupStyleMap();

    assignInterlacing(getReadOnlyMap());

    // Given the interlacing assignment created above, we can
    // use the beefy getPoints routine to extract the graphics
    // of the interlacing.

    pts.resize(getReadOnlyMap()->numEdges() * 6 );
    shadows.resize(getReadOnlyMap()->numEdges() * 2);
    int index = 0;

    for(auto e = getReadOnlyMap()->getEdges()->begin(); e != getReadOnlyMap()->getEdges()->end(); e++)
    {
        EdgePtr edge = *e;
        VertexPtr v1 = edge->getV1();
        VertexPtr v2 = edge->getV2();

        getPoints( edge, v1, v2, width, gap, pts, index * 6,     shadows, index * 2 );
        getPoints( edge, v2, v1, width, gap, pts, index * 6 + 3, shadows, index * 2 + 1 );

        index += 1;
    }

    finalizeMap(getReadOnlyMap());
    qDebug() << "interlace pts="  << index;
    getReadOnlyMap()->verify("interlace",false,true);
}

void Interlace::draw(GeoGraphics * gg)
{
    if (!isVisible())
    {
        return;
    }

    if( pts.size() != 0 )
    {
        gg->pushAndCompose(*getLayerTransform());
        gg->setColor(colors.getNextColor().color);
        for( int idx = 0; idx < pts.size(); idx += 6 )
        {
            QPolygonF poly;
            for (int i=0; i< 6; i++)
            {
                poly << pts[idx + i];
            }
            gg->drawPolygon(poly, true );
        }

        if ( shadow > 0.0 && shadows.size() != 0 )
        {
            qreal h,s,b;
            QColor color = colors.getNextColor().color;
            color.getHsvF(&h,&s,&b);
            QColor c;
            c.setHsvF(h, s * 0.9, b * 0.8 );
            gg->setColor(c);

            for( int idx = 0; idx < pts.size(); idx += 6 )
            {
                //qDebug() << "idx=" << idx;
                if ( shadows[idx / 3] )
                {
                    QPolygonF shadowPts1;
                    shadowPts1 << (pts[ idx + 2 ] + getShadowVector( idx + 2, idx + 3 ) );
                    shadowPts1 <<  pts[ idx + 2 ];
                    shadowPts1 <<  pts[ idx + 0 ];
                    shadowPts1 << (pts[ idx + 0 ] + getShadowVector( idx + 0, idx + 5 ) );
                    gg->drawPolygon(shadowPts1,true);
                }
                if ( shadows[idx / 3 + 1] )
                {
                    QPolygonF shadowPts2;
                    shadowPts2 << (pts[ idx + 3 ] + getShadowVector( idx + 3, idx + 2 ) );
                    shadowPts2 <<  pts[ idx + 3 ];
                    shadowPts2 <<  pts[ idx + 5 ];
                    shadowPts2 << (pts[ idx + 5 ] + getShadowVector( idx + 5, idx + 0 ) );
                    gg->drawPolygon(shadowPts2,true);
                }
            }
        }

        if ( draw_outline )
        {
            gg->setColor(Qt::black);
            for( int idx = 0; idx < pts.size(); idx += 6 )
            {
                gg->drawLine( pts[ idx + 2 ], pts[ idx + 3 ] );
                gg->drawLine( pts[ idx + 5 ], pts[ idx ] );
            }
        }
        gg->pop();
    }
}

qreal Interlace::getGap()
{
    return gap;
}

void Interlace::setGap(qreal gap )
{
    this->gap = gap;
    resetStyleRepresentation();
}

qreal Interlace::getShadow()
{
    return shadow;
}

void Interlace::setShadow(qreal shadow )
{
    this->shadow = shadow;
    resetStyleRepresentation();
}


void Interlace::setIncludeTipVertices(bool include)
{
    includeTipVertices = include;
    resetStyleRepresentation();
}

// Private magic to make it all happen.

QPointF Interlace::getShadowVector( int fromIndex, int toIndex )
{
    QPointF dir = pts[ toIndex ] - pts[ fromIndex ];
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

void Interlace::getPoints(
        EdgePtr   edge,
        VertexPtr from,
        VertexPtr to,
        qreal     width,
        qreal     gap,
        QVector<QPointF> & pts,
        int ptsIndex,
        QVector<bool> & shadows,
        int shadowsIndex )
{
    bool from_under = (edge->getV1() == from ) == edge->getInterlaceInfo()->start_under;  // methinks ugly code

    QPointF pfrom = from->getPosition();
    QPointF pto   = to->getPosition();

    // Four cases:
    //     - cap
    //  - bend
    //  - interlace over
    //  - interlace under

    QPointF below;
    QPointF cen;
    QPointF above;

    int nn = to->numNeighbours();

    if( nn == 1 )
    {
        // cap
        QPointF dir = pto - pfrom;
        Point::normalizeD(dir);
        dir *=width;
        QPointF perp = Point::perp(dir);

        below = pto - perp;
        below += dir;
        cen = pto +dir;
        above = pto + perp ;
        above += dir ;
    }
    else if( nn == 2 )
    {
        // bend
        QVector<QPointF> jps = Outline::getPoints( edge, from, to, width );
        below = jps[ 0 ];
        cen = pto;
        above = jps[ 1 ];
    }
    else
    {
        if( from_under )
        {
            // interlace over
            QVector<EdgePtr> ns;
            int index = 0;
            int edge_idx = -1;

            QVector<EdgePtr> edges = to->getNeighbours();
            for (auto it = edges.begin(); it != edges.end(); it++)
            {
                ns << *it;
                if (*it == edge)
                {
                    edge_idx = index;
                }
                index++;
            }

            int nidx = (edge_idx + 2) % nn;

            QPointF op = ns[ nidx ]->getOther( to->getPosition() )->getPosition();

            below = Outline::getJoinPoint( pto, pfrom, op, width );

            if( below.isNull() )
            {
                QPointF perp = pto - pfrom;
                Point::perpD(perp);
                Point::normalizeD(perp);
                perp *= width;
                below = pto - perp ;
            }

            cen = pto;

            above = Point::convexSum(below, pto, 2.0 );
        }
        else
        {
            // interlace under

            // This is the hard case, fraught with pitfalls for
            // the imprudent (i.e., me).  I think what I've got
            // now does a reasonable job on well-behaved maps
            // and doesn't dump core on badly-behaved ones.

            QVector<EdgePtr> ba = to->getBeforeAndAfter( edge );
            QPointF before_pt = ba[0]->getOther( to->getPosition() )->getPosition();
            QPointF after_pt  = ba[1]->getOther( to->getPosition() )->getPosition();

            below = Outline::getJoinPoint(pto, pfrom, after_pt, width );
            above = Outline::getJoinPoint(pto, before_pt, pfrom, width );
            cen   = Outline::getJoinPoint(pto, before_pt, after_pt, width );

            QPointF dir = pto - pfrom ;
            Point::normalizeD(dir);

            if( below.isNull())
            {
                QPointF perp = Point::perp(dir);
                perp *= width;
                below = pto - perp;
            }
            if( above.isNull())
            {
                QPointF perp = Point::perp(dir);
                perp *= width;
                above = pto + perp;
            }
            if( cen.isNull())
            {
                QPointF ab = after_pt - before_pt ;
                Point::normalizeD(ab);
                Point::perpD(ab);
                ab *= width;
                cen = pto - ab ;
            }

            // FIXMECSK -- The gap size isn't consistent since
            // it's based on subtracting gap scaled unit vectors.
            // Scale gap by 1/sin(theta) to compensate for
            // angle with neighbours and get a _perpendicular_ gap.

            if ( gap > 0.0 )
            {
                below -= ( dir * capGap( below, pfrom, gap ) ) ;
                above -= ( dir * capGap( above, pfrom, gap ) ) ;
                cen   -= ( dir *  capGap( cen,   pfrom, gap ) ) ;
            }

            shadows[ shadowsIndex ] = true;
        }
    }

    pts[ ptsIndex ] = below;
    pts[ ptsIndex + 1 ] = cen;
    pts[ ptsIndex + 2 ] = above;
}

// Propagate the over-under relationship from a vertices to its
// adjacent edges.  The relationship is encapsulated in the
// "edge_under_at_vert" variable, which says whether the
// edge passed in is in the under state at this vertex.
// The whole trick is to manage how neighbours receive modifications
// of edge_under_at_vert.

void Interlace::propagate(VertexPtr vertex, EdgePtr edge, bool edge_under_at_vert, QStack<EdgePtr> & todo )
{
    interlaceInfo  * vi = vertex->getInterlaceInfo();
    vi->visited = true;

    int nn = vertex->numNeighbours();

    if( nn == 2)
    {
        QVector<EdgePtr>  ba = vertex->getBeforeAndAfter( edge );
        EdgePtr oe = ba[0];
        interlaceInfo * oei = oe->getInterlaceInfo();

        if( !oei->visited )
        {
            // With a bend, we don't want to change the underness
            // of the edge we're propagating to.
            if( oe->getV1() == vertex)
            {
                // The new edge starts at the current vertex.
                oei->start_under = !edge_under_at_vert;
            }
            else
            {
                // The new edge ends at the current vertex.
                oei->start_under = edge_under_at_vert;
            }
            oei->visited = true;
            todo.push( oe );
        }
    }
    else if (nn == 1 && includeTipVertices)
    {
        EdgePtr oe = vertex->getNeighbours().at(0);
        interlaceInfo * oei = oe->getInterlaceInfo();

        if( !oei->visited )
        {
            // With a bend, we don't want to change the underness
            // of the edge we're propagating to.
            if( oe->getV1() == vertex)
            {
                // The new edge starts at the current vertex.
                oei->start_under = !edge_under_at_vert;
            }
            else
            {
                // The new edge ends at the current vertex.
                oei->start_under = edge_under_at_vert;
            }
            oei->visited = true;
            todo.push( oe );
        }
    }
    else if( nn > 2 )
    {
        //Q_ASSERT(nn == 4);
        QVector<EdgePtr> ns;
        int index = 0;
        int edge_idx = -1;

        QVector<EdgePtr> edges = vertex->getNeighbours();
        for (auto it = edges.begin(); it != edges.end(); it++)
        {
            ns << *it;
            if (*it == edge)
            {
                edge_idx = index;
            }
            index++;
        }

        bool cur_under = edge_under_at_vert;

        for( int idx = 1; idx < nn; ++idx )
        {
            int cur = (edge_idx + idx) % nn;
            EdgePtr oe = ns[ cur ];
            Q_ASSERT(oe);
            interlaceInfo * oei = oe->getInterlaceInfo();

            if( !oei->visited )
            {
                if( oe->getV1() == vertex)
                {
                    oei->start_under = !cur_under;
                }
                else
                {
                    oei->start_under = cur_under;
                }
                oei->visited = true;
                todo.push( oe );
            }

            cur_under = !cur_under;
        }
    }
}

// Propagate the over-under relation from an edge to its incident
// vertices.

void Interlace::buildFrom(QStack<EdgePtr> &todo )
{
    while ( !todo.empty() )
    {
        EdgePtr edge = todo.pop();
        interlaceInfo * ei = edge->getInterlaceInfo();

        VertexPtr v1 = edge->getV1();
        VertexPtr v2 = edge->getV2();

        if( !v1->getInterlaceInfo()->visited)
        {
            propagate( v1, edge, ei->start_under, todo );
        }
        if( !v2->getInterlaceInfo()->visited)
        {
            propagate( v2, edge, !ei->start_under, todo );
        }
    }
}

void Interlace::initializeMap( constMapPtr map )
{
    for(auto e = map->getEdges()->begin(); e != map->getEdges()->end(); e++)
    {
        EdgePtr edge = *e;
        interlaceInfo * ei = new interlaceInfo();
        edge->setInterlaceInfo(ei);
    }

    for (auto v = map->getVertices()->begin(); v != map->getVertices()->end(); v++)
    {
        VertexPtr vert = *v;
        interlaceInfo * vi = new interlaceInfo();
        vert->setInterlaceInfo(vi);
    }
}

void Interlace::finalizeMap(constMapPtr map )
{
    for(auto e = map->getEdges()->begin(); e != map->getEdges()->end(); e++)
    {
        EdgePtr edge = *e;
        edge->setInterlaceInfo(nullptr);
    }

    for (auto v = map->getVertices()->begin(); v != map->getVertices()->end(); v++)
    {
        VertexPtr vert = *v;
        vert->setInterlaceInfo(nullptr);
    }
}


void Interlace::assignInterlacing(constMapPtr map )
{
    initializeMap( map );

    // Stack of edge to be processed.
    QStack<EdgePtr> todo;

    for(auto e = map->getEdges()->begin(); e != map->getEdges()->end(); e++)
    {
        EdgePtr edge = *e;
        interlaceInfo * ei = edge->getInterlaceInfo();
        if( !ei->visited )
        {
            ei->start_under = true;
            todo.push( edge );
            buildFrom( todo );
        }
    }
}


