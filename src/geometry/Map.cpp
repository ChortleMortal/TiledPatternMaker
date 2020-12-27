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

#include <QtAlgorithms>
#include "geometry/map.h"
#include "base/configuration.h"
#include "base/misc.h"
#include "base/utilities.h"
#include "designs/shapefactory.h"
#include "geometry/edgepoly.h"
#include "geometry/intersect.h"
#include "geometry/loose.h"
#include "geometry/map_cleanser.h"

int Map::refs;

Map::Map(QString Name) : neighbourMap(this)
{
    refs++;
    mname  = Name;
    config = Configuration::getInstance();
}

Map::Map(QString Name, QPolygonF & poly) : neighbourMap(this)
{
    refs++;
    mname  = Name;
    config = Configuration::getInstance();

    VertexPtr v1 = insertVertex(poly[0]);
    VertexPtr v2;
    for (int i=1; i < poly.size(); i++)
    {
        v2 = insertVertex(poly[i]);
        insertEdge(v1,v2);
        v1 = v2;
    }
    if (!poly.isClosed())
    {
        v2 = insertVertex(poly[0]);
        insertEdge(v1,v2);
    }
}

Map::Map(QString Name, EdgePoly & poly) : neighbourMap(this)
{
    refs++;
    mname  = Name;
    config = Configuration::getInstance();

    // we can do all this because the map is empty and this is a constructor
    for (auto edge : poly)
    {
        VertexPtr v1 = edge->getV1();
        if (!vertices.contains(v1))
        {
            vertices.push_back(v1);
        }
        VertexPtr v2 = edge->getV2();
        if (!vertices.contains(v2))
        {
            vertices.push_back(v2);
        }
        edges.push_back(edge);

        neighbourMap.insertNeighbour(edge->getV1(),edge);
        neighbourMap.insertNeighbour(edge->getV2(),edge);
    }
    sortVertices();
    sortAllNeighboursByAngle();
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
        for (auto& edge : np->getNeighbours())
        {
            nnp->insertEdgeSimple(edge);
        }
        ourmap[v] = nnp;
    }
}

Map::~Map()
{
    //qDebug() << "deleting map" << mname;
    refs--;
    wipeout();
}

void Map::wipeout()
{
    // better to remove edges before removing vertices
    edges.clear();          // unnecessary from destructor but not elsewhere
    vertices.clear();       // unneccesary from destructor but not elsewhere
    neighbourMap.clear();
    texts.clear();
}

EdgePoly Map::getEdgePoly()
{
    EdgePoly ep;
    for (auto& edge : edges)
    {
        ep.push_back(edge);
    }
    return ep;
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

    for (auto e : qAsConst(edges))
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
    if (!e) return;

    //qDebug() << "removing edge" << e->getTmpEdgeIndex();

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
    if (!v)
        return;
    qDebug() << "removing vertex" << v->getTmpVertexIndex();

    QVector<EdgePtr> removeList;
    for (auto edge : qAsConst(edges))
    {
        if (edge->contains(v))
        {
            removeList.push_back(edge);
        }
    }
    for (auto& edge : removeList)
    {
        removeEdge(edge);
    }

    vertices.removeAll(v);

    neighbourMap.removeVertex(v);
}

void Map::crop(QRectF rect)
{
    // make a crop map - border has intersection points
    QPolygonF poly(rect);

    MapPtr cropMap = make_shared<Map>(QString("cropped map"),poly);

    mergeMap(cropMap);

    // remove anything with an outside edge
    QVector<EdgePtr> outsideEdges;
    for (auto edge : qAsConst(edges))
    {
        if (!rect.contains(edge->getV1()->getPosition()) || !rect.contains(edge->getV2()->getPosition()))
        {
            outsideEdges.push_back(edge);
        }
    }

    for (auto& edge : outsideEdges)
    {
        removeEdge(edge);
    }
}

