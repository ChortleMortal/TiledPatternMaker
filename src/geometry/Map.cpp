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
#include "geometry/Intersect.h"
#include "base/canvas.h"
#include "base/configuration.h"
#include "designs/shapefactory.h"
#include "base/utilities.h"
#include <QtAlgorithms>

int Map::refs;

Map::Map(QString Name) : neighbourMap(this)
{
    refs++;
    mname = Name;

    config = Configuration::getInstance();
}

Map::Map(const Map & map)  : neighbourMap(this)
{
    mname        = "copy of "  + map.mname;
    vertices     = map.vertices;
    edges        = map.edges;

    neighbourMap = map.neighbourMap;    // fetches identical copy
    // now we have it, we can make another local copy
    NeighbourMap others = neighbourMap;
    // reset our copy
    neighbourMap.clear();

    // rebuild a new neighbour map
    QMap<VertexPtr,NeighboursPtr> & ourmap =neighbourMap.get();
    QMapIterator<VertexPtr,NeighboursPtr> i(others.get());
    while (i.hasNext())
    {
        i.next();
        VertexPtr v = i.key();
        NeighboursPtr np = i.value();
        NeighboursPtr nnp = make_shared<Neighbours>(v);
        for (auto edge : np->getNeighbours())
        {
            nnp->insertEdgeSimple(edge);
        }
        ourmap[v] = nnp;
    }
}

Map::~Map()
{
    qDebug() << "deleting map" << mname;
    refs--;
    wipeout();
}

void Map::wipeout()
{
    // better to remove edges before removing vertices
#if 0
    for (auto e : edges)
    {
        removeEdge(e);
    }

    for (auto v : vertices)
    {
        removeVertex(v);
    }
#endif
    edges.clear();          // unnecessary from destructor but not elsewhere
    vertices.clear();      // unneccesary from destructor but not elsewhere
    neighbourMap.clear();
    texts.clear();
}

// The publically-accessible version.
VertexPtr Map::insertVertex(QPointF  pt)
{
    return getVertex_Complex( pt);
}

// The "correct" version of inserting a vertex.  Make sure the
// map stays consistent.
VertexPtr Map::getVertex_Complex(QPointF pt)
{
    VertexPtr vert = getVertex_Simple(pt);
    splitEdgesByVertex(vert);
    return vert;
}

// Get a Map Vertex given that we're asserting the vertex
// doesn't lie on an edge in the map.
VertexPtr Map::getVertex_Simple(QPointF pt)
{
    for( int idx = 0; idx < vertices.size(); ++idx )
    {
        VertexPtr  v = vertices.at(idx);
        QPointF  cur = v->getPosition();
        int cmp = lexComparePoints( pt, cur );
        if( cmp == 0 )
        {
            return v;
        }
        else if( cmp < 0 )
        {
            VertexPtr vert = make_shared<Vertex>(pt);
            vertices.insert(idx, vert);
            neighbourMap.insertVertex(vert);
            return vert;
        }
    }

    VertexPtr vert = make_shared<Vertex>(pt);
    vertices.push_back(vert);
    neighbourMap.insertVertex(vert);
    return vert;
}

// Split any edge (there is at most one) that intersects
// this new vertex.  You'll want to make sure this vertex isn't
// a duplicate of one already in the map.  That would just
// make life unpleasant.
void Map::splitEdgesByVertex(VertexPtr vert)
{
    QPointF vp = vert->getPosition();
    qreal    x = vp.x();

    for (auto e : edges)
    {
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
            neighbourMap.replaceNeighbour(v2,e,nedge);

            // Fix up the edge object -- it now points to the
            // intervening edge.
            e->setV2(vert);

            // Insert the new edge.
            insertEdge_Simple(nedge);

            // Update the adjacencies for the splitting vertex
            neighbourMap.insertNeighbour(vert,e);
            neighbourMap.insertNeighbour(vert,nedge);

            //verify("splitEdgesByVertex",false,true);
            return;
        }
    }
    //verify("splitEdgesByVertex",false,true);
}

// Insert the edge connecting two vertices, including updating
// the neighbour lists for the vertices.
EdgePtr Map::insertEdge(VertexPtr v1, VertexPtr v2, bool debug)
{
    Q_ASSERT(vertices.contains(v1));
    Q_ASSERT(vertices.contains(v2));

    EdgePtr e = make_shared<Edge>(v1, v2);

    insertEdge(e,debug);

    return e;
}

