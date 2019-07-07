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

////////////////////////////////////////////////////////////////////////////
//
// Map.java
//
// The implementation of a planar map abstraction.  A planar map is
// an (undirected) graph represented on the plane in such a way that
// edges don't cross vertices or other edges.
//
// This is one of the big daddy structures of computational geometry.
// The right way to do it is with a doubly-connected edge list structure,
// complete with half edges and a face abstraction.  Because I'm lazy
// and because of the sheer coding involved, I'm doing something simpler,
// more like an ordinary graph.  The disadvantage is that I don't maintain
// faces explicitly, which can make face colouring for islamic patterns
// tricky later.  But it's more tractable than computing overlays of
// DCELs.

#include "geometry/Map.h"
#include "geometry/Loose.h"
#include "geometry/Transform.h"
#include "geometry/Intersect.h"
#include "base/canvas.h"
#include "base/configuration.h"
#include "base/shapefactory.h"
#include "base/utilities.h"
#include <QtAlgorithms>

#define  LINK_END_TO_START

int Map::refs;

Map::Map()
{
    refs++;
}

Map::~Map()
{
    refs--;
    //qDebug() << "DELETING MAP";
    wipeout();
}

void Map::wipeout()
{
    // better to remove edges before removing vertices
    foreach (EdgePtr e, edges)
    {
        removeEdge(e);
    }

    foreach (VertexPtr v, vertices)
    {
        removeVertex(v);
    }
    edges.clear();          // unnecessary from destructor but not elsewhere
    vertices.clear();       // unneccesary from destructor but not elsewhere
    texts.clear();
}

// Some obvious getters.

QString Map::getInfo() const
{
    return QString("Map: vertices = %1  edges = %2").arg(vertices.size()).arg(edges.size());
}

bool Map::isEmpty()
{
    return  (vertices.size() < 2);
}

int Map::numVertices() const
{
    return vertices.size();
}

int Map::numEdges() const
{
    return edges.size();
}

// Remove stuff from the map.

void Map::removeEdge( EdgePtr e )   // called by wipeout
{
    VertexPtr v1  = e->getV1();
    VertexPtr v2  = e->getV2();

    v1->removeEdge(e);
    v2->removeEdge(e);
    edges.removeAll(e);

    if (v1->numNeighbours() == 0)
    {
        vertices.removeAll(v1);
    }

    if (v2->numNeighbours() == 0)
    {
        vertices.removeAll(v2);
    }
}

void Map::removeVertex(VertexPtr v)
 {
    //qDebug() << v->getPosition() << (void*)v->edges2.get();
    QVector<EdgePtr> & qvep = v->getEdges();
    for (auto it = qvep.begin(); it != qvep.end(); it++)
    {
        EdgePtr edge = *it;
        if (edge)
        {
            edge->getOther(v->getPosition())->removeEdge(edge);
            edges.removeOne(edge);
        }
    }
    vertices.removeOne(v);
 }

void Map::duplicate(MapPtr ret)
{
    this->vertices = ret->vertices;
    this->edges    = ret->edges;
}

MapPtr Map::recreate()
{
    MapPtr ret = make_shared<Map>();

    for(auto e = vertices.begin(); e != vertices.end(); e++)
    {
        VertexPtr vert = *e;
        VertexPtr nv = make_shared<Vertex>( vert->getPosition());

        vert->copy = nv;
        ret->vertices.push_back(nv);
    }

    for(auto it = edges.begin(); it != edges.end(); it++ )
    {
        EdgePtr edge = *it;
        EdgePtr ne = make_shared<Edge>(edge->getV1()->copy, edge->getV2()->copy);

        ret->edges.push_back(ne);

        // CSKFIXME -- this could be a bit faster.  Really we could
        // copy the order of the edges leaving a vertex wholesale.
        // I don't think it'll make that big a difference.  The
        // max expected degree of the vertices in the maps we'll be
        // dealing with is 6.

        ne->getV1()->insertEdge(ne);
        ne->getV2()->insertEdge(ne);
    }

    cleanCopy();

    return ret;
}

void Map::cleanCopy()
{
    for (auto v = vertices.begin(); v != vertices.end(); v++)
    {
        VertexPtr vert = *v;
        vert->copy.reset();
    }
}

// Routines used for spatial sorting of edges and vertices.


