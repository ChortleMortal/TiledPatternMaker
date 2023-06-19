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

#include "geometry/map.h"
#include "settings/configuration.h"
#include "misc/utilities.h"
#include "legacy/shapefactory.h"
#include "geometry/intersect.h"
#include "geometry/edge.h"
#include "geometry/vertex.h"
#include "geometry/neighbours.h"
#include "geometry/loose.h"

using std::make_shared;

int     Map::refs = 0;
QPointF Map::tmpCenter = QPointF();

Map::Map(const QString & name)
{
    refs++;
    mname  = name;
    config = Configuration::getInstance();
}

Map::Map(const QString &  name, const QPolygonF & poly)
{
    refs++;
    mname  = name;
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

Map::Map(QString Name, const EdgePoly &poly)
{
    refs++;
    mname  = Name;
    config = Configuration::getInstance();

    // we can do all this because the map is empty and this is a constructor
    for (auto & edge : poly)
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

void Map::wipeout()
{
    // better to remove edges before removing vertices
    nMap.reset();
    derivedDCEL.reset();
    edges.clear();          // unnecessary from destructor but not elsewhere
    vertices.clear();       // unneccesary from destructor but not elsewhere
}

void Map::set(const constMapPtr & other)
{
    wipeout();
    vertices = other->vertices;
    edges    = other->edges;
    nMap.reset();
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

    for(auto& vert : vertices)
    {
        VertexPtr nv = make_shared<Vertex>(vert->pt);
        vert->copy   = nv;
        ret->vertices.push_back(nv);
    }

    for(auto edge : qAsConst(edges))
    {
        EdgePtr ne = make_shared<Edge>(edge->v1->copy.lock(), edge->v2->copy.lock());
        ne->setSwapState(edge->getSwapState());
        if (edge->getType() == EDGETYPE_CURVE || edge->getType() == EDGETYPE_CHORD)
        {
            ne->setArcCenter(edge->getArcCenter(),edge->isConvex(),(edge->getType() == EDGETYPE_CHORD));
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

void  Map:: XmlInsertDirect(VertexPtr v)
{
    vertices.push_back(v);
    nMap.reset();
}

void Map::XmlInsertDirect(EdgePtr e)
{
    edges.push_back(e);
    nMap.reset();
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
    for (auto v : vertices)
    {
        if (Loose::equalsPt(v->pt,pt))
        {
            return v;
        }
    }
    VertexPtr vp;
    return vp;
}

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

// Insert the edge connecting two vertices
EdgePtr Map::insertEdge(const VertexPtr & v1, const VertexPtr & v2)
{
    Q_ASSERT(vertices.contains(v1));
    Q_ASSERT(vertices.contains(v2));

    EdgePtr e = make_shared<Edge>(v1, v2);

    insertEdge(e);

    return e;
}

EdgePtr Map::makeCopy(const EdgePtr & e)
{
    EdgePtr ep = make_shared<Edge>(_getOrCreateVertex(e->v1->pt),_getOrCreateVertex(e->v2->pt));
    if (e->isCurve())
    {
        ep->setArcCenter(e->getArcCenter(),e->isConvex(),(e->getType() == EDGETYPE_CHORD));
    }
    return ep;
}

EdgePtr Map::makeCopy(const EdgePtr & e, QTransform T)
{
    QPointF p1 = T.map(e->v1->pt);
    QPointF p2 = T.map(e->v2->pt);
    EdgePtr ep = make_shared<Edge>(_getOrCreateVertex(p1),_getOrCreateVertex(p2));
    if (e->isCurve())
    {
        QPointF ac = T.map(e->getArcCenter());
        ep->setArcCenter(ac,e->isConvex(),(e->getType()==EDGETYPE_CHORD));
    }
    return ep;
}

void Map::_insertEdge(const EdgePtr & e)
{
    Q_ASSERT(vertices.contains(e->v1));
    Q_ASSERT(vertices.contains(e->v2));

    _insertEdge_Simple(e);
}

EdgePtr Map::_insertCurvedEdge(const VertexPtr & v1, const VertexPtr & v2, const QPointF & center, bool isConvex, bool isChord, bool debug)
{
    Q_ASSERT(vertices.contains(v1));
    Q_ASSERT(vertices.contains(v2));

    EdgePtr e = make_shared<Edge>(v1, v2,center, isConvex,isChord);

    _insertEdge_Simple(e);

    if (debug)
    {
        //insertDebugMark(v1->pt,QString());
        //insertDebugMark(v2->pt,QString());
    }

    return e;
}

// DAC TODO  - replace with insert edge direct??

// Insert an edge given that we know the edge doesn't interact
// with other edges or vertices other than its endpoints.
void Map::_insertEdge_Simple(const EdgePtr & edge)
{
    edges.push_back(edge);
    nMap.reset();
    //qDebug().noquote() << "inserted" << edge->dump();
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
    for (const auto & edge : qAsConst(edges))
    {
        if (edge->contains(v))
        {
            removeList.push_back(edge);
        }
    }

    for (const auto & edge : qAsConst(removeList))
    {
        removeEdge(edge);
    }

    vertices.removeOne(v);
    nMap.reset();
}

void Map::removeVertexSimple(const VertexPtr &v)
{
    vertices.removeOne(v);
    nMap.reset();
}

void Map::removeEdge(const EdgePtr & e)   // called by wipeout
{
    if (!e) return;

    bool rv = edges.removeOne(e);
    if (rv)
    {
        nMap.reset();
    }
}

//////////////////////////////////////////
///
/// Modifications
///
//////////////////////////////////////////

void Map::embedCrop(const QRectF &rect)
{
    // make a crop map - border has intersection points
    QPolygonF poly(rect);

    MapPtr cropMap = make_shared<Map>(QString("cropped map"),poly);

    mergeMap(cropMap);

    nMap.reset();
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
    for (auto & edge : edges)
    {
        QLineF line = edge->getLine();
        QPointF a;
        QPointF b;
        int count = Utils::findLineCircleLineIntersections( circle.centre,  circle.radius, line, a, b);
        if (count == 1)
        {
            // this is a tangent line
            points.push_back(a);
        }
        else if (count == 2)
        {
            if (Utils::pointOnLine(line,a))
            {
                points.push_back(a);
            }

            if (Utils::pointOnLine(line,b))
            {
                points.push_back(b);
            }
        }
    }

    qDebug() << "points" << points.size();
    QVector<VertexPtr> cverts;
    for (auto & pt : points)
    {
        auto v = insertVertex(pt);
        cverts.push_front(v);
    }

    QPointF center =  circle.centre;

    // points need to be clockwise around circle
    tmpCenter = center;
    std::sort(cverts.begin(),cverts.end(),vertexAngleGreaterThan);

    if (cverts.size() >=2)
    {
        for (int i=0; i < cverts.size(); i++)
        {
            auto v1 = cverts[i];
            int j = i+1;
            if (j == cverts.size())
                j=0;
            auto v2 = cverts[j];
            auto e = edgeExists(v1,v2);
            if (!e)
            {
                // crete new edge
                auto edge =  insertEdge(v1,v2);
                edge->setArcCenter(center,true,false);
            }
            else
            {
                // convert line to chord
                e->setArcCenter(center,true,true);
            }
        }
    }
    else
    {
        EdgePoly ep(circle);
        auto nmap = make_shared<Map>("Circle",ep);
        mergeMap(nmap);
    }
    nMap.reset();
}

void Map::cropOutside(const QRectF &rect)
{
    // remove anything with an outside edge
    QVector<EdgePtr>  outsideEdges;
    for (auto & edge : qAsConst(edges))
    {
        if (!Utils::rectContains(rect,edge->v1->pt) || !Utils::rectContains(rect,edge->v2->pt))
        {
            outsideEdges.push_back(edge);
        }
    }

    qDebug() << "outside edges" << outsideEdges.size();
    for (auto & edge : qAsConst(outsideEdges))
    {
        removeEdge(edge);
    }

    _cleanseVertices();
}

void  Map::cropOutside(const QPolygonF & poly)
{
    Q_UNUSED(poly);
    qFatal("Not implemented yet");
}

void Map::cropOutside(Circle &circle)
{
    // remove anything outside of circle
    QVector<EdgePtr>  outsideEdges;
    for (auto & edge : qAsConst(edges))
    {
        QPointF p1 = edge->v1->pt;
        QPointF p2 = edge->v2->pt;
        if (!Utils::pointInCircle(p1,circle) && !Utils::pointOnCircle(p1,circle))
        {
            outsideEdges.push_back(edge);
        }
        else if (!Utils::pointInCircle(p2,circle) && !Utils::pointOnCircle(p2,circle))
        {
            outsideEdges.push_back(edge);
        }
    }

    qDebug() << "outside edges" << outsideEdges.size();
    for (auto & edge : qAsConst(outsideEdges))
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
    for (auto & mark : debugTexts)
    {
        mark.first = T.map(mark.first);
    }
}

MapPtr Map::getTransformed(const QTransform &T) const
{
    MapPtr m2 = recreate();
    for (auto & vert : m2->vertices)
    {
        vert->pt = T.map(vert->pt);
    }
    for (auto edge: m2->edges)
    {
        if (edge->getType() == EDGETYPE_CURVE || edge->getType() == EDGETYPE_CHORD)
        {
            edge->setArcCenter(T.map(edge->getArcCenter()),edge->isConvex(),(edge->getType()==EDGETYPE_CHORD));
        }
    }
    return m2;
}

// Applying a motion made up only of uniform scales and translations,
// Angles don't change.  So we can just transform each vertex.
void Map::_applyTrivialRigidMotion(QTransform T)
{
    for (auto & vert : vertices)
    {
        vert->pt = T.map(vert->pt);
    }
}

// In the general case, the vertices and edges must be re-sorted.
void Map::_applyGeneralRigidMotion(QTransform T)
{
    // Transform all the vertices.
    for (auto vert : vertices)
    {
        vert->applyRigidMotion(T);
    }

    // Now sort everything.
    //sortVertices();
    //sortEdges();
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
void Map::_splitEdgesByVertex(const VertexPtr & vert)
{
    while (_splitTwoEdgesByVertex(vert))
        ;
}

bool Map::_splitTwoEdgesByVertex(const VertexPtr & vert)
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
    nMap.reset();
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
    for (auto & edge : qAsConst(other->edges))
    {
        EdgePtr cutter = makeCopy(edge);
        insertEdge(cutter);
    }
}

void Map::insertEdge(const EdgePtr & cutter)
{
    Q_ASSERT(contains(cutter->v1));
    Q_ASSERT(contains(cutter->v2));

    auto edge = edgeExists(cutter);
    if (edge)
    {
        return;
    }

    QStack<Isect> intersections;

    QPointF op1 = cutter->v1->pt;
    QPointF op2 = cutter->v2->pt;
    for (auto & edge : edges)
    {
        if (cutter->isLine() && edge->isLine())
        {
            QPointF p1 = edge->v1->pt;
            QPointF p2 = edge->v2->pt;

            QPointF ipt;
            if (Intersect::getTrueIntersection(op1, op2, p1, p2, ipt))
            {
                // note - some of these intersects are at end points
                // so don't need splitting
                intersections.push(Isect(edge,cutter,_getOrCreateVertex(ipt)));
            }
        }
        else if (cutter->isLine() && edge->isCurve())
        {
            QPointF isect1;
            QPointF isect2;
            int count = Utils::findLineCircleLineIntersections(edge->getArcCenter(),edge->getRadius(),cutter->getLine(),isect1,isect2);

            if (count == 2)
            {
                if (edge->pointWithinSpan(isect1,edge->getArcSpan()))
                    intersections.push(Isect(edge,cutter,_getOrCreateVertex(isect1)));
                if (edge->pointWithinSpan(isect2,edge->getArcSpan()))
                    intersections.push(Isect(edge,cutter,_getOrCreateVertex(isect2)));
            }
            else if (count == 1)
            {
                if (edge->pointWithinSpan(isect1,edge->getArcSpan()))
                    intersections.push(Isect(edge,cutter,_getOrCreateVertex(isect1)));
            }
        }
        else if (cutter->isCurve() && edge->isLine())
        {
            QPointF isect1;
            QPointF isect2;
            int count = Utils::findLineCircleLineIntersections(cutter->getArcCenter(),cutter->getRadius(),edge->getLine(),isect1,isect2);
            if (count == 2)
            {
                if (cutter->pointWithinSpan(isect1,cutter->getArcSpan()))
                    intersections.push(Isect(edge,cutter,_getOrCreateVertex(isect1)));
                if (cutter->pointWithinSpan(isect2,cutter->getArcSpan()))
                    intersections.push(Isect(edge,cutter,_getOrCreateVertex(isect2)));
            }
            else if (count == 1)
            {
                if (cutter->pointWithinSpan(isect1,cutter->getArcSpan()))
                    intersections.push(Isect(edge,cutter,_getOrCreateVertex(isect1)));
            }
        }
        else if (cutter->isCurve() && edge->isCurve())
        {
            QPointF isect1;
            QPointF isect2;
            Circle cutterC(cutter->getArcCenter(), cutter->getRadius());
            Circle edgeC(edge->getArcCenter(),   edge->getRadius());
            int count = Utils::circleCircleIntersectionPoints(cutterC, edgeC,isect1,isect2);
            if (count == 2)
            {
                if (cutter->pointWithinSpan(isect1,edge->getArcSpan()) && edge->pointWithinSpan(isect1,cutter->getArcSpan()))
                    intersections.push(Isect(edge,cutter,_getOrCreateVertex(isect1)));
                if (cutter->pointWithinSpan(isect2,edge->getArcSpan()) && edge->pointWithinSpan(isect2,cutter->getArcSpan()))
                    intersections.push(Isect(edge,cutter,_getOrCreateVertex(isect2)));
            }
            else if (count == 1)
            {
                if (cutter->pointWithinSpan(isect1,edge->getArcSpan()) && edge->pointWithinSpan(isect1,cutter->getArcSpan()))
                    intersections.push(Isect(edge,cutter,_getOrCreateVertex(isect1)));
            }
        }
    }
    if (intersections.isEmpty())
    {
        // Add the edges in the trivial way, since now there are no intersections.
        _insertEdge_Simple(cutter);
        return;
    }

    //qDebug() << "intersects" << intersections.size();

    //  process intersects in this map
    while (!intersections.isEmpty())
    {
        // this divides edge in two and cutter in two (except if intersection is at endpoint)
        EdgePtr cutter2;                // the other part after it is split

        Isect is = intersections.pop();
        EdgePtr edge   = is.edge.lock();
        Q_ASSERT(edge);
        EdgePtr cutter = is.cutter.lock();
        Q_ASSERT(cutter);
        VertexPtr vert = is.vertex;

        if (vert != edge->v1 && vert != edge->v2)
        {
            // make local change
            EdgePtr edge2 = edge->getCopy();
            edge->v2      = vert;
            edge2->v1     = vert;
            _insertEdge_Simple(edge2);
        }

        // add edges from other
        if (vert != cutter->v1 && vert != cutter->v2)
        {
            // make local change
            cutter2      = cutter->getCopy();
            cutter->v2   = vert;
            cutter2->v1  = vert;
            _insertEdge_Simple(cutter);
            _insertEdge_Simple(cutter2);
        }
        else
        {
            _insertEdge_Simple(cutter);
        }
        // At this point need to go through the intersections again and fix up the Isect
        // The point in eaach Isect is now either in cutter or cutter;
        if (cutter2)
        {
            for (auto & isect : intersections)
            {
                QPointF pt = isect.vertex->pt;
                if (Point::isOnLine(pt,cutter2->getLine()))
                {
                    isect.cutter = cutter2;
                }
                // you might think that if the pt is not on cutter2 then
                // it must be on cutter - but that is not true
            }
        }
    }
}

void Map::mergeMany(const constMapPtr & other, const Placements & placements)
{
    // this function is significantly different and SLOWER than Kaplan's
    // becuse Taprats assumed PIC (polygons in contact).  This allows
    // overlapping tiles, hence the need to do a complete merge
    for (const auto & T : placements)
    {
#if 1
        MapPtr mp = other->getTransformed(T);
        mergeMap(mp);
#else
        for (auto & edge : other->edges)
        {
            EdgePtr nedge = makeCopy(edge,T);
            if (!_edgeExists(nedge))
            {
                _insertEdge(nedge);
            }
        }
#endif
    }
}

// It's often the case that we want to merge a transformed copy of
// a map into another map, or even a collection of transformed copies.
// Since transforming a map requires a slow cloning, we can save lots
// of time and memory by transforming and merging simultaneously.
// Here, we transform vertices as they are put into the current map.
void Map::mergeSimpleMany(constMapPtr & other, const Placements &transforms)
{
    for (auto& T : transforms)
    {
        for (const auto & overt :  other->vertices)
        {
            // this makes vertex and inserts it in neighbours table
            overt->copy = _getOrCreateVertex(T.map(overt->pt));
        }

        for (const auto & oedge : other->edges)
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
                bool  convex = oedge->isConvex();
                nedge = make_shared<Edge>(ov1, ov2,pt,convex,false);
            }

            edges.push_back(nedge);

        }
    }
    _cleanCopy();
    nMap.reset();
}

void Map::removeMap(MapPtr other)
{
    for (const auto & edge : other->edges)
    {
        removeEdge(edge);
    }
    nMap.reset();
}

NeighboursPtr Map::getNeighbours(const VertexPtr & vert)
{
    if (!nMap)
    {
        nMap = make_shared<NeighbourMap>(edges);
    }
    return nMap->getNeighbours(vert);
}

NeighbourMapPtr Map::getNeighbourMap()
{
    if (!nMap)
    {
        nMap = make_shared<NeighbourMap>(edges);
    }
    return nMap;
}


void  Map::addMap(MapPtr other)

{
    for (const auto & edge : other->edges)
    {
        EdgePtr ep = make_shared<Edge>(_getOrCreateVertex(edge->v1->pt),_getOrCreateVertex(edge->v2->pt));
        _insertEdge_Simple(ep);
    }

    nMap.reset();
}

//////////////////////////////////////////
///
/// Getters
///
//////////////////////////////////////////

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
VertexPtr Map::_getOrCreateVertex(const QPointF & pt)
{
    for (auto & v : vertices)
    {
        QPointF  cur = v->pt;
        if (Point::dist2(pt,cur) < Loose::TOL)
        //if (comparePoints(pt, cur) == COMP_EQUAL)
        {
            return v;
        }
    }

    VertexPtr vert = make_shared<Vertex>(pt);
    vertices.push_back(vert);
    nMap.reset();
    return vert;
}

//////////////////////////////////////////
///
/// Info
///
//////////////////////////////////////////

QString Map::namedSummary() const
{
    return QString("%1 : vertices=%2 edges=%3").arg(mname).arg(vertices.size()).arg(edges.size());
}

QString Map::summary() const
{
    return QString("<%3> vertices=%1 edges=%2").arg(vertices.size()).arg(edges.size()).arg(mname);
}

void Map::dumpMap(bool full)
{
    qDebug() << "vertices =" << vertices.size() << "edges =" << edges.size();

    if (full)
    {
        qDebug() << "=== start map" << this << mname;
        _dumpVertices(full);
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

    for (auto& v : vertices)
    {
        NeighboursPtr n = getNeighbours(v);
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
    for (auto & edge : edges)
    {
        if (edge->sameAs(v1,v2))
        {
            return edge;
        }
    }
    EdgePtr rv;
    return rv;
}


void Map::_dumpVertices(bool full)
{
    for (auto & vp : qAsConst(vertices))
    {
        NeighboursPtr n = getNeighbours(vp);
        qDebug() <<  "vertex: "  << vertexIndex(vp) << "at" << vp->pt << "num neighbours" << n->size();
        if (full)
        {
            for (auto & wedge : *n)
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
    for(auto edge : qAsConst(edges))
    {
        qDebug() << ((edge->getType() == EDGETYPE_LINE) ? "Line" : "Curve") << "edge" << idx++
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
    std::vector<VertexPtr> baddies;
    for (auto & v : vertices)
    {
        NeighboursPtr n = getNeighbours(v);
        if (n->size() == 0)
        {
            baddies.push_back(v);
        }
    }
    qDebug() << "Bad vertices to delete:" << baddies.size();
    for (auto & v  : baddies)
    {
        vertices.removeOne(v);
        nMap.reset();
    }
}

const QVector<QPointF>  Map::getPoints()
{
    QVector<QPointF> pts;
    for (const auto & v : vertices)
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

void Map::draw(QPainter * painter)
{
    for (EdgePtr edge : edges)
    {
        painter->drawLine(edge->getLine());
    }
}


//////////////////////////////////////////
///
/// DebugMap
///
//////////////////////////////////////////

DebugMap::DebugMap(const QString & name) : Map(name)
{}


void DebugMap::insertDebugMark(QPointF m, QString txt)
{
    if (!config->debugMapEnable)
        return;

    qreal x = m.x();
    qreal y = m.y();

    qreal size = 0.05;
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

    if (txt.isEmpty())
        return;

    for (auto & pair : debugTexts)
    {
        if (Loose::equalsPt(pair.first,m))
        {
            pair.second += "  ";
            pair.second += txt;
            return;
        }
    }
    debugTexts.push_back(QPair<QPointF,QString>(m,txt));
}

void DebugMap::insertDebugLine(QLineF l1)
{
    insertDebugLine(l1.p1(),l1.p2());
}

void DebugMap::insertDebugLine(EdgePtr edge)
{
    if (!config->debugMapEnable)
        return;
    insertEdge(edge->v1,edge->v2);
}

void DebugMap::insertDebugLine(QPointF p1, QPointF p2)
{
    if (!config->debugMapEnable)
        return;

    VertexPtr v1 = insertVertex(p1);
    VertexPtr v2 = insertVertex(p2);
    insertEdge(v1,v2);
}

void DebugMap::insertDebugPoints(QPolygonF & poly)
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
}

void DebugMap::insertDebugPolygon(QPolygonF & poly)
{
    insertDebugPoints(poly);
    if (!poly.isClosed())
    {
        QPointF p1 = poly.at(poly.size()-1);
        QPointF p2 = poly.at(0);
        VertexPtr v1 = insertVertex(p1);
        VertexPtr v2 = insertVertex(p2);
        insertEdge(v1,v2);
    }
}