void Map::insertEdge(EdgePtr e, bool debug)
{
    insertEdge_Simple(e);

    neighbourMap.insertNeighbour(e->getV1(),e);
    neighbourMap.insertNeighbour(e->getV2(),e);

    if (debug)
    {
        insertDebugMark(e->getV1()->getPosition(),QString());
        insertDebugMark(e->getV2()->getPosition(),QString());
    }
}


EdgePtr Map::insertCurvedEdge(VertexPtr v1, VertexPtr v2, QPointF center, bool isConvex,bool debug)
{
    Q_ASSERT(vertices.contains(v1));
    Q_ASSERT(vertices.contains(v2));

    EdgePtr e = make_shared<Edge>(v1, v2,center, isConvex);
    insertEdge_Simple(e);

    neighbourMap.insertNeighbour(v1,e);
    neighbourMap.insertNeighbour(v2,e);

    if (debug)
    {
        insertDebugMark(v1->getPosition(),QString());
        insertDebugMark(v2->getPosition(),QString());
    }

    return e;
}
// Insert an edge given that we know the edge doesn't interact
// with other edges or vertices other than its endpoints.
void Map::insertEdge_Simple(EdgePtr edge)
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

void Map::removeEdge(EdgePtr e)   // called by wipeout
{
    qDebug() << "removing edge" << e->getTmpEdgeIndex();

    VertexPtr v1  = e->getV1();
    if (v1)
    {
        neighbourMap.removeNeighbour(v1,e);
        if (neighbourMap.numNeighbours(v1) == 0)
        {
            vertices.removeAll(v1);
            neighbourMap.removeVertex(v1);
        }
    }

    VertexPtr v2  = e->getV2();
    if (v2 && v2 != v1)
    {
        neighbourMap.removeNeighbour(v2,e);
        if (neighbourMap.numNeighbours(v2) == 0)
        {
            vertices.removeAll(v2);
            neighbourMap.removeVertex(v2);
        }
    }

    edges.removeAll(e);
 }

void Map::removeVertex(VertexPtr v)
{
    qDebug() << "removing vertex" << v->getTmpVertexIndex();

    QVector<EdgePtr> removeList;
    for (auto edge : edges)
    {
        if (edge->contains(v))
        {
            removeList.push_back(edge);
        }
    }
    for (auto edge : removeList)
    {
        removeEdge(edge);
    }
    vertices.removeAll(v);
 }

MapPtr Map::recreate()
{
    MapPtr ret = make_shared<Map>("recreated map");

    for(auto vert : vertices)
    {
        VertexPtr nv = make_shared<Vertex>(vert->getPosition());
        vert->copy   = nv;
        ret->vertices.push_back(nv);
    }

    for(auto edge : edges)
    {
        EdgePtr ne = make_shared<Edge>(edge->getV1()->copy, edge->getV2()->copy);

        ret->edges.push_back(ne);

        // CSKFIXME -- this could be a bit faster.  Really we could
        // copy the order of the edges leaving a vertex wholesale.
        // I don't think it'll make that big a difference.  The
        // max expected degree of the vertices in the maps we'll be
        // dealing with is 6.

        ret->neighbourMap.insertNeighbour(edge->getV1()->copy,ne);
        ret->neighbourMap.insertNeighbour(edge->getV2()->copy,ne);
    }

    cleanCopy();

    return ret;
}

MapPtr Map::compress()
{
    const Map & cmap = *this;
    MapPtr m = make_shared<Map>(cmap);
    m->dumpMap(false);
    m->setTmpIndices();

    bool changed = false;
    do
    {
        changed = m->joinOneColinearEdgeIgnoringIntersects();
    } while (changed);

    m->dumpMap(false);
    return m;
}

bool Map::joinOneColinearEdgeIgnoringIntersects()
{
    for (auto edge : edges)
    {
        for (auto edge2 : edges)
        {
            if (edge2 == edge)
                continue;

            if (edge2->isColinearAndTouching(edge))
            {
                joinEdges(edge,edge2);
                return true;
            }
        }
    }
    return false;
}