int Map::lexCompareEdges( qreal a, qreal b )
{
    qreal d = a - b;

    if( Loose::zero( d ) )
    {
        return 0;
    }
    else if( d < 0.0 )
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

int Map::lexComparePoints( QPointF a, QPointF b )
{
    bool verbose = false;

    qreal dx = a.x() - b.x();

    if( Loose::zero( dx ) )
    {
        qreal dy = a.y() - b.y();

        if( Loose::zero( dy ) )
        {
            if (verbose) qDebug () << a << b << "L0";
            return 0;
        }
        else if( dy < 0.0 )
        {
            if (verbose) qDebug () << a << b << "L-1";
            return -1;
        }
        else
        {
            if (verbose) qDebug () << a << b << "L1";
            return 1;
        }
    }
    else if( dx < 0.0 )
    {
        if (verbose) qDebug () << a << b << "D-1";
        return -1;
    }
    else
    {
        if (verbose) qDebug () << a << b << "O1";
        return 1;
    }
}

bool Map::vertexLessThan(VertexPtr a, VertexPtr b )
{
    if (Map::lexComparePoints( a->getPosition(), b->getPosition()) == -1)
        return true;
    else
        return false;
}

void Map::sortVertices()
{
    qSort( vertices.begin(),vertices.end(),vertexLessThan);
}


bool Map::edgeLessThan( EdgePtr  a,  EdgePtr  b )
{
    int rv =  lexCompareEdges( a->getMinX(), b->getMinX() );
    if (rv ==  -1)
        return true;
    else
        return false;
}

void Map::sortEdges()
{
    qSort( edges.begin(), edges.end(), edgeLessThan );
}


void Map::sortAllNeighboursByAngle()
{
    for (auto it = vertices.begin(); it != vertices.end(); it++)
    {
        VertexPtr vp = *it;
        vp->sortEdgesByAngle();
    }
}

// Get a Map Vertex given that we're asserting the vertex
// doesn't lie on an edge in the map.
VertexPtr Map::getVertex_Simple( QPointF pt )
{
    for( int idx = 0; idx < vertices.size(); ++idx )
    {
        VertexPtr  v = vertices.at( idx );
        QPointF cur = v->getPosition();
        int cmp = lexComparePoints( pt, cur );

        if( cmp == 0 )
        {
            return v;
        }
        else if( cmp < 0 )
        {
            VertexPtr vert = make_shared<Vertex>(pt);
            vertices.insert( idx, vert );
            return vert;
        }
    }

    VertexPtr vert = make_shared<Vertex>(pt);
    vertices.push_back(vert );
    return vert;
}

// Insert an edge given that we know the edge doesn't interact
// with other edges or vertices other than its endpoints.
void Map::insertEdge_Simple(EdgePtr edge )
{
    qreal xm = edge->getMinX();

    for( int idx = 0; idx < edges.size(); ++idx )
    {
        EdgePtr e = edges.at( idx ) ;
        qreal xmcur = e->getMinX();

        if( lexCompareEdges( xm, xmcur ) < 0 )
        {
            edges.insert(idx,edge );
            return;
        }
    }

    edges.push_back(edge);
}

// Insert the edge connecting two vertices, including updating
// the neighbour lists for the vertices.
EdgePtr Map::insertEdge(VertexPtr v1, VertexPtr v2, bool debug)
{
    EdgePtr e = make_shared<Edge>(v1, v2 );
    insertEdge_Simple( e );
    v1->insertEdge( e );
    v2->insertEdge( e );
    if (debug)
    {
        insertDebugMark(v1->getPosition(),QString());
        insertDebugMark(v2->getPosition(),QString());
    }
    return e;
}

// Split any edge (there is at most one) that intersects
// this new vertex.  You'll want to make sure this vertex isn't
// a duplicate of one already in the map.  That would just
// make life unpleasant.
// casper: modified to detect point on more than one edge
void Map::splitEdgesByVertex(VertexPtr vert)
{
    QPointF vp = vert->getPosition();
    qreal x = vp.x();

    for( int idx = 0; idx < edges.size(); ++idx )
    {
        EdgePtr e = edges.at( idx );
        qreal xm = e->getMinX();

        if( lexCompareEdges( xm, x ) > 0 )
        {
            // The edges are sorted by xmin, and this xmin exceeds
            // the x value of the vertex.  No more interactions.
            return;
        }

        VertexPtr v1 = e->getV1();
        VertexPtr v2 = e->getV2();

        if( Loose::zero(Point::distToLine(vp, v1->getPosition(), v2->getPosition() ) ) )
        {
            if ( Loose::zero( Point::dist(vp, v1->getPosition())) ||
                 Loose::zero( Point::dist(vp ,v2->getPosition()) ) )
            {
                // Don't split if too near endpoints.
                continue;
            }

            // Create the new edge instance.
            EdgePtr nedge = make_shared<Edge>(vert, v2);

            // We don't need to fix up v1 -- it can still point
            // to the same edge.

            // Fix up v2.
            v2->swapEdge( v1, nedge);

            // Fix up the edge object -- it now points to the
            // intervening edge.
            e->setV2(vert);

            // Insert the new edge.
            edges.remove( idx );
            insertEdge_Simple( nedge );
            insertEdge_Simple( e );

            // Update the adjacencies for the splitting vertex
            vert->insertEdge( e);
            vert->insertEdge( nedge );

            // taprats: That's it.
            // casper: continue
            //return;
        }
    }
    //verify("splitEdgesByVertex",false,true);
}


// The "correct" version of inserting a vertex.  Make sure the
// map stays consistent.
VertexPtr Map::getVertex_Complex(QPointF pt)
{
    VertexPtr vert = getVertex_Simple( pt );
    splitEdgesByVertex( vert);
    return vert;
}

// The publically-accessible version.
VertexPtr Map::insertVertex( QPointF  pt)
{
    return getVertex_Complex( pt);
}

void Map::insertPolygon(Polyform  * poly)
{
    //qDebug() << "Map::insertPolygon";
    for (int i=0; i < poly->size() -1; i++)
    {
        QPointF p1 = poly->at(i);
        QPointF p2 = poly->at(i+1);
        VertexPtr v1 = insertVertex(p1);
        VertexPtr v2 = insertVertex(p2);
        insertEdge(v1,v2);
    }
    //verify("insertPolygon",false,false,false);
}

void Map::insertPolyline(Polyform * poly)
{
    //qDebug() << "Map::insertPolyline";
    for (int i=0; i < poly->size() -1; i++)
    {
        QPointF p1 = poly->at(i);
        QPointF p2 = poly->at(i+1);
        VertexPtr v1 = insertVertex(p1);
        VertexPtr v2 = insertVertex(p2);
        insertEdge(v1,v2);
    }
    //verify("insertPolyline",false,false,false);
}

void Map::insertDebugMark(QPointF m, QString txt, qreal size)
{
    Configuration * config = Configuration::getInstance();
    if (!config->debugMapEnable)
        return;

    qreal x = m.x();
    qreal y = m.y();
    QPointF p1(x-size,y);
    QPointF p2(x+size,y);
    QPointF p3(x,y+size);
    QPointF p4(x,y-size);
    VertexPtr v1 = insertVertex(p1);
    VertexPtr v2 = insertVertex(p2);
    insertEdge(v1,v2);
    v1 = insertVertex(p3);
    v2 = insertVertex(p4);
    insertEdge(v1,v2);

    if (!txt.isEmpty())
    {
        sText stxt;
        stxt.pt  = m;
        stxt.txt = txt;
        texts.push_back(stxt);
    }
}

void Map::insertDebugLine(QLineF l1)
{
    insertDebugLine(l1.p1(),l1.p2());
}

void Map::insertDebugLine(EdgePtr edge)
{
    Configuration * config = Configuration::getInstance();
    if (!config->debugMapEnable)
        return;
    insertEdge(edge->getV1(),edge->getV2(),true);
}

void Map::insertDebugLine(QPointF p1, QPointF p2)
{
    Configuration * config = Configuration::getInstance();
    if (!config->debugMapEnable)
        return;

    VertexPtr v1 = insertVertex(p1);
    VertexPtr v2 = insertVertex(p2);
    insertEdge(v1,v2);
}

void Map::insertDebugPolygon(QPolygonF & poly)
{
    Configuration * config = Configuration::getInstance();
    if (!config->debugMapEnable)
        return;

    for (int i=0; i < poly.size() -1; i++)
    {
        QPointF p1 = poly.at(i);
        QPointF p2 = poly.at(i+1);
        VertexPtr v1 = insertVertex(p1);
        VertexPtr v2 = insertVertex(p2);
        insertEdge(v1,v2);
    }
    if (!poly.isClosed())
    {
        QPointF p1 = poly.at(poly.size()-1);
        QPointF p2 = poly.at(0);
        VertexPtr v1 = insertVertex(p1);
        VertexPtr v2 = insertVertex(p2);
        insertEdge(v1,v2);
    }
}

// Applying a motion made up only of uniform scales and translations,
// Angles don't change.  So we can just transform each vertex.
void Map::applyTrivialRigidMotion( Transform T )
{
    for (auto e = vertices.begin(); e != vertices.end(); e++)
    {
        VertexPtr vert = *e;
        vert->pos = T.apply( vert->getPosition() );
    }
}

void Map::scale( qreal s )
{
    applyTrivialRigidMotion( Transform::scale( s ) );
}

void Map::rotate( qreal r )
{
    applyTrivialRigidMotion( Transform::rotate(r));
}

void Map::translate(qreal x, qreal y )
{
    applyTrivialRigidMotion( Transform::translate( x, y ) );
}

// In the general case, the vertices and edges must be re-sorted.
void Map::applyGeneralRigidMotion( Transform T )
{
    // Transform all the vertices.
    for (auto e = vertices.begin(); e != vertices.end(); e++)
    {
        VertexPtr vert = *e;
        vert->applyRigidMotion( T );
    }

    // Now sort everything.
    sortVertices();
    sortEdges();
}

void Map::transformMap( Transform T )
{
    applyGeneralRigidMotion( T );
}

#if 0
// Transform a single vertex.
public synchronized void transformVertex( Vertex v, Transform T )
{
    // This isn't entirely trivial:
    // 	1. Transform the vertex itself and recalculate the neighbour
    //     angles.
    //  2. For each vertex adjacent to this one, reinsert the connecting
    //     edge into the other vertex's neighbour list.
    //  3. Reinsert the vertex into the map's vertex list -- its
    //     x position may have changed.
    //  4. Re-sort the edge list, since edges which began at the
    //     transformed vertex may shift in the edge list.
    //
    // Right now most of this, especially steps 3 and 4, are done
    // in the obvious, slow way.  With more careful programming, we
    // could break the process down further and do some parts faster,
    // like searching for the vertex's new position relative to its
    // current position.  At this point, however, that's not a huge
    // win.

    // Transform the position of the vertex.
    v.applyRigidMotion( T );

    // Reorganize the vertices adjacent to this vertex.
    for( Enumeration<Edge> e = v.neighbours(); e.hasMoreElements(); ) {
        Edge edge = e.nextElement();
        Vertex other = edge.getOther( v );

        other.removeEdge( edge );
        other.insertEdge( edge );
    }

    // This vertex's x position may have changed.  Reposition
    // it in the array of vertices.
    vertices.removeElement( v );
    Point pt = v.getPosition();

    for( int idx = 0; idx < vertices.size(); ++idx ) {
        Vertex vo = (Vertex)( vertices.elementAt( idx ) );
        Point cur = vo.getPosition();
        int cmp = lexComparePoints( pt, cur );

        if( cmp < 0 ) {
            vertices.insertElementAt( v, idx );
            return;
        }
    }

    vertices.addElement( v );

    // Sigh -- I guess resort the edges.
    sortEdges();
}
#endif

// Given another vector of vertices, add them to the vertices of the
// current map.  We can do this in linear time with a simple merge
// algorithm.  Note that we want to coalesce identical vertices to
// eliminate duplicates.
void Map::mergeVertices( QVector<VertexPtr> & your_verts )
{
    int a = 0;
    int b = 0;
    int c = 0;
    int d = 0;
    int e = 0;
    QVector<VertexPtr> my_verts = vertices;

    int my_size = my_verts.size();
    int your_size = your_verts.size();

    vertices.clear();

    int my_i = 0;
    int your_i = 0;

    while( true )
    {
        if( my_i == my_size )
        {
            if( your_i == your_size )
            {
                // done!
                //qDebug() << "a-e" << a << b << c << d << e;
                return;
            }
            else
            {
                VertexPtr your_v = your_verts.at( your_i );
                VertexPtr nv = make_shared<Vertex>(your_v->getPosition());
                vertices.push_back( nv );
                your_v->copy = nv;
                ++your_i;
                a++;
            }
        }
        else
        {
            if( your_i == your_size )
            {
                vertices.push_back( my_verts.at( my_i ) );
                ++my_i;
                b++;
            }
            else
            {
                // Darn -- have to actually merge.
                VertexPtr my_v = my_verts.at( my_i );
                VertexPtr your_v = your_verts.at( your_i );
                int cmp = lexComparePoints( my_v->getPosition(), your_v->getPosition() );

                if( cmp < 0 )
                {
                    // my_v goes first.
                    vertices.push_back( my_v );
                    ++my_i;
                    c++;
                }
                else if( cmp == 0 )
                {
                    // It's a toss up.
                    vertices.push_back( my_v );
                    your_v->copy = my_v;
                    ++my_i;
                    ++your_i;
                    d++;
                }
                else if( cmp > 0 )
                {
                    // your_v goes first.
                    VertexPtr nv = make_shared<Vertex>(your_v->getPosition());
                    vertices.push_back(nv);
                    your_v->copy = nv;
                    ++your_i;
                    e++;
                }
            }
        }
    }
}

// Merge two maps.  The bread and butter of the Map class.  This is a
// complicated computational geometry algorithm with a long and
// glorious tradition :^)
//
// The goal is to form a map from the union of the two sets of
// vertices (eliminating duplicates) and the union of the two sets
// of edges (splitting edges whenever intersections occur).
//
// There are very efficient ways to do this, reporting edge-edge
// intersections using a plane-sweep algorithm.  Implementing
// this merge code in all its glory would be too much work.  Since
// I have to use all my own code, I'm going to resort to a simplified
// (and slower) version of the algorithm.
void Map::mergeMap(MapPtr other /*, bool consume */)
{
    static bool verifyMap   = false;
    static bool traceMerge  = false;

    bool good_at_start      = true;

    if (traceMerge) qDebug() << "** mergeMap start";
    if (verifyMap)
    {
        if (traceMerge) qDebug() << "** mergeMap verify";
        //Sanity checks to use when debugging.
        good_at_start = verify("merge map",true,true);
        if( !good_at_start )
        {
            badness( "Bad at start.");
        }
        if ( !other->verify("other map",false))
        {
            badness("Other bad at start.");
        }
    }

    // Here's how I'm going to do this.
    //
    // 1. Check all intersections between edges from this map
    //    and from the other map.  Record the positions of the
    //    intersections.
    //
    // 2. For each intersection position, get the vertex in both
    //    maps, forcing edges to split where the intersections will
    //    occur.
    //
    // 3. Merge the vertices.
    //
    // 4. Add the edges in the trivial way, since now there will
    //    be no intersections.
    //
    // Yech -- pretty slow.  But I shudder at the thought of
    // doing it with maximal efficiency.
    //
    // In practice, this routine has proven to be efficient enough
    // for the Islamic design tool.  Phew!

    // Step 0 -- setup.
    // Create a vector to hold the intersections.
/*
    if( !consume )
    {
        // Copy the other map so we don't destroy it.
        other = other->clone();
        if ( !other->verify("other clone",false))
        {
            qCritical("Other clone bad at start.");
        }
    }
*/
    QVector<QPointF> intersections;

    if (traceMerge) qDebug() << "Step 1";
    // 1. Check all intersections between edges from this map
    //    and from the other map.  Record the positions of the
    //    intersections.
    const QVector<EdgePtr> * oedges = other->getEdges();
    for(auto it = oedges->begin(); it != oedges->end(); it++)
    {
        EdgePtr edge = *it;

        // To check all intersections of this edge with edges in
        // the current map, we can use the optimization that
        // edges are stored in sorted order by min x.  So at the
        // very least, skip the suffix of edges whose minimum x values
        // are past the max x of the edge to check.

        QPointF ep = edge->getV1()->getPosition();
        QPointF eq = edge->getV2()->getPosition();

        qreal exm = qMax( ep.x(), eq.x() );

        for (auto me = edges.begin(); me != edges.end(); me++ )
        {
            EdgePtr cur = *me;

            QPointF cp = cur->getV1()->getPosition();
            QPointF cq = cur->getV2()->getPosition();

            if( lexCompareEdges( cur->getMinX(), exm ) > 0 )
            {
                break;
            }

            QPointF ipt = Intersect::getTrueIntersection( ep, eq, cp, cq );

            if( ipt !=  QPointF(0,0) )
            {
                intersections.push_back( ipt );
            }
        }
    }
    if (verifyMap) verify("merge map - step 1",true,true);

    if (traceMerge) qDebug() << "Step 2";
    // 2. For each intersection position, get the vertex in both
    //    maps, forcing edges to split where the intersections will
    //    occur.
    //

    for(auto it2 = intersections.begin(); it2 != intersections.end(); it2++)
    {
        QPointF p = *it2;
        getVertex_Complex(p);
        other->getVertex_Complex(p);
    }
    if (verifyMap) verify("merge map - step 2",true,true);

    if (traceMerge) qDebug() << "Step 3";
    // 3. Merge the vertices.
    mergeVertices( other->vertices );
    if (verifyMap) verify("merge map - step 3",true,true);

    if (traceMerge) qDebug() << "Step 4";
    // 4. Add the edges in the trivial way, since now there will
    //    be no intersections.

    for(auto it = oedges->begin(); it != oedges->end(); it++)
    {
        EdgePtr edge = *it;

        VertexPtr v1 = edge->getV1()->copy;
        VertexPtr v2 = edge->getV2()->copy;
        if (v1->pos == v2->pos)
        {
            qWarning("v1==v2");
            continue;
        }

        EdgePtr ne = make_shared<Edge>(v1,v2);

        // Rather than using insertEdge_Simple, we tack all new edges
        // to the end of the edge list...
        edges.push_back( ne );

        v1->insertEdge( ne );
        v2->insertEdge( ne );
    }
    if (verifyMap) verify("merge map-step 4",true,true);

    // ... and then sort everything together for speed.

    if (traceMerge) qDebug() << "Step 5";
    sortEdges();
    if (verifyMap) verify("merge map-step 5",true,true);

    cleanNeighbours();

    if (traceMerge) qDebug() << "Step 6";
    cleanCopy();
    if (verifyMap) verify("merge map-step 6",true,true);

    // I guess that's it!
    bool good_at_end = verify("merge map", false, true);

    if(!good_at_end)
    {
        badness("bad merge" );
    }
    else if (verifyMap)
    {
        qDebug() << "good merge";
    }
    if (traceMerge) qDebug() << "** mergeMap end";
}

/*
void Map::mergeMap(MapPtr other )
{
    mergeMap( other, false );
}*/

#if 0
// A simpler merge routine that assumes that the two maps
// don't interact except at vertices, so edges don't need to
// be checked for intersections.
public synchronized void mergeSimple( Map other )
{
    // System.err.println( "mergeSimple begin" );

    mergeVertices( other.vertices );

    for( Enumeration<Edge> e = other.getEdges(); e.hasMoreElements(); ) {
        Edge edge = e.nextElement();
        Vertex v1 = edge.getV1().copy;
        Vertex v2 = edge.getV2().copy;

        // insertEdge( edge.getV1().copy, edge.getV2().copy );
        Edge nedge = new Edge( this, v1, v2 );
        v1.insertEdge( nedge );
        v2.insertEdge( nedge );
        edges.addElement( nedge );
    }

    sortEdges();

    cleanCopy();
}
#endif
// It's often the case that we want to merge a transformed copy of
// a map into another map, or even a collection of transformed copies.
// Since transforming a map requires a slow cloning, we can save lots
// of time and memory by transforming and merging simultaneously.
// Here, we transform vertices as they are put into the current map.
void Map::mergeSimpleMany(MapPtr other, const QVector<Transform> &transforms)
{
    for (auto e = transforms.begin(); e != transforms.end(); e++)
    {
        Transform T = *e;
        //qDebug() << T->toString();
        for (auto v = other->getVertices()->begin(); v != other->getVertices()->end(); v++)
        {
            VertexPtr vert = *v;
            vert->copy = getVertex_Simple(T.apply(vert->getPosition()));
            //qDebug() << vert->copy->getPosition();
        }


        for (auto ed = other->getEdges()->begin(); ed != other->getEdges()->end(); ed++)
        {
            EdgePtr edge = *ed;

            VertexPtr v1 = edge->getV1()->copy;
            if (!v1)
                qDebug() << "woops1";
            VertexPtr v2 = edge->getV2()->copy;
            if (!v2)
                qDebug() << "woops2";
            EdgePtr nedge = make_shared<Edge>(v1, v2);
            v1->insertEdge( nedge);
            v2->insertEdge( nedge);
            edges.push_back( nedge );
        }
    }

    verify("merge simple-many",false,true,false);
    sortEdges();
    cleanCopy();
}


void Map::divideIntersectingEdges()
{
    verify("deIntersectEdges- start",false,true);

    QVector<QPointF> intersects;
    for(auto it = edges.begin(); it != edges.end(); it++)
    {
        EdgePtr edge = *it;

        // To check all intersections of this edge with edges in
        // the current map, we can use the optimization that
        // edges are stored in sorted order by min x.  So at the
        // very least, skip the suffix of edges whose minimum x values
        // are past the max x of the edge to check.

        QPointF ep = edge->getV1()->getPosition();
        QPointF eq = edge->getV2()->getPosition();

        qreal exm = qMax( ep.x(), eq.x() );

        for (auto me = edges.begin(); me != edges.end(); me++ )
        {
            EdgePtr cur = *me;
            if (cur == edge)
            {
                continue;
            }

            QPointF cp = cur->getV1()->getPosition();
            QPointF cq = cur->getV2()->getPosition();

            if( lexCompareEdges( cur->getMinX(), exm ) > 0 )
            {
                break;
            }

            QPointF ipt = Intersect::getTrueIntersection( ep, eq, cp, cq );

            if( ipt !=  QPointF(0,0) )
            {
                if (!intersects.contains(ipt))
                {
                    intersects.push_back( ipt );
                }
                else
                    qDebug() << "duplicate intersect omitted";
            }
        }
    }
    // now split the edges
    for (auto it = intersects.begin(); it != intersects.end(); it++)
    {
        QPointF pt = *it;
        getVertex_Complex(pt);
    }

    sortVertices();
    sortEdges();
    cleanNeighbours();
    verify("deIntersectEdges",false,true);
}

void Map::joinColinearEdges()
{
    bool changed = false;
    do
    {
        changed = joinOneColinearEdge();
    } while (changed);

    sortVertices();
    sortEdges();
    cleanNeighbours();
    verify("joinColinearEdges",false,true);
}

bool Map::joinOneColinearEdge()
{
    for (auto it = vertices.begin(); it != vertices.end(); it++)
    {
        VertexPtr vp = *it;
        int count = vp->numNeighbours();
        if (count == 2)
        {
            QVector<EdgePtr> qvep = vp->getEdges();
            QLineF a = qvep[0]->getLine();
            QLineF b = qvep[1]->getLine();
            qreal angle = a.angle(b);
            if (Loose::zero(angle) || Loose::equals(angle,180.0))
            {
                // need to remove one edge, extend the other, and remove vertex
                combineLinearEdges(qvep[0],qvep[1],vp);
                return true;
            }
        }
        else if (count == 1)
        {
#if 0
            // this is a dangling edge which sometimes is ok
            EdgePtr      ep = vp->getNeighbours().at(0);
            QLineF        a = ep->getLine();
            VertexPtr other = ep->getOther(vp->getPosition());
            QVector<EdgePtr> qvep = other->getEdges();
            for (auto it = qvep.begin(); it != qvep.end(); it++)
            {
                EdgePtr otherEdge = *it;
                QLineF          b = otherEdge->getLine();
                if (a == b)
                {
                    continue;
                }
                qreal       angle = a.angle(b);
                if (Loose::zero(angle) || Loose::equals(angle,180.0))
                {
                    // this is a case where it needs to be deleeted
                    removeVertex(vp);
                    return true;
                }
            }
#endif
        }
    }
    return false;
}

void Map::combineLinearEdges(EdgePtr a, EdgePtr b,VertexPtr common)
{
    VertexPtr modifiedVp;

    // let's keep a, and remove b

    if (a->getV1() == common)
    {
        if (b->getV1() == common)
        {
            a->setV1(b->getV2());
            modifiedVp = b->getV2();
        }
        else
        {
            Q_ASSERT(b->getV2() == common);
            a->setV1(b->getV1());
            modifiedVp = b->getV1();
        }
    }
    else
    {
        Q_ASSERT(a->getV2() == common);
        if (b->getV1() == common)
        {
            a->setV2(b->getV2());
            modifiedVp = b->getV2();
        }
        else
        {
            Q_ASSERT(b->getV2() == common);
            a->setV2(b->getV1());
            modifiedVp = b->getV1();
        }
    }
    // need to change the neighbours for modified vertex
    QVector<EdgePtr> & qvep = modifiedVp->getNeighbours();
    for (auto it = qvep.begin(); it != qvep.end(); it++)
    {
        EdgePtr ep = *it;
        if (ep == b)
        {
            *it = a;  //replace in vertex
        }
    }
    // can now remove b
    edges.removeOne(b);
    // can now remove vertex
    vertices.removeOne(common);
}

void Map::splitEdge(EdgePtr e)
{
    verify("split edge start",false,true);

    VertexPtr oldV1 = e->getV1();
    VertexPtr oldV2 = e->getV2();

    // remove existing
    removeEdge(e);
    verify("split edge mid",false,true);

    // insert new

    QPointF mid = e->getLine().center();
    VertexPtr v = insertVertex(mid);

    EdgePtr e1 = insertEdge(oldV1,v);
    EdgePtr e2 = insertEdge(v,oldV2);

    verify("split edge end",false,true);
}

void Map::deDuplicateVertices(QVector<VertexPtr> & vec)
{
    QVector<VertexPtr> avec;
    for (auto it = vec.begin();it != vec.end(); it++)
    {
        VertexPtr vp = *it;
        if (!avec.contains(vp))
        {
            avec.push_back(vp);
        }
    }
    vec = avec;
}

void Map::deDuplicateEdges(QVector<EdgePtr> & vec)
{
    // simple removal of duplicate edge pointers
    QVector<EdgePtr> avec;
    for (auto it = vec.begin();it != vec.end(); it++)
    {
        EdgePtr ep = *it;
        if (!avec.contains(ep))
        {
            avec.push_back(ep);
        }
    }
    vec = avec;

    // examining the positions edges of associated with the vertex
    int dupCount = 0;
    QVector<EdgePtr> duplicateEdges;
    for (auto it = vec.begin(); it != vec.end(); it++)
    {
        EdgePtr e = *it;
        auto it2 = it;
        it2++;
        for ( ; it2 != vec.end(); it2++)
        {
            EdgePtr e2 = *it2;
            if (e->sameAs(e2))
            {
                if (dupCount == 0)
                {
                    qWarning() << "first duplicate in design";
                }
                qWarning() << "duplicate edge detected"  << ++dupCount;
                duplicateEdges.push_back(e2);
            }
        }
    }
    // remove duplicates from the map
    for (auto it = duplicateEdges.begin(); it != duplicateEdges.end(); it++)
    {
        EdgePtr e = *it;
        removeEdge(e);
    }
}

// Print a text version of the map.
QString Map::summary()
{
    QString str = QString("vertices=%1 edges=%2").arg(vertices.size()).arg(edges.size());
    return str;
}

void Map::dump(bool full) const
{
    qDebug() << "vertices =" << vertices.size() << "edges = " << edges.size();
    if (true)
    {
        for( int idx = 0; idx < vertices.size(); ++idx )
        {
            VertexPtr v = vertices.at(idx);
            v->setTmpIndex(idx);
        }
        for( int idx = 0; idx < edges.size(); ++idx )
        {
            EdgePtr edge = edges.at(idx);
            edge->setTmpIndex(idx);
        }
        qDebug() << "=== start map" << Utils::addr(this);
        dumpVertices(full);
        dumpEdges(full);
        qDebug() << "=== end  map" << Utils::addr(this);
    }
}

void Map::dumpVertices(bool full) const
{
    for( int idx = 0; idx < vertices.size(); ++idx )
    {
        VertexPtr v = vertices.at(idx);
        qDebug() <<  "vertex"  << idx << "at" << v->getPosition();
        if (full)
        {
            v->dumpNeighbours();
        }
    }
}

QString Map::verticesToString()
{
    QString astring;
    QDebug  deb(&astring);

    for (auto it = vertices.begin(); it != vertices.end(); it++)
    {
        VertexPtr v = *it;
        QPointF pt = v->getPosition();
        deb << pt;
    }
    astring.remove("QPointF");
    return astring;
}

QString Map::vptrsToString()
{
    QString astring;
    QDebug  deb(&astring);

    for (auto it = vertices.begin(); it != vertices.end(); it++)
    {
        VertexPtr v = *it;
        deb << Utils::addr(v.get());
    }
    astring.remove("QPointF");
    return astring;
}



void Map::dumpEdges(bool full) const
{
    Q_UNUSED(full);
    for( int idx = 0; idx < edges.size(); ++idx )
    {
        EdgePtr edge = edges.at(idx);
        qDebug() <<  "edge" << idx << "from" << vertices.indexOf(edge->getV1()) << "to"  << vertices.indexOf(edge->getV2());
    }
}

// A big 'ole sanity check for maps.  Throw together a whole collection
// of consistency checks.  When debugging maps, call early and call often.
//
// It would probably be better to make this function provide the error
// messages through a return value or exception, but whatever.

bool Map::verify(QString mapname, bool verbose, bool detailed, bool doDump) const
{
    Configuration * config = Configuration::getInstance();
    if (!config->verifyMaps)
    {
        return true;
    }

    bool good = true;

    Canvas::getInstance()->dump(true);
    qDebug().noquote() << "Verifying:" << mapname << "Edges:" << edges.size() << "Vertices:" << vertices.size() ;

    if (vertices.size() == 0 && edges.size() == 0)
    {
        qDebug() << "empty map";
        return true;
    }

    if (edges.size() == 0)
    {
        qWarning("No edges");
        good = false;
    }

    if (doDump)
    {
        dump(true);
    }

    if (detailed)
    {
        // Make sure there are no trivial edges.

        for( int idx = 0; idx < edges.size(); ++idx )
        {
            EdgePtr e = edges.at( idx );
            if (e->getV1() == e->getV2())
            {
                qWarning() << "Trivial edge " << idx ;
                good = false;
            }
        }

        for (auto e = edges.begin(); e != edges.end(); e++)
        {
            EdgePtr edge = * e;
            if (verbose) qDebug() <<  "verifying edge: from" << vertices.indexOf(edge->getV1()) << "to"  << vertices.indexOf(edge->getV2());

            if (edge->getV1()->pos == edge->getV2()->pos)
            {
                qWarning() << "edge" << edges.indexOf( edge ) << "not really an edge";
                good = false;
            }

            // Make sure that for every edge, the edge's endpoints
            // are know vertices in the map.
            if( !vertices.contains( edge->getV1()))
            {
                qWarning() << "edge" << edges.indexOf( edge ) << "V1 not in vertex list.";
                good = false;
            }
            if( !vertices.contains( edge->getV2() ) )
            {
                qWarning() << "edge" << edges.indexOf( edge ) << "V2 not in vertex list.";
                good = false;
            }

            // Make sure that the edge's endpoints know about the
            // edge, and that the edge is the only connection between
            // the two endpoints.
            EdgePtr ed = edge->getV1()->getNeighbour( edge->getV2());
            if(!ed)
            {
                qWarning() << "edge" << edges.indexOf( edge ) << "not found in vertex v1 neighbours for vertex" <<  vertices.indexOf(edge->getV1());
                good = false;
            }
            else if( !(ed == edge ) )
            {
                qWarning() << "edge" << edges.indexOf( edge ) << "not matched in vertex v1 neighbours for vertex" << vertices.indexOf(edge->getV1());
                good = false;
            }

            ed = edge->getV2()->getNeighbour(edge->getV1());
            if (!ed)
            {
                qWarning() << "edge" << edges.indexOf( edge ) << "not found   in vertex v2 neighbours for vertex" <<  vertices.indexOf(edge->getV2());
                good = false;
            }
            else if( !( ed == edge ) )
            {
                qWarning() << "edge" << edges.indexOf( edge ) << "not found   in vertex v2 neighbours for vertex" <<  vertices.indexOf(edge->getV2());
                good = false;
            }
        }

        // Make sure the edges are in sorted order.
        for( int idx = 1; idx < edges.size(); ++idx )
        {
            EdgePtr e1 = edges.at( idx - 1 );
            EdgePtr e2 = edges.at( idx );

            double e1x = e1->getMinX();
            double e2x = e2->getMinX();

            if( e1x > (e2x + Loose::TOL) )
            {
                qInfo() << "Sortedness check failed for edges.";
                //good = false;
            }
        }

        // Make sure the vertices are in sorted order.
        for( int idx = 1; idx < vertices.size(); ++idx )
        {
            VertexPtr v1 = vertices.at( idx - 1 );
            VertexPtr v2 = vertices.at( idx );

            int cmp = lexComparePoints( v1->getPosition(), v2->getPosition() );

            if( cmp == 0 )
            {
                qWarning() << "Duplicate vertices.";
                good = false;
            }
            else if( cmp > 0 )
            {
                qInfo() << "Sortedness check failed for vertices.";
                good = false;
            }
        }

        // Make sure the vertices each have a neighbour and all neigours are good
        for (auto it = vertices.begin(); it != vertices.end(); it++)
        {
            VertexPtr vp = *it;
            if (vp->numNeighbours() == 0)
            {
                qWarning() << "Vertex at position" << vp->getPosition() << "has no neigbours, is floating in space";
                good = false;
            }
            QVector<EdgePtr> qvep = vp->getNeighbours();
            for (auto it = qvep.begin(); it != qvep.end(); it++)
            {
                EdgePtr edge = *it;
                if (!edge)
                {
                    qWarning("Bad neighbour: no edge");
                    good = false;
                }
                else if (!edge->getV1())
                {
                    qWarning("Bad neighbour edge: no V1");
                    good = false;
                }
                else if (!edge->getV2())
                {
                    qWarning("Bad neighbour edge: no V2");
                    good = false;
                }
            }
        }

        if (!good)
        {
            badness() << mapname << "did NOT verify!";
        }
        else
        {
            qDebug() << "verify OK";
        }
    }

    return good;
}

void Map::addShapeFactory(ShapeFactory * sf)
{
    for (auto it = sf->getPolyforms().begin(); it != sf->getPolyforms().end(); it++)
    {
        Polyform * p = *it;
        switch (p->polytype)
        {
        case POLYGON2:
            insertPolygon(p);
            break;
        case POLYLINE2:
            insertPolyline(p);
            break;
        case CIRCLE2:
            qWarning() << "Circle cant be stylized yet";
            break;
        }
    }
}

bool Map::analyzeVertices() const
{
    calcVertexEdgeCounts();

#if 1
    return false;   // map not changed
#else
    removeVerticesWithEdgeCount(1);
    //removeVerticesWithEdgeCount(3);
    //removeVerticesWithEdgeCount(1);

    calcVertexEdgeCounts();

    return true;    // map changed
#endif
}

void Map::calcVertexEdgeCounts() const
{
    // calc
    int vEdgeCounts[MAP_EDGECOUNT_MAX]; // for analysis, indexed by number of edges per vertex

    for (int i=0; i < MAP_EDGECOUNT_MAX; i++)
    {
        vEdgeCounts[i] = 0;
    }

    for (auto it = vertices.begin(); it != vertices.end(); it++)
    {
        VertexPtr v = *it;
        int count = v->numEdges();
        if (count <= MAP_EDGECOUNT_MAX)
        {
            vEdgeCounts[count]++;
        }
        else
        {
            qWarning() << "Unexpected large edge count for vertex=" << count;
            qFatal("large edge count");
        }
    }

    // dump
    QString str;
    QTextStream ss(&str);
    ss << "Vertex Edge counts: ";
    for (int i=0; i < MAP_EDGECOUNT_MAX; i++)
    {
        if (vEdgeCounts[i] != 0)
        {
            ss << i << "=" << vEdgeCounts[i] << " ";
        }
    }
    qDebug().noquote() << str;
}


void Map::removeVerticesWithEdgeCount(int edgeCount)
{
    QVector<VertexPtr> verts;
    for (auto it = vertices.begin(); it != vertices.end(); it++)
    {
        VertexPtr v = *it;
        if( edgeCount == v->numEdges())
        {
            verts.push_back(v);
        }
    }

    for (auto it = verts.begin(); it != verts.end(); it++)
    {
        VertexPtr v = *it;
        removeVertex(v);
    }
}

QRectF Map::calcBoundingRect()
{
   QPolygonF poly;

    QVector<VertexPtr> verts;
    for (auto it = vertices.begin(); it != vertices.end(); it++)
    {
        VertexPtr v = *it;
        poly << v->getPosition();
    }
    return poly.boundingRect();
}

void Map::cleanNeighbours()
{
    qDebug() << "Map::cleanNeighbours edges=" << edges.size()  << "vertices=" << vertices.size();

    for (auto it = vertices.begin(); it != vertices.end(); it++)
    {
        // examining a vertex
        VertexPtr v = *it;
        QVector<EdgePtr> & edgeVec = v->getEdges();
        deDuplicateEdges(edgeVec);
    }
    qDebug() << "Map::cleanNeighbours edges=" << edges.size()  << "vertices=" << vertices.size();
}

void Map::removeDanglingVertices()
{
    QVector<VertexPtr> baddies;

    for (auto it = vertices.begin(); it != vertices.end(); it++)
    {
        // examining a vertex
        VertexPtr v = *it;
        if (v->numNeighbours() == 0)
        {
            qDebug() << "removing unconnected vertex)";
            baddies.push_back(v);
        }
    }
    for (auto it = baddies.begin(); it != baddies.end(); it++)
    {
        VertexPtr v = *it;
        removeVertex(v);
    }
}

void Map::removeNullEdges()
{
    QVector<EdgePtr> baddies;

    for (auto it = edges.begin(); it != edges.end(); it++)
    {
        // examining a vertex
        EdgePtr e = *it;
        if (e->getV1() == e->getV2())
        {
            qDebug() << "removing null edge (1)";
            baddies.push_back(e);
        }
        else if (e->getV1()->pos == e->getV2()->pos)
        {
            qDebug() << "removing null edge (2)";
            baddies.push_back(e);
        }
    }
    for (auto it = baddies.begin(); it != baddies.end(); it++)
    {
        EdgePtr e = *it;
        removeEdge(e);
    }
}

void Map::cleanse()
{
    QElapsedTimer qet;
    qet.start();

    qDebug() << "Map::cleanse - start";
    verify("cleanse-start",false,true);
    qDebug() << "removeDanglingVertices" << qet.elapsed();
    removeDanglingVertices();
    qDebug() << "removeNullEdges" << qet.elapsed();
    removeNullEdges();
#if 0
    qDebug() << "divideIntersectingEdges" << qet.elapsed();
    divideIntersectingEdges();
#endif
    qDebug() << "joinColinearEdges" << qet.elapsed();
    joinColinearEdges();
    qDebug() << "cleanNeighbours" << qet.elapsed();
    cleanNeighbours();
    qDebug() << "sortAllNeighboursByAngle" << qet.elapsed();
    sortAllNeighboursByAngle();
    qDebug() << "sortVertices" << qet.elapsed();
    sortVertices();
    qDebug() << "sortEdges" << qet.elapsed();
    sortEdges();
    verify("cleanse-end",false,true);
    qDebug() << "Map::cleanse - end" << qet.elapsed();
}