MapPtr Map::recreate()
{
    MapPtr ret = make_shared<Map>("recreated map");

    for(auto& vert : vertices)
    {
        VertexPtr nv = make_shared<Vertex>(vert->getPosition());
        vert->copy   = nv;
        ret->vertices.push_back(nv);
    }

    for(auto edge : qAsConst(edges))
    {
        EdgePtr ne = make_shared<Edge>(edge->getV1()->copy, edge->getV2()->copy);
        ne->setSwapState(edge->getSwapState());
        if (edge->getType() == EDGETYPE_CURVE)
        {
            ne->setArcCenter(edge->getArcCenter(),edge->isConvex());
        }

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
    for (auto& edge : qAsConst(edges))
    {
        for (auto edge2 : qAsConst(edges))
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
    for (auto& vert : vertices)
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

    std::sort( vertices.begin(),vertices.end(),vertexLessThan);
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

    std::sort( edges.begin(), edges.end(), edgeLessThan );
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

void Map::insertDebugMark(QPointF m, QString txt, qreal size, QPointF offset)
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
        stxt.pt  = m + offset;
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

    MapCleanser cleanMap(shared_from_this());
    MapCleanser cleanOther(other);

    if (trace) qDebug() << "** mergeMap start";
    if (debug)
    {
        if (trace) qDebug() << "** mergeMap verify";
        //Sanity checks to use when debugging.
        good_at_start = cleanMap.verifyMap("start merge map");
        if(!good_at_start)
        {
            badness("Bad at start.");
        }
        if (!cleanOther.verifyMap("other map"))
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
    UniqueQVector<QPointF> intersections;

    // 1. Check all intersections between edges from this map
    //    and from the other map.  Record the positions of the
    //    intersections.

    if (trace) qDebug() << "Step 1";
    const QVector<EdgePtr> & oedges = other->getEdges();
    for (auto edge : oedges)
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

            QPointF ipt;
            if (Intersect::getTrueIntersection( ep, eq, cp, cq, ipt))
            {
                intersections.push_back( ipt );
            }
        }
    }
    if (debug) cleanMap.verifyMap("merge map - step 1");

    // 2. For each intersection position, get the vertex in both
    //    maps, forcing edges to split where the intersections will occur.

    if (trace) qDebug() << "Step 2";
    for(auto pt : intersections)
    {
        getVertex_Complex(pt);
        other->getVertex_Complex(pt);
    }
    if (debug)cleanMap.verifyMap("merge map - step 2");

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

        if (edge->getType() == EDGETYPE_LINE)
        {
            insertEdge(v1,v2);
        }
        else
        {
            insertCurvedEdge(v1,v2,edge->getArcCenter(),edge->isConvex());
        }
    }
    if (debug) cleanMap.verifyMap("merge map-step 4");

    // ... and then sort everything together for speed.
    if (trace) qDebug() << "Step 5";
    sortEdges();
    if (debug) cleanMap.verifyMap("merge map-step 5");

    if (trace) qDebug() << "Step 6";
    cleanMap.cleanNeighbours();
    cleanCopy();
    if (debug) cleanMap.verifyMap("merge map-step 6");

    // I guess that's it!
    bool good_at_end = cleanMap.verifyMap("end merge map");
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
    for (auto& T : transforms)
    {
        for (auto& overt :  other->getVertices())
        {
            // this makes vertex and inserts it in neighbours table
            overt->copy = getVertex_Simple(T.map(overt->getPosition()));
        }

        for (auto oedge : other->getEdges())
        {
            EdgePtr nedge;

            VertexPtr ov1 = oedge->getV1()->copy;
            VertexPtr ov2 = oedge->getV2()->copy;

            if (oedge->getType() == EDGETYPE_LINE)
            {
                nedge = make_shared<Edge>(ov1, ov2);
            }
            else if (oedge->getType() == EDGETYPE_CURVE)
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
    MapCleanser cleanmap(shared_from_this());
    cleanmap.cleanNeighbours();
    cleanCopy();
}

void Map::splitEdge(EdgePtr e)
{
    MapCleanser cleanser(shared_from_this());
    cleanser.verifyMap("split edge start");

    VertexPtr oldV1 = e->getV1();
    VertexPtr oldV2 = e->getV2();

    // remove existing
    removeEdge(e);
    cleanser.verifyMap("split edge mid");

    // insert new

    QPointF mid = e->getLine().center();
    VertexPtr v = insertVertex(mid);

    EdgePtr e1 = insertEdge(oldV1,v);
    EdgePtr e2 = insertEdge(v,oldV2);

    cleanser.verifyMap("split edge end");
}

// Print a text version of the map.

QString Map::getInfo() const
{
    return QString("Map: vertices = %1  edges = %2").arg(vertices.size()).arg(edges.size());
}

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
        qDebug() << "=== start map" << this;
        dumpVertices(full);
        dumpEdges(full);
        neighbourMap.dump();
        qDebug() << "=== end  map" << this;
    }
}

void Map::dumpVertices(bool full)
{
    for (auto vp : qAsConst(vertices))
    {
        qDebug() <<  "vertex: "  << vp->getTmpVertexIndex() << "at" << vp->getPosition();
        if (full)
        {
            NeighboursPtr np = neighbourMap.getNeighbours(vp);
            np->dumpNeighbours();
        }
    }
}

void Map::dumpEdges(bool full)
{
    Q_UNUSED(full)
    int idx = 0;
    for(auto edge : qAsConst(edges))
    {
        qDebug() << ((edge->getType() == EDGETYPE_LINE) ? "Line" : "Curve") << "edge" << idx++ << Utils::addr(edge.get())
                 << "from" << vertices.indexOf(edge->getV1()) << edge->getV1()->getPosition()
                 << "to"   << vertices.indexOf(edge->getV2()) << edge->getV2()->getPosition();
    }
}

void Map::addShapeFactory(ShapeFPtr sf)
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

QString Map::calcVertexEdgeCounts()
{
    // calc
    int vEdgeCounts[MAP_EDGECOUNT_MAX]; // for analysis, indexed by number of edges per vertex

    for (int i=0; i < MAP_EDGECOUNT_MAX; i++)
    {
        vEdgeCounts[i] = 0;
    }

    for (auto& v : vertices)
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
    return str;
}