void Map::joinEdges(EdgePtr e1, EdgePtr e2)
{
    // find common vertex
    VertexPtr comV;
    if (e2->contains(e1->getV1()))
        comV = e1->getV1();
    else if (e2->contains(e1->getV2()))
        comV = e1->getV2();
    Q_ASSERT(comV);

#if 0
    combineLinearEdges(e1,e2,comV);
#else
    VertexPtr v1 = e1->getOtherV(comV);
    VertexPtr v2 = e2->getOtherV(comV);

    // make new Edge
    EdgePtr e = make_shared<Edge>(v2,v1);
    e->setTmpEdgeIndex(e1->getTmpEdgeIndex() + 100);
    insertEdge(e);

    // delete other edge and vertex
    removeEdge(e1);
    removeEdge(e2);
    //qDebug() << "Joined" << e1->getTmpEdgeIndex() << "to" << e2->getTmpEdgeIndex() << "making" << e->getTmpEdgeIndex();
#endif

}


void Map::cleanCopy()
{
    for (auto vert : vertices)
    {
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
    qDebug() << "sortVertices";

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
    qDebug() << "sortEdges";

    qSort( edges.begin(), edges.end(), edgeLessThan );
}


void Map::sortAllNeighboursByAngle()
{
    qDebug() << "sortAllNeighboursByAngle";

    neighbourMap.sortByAngle();
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
    if (!config->debugMapEnable)
        return;
    insertEdge(edge->getV1(),edge->getV2(),true);
}

void Map::insertDebugLine(QPointF p1, QPointF p2)
{
    if (!config->debugMapEnable)
        return;

    VertexPtr v1 = insertVertex(p1);
    VertexPtr v2 = insertVertex(p2);
    insertEdge(v1,v2);
}

void Map::insertDebugPolygon(QPolygonF & poly)
{
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
void Map::applyTrivialRigidMotion(QTransform T)
{
    for (auto e = vertices.begin(); e != vertices.end(); e++)
    {
        VertexPtr vert = *e;
        vert->pos = T.map(vert->getPosition());
    }
}

void Map::scale( qreal s )
{
    applyTrivialRigidMotion(QTransform().scale(s,s));
}

void Map::rotate( qreal r )
{
    applyTrivialRigidMotion(QTransform().rotateRadians(r));
}

void Map::translate(qreal x, qreal y)
{
    applyTrivialRigidMotion(QTransform::fromTranslate(x,y));
}

// In the general case, the vertices and edges must be re-sorted.
void Map::applyGeneralRigidMotion(QTransform T)
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

void Map::transformMap(QTransform T)
{
    applyGeneralRigidMotion(T);
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
void Map::mergeVertices(MapPtr other)
{
    QVector<VertexPtr> & your_verts = other->vertices;          // reference
  //NeighbourMap & your_neighbours  = other->getNeighbourMap(); // referemce
    QVector<VertexPtr> my_verts     = vertices;                 // local copy
    NeighbourMap my_neighbours      = neighbourMap;             // local copy
    int my_size                     = my_verts.size();
    int your_size                   = your_verts.size();

    vertices.clear();

    int my_i   = 0;
    int your_i = 0;

    while( true )
    {
        if (my_i == my_size)
        {
            if( your_i == your_size )
            {
                // done!
                return;
            }
            else
            {
                VertexPtr your_v = your_verts.at(your_i);
                vertices.push_back(your_v);
                neighbourMap.insertVertex(your_v);
                your_v->copy = your_v;
                ++your_i;
            }
        }
        else
        {
            if( your_i == your_size )
            {
                VertexPtr vp = my_verts.at(my_i);
                vertices.push_back(vp);
                neighbourMap.insertVertex(vp);
                ++my_i;
            }
            else
            {
                // Darn -- have to actually merge.
                VertexPtr my_v   = my_verts.at(my_i);
                VertexPtr your_v = your_verts.at(your_i);
                int cmp = lexComparePoints(my_v->getPosition(), your_v->getPosition());

                if (cmp < 0)
                {
                    // my_v goes first.
                    vertices.push_back(my_v);
                    neighbourMap.insertVertex(my_v);
                    ++my_i;
                }
                else if (cmp == 0)
                {
                    // It's a toss up.
                    vertices.push_back(my_v);
                    neighbourMap.insertVertex(my_v);
                    your_v->copy = my_v;
                    ++my_i;
                    ++your_i;
                }
                else if (cmp > 0)
                {
                    // your_v goes first.
                    vertices.push_back(your_v);
                    neighbourMap.insertVertex(your_v);
                    your_v->copy = your_v;
                    ++your_i;
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


void Map::mergeMap(MapPtr other)
{
    const bool debug   = false;
    const bool trace   = false;

    bool good_at_start = true;

    if (trace) qDebug() << "** mergeMap start";
    if (debug)
    {
        if (trace) qDebug() << "** mergeMap verify";
        //Sanity checks to use when debugging.
        good_at_start = verifyMap("start merge map");
        if( !good_at_start )
        {
            badness( "Bad at start.");
        }
        if ( !other->verifyMap("other map"))
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
    QVector<QPointF> intersections;

    // 1. Check all intersections between edges from this map
    //    and from the other map.  Record the positions of the
    //    intersections.
    if (trace) qDebug() << "Step 1";
    const QVector<EdgePtr> & oedges = other->getEdges();
    for ( auto edge : oedges)
    {
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
    if (debug) verifyMap("merge map - step 1");

    // 2. For each intersection position, get the vertex in both
    //    maps, forcing edges to split where the intersections will
    //    occur.
    //
    if (trace) qDebug() << "Step 2";
    for(auto it2 = intersections.begin(); it2 != intersections.end(); it2++)
    {
        QPointF p = *it2;
        getVertex_Complex(p);
        other->getVertex_Complex(p);
    }
    if (debug) verifyMap("merge map - step 2");

    // 3. Merge the vertices.
    if (trace) qDebug() << "Step 3";
    mergeVertices(other);       // don't verify here - only vertices are merged

    // 4. Add the edges in the trivial way, since now there will
    //    be no intersections.
    if (trace) qDebug() << "Step 4";
    for (auto edge : oedges)
    {
        VertexPtr v1 = edge->getV1()->copy;
        VertexPtr v2 = edge->getV2()->copy;

        if (edge->getType() == EDGE_LINE)
        {
            insertEdge(v1,v2);
        }
        else
        {
            insertCurvedEdge(v1,v2,edge->getArcCenter(),edge->isConvex());
        }
    }
    setTmpIndices();
    if (debug) verifyMap("merge map-step 4");

    // ... and then sort everything together for speed.
    if (trace) qDebug() << "Step 5";
    sortEdges();
    if (debug) verifyMap("merge map-step 5");

    if (trace) qDebug() << "Step 6";
    cleanNeighbours();
    cleanCopy();
    if (debug) verifyMap("merge map-step 6");

    // I guess that's it!
    bool good_at_end = verifyMap("end merge map");
    if(!good_at_end)
    {
        badness("bad merge" );
    }
    else if (debug)
    {
        qDebug() << "good merge";
    }
    if (trace) qDebug() << "** mergeMap end";
}

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
void Map::mergeSimpleMany(constMapPtr other, const QVector<QTransform> &transforms)
{
    for (auto T : transforms)
    {
        for (auto overt :  other->getVertices())
        {
            // this makes vertex and inserts it in neighbours table
            overt->copy = getVertex_Simple(T.map(overt->getPosition()));
        }

        for (auto oedge : other->getEdges())
        {
            EdgePtr nedge;

            VertexPtr ov1 = oedge->getV1()->copy;
            VertexPtr ov2 = oedge->getV2()->copy;

            if (oedge->getType() == EDGE_LINE)
            {
                nedge = make_shared<Edge>(ov1, ov2);
            }
            else if (oedge->getType() == EDGE_CURVE)
            {
                QPointF pt   = T.map(oedge->getArcCenter());
                bool  convex = oedge->isConvex();
                nedge = make_shared<Edge>(ov1, ov2,pt,convex);
            }

            neighbourMap.insertNeighbour(ov1,nedge);
            neighbourMap.insertNeighbour(ov2,nedge);

            edges.push_back(nedge);
        }
    }

    sortEdges();
    cleanNeighbours();
    cleanCopy();
    setTmpIndices();
}


void Map::divideIntersectingEdges()
{
    qDebug() << "divideIntersectingEdges";

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

#if 0
    sortVertices();
    sortEdges();
    cleanNeighbours();
#endif
}

void Map::joinColinearEdges()
{
    qDebug() << "joinColinearEdges";

    bool changed = false;
    do
    {
        changed = joinOneColinearEdge();
        //verifyMap("joinColinearEdges",false,true);
    } while (changed);

#if 0
    sortVertices();
    sortEdges();
    cleanNeighbours();
    setTmpIndices();
#endif
}

bool Map::joinOneColinearEdge()
{
    for (auto vp : vertices)
    {
        int count = neighbourMap.numNeighbours(vp);
        if (count == 2)
        {
            NeighboursPtr np = neighbourMap.getNeighbours(vp);
            QLineF a = np->getEdge(0)->getLine();
            QLineF b = np->getEdge(1)->getLine();
            qreal angle = a.angle(b);
            if (Loose::zero(angle) || Loose::equals(angle,180.0))
            {
                // need to remove one edge, extend the other, and remove vertex
                combineLinearEdges(np->getEdge(0),np->getEdge(1),vp);
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
    VertexPtr newV1 = a->getOtherV(common);
    VertexPtr newV2 = b->getOtherV(common);

    insertEdge(newV1,newV2);
    removeEdge(a);
    removeEdge(b);

#if 0
    VertexPtr av1 = a->getV1();
    VertexPtr av2 = a->getV2();
    VertexPtr bv1 = b->getV1();
    VertexPtr bv2 = b->getV2();
    VertexPtr modifiedVp;

    // let's keep a, and remove b
    if (av1 == common)
    {
        if (bv1 == common)
        {
            modifiedVp = bv2;
        }
        else
        {
            Q_ASSERT(bv2 == common);
            modifiedVp = bv1;
        }
        a->setV1(modifiedVp);
    }
    else
    {
        Q_ASSERT(av2 == common);
        if (bv1 == common)
        {
            modifiedVp = bv2;
        }
        else
        {
            Q_ASSERT(bv2 == common);
            modifiedVp = bv1;
        }
        a->setV2(modifiedVp);
    }

    // need to change the neighbours for modified vertex
    removeEdge(b);
    //neighbourMap.replaceNeighbour(modifiedVp,b,a);
#endif
}

void Map::splitEdge(EdgePtr e)
{
    verifyMap("split edge start");

    VertexPtr oldV1 = e->getV1();
    VertexPtr oldV2 = e->getV2();

    // remove existing
    removeEdge(e);
    verifyMap("split edge mid");

    // insert new

    QPointF mid = e->getLine().center();
    VertexPtr v = insertVertex(mid);

    EdgePtr e1 = insertEdge(oldV1,v);
    EdgePtr e2 = insertEdge(v,oldV2);

    verifyMap("split edge end");
}

void Map::fixNeighbours()
{
    qDebug() << "fixNeighbours";

    int index = 200;
    for (auto e : edges)
    {
        VertexPtr v1 = e->getV1();
        if (!vertices.contains(v1))
        {
            vertices.push_back(v1);
            v1->setTmpVertexIndex(index++);
        }

        VertexPtr v2 = e->getV2();
        if (!vertices.contains(v2))
        {
            vertices.push_back(v1);
            v2->setTmpVertexIndex(index++);
        }

        NeighboursPtr np = neighbourMap.getNeighbours(v1);
        if (!np)
        {
            neighbourMap.insertNeighbour(v1,e);
        }
        else
        {
            if (!np->contains(e))
            {
                np->insertEdge(e);
            }
        }

        np = neighbourMap.getNeighbours(v2);
        if (!np)
        {
            neighbourMap.insertNeighbour(v2,e);
        }
        else
        {
            if (!np->contains(e))
            {
                np->insertEdge(e);
            }
        }
    }
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

void Map::dumpMap(bool full)
{
    qDebug() << "vertices =" << vertices.size() << "edges =" << edges.size() << "neighbours =" << neighbourMap.size();

    if (full)
    {
        qDebug() << "=== start map" << Utils::addr(this);
        dumpVertices(full);
        dumpEdges(full);
        neighbourMap.dump();
        qDebug() << "=== end  map" << Utils::addr(this);
    }
}

void Map::setTmpIndices() const
{
    for (int idx = 0; idx < vertices.size(); ++idx)
    {
        VertexPtr v = vertices.at(idx);
        v->setTmpVertexIndex(idx);
    }

    for(int idx = 0; idx < edges.size(); ++idx)
    {
        EdgePtr edge = edges.at(idx);
        edge->setTmpEdgeIndex(idx);
    }
}

void Map::dumpVertices(bool full)
{
    for (auto vp : vertices)
    {
        qDebug() <<  "vertex: "  << vp->getTmpVertexIndex() << "at" << vp->getPosition();
        if (full)
        {
            NeighboursPtr np = neighbourMap.getNeighbours(vp);
            np->dumpNeighbours();
        }
    }
}

QString Map::verticesToString()
{
    QString astring;
    QDebug  deb(&astring);

    for (auto v : vertices)
    {
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

    for (auto v : vertices)
    {
        deb << Utils::addr(v.get());
    }
    astring.remove("QPointF");
    return astring;
}

void Map::dumpEdges(bool full)
{
    Q_UNUSED(full)
    for( int idx = 0; idx < edges.size(); ++idx )
    {
        EdgePtr edge = edges.at(idx);
        qDebug() << ((edge->getType() == EDGE_LINE) ? "Line" : "Curve") << "edge" << idx  << Utils::addr(edge.get())<< "from" << vertices.indexOf(edge->getV1()) << "to"  << vertices.indexOf(edge->getV2());
    }
}

// A big 'ole sanity check for maps.  Throw together a whole collection
// of consistency checks.  When debugging maps, call early and call often.
//
// It would probably be better to make this function provide the error
// messages through a return value or exception, but whatever.

bool Map::verifyMap(QString mapname, bool force)
{
    if (!config->verifyMaps && !force)
    {
        return true;
    }

    bool good = true;

    qDebug().noquote() << "$$$$ Verifying:" << mapname << "[" << mname << "]" << "Edges:" << edges.size() << "Vertices:" << vertices.size() << "Neighbours:" << neighbourMap.size() << "Neighbouring edges:" << neighbourMap.countNeighbouringEdges();

    if (vertices.size() == 0 && edges.size() == 0)
    {
        qWarning("empty map");
        return true;
    }

    if (edges.size() == 0)
    {
        qWarning("No edges");
        good = false;
    }

    if (vertices.size() == 0)
    {
        qWarning("No vertices");
        good = false;
    }

    if (neighbourMap.size() == 0)
    {
        qWarning("No neigbours");
        good = false;
    }

    setTmpIndices();

    if (config->verifyDump)
    {
        dumpMap(true);
    }

    if (!verifyVertices())
        good = false;

    if (!verifyEdges())
        good = false;

    if (!verifyNeighbours())
        good = false;

    if (!good)
    {
        badness().noquote() << mapname << "[" << mname << "]" << "did NOT verify!"  << "(Edges:" << edges.size() << "Vertices:" << vertices.size() << "Neighbours:" << neighbourMap.size() << "Neighbouring edges:" << neighbourMap.countNeighbouringEdges() << ")";
    }
    else
    {
        qDebug().noquote() << mapname << "[" << mname << "]" << "verify OK" <<  "(Edges:" << edges.size() << "Vertices:" << vertices.size() << "Neighbours:" << neighbourMap.size() << "Neighbouring edges:" << neighbourMap.countNeighbouringEdges() << ")";
    }

    qDebug() << "$$$$ Verify end";
    return good;
}

bool Map::verifyVertices()
{
    bool rv = true;

    for (auto v : vertices)
    {
        NeighboursPtr np = neighbourMap.getNeighbours(v);
        if (!neighbourMap.contains(v))
        {
            qWarning() << "vertex" << v->getTmpVertexIndex() << "not found in neighbour map";
            rv = false;
        }
        else if (np->numNeighbours() == 0)
        {
            qWarning() << "vertex" << v->getTmpVertexIndex() << "is disconnected - has no neighbours";
            rv = false;
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
            rv = false;
        }
        else if( cmp > 0 )
        {
            qInfo() << "Sortedness check failed for vertices.";
            rv = false;
        }
    }

    return rv;
}

bool Map::verifyEdges()
{
    bool rv = true;

    for (auto edge: edges)
    {
        VertexPtr v1 = edge->getV1();
        VertexPtr v2 = edge->getV2();

        if (v1 == v2)
        {
            qWarning() << "Trivial edge " << edge->getTmpEdgeIndex() << "v1==v2" << v1->getTmpVertexIndex();
            rv = false;
        }

        if (edge->getType() != EDGE_CURVE)
        {
            if (config->verifyVerbose) qDebug() <<  "verifying edge (" << edge->getTmpEdgeIndex() << ") : from" << v1->getTmpVertexIndex() << "to"  << v2->getTmpVertexIndex();
        }
        else
        {
            if (config->verifyVerbose) qDebug() <<  "verifying CURVED edge (" << edge->getTmpEdgeIndex() << ") : from" << v1->getTmpVertexIndex() << "to"  << v2->getTmpVertexIndex();
        }

        if (v1->pos == v2->pos)
        {
            qWarning() << "edge" << edge->getTmpEdgeIndex() << "not really an edge";
            rv = false;
        }

        // Make sure that for every edge, the edge's endpoints
        // are know vertices in the map.
        if (!vertices.contains(v1))
        {
            qWarning() << "edge" << edge->getTmpEdgeIndex() << "V1 not in vertex list.";
            rv = false;
        }
        if (!vertices.contains(v2))
        {
            qWarning() << "edge" << edge->getTmpEdgeIndex() << "V2 not in vertex list.";
            rv = false;
        }

        // Make sure that the edge's endpoints know about the
        // edge, and that the edge is the only connection between
        // the two endpoints.
        //qDebug() << "   V1";
        NeighboursPtr np = neighbourMap.getNeighbours(v1);
        EdgePtr ed       = np->getNeighbour(v2);
        if (!ed)
        {
            qWarning() << "edge" << edge->getTmpEdgeIndex() << "not found in vertex v1 neighbours for vertex" <<  v1->getTmpVertexIndex();
            rv = false;
        }
        else if (ed != edge )
        {
            qWarning() << "edge" << edge->getTmpEdgeIndex() << "not matched in vertex v1 neighbours for vertex" << v1->getTmpVertexIndex();
            if (ed->sameAs(edge))
            {
                qWarning("woops:  edge same as other edge");
            }
            rv = false;
        }

        //qDebug() << "   V2";
        np = neighbourMap.getNeighbours(v2);
        ed = np->getNeighbour(v1);
        if (!ed)
        {
            qWarning() << "edge" << edge->getTmpEdgeIndex() << "not found in vertex v2 neighbours for vertex" <<  v2->getTmpVertexIndex();
            rv = false;
        }
        else if (ed != edge)
        {
            qWarning() << "edge" << edge->getTmpEdgeIndex() << "not found in vertex v2 neighbours for vertex" <<  v2->getTmpVertexIndex();
            rv = false;
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
    return rv;
}
bool Map::verifyNeighbours()
{
    bool rv = true;

    if (!neighbourMap.verify())
    {
        rv = false;
    }

    // Make sure the vertices each have a neighbour and all neigours are good
    for (auto vp : vertices)
    {
        NeighboursPtr np = neighbourMap.getNeighbours(vp);
        if (np->numNeighbours() == 0)
        {
            qWarning() << "Vertex" << vp->getTmpVertexIndex() << "at position" << vp->getPosition() << "has no neigbours, is floating in space";
            rv = false;
        }
        for (auto edge : np->getNeighbours())
        {
            if (!edge)
            {
                qWarning("Bad neighbour: no edge");
                rv = false;
            }
            else if (!edge->getV1())
            {
                qWarning("Bad neighbour edge: no V1");
                rv = false;
            }
            else if (!edge->getV2())
            {
                qWarning("Bad neighbour edge: no V2");
                rv = false;
            }
            if (!edges.contains(edge))
            {
                qWarning() << "Unknown edge" << edge->getTmpEdgeIndex() <<  "in neigbours list";
                rv = false;
            }
        }
    }
    return rv;
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

bool Map::analyzeVertices()
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

void Map::calcVertexEdgeCounts()
{
    // calc
    int vEdgeCounts[MAP_EDGECOUNT_MAX]; // for analysis, indexed by number of edges per vertex

    for (int i=0; i < MAP_EDGECOUNT_MAX; i++)
    {
        vEdgeCounts[i] = 0;
    }

    for (auto v : vertices)
    {
        NeighboursPtr np = neighbourMap.getNeighbours(v);
        int count = np->numEdges();
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
    for (auto v : vertices)
    {
        NeighboursPtr np = neighbourMap.getNeighbours(v);

        if (edgeCount == np->numEdges())
        {
            verts.push_back(v);
        }
    }

    for (auto v : verts)
    {
        removeVertex(v);
    }
}

QRectF Map::calcBoundingRect()
{
   QPolygonF poly;

    QVector<VertexPtr> verts;
    for (auto v : vertices)
    {
        poly << v->getPosition();
    }
    return poly.boundingRect();
}

void Map::cleanNeighbours()
{
    qDebug() << "cleanNeighbours BEGIN edges=" << edges.size()  << "vertices=" << vertices.size();

    for (auto v :  vertices)
    {
        // examining a vertex
        NeighboursPtr np = neighbourMap.getNeighbours(v);
        QVector<EdgePtr> & edgeVec = np->getNeighbours();
        deDuplicateEdges(edgeVec);
    }
    qDebug() << "cleanNeighbours END   edges=" << edges.size()  << "vertices=" << vertices.size();
}

void Map::removeDanglingVertices()
{
    qDebug() << "removeDanglingVertices";

    QVector<VertexPtr> baddies;

    for (auto v : vertices)
    {
        // examining a vertex
        NeighboursPtr np = neighbourMap.getNeighbours(v);
        if (np->numNeighbours() == 0)
        {
            baddies.push_back(v);
        }
    }

    for (auto v : baddies)
    {
        removeVertex(v);
    }
}

void Map::removeBadEdges()
{
    qDebug() << "removeBadEdges";

    QVector<EdgePtr> baddies;

    for (auto e : edges)
    {
        // examining a vertex
        if (e->getV1() == e->getV2())
        {
            qDebug() << "removing null edge (1)" << e->getTmpEdgeIndex();
            baddies.push_back(e);
        }
        else if (e->getV1()->pos == e->getV2()->pos)
        {
            qDebug() << "removing null edge (2)" << e->getTmpEdgeIndex();
            baddies.push_back(e);
        }
        if (!e->getV1())
        {
            qDebug() << "not really an edge (3)" << e->getTmpEdgeIndex();
            baddies.push_back(e);
        }
        if (!e->getV2())
        {
            qDebug() << "not really an edge (4)" << e->getTmpEdgeIndex();
            baddies.push_back(e);
        }
    }

    for (auto e : baddies)
    {
        removeEdge(e);
    }
}

void Map::removeBadEdgesFromNeighboursMap()
{
    qDebug() << "removeBadEdgesFromNeighboursMap";

    for (auto vp : vertices)
    {
        NeighboursPtr np = neighbourMap.getNeighbours(vp);
        QVector<EdgePtr> baddies;
        for (auto edge : np->getNeighbours())
        {
            if (!edges.contains(edge))
            {
                baddies.push_back(edge);
            }
        }
        for (auto edge : baddies)
        {
            np->removeEdge(edge);
        }
    }
}

void Map::cleanse()
{
    const bool debug = false;

    qDebug() << "Map::cleanse - start";
    verifyMap("cleanse-start");

    removeBadEdges();
    if (debug) verifyMap("Map:removeBadEdges");

    fixNeighbours();
    if (debug) verifyMap("Map:fixNeighbours");

    removeDanglingVertices();
    if (debug) verifyMap("Map:removeDanglingVertices");

    removeBadEdgesFromNeighboursMap();
    if (debug) verifyMap("Map:removeBadEdgesFromNeighboursMap");

#if 0
    divideIntersectingEdges();
    if (debug) verifyMap("Map:divideIntersectingEdges");
#endif

#if 0
    joinColinearEdges();
    if (debug) verifyMap("Map:joinColinearEdges");
#endif

    cleanNeighbours();
    if (debug) verifyMap("Map:cleanNeighbours");

    neighbourMap.cleanse();
    if (debug) verifyMap("NeighbourMap::cleanse");

    sortAllNeighboursByAngle();
    if (debug) verifyMap("Map:sortAllNeighboursByAngle");

    sortVertices();
    if (debug) verifyMap("Map:sortVertices");

    sortEdges();

    verifyMap("cleanse-end");
    qDebug() << "Map::cleanse - end";
}
