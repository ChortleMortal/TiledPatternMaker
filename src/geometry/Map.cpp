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

#include "geometry/map.h"
#include "settings/configuration.h"
#include "base/utilities.h"
#include "designs/shapefactory.h"
#include "geometry/intersect.h"
#include "geometry/dcel.h"
#include "geometry/edge.h"
#include "geometry/vertex.h"
#include "geometry/neighbours.h"
#include "geometry/loose.h"

using std::make_shared;

int Map::refs;

Map::Map(QString Name)
{
    refs++;
    mname  = Name;
    config = Configuration::getInstance();
}

Map::Map(QString Name, QPolygonF & poly)
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

Map::Map(QString Name, EdgePoly & poly)
{
    refs++;
    mname  = Name;
    config = Configuration::getInstance();

    // we can do all this because the map is empty and this is a constructor
    for (auto & edge : poly)
    {
        VertexPtr v1 = edge->v1;
        if (!vertices.contains(v1))
        {
            vertices.push_back(v1);
        }
        VertexPtr v2 = edge->v2;
        if (!vertices.contains(v2))
        {
            vertices.push_back(v2);
        }
        edges.push_back(edge);
    }

    sortVertices();
    sortEdges();
    buildNeighbours();
}

Map::Map(const Map & map)
{
    mname        = "copy of "  + map.mname;
    vertices     = map.vertices;
    edges        = map.edges;
}

Map::~Map()
{
#ifdef EXPLICIT_DESTRUCTOR
    qDebug() << "deleting map" << mname;
    if (dcel)
        qDebug() << "has dcel";
    wipeout();
#endif
    refs--;
}

void Map::wipeout()
{
    // better to remove edges before removing vertices
    dcel.reset();
    edges.clear();          // unnecessary from destructor but not elsewhere
    vertices.clear();       // unneccesary from destructor but not elsewhere
    texts.clear();
    status.reset();
}

MapPtr Map::recreate() const
{
    MapPtr ret = make_shared<Map>("recreated map");

    for(auto& vert : vertices)
    {
        VertexPtr nv = make_shared<Vertex>(vert->pt);
        vert->copy   = nv;
        ret->vertices.push_back(nv);
    }

    for(auto edge : qAsConst(edges))
    {
        EdgePtr ne = make_shared<Edge>(edge->v1->copy, edge->v2->copy);
        ne->setSwapState(edge->getSwapState());
        if (edge->getType() == EDGETYPE_CURVE)
        {
            ne->setArcCenter(edge->getArcCenter(),edge->isConvex());
        }

        // CSKFIXME -- this could be a bit faster.  Really we could
        // copy the order of the edges leaving a vertex wholesale.
        // I don't think it'll make that big a difference.  The
        // max expected degree of the vertices in the maps we'll be
        // dealing with is 6.

        ret->_insertEdge(ne);
    }

    _cleanCopy();

    return ret;
}

//////////////////////////////////////////
///
/// Insertions
///
//////////////////////////////////////////

void  Map::insertDirect(VertexPtr v)
{
    vertices.push_back(v);
    status.verticesSorted = false;
}

void Map::insertDirect(EdgePtr e)
{
    edges.push_back(e);
    status.edgesSorted = false;
}

// The publically-accessible version.
// The "correct" version of inserting a vertex.  Make sure the map stays consistent.
VertexPtr Map::insertVertex(QPointF  pt)
{
    VertexPtr vert = _getOrCreateVertex(pt);
    _splitEdgesByVertex(vert);
    return vert;
}

// Insert the edge connecting two vertices, including updating
// the neighbour lists for the vertices.
EdgePtr Map::insertEdge(VertexPtr v1, VertexPtr v2, bool debug)
{
    Q_ASSERT(vertices.contains(v1));
    Q_ASSERT(vertices.contains(v2));

    EdgePtr e = make_shared<Edge>(v1, v2);

    _insertEdge(e,debug);

    return e;
}



void Map::_insertEdge(EdgePtr e, bool debug)
{
    Q_ASSERT(vertices.contains(e->v1));
    Q_ASSERT(vertices.contains(e->v2));

    _insertEdge_Simple(e);

    if (debug)
    {
        insertDebugMark(e->v1->pt,QString());
        insertDebugMark(e->v2->pt,QString());
    }
}

EdgePtr Map::_insertCurvedEdge(VertexPtr v1, VertexPtr v2, QPointF center, bool isConvex,bool debug)
{
    Q_ASSERT(vertices.contains(v1));
    Q_ASSERT(vertices.contains(v2));

    EdgePtr e = make_shared<Edge>(v1, v2,center, isConvex);

    _insertEdge_Simple(e);

    if (debug)
    {
        insertDebugMark(v1->pt,QString());
        insertDebugMark(v2->pt,QString());
    }

    return e;
}

