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

#include <QDebug>
#include <QStack>

#include "gui/viewers/geo_graphics.h"
#include "legacy/shapefactory.h"
#include "model/settings/configuration.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/intersect.h"
#include "sys/geometry/loose.h"
#include "sys/geometry/map.h"
#include "sys/geometry/map_cleanser.h"
#include "sys/geometry/neighbour_map.h"
#include "sys/geometry/vertex.h"

using std::make_shared;

int     Map::refs = 0;
QPointF Map::tmpCenter = QPointF();

Map::Map(const QString & name)
{
    refs++;
    mname  = name;
}

Map::Map(const QString &  name, const QPolygonF & poly)
{
    refs++;
    mname  = name;

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

Map::Map(QString Name, const EdgePoly &poly)
{
    refs++;
    mname  = Name;

    // we can do all this because the map is empty and this is a constructor
    for (auto & edge : poly.get())
    {
        VertexPtr v1 = edge->v1;
        vertices.push_back(v1);

        VertexPtr v2 = edge->v2;
        vertices.push_back(v2);

        edges.push_back(edge);
    }

    //sortVertices();
    //sortEdges();
    //buildNeighbours();
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

void Map::clear()
{
    // better to remove edges before removing vertices
    derivedDCEL.reset();
    MapBase::wipeout();
}

void Map::set(const constMapPtr & other)
{
    clear();
    vertices = other->vertices;
    edges    = other->edges;
}

MapPtr Map::copy() const
{
    MapPtr newmap    = make_shared<Map>("copy of " + mname);
    newmap->vertices = vertices;
    newmap->edges    = edges;
    return newmap;
}

MapPtr Map::recreate() const
{
    MapPtr ret = make_shared<Map>("recreated map");

    for (const auto & vert : std::as_const(vertices))
    {
        VertexPtr nv = make_shared<Vertex>(vert->pt);
        vert->copy   = nv;
        ret->vertices.push_back(nv);
    }

    for (const auto & edge : std::as_const(edges))
    {
        EdgePtr ne = make_shared<Edge>(edge->v1->copy.lock(), edge->v2->copy.lock());

        if (edge->getType() == EDGETYPE_CURVE)
        {
            ne->chgangeToCurvedEdge(edge->getArcCenter(),edge->getCurveType());
        }

        ret->_insertEdgeSimple(ne);
    }

    _cleanCopy();

    return ret;
}

void Map::rebuildD()
{
    qDebug() << "Map::rebuild";
    qDebug().noquote() << info();

    MapPtr ret = make_shared<Map>("recreated map");

    for (const auto & edge : std::as_const(edges))
    {
        VertexPtr v1 = ret->insertVertex(edge->v1->pt);
        VertexPtr v2 = ret->insertVertex(edge->v2->pt);
        ret->insertEdge(v1,v2);
    }

    edges.clear();
    vertices.clear();

    vertices = ret->vertices;
    edges    = ret->edges;

    qDebug().noquote() << info();
}

//////////////////////////////////////////
///
/// Insertions
///
//////////////////////////////////////////

// Insert the edge connecting two vertices
EdgePtr Map::insertEdge(const VertexPtr & v1, const VertexPtr & v2)
{
    Q_ASSERT(vertices.contains(v1));
    Q_ASSERT(vertices.contains(v2));

    if (v1 == v2)
    {
        qWarning() << "Map::insertEdge - zero length v1 = v2";
    }

    auto edge = edgeExists(v1,v2);
    if (!edge)
    {
        edge = make_shared<Edge>(v1, v2);
        _insertEdge(edge);
    }
    return edge;
}

EdgePtr Map::insertEdge(const VertexPtr & v1, const VertexPtr & v2, const QPointF & arcCenter, eCurveType ctype)
{
    Q_ASSERT(vertices.contains(v1));
    Q_ASSERT(vertices.contains(v2));

    if (v1 == v2)
    {
        qWarning() << "Map::insertEdge - zero length v1 = v2";
    }

    auto edge = edgeExists(v1,v2);
    if (!edge)
    {
        edge = make_shared<Edge>(v1, v2, arcCenter, ctype);
        _insertEdge(edge);
    }
    return edge;
}


void Map::_insertEdge(const EdgePtr & cutter)
{
    auto isects = findIntersections(cutter);

    if (isects.isEmpty())
    {
        // Add the edges in the trivial way, since now there are no intersections.
        _insertEdgeSimple(cutter);
    }
    else
    {
        processIntersections(isects);
    }
}

#if 0
EdgePtr Map::insertEdge(const QLineF & line)
{
    return insertEdge(line.p1(),line.p2());
}

EdgePtr Map::insertEdge(const QPointF & p1, const QPointF & p2)
{
    VertexPtr v1 = insertVertex(p1);
    VertexPtr v2 = insertVertex(p2);

    return insertEdge(v1,v2);
}
#endif

void  Map:: XmlInsertDirect(VertexPtr v)
{
    vertices.push_back(v);
}

void Map::XmlInsertDirect(EdgePtr e)
{
    edges.push_back(e);
}

// The publically-accessible version.
// The "correct" version of inserting a vertex.  Make sure the map stays consistent.
VertexPtr Map::insertVertex(const QPointF & pt)
{
    VertexPtr vert = _getOrCreateVertex(pt);
    _splitEdgesByVertex(vert);
    return vert;
}

VertexPtr Map:: getVertex(const QPointF & pt) const
{
    for (const auto & v : std::as_const(vertices))
    {
        if (Loose::equalsPt(v->pt,pt))
        {
            return v;
        }
    }
    VertexPtr vp;
    return vp;
}

EdgePtr Map::makeCopy(const EdgePtr & e, QTransform T)
{
    QPointF p1 = T.map(e->v1->pt);
    QPointF p2 = T.map(e->v2->pt);
    EdgePtr ep = make_shared<Edge>(_getOrCreateVertex(p1),_getOrCreateVertex(p2));
    if (e->isCurve())
    {
        QPointF ac = T.map(e->getArcCenter());
        ep->chgangeToCurvedEdge(ac,e->getCurveType());
    }
    return ep;
}

EdgePtr Map::_insertCurvedEdge(const VertexPtr & v1, const VertexPtr & v2, const QPointF & center, eCurveType ctype)
{
    EdgePtr e = make_shared<Edge>(v1, v2,center, ctype);

    _insertEdgeSimple(e);

    return e;
}

// Insert an edge given that we know the edge doesn't interact
// with other edges or vertices other than its endpoints.
void Map::_insertEdgeSimple(const EdgePtr & edge)
{
    Q_ASSERT(vertices.contains(edge->v1));
    Q_ASSERT(vertices.contains(edge->v2));
    // NOTE - there is no need to do any further validation here
    // this has been tested and the UniqueQVector catches everything

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


void Map::removeVertex(const VertexPtr & v)
{
    if (!v)
        return;

    //qDebug() << "removing vertex" << vertexIndex(v);

    QVector<EdgePtr> removeList;
    for (const auto & edge : std::as_const(edges))
    {
        if (edge->contains(v))
        {
            removeList.push_back(edge);
        }
    }

    for (const auto & edge : std::as_const(removeList))
    {
        removeEdge(edge);
    }

    vertices.removeOne(v);
}

void Map::removeVertexSimple(const VertexPtr &v)
{
    vertices.removeOne(v);
}

void Map::removeEdge(const EdgePtr & e)   // called by wipeout
{
    if (!e) return;

    edges.removeOne(e);
}

//////////////////////////////////////////
///
/// Modifications
///
//////////////////////////////////////////

void Map::embedCrop(const QRectF &rect)
{
    QPolygonF poly(rect);

    embedCrop(poly);
}

void Map::embedCrop(const QPolygonF &poly)
{
    // make a crop map - border has intersection points
    MapPtr cropMap = make_shared<Map>(QString("cropped map"),poly);

    mergeMap(cropMap);
}

bool Map::vertexAngleGreaterThan(const VertexPtr & a, const VertexPtr & b)
{
    return (QLineF(tmpCenter,a->pt).angle() > QLineF(tmpCenter,b->pt).angle());
}

void Map::embedCrop(const Circle & circle)
{
    // this assumes buildEditorDB() has been called which has created an array of points
    // some of these points are touhing a circle, whihc is the one we want
    // DAC - might be better if the info for PT_CIRCLE indicated which crop circle

    QVector<QPointF> points;
    for (auto & edge : std::as_const(edges))
    {
        QLineF line = edge->getLine();
        QPointF a;
        QPointF b;
        int count = Geo::findLineCircleIntersections(circle.centre,  circle.radius, line, a, b);
        if (count == 1)
        {
            // this is a tangent line
            points.push_back(a);
        }
        else if (count == 2)
        {
            if (Geo::pointOnLine(line,a))
            {
                points.push_back(a);
            }

            if (Geo::pointOnLine(line,b))
            {
                points.push_back(b);
            }
        }
    }

    qDebug() << "points" << points.size();
    QVector<VertexPtr> cverts;
    for (const auto & pt : std::as_const(points))
    {
        auto v = insertVertex(pt);
        cverts.push_front(v);
    }
    if (cverts.front() != cverts.last())
    {
        // completes the circle
        cverts.push_front(cverts.last());
    }

    // points need to be clockwise around circle
    std::sort(cverts.begin(),cverts.end(),vertexAngleGreaterThan);

    QPointF center =  circle.centre;
    if (cverts.size() >=2)
    {
        tmpCenter = center;
        for (int i=0; i < cverts.size(); i++)
        {
            auto v1 = cverts[i];
            int j = i+1;
            if (j == cverts.size())
                j=0;
            auto v2 = cverts[j];
            auto e = edgeExists(v1,v2);

            if (v1->equals(v2))
                continue;

            if (!e)
            {
                // crete new edge
                insertEdge(v1,v2,center,CURVE_CONVEX);
            }
            else
            {
                // convert line to chord
                e->chgangeToCurvedEdge(center,CURVE_CONVEX);
            }
        }
    }
    else
    {
        EdgePoly ep(circle);
        auto nmap = make_shared<Map>("Circle",ep);
        mergeMap(nmap);
    }
}

void Map::cropOutside(const QRectF &rect)
{
    // remove anything with an outside edge
    QVector<EdgePtr>  outsideEdges;
    for (auto & edge : std::as_const(edges))
    {
        if (!Geo::rectContains(rect,edge->v1->pt) || !Geo::rectContains(rect,edge->v2->pt))
        {
            outsideEdges.push_back(edge);
        }
    }

    qDebug() << "outside edges" << outsideEdges.size();
    for (auto & edge : std::as_const(outsideEdges))
    {
        removeEdge(edge);
    }

    MapCleanser mc(this);
    mc.cleanseVertices();
}

void  Map::cropOutside(const QPolygonF & poly)
{
    // remove anything with an outside edge
    QVector<EdgePtr>  outsideEdges;
    for (auto & edge : std::as_const(edges))
    {
        auto p1 = edge->v1->pt;
        auto p2 = edge->v2->pt;

        bool t1 = poly.containsPoint(p1,Qt::OddEvenFill);
      //bool t2 = poly.contains(p1);
      //bool t3 = Geo::pointInPolygon(p1,poly);
      //bool t4 = Geo::point_in_polygon(p1,poly);
        bool t5 = Geo::point_on_poly_edge(p1,poly);

        bool t12 = poly.containsPoint(p2,Qt::OddEvenFill);
      //bool t22 = poly.contains(p2);
      //bool t32 = Geo::pointInPolygon(p2,poly);
      //bool t42 = Geo::point_in_polygon(p2,poly);
        bool t52 = Geo::point_on_poly_edge(p2,poly);

        if ((!t1 && !t5) || (!t12 && !t52))
        {
            outsideEdges.push_back(edge);
        }
    }

    qDebug() << "outside edges" << outsideEdges.size();
    for (auto & edge : std::as_const(outsideEdges))
    {
        removeEdge(edge);
    }

    MapCleanser mc(this);
    mc.cleanseVertices();
}

void Map::cropOutside(const Circle &circle)
{
    // remove anything outside of circle
    QVector<EdgePtr>  outsideEdges;
    for (auto & edge : std::as_const(edges))
    {
        QPointF p1 = edge->v1->pt;
        QPointF p2 = edge->v2->pt;
        if (!Geo::pointInCircle(p1,circle) && !Geo::pointOnCircle(p1,circle))
        {
            outsideEdges.push_back(edge);
        }
        else if (!Geo::pointInCircle(p2,circle) && !Geo::pointOnCircle(p2,circle))
        {
            outsideEdges.push_back(edge);
        }
    }

    qDebug() << "outside edges" << outsideEdges.size();
    for (auto & edge : std::as_const(outsideEdges))
    {
        removeEdge(edge);
    }

    MapCleanser mc(this);
    mc.cleanseVertices();
}

MapPtr Map::getTransformed(const QTransform &T) const
{
    MapPtr m2 = recreate();
    m2->transform(T);
    return m2;
}

void Map::splitEdge(EdgePtr e)
{
    VertexPtr oldV1 = e->v1;
    VertexPtr oldV2 = e->v2;

    // remove existing
    removeEdge(e);

    // insert new
    QPointF mid = e->getLine().center();
    VertexPtr v = insertVertex(mid);

    insertEdge(oldV1,v);
    insertEdge(v,oldV2);
}

// Split any edge (there is at most one) that intersects
// this new vertex.  You'll want to make sure this vertex isn't
// a duplicate of one already in the map.  That would just
// make life unpleasant.
void Map::_splitEdgesByVertex(const VertexPtr & vert)
{
    while (_splitTwoEdgesByVertex(vert))
        ;
}

bool Map::_splitTwoEdgesByVertex(const VertexPtr & vert)
{
    // FIXME - should this handle curves?
    QPointF vp = vert->pt;
    //qreal    x = vp.x();

    for (const auto & e : std::as_const(edges))
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
        
        if( Loose::zero(Geo::distToLine(vp, v1->pt, v2->pt) ) )
        {
            if ( Loose::zero( Geo::dist(vp, v1->pt)) ||
                 Loose::zero( Geo::dist(vp ,v2->pt) ) )
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
            _insertEdgeSimple(nedge);

            return true;
        }
    }
    return false;
}

#if 0
// DAC - not used
void Map::_joinEdges(const EdgePtr & e1, const EdgePtr & e2)
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
#endif

// Given another vector of vertices, add them to the vertices of the
// current map.  We can do this in linear time with a simple merge
// algorithm.  Note that we want to coalesce identical vertices to
// eliminate duplicates.
void Map::_mergeVertices(const constMapPtr & other, qreal tolerance)
{
    const QVector<VertexPtr> & your_verts = other->vertices;          // reference
    QVector<VertexPtr> my_verts     = vertices;                 // local copy
    int my_size                     = my_verts.size();
    int your_size                   = your_verts.size();

    vertices.clear();

    int my_i   = 0;
    int your_i = 0;

    while (true)
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
                switch (comparePoints(my_v->pt, your_v->pt,tolerance))
                {
                case COMP_LESS:
                    // my_v goes first.
                    vertices.push_back(my_v);
                    ++my_i;
                    break;
                case COMP_EQUAL:
                    // It's a toss up.
                    vertices.push_back(my_v);
                    your_v->copy = my_v;
                    ++my_i;
                    ++your_i;
                    break;
                case COMP_GREATER:
                    // your_v goes first.
                    vertices.push_back(your_v);
                    your_v->copy = your_v;
                    ++your_i;
                    break;
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
//
// Yech -- pretty slow.  But I shudder at the thought of
// doing it with maximal efficiency.
//
// In practice, this routine has proven to be efficient enough
// for the Islamic design tool.  Phew!
//
// Casper -revised subtstantially to fix the modification of
// the merged map. But still the same idea

void Map::mergeMap(const constMapPtr & other, qreal tolerance)
{
    mergeMap(other.get(),tolerance);
}

void Map::mergeMap(const Map * other, qreal tolerance)
{
    Q_UNUSED(tolerance);

    for (const auto & edge : std::as_const(other->edges))
    {
        VertexPtr v1 = _getOrCreateVertex(edge->v1->pt);
        VertexPtr v2 = _getOrCreateVertex(edge->v2->pt);

        if (edge->isCurve())
            insertEdge(v1, v2, edge->getArcCenter(),edge->getCurveType());
        else
            insertEdge(v1,v2);
    }

    if (Sys::config->slowCleanseMapMerges)
    {
        // this removed duplicate edges but is very slow
        // so in general should be turned offf
        MapCleanser mc(this);
        mc.deDuplicateEdgesUsingNeighbours(true);
    }
}

QStack<Isect> Map::findIntersections(EdgePtr cutter)
{
    QStack<Isect> isects;

    //qDebug() << "edge count =" << edges.size();
    for (auto & edge : std::as_const(edges))
    {
        //qDebug() << "edge" << ecount++;
        if (cutter == edge)
            continue;

        if (cutter->isLine() && edge->isLine())
        {
            QPointF op1 = cutter->v1->pt;
            QPointF op2 = cutter->v2->pt;
            QPointF p1  = edge->v1->pt;
            QPointF p2  = edge->v2->pt;

            QPointF ipt;
            if (Intersect::getTrueIntersection(op1, op2, p1, p2, ipt))
            {
                // note - some of these intersects are at end points - so don't need splitting
                isects.push(Isect(edge,cutter,_getOrCreateVertex(ipt)));
            }
        }
        else if (cutter->isLine() && edge->isCurve())
        {
            QPointF isect1;
            QPointF isect2;
            int count = Geo::findLineCircleIntersections(edge->getArcCenter(),edge->getRadius(),cutter->getLine(),isect1,isect2);
            //qDebug() << "count" << count;
            if (count && edge->pointWithinArc(isect1))
                isects.push(Isect(edge,cutter,_getOrCreateVertex(isect1)));

            if (count == 2 && edge->pointWithinArc(isect2))
                isects.push(Isect(edge,cutter,_getOrCreateVertex(isect2)));
        }
        else if (cutter->isCurve() && edge->isLine())
        {
            QPointF isect1;
            QPointF isect2;
            int count = Geo::findLineCircleIntersections(cutter->getArcCenter(),cutter->getRadius(),edge->getLine(),isect1,isect2);
            //qDebug() << "count" << count;

            if (count && cutter->pointWithinArc(isect1))
                isects.push(Isect(edge,cutter,_getOrCreateVertex(isect1)));

            if (count == 2 && cutter->pointWithinArc(isect2))
                isects.push(Isect(edge,cutter,_getOrCreateVertex(isect2)));
        }
        else if (cutter->isCurve() && edge->isCurve())
        {
            QPointF isect1;
            QPointF isect2;
            Circle cutterC(cutter->getArcCenter(), cutter->getRadius());
            Circle edgeC(edge->getArcCenter(),   edge->getRadius());
            int count = Geo::circleCircleIntersectionPoints(cutterC, edgeC,isect1,isect2);
            //qDebug() << "curve-curve count" << count;

            if (count && cutter->pointWithinArc(isect1) && edge->pointWithinArc(isect1))
                isects.push(Isect(edge,cutter,_getOrCreateVertex(isect1)));

            if (count == 2 && cutter->pointWithinArc(isect2) && edge->pointWithinArc(isect2))
                isects.push(Isect(edge,cutter,_getOrCreateVertex(isect2)));
        }
    }
    //qDebug() << "intersects" << isects.size();
    return isects;
}

void Map::processIntersections(QStack<Isect> & isects)
{
    //  process intersects in this map
    while (!isects.isEmpty())
    {
        // this divides edge in two by a cutter whihc itself divided by the intersect point
        Isect is       = isects.pop();
        EdgePtr edge   = is.edge;
        EdgePtr cutter = is.cutter;
        VertexPtr vert = is.vertex;

        Q_ASSERT(edge);
        Q_ASSERT(cutter);

        //Sys::debugView->getMap()->insertDebugMark(vert->pt,"is");

        // make change to the edge which has been cut
        if (vert != edge->v1 && vert != edge->v2)
        {
            EdgePtr edge2 = make_shared<Edge>(edge);
            edge->setV2(vert);
            edge2->setV1(vert);
            _insertEdgeSimple(edge2);
        }

        if (vert != cutter->v1 && vert != cutter->v2)
        {
            EdgePtr cutter2 = make_shared<Edge>(cutter); // the other part after it is split
            cutter->setV2(vert);
            cutter2->setV1(vert);

            // add the two parts of the cutter
            _insertEdgeSimple(cutter);
            _insertEdgeSimple(cutter2);

            // Go through the intersections again and fix up the Isect
            // The point in eaach Isect is now either in cutter or cutter2
            for (Isect & isect : isects)
            {
                QPointF pt = isect.vertex->pt;
                if (cutter->isCurve())
                {
                    if (cutter2->pointWithinArc(pt))
                    {
                        isect.cutter = cutter2;
                    }
                }
                else
                {
                    if (Geo::isOnLine(pt,cutter2->getLine()))
                    {
                        isect.cutter = cutter2;
                    }
                }
                // you might think that if the pt is not on cutter2 then
                // it must be on cutter - but that is not true
            }
        }
        else
        {
            // the meets at a vertex of the edge
            _insertEdgeSimple(cutter);
        }
    }
}

void Map::mergeMany(const constMapPtr & other, const Placements & placements)
{
    // this function is significantly different and SLOWER than Kaplan's
    // becuse Taprats assumed PIC (polygons in contact).  This allows
    // overlapping tiles, hence the need to do a complete merge
    for (const auto & T : std::as_const(placements))
    {
        MapPtr mp = other->getTransformed(T);
        mergeMap(mp);
    }
}

// It's often the case that we want to merge a transformed copy of
// a map into another map, or even a collection of transformed copies.
// Since transforming a map requires a slow cloning, we can save lots
// of time and memory by transforming and merging simultaneously.
// Here, we transform vertices as they are put into the current map.
void Map::mergeSimpleMany(constMapPtr & other, const Placements &transforms)
{
    for (auto & T : std::as_const(transforms))
    {
        for (const auto & overt :  std::as_const(other->vertices))
        {
            // this makes vertex and inserts it in neighbours table
            overt->copy = _getOrCreateVertex(T.map(overt->pt));
        }

        for (const auto & oedge : std::as_const(other->edges))
        {
            EdgePtr nedge;

            VertexPtr ov1 = oedge->v1->copy.lock();
            VertexPtr ov2 = oedge->v2->copy.lock();

            if (oedge->getType() == EDGETYPE_LINE)
            {
                nedge = make_shared<Edge>(ov1, ov2);
            }
            else if (oedge->getType() == EDGETYPE_CURVE)
            {
                QPointF pt   = T.map(oedge->getArcCenter());
                nedge = make_shared<Edge>(ov1, ov2,pt,oedge->getCurveType());
            }

            edges.push_back(nedge);

        }
    }
    _cleanCopy();
}

void Map::removeMap(MapPtr other)
{
    for (const auto & edge : std::as_const(other->edges))
    {
        removeEdge(edge);
    }
}

void  Map::addMap(MapPtr other)
{
    for (const auto & edge : std::as_const(other->edges))
    {
        EdgePtr ep = make_shared<Edge>(_getOrCreateVertex(edge->v1->pt),_getOrCreateVertex(edge->v2->pt));
        _insertEdgeSimple(ep);
    }
}

//////////////////////////////////////////
///
/// Getters
///
//////////////////////////////////////////

// Get a Map Vertex given that we're asserting the vertex
// doesn't lie on an edge in the map.
VertexPtr Map::_getOrCreateVertex(const QPointF & pt)
{
    for (const auto & v : std::as_const(vertices))
    {
        QPointF  cur = v->pt;
        if (Geo::dist2(pt,cur) < Sys::TOL)
        //if (comparePoints(pt, cur) == COMP_EQUAL)
        {
            return v;
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
    return QString("%1 : vertices=%2 edges=%3").arg(mname).arg(vertices.size()).arg(edges.size());
}

QString Map::info() const
{
    return QString("vertices=%1 edges=%2").arg(vertices.size()).arg(edges.size());
}

void Map::dump(bool full)
{
    qDebug().noquote() << summary();

    if (full)
    {
        qDebug() << "=== start map" << this << mname;
        _dumpVertices(false);
        _dumpEdges(full);
        qDebug() << "=== end  map" << this << mname;
    }
}

QString Map::displayVertexEdgeCounts()
{
    // calc
    int vEdgeCounts[MAP_EDGECOUNT_MAX]; // for analysis, indexed by number of edges per vertex

    for (int i=0; i < MAP_EDGECOUNT_MAX; i++)
    {
        vEdgeCounts[i] = 0;
    }

    NeighbourMap nmap(this);
    for (const auto & v : std::as_const(vertices))
    {
        NeighboursPtr n = nmap.getNeighbours(v);
        int count = n->numNeighbours();
        if (count <= MAP_EDGECOUNT_MAX)
        {
            vEdgeCounts[count]++;
        }
        else
        {
            qWarning() << "Unexpected large edge count for vertex=" << count;
            return "Unknown";
        }
    }

    // dump
    QString str;
    QTextStream ss(&str);
    ss << " vertex edge counts: ";
    for (int i=0; i < MAP_EDGECOUNT_MAX; i++)
    {
        if (vEdgeCounts[i] != 0)
        {
            ss << i << "=" << vEdgeCounts[i] << " ";
        }
    }
    return str;
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
    for(const auto & edge : std::as_const(edges))
    {
        // To check all intersections of this edge with edges in
        // the current map, we can use the optimization that
        // edges are stored in sorted order by min x.  So at the
        // very least, skip the suffix of edges whose minimum x values
        // are past the max x of the edge to check.
        // Casper 12DEC02 - removed optimisation and simplified code

        QLineF e = edge->getLine();
        for (const auto & cur : std::as_const(edges))
        {
            if (cur == edge)
            {
                continue;
            }

            QLineF  c  = cur->getLine();
            QPointF apt;
            if (Intersect::getTrueIntersection(e, c, apt))
            {
                qInfo() << "Map has overlaps (intersecting edges)";
                return true;
            }
        }
    }
    return false;
}

EdgePtr Map::edgeExists(const EdgePtr &edge) const
{
    return edgeExists(edge->v1,edge->v2);
}

EdgePtr Map::edgeExists(const VertexPtr & v1, const VertexPtr & v2) const
{
    for (const auto & edge : std::as_const(edges))
    {
        if (edge->sameAs(v1,v2))
        {
            return edge;
        }
    }
    EdgePtr rv;
    return rv;
}

EdgePtr Map::edgeExists(const QPointF & p1, const QPointF & p2) const
{
    for (const auto & edge : std::as_const(edges))
    {
        if (edge->sameAs(p1,p2))
        {
            return edge;
        }
    }
    EdgePtr rv;
    return rv;
}

void Map::_dumpVertices(bool full)
{
    NeighbourMap nmap(this);
    for (const auto & vp : std::as_const(vertices))
    {
        NeighboursPtr n = nmap.getNeighbours(vp);
        qDebug() <<  "vertex: "  << vertexIndex(vp) << "at" << vp->pt << "num neighbours" << n->size();
        if (full)
        {
            for (const auto & wedge : std::as_const(*n))
            {
                auto edge = wedge.lock();
                if (edge)
                {
                    qDebug() << "      edge: " << edgeIndex(edge)
                             << "from" << vertexIndex(edge->v1)
                             << "to" << vertexIndex(edge->v2)
                             <<  edge->v1->pt << edge->v2->pt;
                }
                else
                {
                    qWarning("WEAK EDGE DOES NOT LOCK in dumpNeighbours");
                }
            }
        }
    }
}

void Map::_dumpEdges(bool full) const
{
    Q_UNUSED(full)
    int idx = 0;
    for (const auto & edge : std::as_const(edges))
    {
        qDebug() << ((edge->getType() == EDGETYPE_LINE) ? "Line" : "Curve") << "edge" << idx++
                 << "from" << vertices.indexOf(edge->v1) << edge->v1->pt
                 << "to"   << vertices.indexOf(edge->v2) << edge->v2->pt;
    }
}

void Map::_cleanCopy() const
{
    for (auto & vert : std::as_const(vertices))
    {
        vert->copy.reset();
    }
}

const QVector<QPointF>  Map::getPoints()
{
    QVector<QPointF> pts;
    for (const auto & v : std::as_const(vertices))
    {
        pts.push_back(v->pt);
    }
    return pts;
}

//////////////////////////////////////////
///
/// Utilities
/// Routines used for spatial sorting of edges and vertices.
///
//////////////////////////////////////////

// comparison of x then y
eCompare Map::comparePoints(const QPointF & a, const QPointF & b, qreal tolerance)
{
    bool verbose = false;

    qreal dx = a.x() - b.x();

    if (Loose::zero(dx,tolerance))
    {
        qreal dy = a.y() - b.y();

        if (Loose::zero(dy,tolerance))
        {
            if (verbose) qDebug () << a << b << "L0";
            return COMP_EQUAL;
        }
        else if (dy < 0.0)
        {
            if (verbose) qDebug () << a << b << "L-1";
            return COMP_LESS;
        }
        else
        {
            if (verbose) qDebug () << a << b << "L1";
            return COMP_GREATER;
        }
    }
    else if (dx < 0.0)
    {
        if (verbose) qDebug () << a << b << "D-1";
        return COMP_LESS;
    }
    else
    {
        if (verbose) qDebug () << a << b << "O1";
        return COMP_GREATER;
	}
}

void Isect::dump() const
{
    qDebug().noquote() << "ISECT edge" << edge->summary() << edge->v1->pt << edge->v2->pt << "cutter" << cutter->summary() << cutter->v1->pt << cutter->v2->pt << "isect" << vertex->pt;
}