// DAC TODO  - replace with insert edge direct??

// Insert an edge given that we know the edge doesn't interact
// with other edges or vertices other than its endpoints.
void Map::_insertEdge_Simple(EdgePtr edge)
{
    status.neighboursBuilt = false;

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

void Map::addShapeFactory(ShapeFPtr sf)
{
    for (auto it = sf->getPolyforms().begin(); it != sf->getPolyforms().end(); it++)
    {
        Polyform * p = *it;
        switch (p->polytype)
        {
        case POLYGON2:
            _insertPolygon(p);
            break;
        case POLYLINE2:
            _insertPolyline(p);
            break;
        case CIRCLE2:
            qWarning() << "Circle cant be stylized yet";
            break;
        }
    }
}

void Map::_insertPolygon(Polyform  * poly)
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

void Map::_insertPolyline(Polyform * poly)
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

//////////////////////////////////////////
///
/// Deletions
///
//////////////////////////////////////////


void Map::removeVertex(VertexPtr v)
{
    if (!v)
        return;

    //qDebug() << "removing vertex" << vertexIndex(v);

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

    vertices.removeOne(v);
    neighbours.erase(v);
}

void Map::removeVertexSimple(VertexPtr v)
{
    vertices.removeOne(v);
    status.neighboursBuilt = false;
}

void Map::removeEdge(EdgePtr e)   // called by wipeout
{
    if (!e) return;

    edges.removeOne(e);
    status.neighboursBuilt = false;
}

//////////////////////////////////////////
///
/// Modifications
///
//////////////////////////////////////////

void Map::addCropBorder(QRectF rect)
{
    // make a crop map - border has intersection points
    QPolygonF poly(rect);

    MapPtr cropMap = make_shared<Map>(QString("cropped map"),poly);

    mergeMap(cropMap);

    sortEdges();
    sortVertices();
    buildNeighbours();
}

void Map::removeOutisde(QRectF rect)
{
    // remove anything with an outside edge
    QVector<EdgePtr>  outsideEdges;
    for (auto edge : qAsConst(edges))
    {
        if (!Utils::rectContains(rect,edge->v1->pt) || !Utils::rectContains(rect,edge->v2->pt))
        {
            outsideEdges.push_back(edge);
        }
    }

    qDebug() << "outside edges" << outsideEdges.size();
    for (auto edge : outsideEdges)
    {
        removeEdge(edge);
    }

    _cleanseVertices();
}

void Map::scale( qreal s )
{
    _applyTrivialRigidMotion(QTransform().scale(s,s));
}

void Map::rotate( qreal r )
{
    _applyTrivialRigidMotion(QTransform().rotateRadians(r));
}

void Map::translate(qreal x, qreal y)
{
    _applyTrivialRigidMotion(QTransform::fromTranslate(x,y));
}

void Map::transformMap(QTransform T)
{
    _applyGeneralRigidMotion(T);
}

// Applying a motion made up only of uniform scales and translations,
// Angles don't change.  So we can just transform each vertex.
void Map::_applyTrivialRigidMotion(QTransform T)
{
    for (auto e = vertices.begin(); e != vertices.end(); e++)
    {
        VertexPtr vert = *e;
        vert->pt = T.map(vert->pt);
    }
}

// In the general case, the vertices and edges must be re-sorted.
void Map::_applyGeneralRigidMotion(QTransform T)
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

void Map::splitEdge(EdgePtr e)
{
    verify();

    VertexPtr oldV1 = e->v1;
    VertexPtr oldV2 = e->v2;

    // remove existing
    removeEdge(e);
    verify();

    // insert new

    QPointF mid = e->getLine().center();
    VertexPtr v = insertVertex(mid);

    EdgePtr e1 = insertEdge(oldV1,v);
    EdgePtr e2 = insertEdge(v,oldV2);

    verify();
}

// Split any edge (there is at most one) that intersects
// this new vertex.  You'll want to make sure this vertex isn't
// a duplicate of one already in the map.  That would just
// make life unpleasant.
void Map::_splitEdgesByVertex(VertexPtr vert)
{
    while (_splitTwoEdgesByVertex(vert))
        ;
}

bool Map::_splitTwoEdgesByVertex(VertexPtr vert)
{
    QPointF vp = vert->pt;
    //qreal    x = vp.x();

    for (auto e : qAsConst(edges))
    {
#if 0
        qreal xm = e->getMinX();

        if( lexCompareEdges( xm, x ) > 0 )
        {
            // The edges are sorted by xmin, and this xmin exceeds
            // the x value of the vertex.  No more interactions.
            return false;
        }
#endif
        VertexPtr v1 = e->v1;
        VertexPtr v2 = e->v2;

        if( Loose::zero(Point::distToLine(vp, v1->pt, v2->pt ) ) )
        {
            if ( Loose::zero( Point::dist(vp, v1->pt)) ||
                Loose::zero( Point::dist(vp ,v2->pt) ) )
            {
                // Don't split if too near endpoints.
                continue;
            }

            // Create the new edge instance.
            EdgePtr nedge = make_shared<Edge>(vert, v2);

            // We don't need to fix up v1 -- it can still point to the same edge.
            // Fix up v2.
            // Fix up the edge object -- it now points to the intervening edge.
            e->setV2(vert);

            // Insert the new edge.
            _insertEdge_Simple(nedge);

            return true;
        }
    }
    return false;
}

// DAC - not used
void Map::_joinEdges(EdgePtr e1, EdgePtr e2)
{
    // find common vertex
    VertexPtr comV;
    if (e2->contains(e1->v1))
        comV = e1->v1;
    else if (e2->contains(e1->v2))
        comV = e1->v2;
    Q_ASSERT(comV);

#if 0
    combineLinearEdges(e1,e2,comV);
#else
    VertexPtr v1 = e1->getOtherV(comV);
    VertexPtr v2 = e2->getOtherV(comV);

    // make new Edge
    EdgePtr e = make_shared<Edge>(v2,v1);
    _insertEdge(e);

    // delete other edge and vertex
    removeEdge(e1);
    removeEdge(e2);
    //qDebug() << "Joined" << e1->getTmpEdgeIndex() << "to" << e2->getTmpEdgeIndex() << "making" << e->getTmpEdgeIndex();
#endif

}

// Given another vector of vertices, add them to the vertices of the
// current map.  We can do this in linear time with a simple merge
// algorithm.  Note that we want to coalesce identical vertices to
// eliminate duplicates.
void Map::_mergeVertices(MapPtr other)
{
    QVector<VertexPtr> & your_verts = other->vertices;          // reference
    QVector<VertexPtr> my_verts     = vertices;                 // local copy
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
                ++my_i;
            }
            else
            {
                // Darn -- have to actually merge.
                VertexPtr my_v   = my_verts.at(my_i);
                VertexPtr your_v = your_verts.at(your_i);
                int cmp = lexComparePoints(my_v->pt, your_v->pt);

                if (cmp < 0)
                {
                    // my_v goes first.
                    vertices.push_back(my_v);
                    ++my_i;
                }
                else if (cmp == 0)
                {
                    // It's a toss up.
                    vertices.push_back(my_v);
                    your_v->copy = my_v;
                    ++my_i;
                    ++your_i;
                }
                else if (cmp > 0)
                {
                    // your_v goes first.
                    vertices.push_back(your_v);
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

    for (auto & edge : qAsConst(other->edges))
    {
        // To check all intersections of this edge with edges in
        // the current map, we can use the optimization that
        // edges are stored in sorted order by min x.  So at the
        // very least, skip the suffix of edges whose minimum x values
        // are past the max x of the edge to check.

        QPointF ep = edge->v1->pt;
        QPointF eq = edge->v2->pt;

        qreal exm = qMax( ep.x(), eq.x() );

        for (auto cur : edges)
        {
            QPointF cp = cur->v1->pt;
            QPointF cq = cur->v2->pt;

            if (lexCompareEdges( cur->getMinX(), exm ) > 0)
            {
                break;
            }

            QPointF ipt;
            if (Intersect::getTrueIntersection(ep, eq, cp, cq, ipt))
            {
                intersections.push_back(ipt);
            }
        }
    }

    // 2. For each intersection position, get the vertex in both
    //    maps, forcing edges to split where the intersections will occur.
    for (auto pt : intersections)
    {
        insertVertex(pt);
        other->insertVertex(pt);
    }

    // 3. Merge the vertices.
    _mergeVertices(other);       // don't verify map here - only vertices are merged

    // 4. Add the edges in the trivial way, since now there will
    //    be no intersections.
    for (auto edge : qAsConst(other->edges))
    {
        VertexPtr v1 = edge->v1->copy;
        VertexPtr v2 = edge->v2->copy;
        if (_edgeExists(v1,v2))
        {
            continue;           // fix 13MAR21
        }

        if (edge->getType() == EDGETYPE_LINE)
        {
            insertEdge(v1,v2);
        }
        else
        {
            _insertCurvedEdge(v1,v2,edge->getArcCenter(),edge->isConvex());
        }
    }

    _cleanCopy();    // DAC required:  copies made by merge vertices

    // I guess that's it!
}

// It's often the case that we want to merge a transformed copy of
// a map into another map, or even a collection of transformed copies.
// Since transforming a map requires a slow cloning, we can save lots
// of time and memory by transforming and merging simultaneously.
// Here, we transform vertices as they are put into the current map.
void Map::mergeSimpleMany(constMapPtr other, const QVector<QTransform> &transforms)
{
    for (auto& T : transforms)
    {
        for (auto& overt :  other->vertices)
        {
            // this makes vertex and inserts it in neighbours table
            overt->copy = _getOrCreateVertex(T.map(overt->pt));
        }

        for (auto oedge : other->edges)
        {
            EdgePtr nedge;

            VertexPtr ov1 = oedge->v1->copy;
            VertexPtr ov2 = oedge->v2->copy;

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

            edges.push_back(nedge);

            status.neighboursBuilt = 0;
        }
    }
    _cleanCopy();
}

void Map::mergeMany(constMapPtr other, const QVector<QTransform> &transforms)
{
    for (auto& T : transforms)
    {
        MapPtr m  = other->recreate();
        m->transformMap(T);
        mergeMap(m);
    }
}

void Map::sortVertices()
{
    //qDebug() << "sortVertices";
    std::sort( vertices.begin(),vertices.end(),vertexLessThan);
}

void Map::sortEdges()
{
    //qDebug() << "sortEdges";
    std::sort( edges.begin(), edges.end(), edgeLessThan );
}

void Map::buildNeighbours()
{
    for (auto & v : qAsConst(vertices))
    {
        NeighboursPtr n = getRawNeighbours(v);
        n->eraseNeighbours();
    }

    for (auto & e : qAsConst(edges))
    {
        NeighboursPtr n = getRawNeighbours(e->v1);
        n->insertNeighbour(e);
        NeighboursPtr n2 = getRawNeighbours(e->v2);
        n2->insertNeighbour(e);
    }

    status.neighboursBuilt = true;

    _cleanseVertices();  // removes unconnected vertices

}


//////////////////////////////////////////
///
/// Getters
///
//////////////////////////////////////////

NeighboursPtr  Map::getBuiltNeighbours(VertexPtr v)
{
    if (!status.neighboursBuilt)
    {
        buildNeighbours();
    }
    NeighboursPtr np = neighbours[v];
    Q_ASSERT(np);
    return np;
}

NeighboursPtr  Map::getRawNeighbours(VertexPtr v)
{
    NeighboursPtr np = neighbours[v];
    if (!np)
    {
        np = make_shared<Neighbours>(v);
        neighbours[v] = np;
    }
    return np;
}

DCELPtr Map::getDCEL()
{
    if (!dcel)
    {
        dcel = make_shared<DCEL>(shared_from_this());
#if 0
        dcel->displayDCEL(0x08);
#endif
    }
    return dcel;
}

EdgePoly Map::getEdgePoly() const
{
    EdgePoly ep;
    for (auto& edge : edges)
    {
        ep.push_front(edge);  // yes front
    }
    ep.dump();
    return ep;
}

// Get a Map Vertex given that we're asserting the vertex
// doesn't lie on an edge in the map.
VertexPtr Map::_getOrCreateVertex(QPointF pt)
{
    for( int idx = 0; idx < vertices.size(); ++idx )
    {
        VertexPtr  v = vertices.at(idx);
        QPointF  cur = v->pt;
        int cmp = lexComparePoints( pt, cur );
        if( cmp == 0 )
        {
            return v;
        }
        else if( cmp < 0 )
        {
            VertexPtr vert = make_shared<Vertex>(pt);
            vertices.insert(idx, vert);
            return vert;
        }
    }

    VertexPtr vert = make_shared<Vertex>(pt);
    vertices.push_back(vert);
    return vert;
}


//////////////////////////////////////////
///
/// Info
///
//////////////////////////////////////////

QString Map::summary() const
{
    return QString("vertices=%1 edges=%2").arg(vertices.size()).arg(edges.size());
}

QString Map::displayVertexEdgeCounts()
{
    // calc
    int vEdgeCounts[MAP_EDGECOUNT_MAX]; // for analysis, indexed by number of edges per vertex

    for (int i=0; i < MAP_EDGECOUNT_MAX; i++)
    {
        vEdgeCounts[i] = 0;
    }

    for (auto& v : vertices)
    {
        NeighboursPtr n = getBuiltNeighbours(v);
        int count = n->numNeighbours();
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


bool Map::isEmpty() const
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


bool Map::hasIntersectingEdges() const
{
    UniqueQVector<QPointF> intersects;
    for(auto edge : qAsConst(edges))
    {
        // To check all intersections of this edge with edges in
        // the current map, we can use the optimization that
        // edges are stored in sorted order by min x.  So at the
        // very least, skip the suffix of edges whose minimum x values
        // are past the max x of the edge to check.
        // Casper 12DEC02 - removed optimisation and simplified code

        QLineF e = edge->getLine();
        for (auto cur : qAsConst(edges))
        {
            if (cur == edge)
            {
                continue;
            }

            QLineF  c  = cur->getLine();
            QPointF apt;
            if (Intersect::getTrueIntersection(e, c, apt))
            {
                return true;
            }
        }
    }
    return false;
}

bool Map::_edgeExists(VertexPtr v1, VertexPtr v2) const
{
    for (auto & edge : edges)
    {
        if (edge->sameAs(v1,v2))
        {
            return true;
        }
    }
    return false;
}

//////////////////////////////////////////
///
/// Debug
///
//////////////////////////////////////////

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
    insertEdge(edge->v1,edge->v2,true);
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


void Map::dumpMap(bool full)
{
    qDebug() << "vertices =" << vertices.size() << "edges =" << edges.size();

    if (full)
    {
        qDebug() << "=== start map" << this;
        _dumpVertices(full);
        _dumpEdges(full);
        qDebug() << "=== end  map" << this;
    }
}

void Map::_dumpVertices(bool full)
{
    for (auto & vp : qAsConst(vertices))
    {
        qDebug() <<  "vertex: "  << vertexIndex(vp) << "at" << vp->pt;
        if (full)
        {
            NeighboursPtr n = getRawNeighbours(vp);
            n->dumpNeighbours();
        }
    }
}

void Map::_dumpEdges(bool full) const
{
    Q_UNUSED(full)
    int idx = 0;
    for(auto edge : qAsConst(edges))
    {
        qDebug() << ((edge->getType() == EDGETYPE_LINE) ? "Line" : "Curve") << "edge" << idx++ << Utils::addr(edge.get())
                 << "from" << vertices.indexOf(edge->v1) << edge->v1->pt
                 << "to"   << vertices.indexOf(edge->v2) << edge->v2->pt;
    }
}

//////////////////////////////////////////
///
/// Cleanse
///
//////////////////////////////////////////

void Map::_cleanCopy() const
{
    for (auto& vert : vertices)
    {
        vert->copy.reset();
    }
}

void Map::_cleanseVertices()
{
    if (!status.neighboursBuilt)
    {
        buildNeighbours();
    }
    std::vector<VertexPtr> baddies;
    for (auto & v : vertices)
    {
        NeighboursPtr n = getRawNeighbours(v);
        if (n->size() == 0)
        {
            baddies.push_back(v);
        }
    }
    for (auto & v  : baddies)
    {
        vertices.removeOne(v);
        neighbours.erase(v);
    }
}

//////////////////////////////////////////
///
/// Utilities
/// Routines used for spatial sorting of edges and vertices.
///
//////////////////////////////////////////


int Map::lexCompareEdges(qreal a, qreal b)
{
    qreal d = a - b;

    if (Loose::zero(d))
    {
        return 0;
    }
    else if (d < 0.0)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

int Map::lexComparePoints(QPointF a, QPointF b)
{
    bool verbose = false;

    qreal dx = a.x() - b.x();

    if (Loose::zero(dx))
    {
        qreal dy = a.y() - b.y();

        if (Loose::zero(dy))
        {
            if (verbose) qDebug () << a << b << "L0";
            return 0;
        }
        else if (dy < 0.0)
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
    else if (dx < 0.0)
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
    if (lexComparePoints( a->pt, b->pt) == -1)
        return true;
    else
        return false;
}

bool Map::edgeLessThan( EdgePtr  a,  EdgePtr  b )
{
    int rv = lexCompareEdges( a->getMinX(), b->getMinX() );
    if (rv ==  -1)
        return true;
    else
        return false;
}

void Map::draw(QPainter * painter)
{
    for (EdgePtr edge : edges)
    {
        painter->drawLine(edge->getLine());
    }
}
