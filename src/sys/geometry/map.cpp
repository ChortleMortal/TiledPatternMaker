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

#include <cmath>
#include <limits>
#include <QDebug>
#include <QSet>
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

    // we can do all this because the map is empty and this is a constructor.
    // Consecutive edges in an EdgePoly share vertex objects (v2 of one edge
    // is v1 of the next), and poly.get() can itself repeat the same Edge
    // object, so both vertices and edges need identity dedup here - a plain
    // QVector doesn't do that itself, so track it with local sets.
    QSet<Vertex*> seenV;
    QSet<Edge*>   seenE;
    for (auto & edge : poly.get())
    {
        VertexPtr v1 = edge->v1;
        if (!seenV.contains(v1.get()))
        {
            seenV.insert(v1.get());
            vertices.push_back(v1);
        }

        VertexPtr v2 = edge->v2;
        if (!seenV.contains(v2.get()))
        {
            seenV.insert(v2.get());
            vertices.push_back(v2);
        }

        if (!seenE.contains(edge.get()))
        {
            seenE.insert(edge.get());
            edges.push_back(edge);
        }
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
    _invalidateGrid();
    _invalidateAdjacency();
    _invalidateEdgeGrid();
}

void Map::set(const constMapPtr & other)
{
    clear();
    vertices = other->vertices;
    edges    = other->edges;
    _invalidateGrid();
    _invalidateAdjacency();
    _invalidateEdgeGrid();
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
    // paranoid O(V) scans - disabled, see _getOrCreateVertex/edgeAdjacency profiling work
    //Q_ASSERT(vertices.contains(v1));
    //Q_ASSERT(vertices.contains(v2));

    EdgePtr edge;

    if (v1 == v2)
    {
        qWarning() << "Map::insertEdge - zero length v1 = v2";
        return edge;
    }

    edge = edgeExists(v1,v2);
    if (!edge)
    {
        edge = make_shared<Edge>(v1, v2);
        _insertEdge(edge);
    }
    return edge;
}

EdgePtr Map::insertEdge(const VertexPtr & v1, const VertexPtr & v2, const QPointF & arcCenter, eCurveType ctype)
{
    // paranoid O(V) scans - disabled, see _getOrCreateVertex/edgeAdjacency profiling work
    //Q_ASSERT(vertices.contains(v1));
    //Q_ASSERT(vertices.contains(v2));

    EdgePtr edge;
    if (v1 == v2)
    {
        qWarning() << "Map::insertEdge - zero length v1 = v2";
        return edge;
    }

    edge = edgeExists(v1,v2);
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

// Insert a specific, possibly-already-known, vertex object into the map
// without splitting any edges (contrast with insertVertex(QPointF), which
// creates-or-finds by position and does split). Callers can legitimately
// pass the same VertexPtr more than once (e.g. XmlInsertDirect below, for
// XML files that reference the same vertex from multiple edges; or
// AdaptiveExtender re-inserting a tile vertex it already holds a pointer
// to) - without dedup the same vertex would appear in `vertices` more than
// once, corrupting vertex counts and anything that assumes one entry per
// vertex. Reuses vertexGrid (rebuilt lazily, then maintained incrementally
// here) for an O(1)-average identity check instead of an O(V) contains()
// scan.
void Map::insertVertex(VertexPtr v)
{
    if (_isVertexIndexed(v))
        return;

    // _isVertexIndexed above already rebuilt the grid if it was stale, so
    // it's guaranteed valid here - maintain it incrementally rather than
    // invalidating, so later calls stay O(1) average too.
    int newIdx = vertices.size();
    vertices.push_back(v);

    qint64 cx = static_cast<qint64>(std::floor(v->pt.x() / GRID_CELL));
    qint64 cy = static_cast<qint64>(std::floor(v->pt.y() / GRID_CELL));
    vertexGrid[_gridKey(cx,cy)].push_back({newIdx, v});
}

void Map::XmlInsertDirect(VertexPtr v)
{
    insertVertex(v);
}

// Same identity-dedup need as above, for edges - but this can't reuse
// _insertEdgeSimple's v1-keyed adjacency check. MosaicReader::getEdge
// caches an Edge object (for circular/forward references) before setting
// its v1/v2 - "early for recursion" - so the very same edge object can
// reach here once with v1 still null (mid-recursion, via another edge's
// nested reference) and again later with v1 set. A check keyed by the
// *current* v1 can't recognize those two calls as the same edge, so this
// needs a true identity check against `edges` itself instead. Explicit
// maps read from XML are small, so the resulting O(E) scan (matching the
// old UniqueQVector-backed behaviour) is negligible in practice.
void Map::XmlInsertDirect(EdgePtr e)
{
    if (edges.contains(e))
        return;

    edges.push_back(e);
    _invalidateAdjacency();
    _invalidateEdgeGrid();
}
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
    // An edge reaching this function is not always guaranteed to be new:
    // processIntersections() can call this twice for the same Edge object
    // when a cutter crosses more than one existing edge (see the
    // isect.cutter reassignment loop there) - so we can't unconditionally
    // skip the identity-dedup check. Answer it via the adjacency index
    // (rebuilt lazily if stale) in O(degree) rather than an O(E) scan of
    // `edges`: an edge already in `edges` is always also in
    // edgeAdjacency[edge->v1.get()] (v1 is never mutated on an
    // already-inserted edge - only fresh copies get setV1'd - so this is a
    // stable key), so absence there proves absence in `edges` too.
    if (!adjacencyValid)
        _rebuildAdjacency();

    if (edgeAdjacency.value(edge->v1.get()).contains(edge))
        return;

    edges.push_back(edge);
    edgeAdjacency[edge->v1.get()].push_back(edge);
    edgeAdjacency[edge->v2.get()].push_back(edge);

    if (edgeGridValid)
    {
        _addEdgeToGrid(edge);
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
    _invalidateGrid();
}

void Map::removeVertexSimple(const VertexPtr &v)
{
    vertices.removeOne(v);
    _invalidateGrid();
}

void Map::removeEdge(const EdgePtr & e)   // called by wipeout
{
    if (!e) return;

    edges.removeOne(e);
    _invalidateAdjacency();
    _invalidateEdgeGrid();
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

// Deletes every edge whose midpoint falls outside `rect` (kept if inside or exactly on the
// boundary - QRectF::contains is boundary-inclusive), then cleans up.
//
// This used to call embedCrop(rect) on itself first, unconditionally, but that made it
// impossible for a caller to crop WITHOUT the crop boundary's own edges ending up permanently
// embedded in the map - splitting an edge that crosses the boundary (so its inside portion
// survives as its own edge rather than the whole edge being kept-or-deleted based on one
// midpoint) and embedding (leaving the boundary shape itself behind as part of the map) are
// different decisions, so they're now separate calls. If you need edges crossing the boundary
// truncated to their inside portion rather than kept-or-deleted whole, call embedCrop(rect) on
// this map first. If every edge you're cropping either starts already on the boundary or is
// otherwise known to never legitimately cross it mid-segment, you don't need to - see
// AdaptiveMotif::build()'s cropToTile block for an example of the latter.
void Map::cropOutside(const QRectF &rect)
{
    QVector<EdgePtr>  outsideEdges;
    for (auto & edge : std::as_const(edges))
    {
        if (!Geo::rectContains(rect,edge->getMidPoint()))
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

// See cropOutside(QRectF) above - same "call embedCrop(poly) first if you need boundary-crossing
// edges truncated rather than kept-or-deleted whole" note.
void  Map::cropOutside(const QPolygonF & poly)
{
    QVector<EdgePtr>  outsideEdges;
    for (auto & edge : std::as_const(edges))
    {
        QPointF mid = edge->getMidPoint();

        bool inside   = poly.containsPoint(mid,Qt::OddEvenFill);
        bool onBorder = Geo::point_on_poly_edge(mid,poly);

        if (!inside && !onBorder)
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

// See cropOutside(QRectF) above - same "call embedCrop(circle) first if you need boundary-crossing
// edges truncated rather than kept-or-deleted whole" note.
void Map::cropOutside(const Circle &circle)
{
    QVector<EdgePtr>  outsideEdges;
    for (auto & edge : std::as_const(edges))
    {
        QPointF mid = edge->getMidPoint();

        bool inside   = Geo::pointInCircle(mid,circle);
        bool onBorder = Geo::pointOnCircle(mid,circle);

        if (!inside && !onBorder)
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

// Instead of deleting geometry outside `poly` (see cropOutside above), pulls every vertex that
// lies outside back onto the boundary: it's moved to where the line from poly's center to that
// vertex crosses the boundary.
void Map::cropOutside2(const QPolygonF & poly)
{
    QPointF center = Geo::center(poly);

    for (const auto & v : std::as_const(vertices))
    {
        bool inside   = poly.containsPoint(v->pt,Qt::OddEvenFill);
        bool onBorder = Geo::point_on_poly_edge(v->pt,poly);

        if (inside || onBorder)
            continue;

        QPointF isect;
        if (Geo::intersectPoly(QLineF(center,v->pt),poly,isect))
        {
            v->setPt(isect);
        }
    }

    _invalidateGrid();
    _invalidateEdgeGrid();

    MapCleanser mc(this);
    mc.cleanseVertices();
}

// Builds on cropOutside2: snapping a vertex straight to where the center->vertex line first
// crosses the boundary can still leave that vertex's own edges cutting back across a tile edge
// (the crossing point isn't necessarily a safe place to be, only a point on the boundary). So
// instead this walks the vertex further inward - along the same center->vertex line - until
// neither of its incident edges crosses a tile boundary edge any more. Only handles a vertex
// whose neighbours are all inside/on-border already; a vertex with an outside neighbour (both
// ends of that edge outside) is left untouched - a two-outside-endpoint case, future work.
//
// Second pass: on a concave tile, a straight edge between two points that are ALREADY
// inside/on-border can still cut across a boundary bay in between. For each such edge, tries
// pulling each endpoint toward center in turn and keeps whichever needs the smaller move to
// clear the crossing.
void Map::cropOutside3(const QPolygonF & poly)
{
    QPolygonF bounds = poly;
    if (!bounds.isClosed())
        bounds << bounds[0];

    QPointF center = Geo::center(poly);

    // true if segment (a,b) crosses a boundary edge somewhere other than at a or b themselves
    auto crossesBoundary = [&bounds](QPointF a, QPointF b) -> bool
    {
        QLineF seg(a,b);
        for (int i = 0; i < bounds.size() - 1; ++i)
        {
            QLineF  bedge(bounds[i],bounds[i+1]);
            QPointF pt;
            if (seg.intersects(bedge,&pt) == QLineF::BoundedIntersection
                && !Loose::equalsPt(pt,a) && !Loose::equalsPt(pt,b))
            {
                return true;
            }
        }
        return false;
    };

    NeighbourMap nmap(this);

    QVector<VertexPtr> origVertices = vertices;   // snapshot - vertex positions are moved below

    for (const auto & v : std::as_const(origVertices))
    {
        QPointF p0 = v->pt;

        bool inside   = poly.containsPoint(p0,Qt::OddEvenFill);
        bool onBorder = Geo::point_on_poly_edge(p0,poly);
        if (inside || onBorder)
            continue;

        NeighboursPtr n = nmap.getNeighbours(v);

        QVector<QPointF> neighbourPts;
        bool deferred = false;
        for (const auto & wedge : std::as_const(*n))
        {
            EdgePtr edge = wedge.lock();
            if (!edge)
                continue;

            QPointF op = edge->getOtherV(v)->pt;
            bool oInside   = poly.containsPoint(op,Qt::OddEvenFill);
            bool oOnBorder = Geo::point_on_poly_edge(op,poly);
            if (!oInside && !oOnBorder)
            {
                deferred = true;   // other end is also outside - leave this vertex for now
                break;
            }
            neighbourPts.push_back(op);
        }

        if (deferred || neighbourPts.isEmpty())
            continue;

        auto stillCrosses = [&](QPointF candidate) -> bool
        {
            for (const auto & np : std::as_const(neighbourPts))
            {
                if (crossesBoundary(candidate,np))
                    return true;
            }
            return false;
        };

        // walk from p0 toward center, bisecting for the point closest to p0 (least movement)
        // at which neither incident edge crosses the boundary any more
        QPointF best = center;
        if (stillCrosses(center))
        {
            qWarning() << "Map::cropOutside3: vertex still crosses tile boundary at tile center - using center anyway";
        }
        else
        {
            qreal lo = 0.0;   // p0 - known to still cross
            qreal hi = 1.0;   // center - known clear
            for (int i = 0; i < 40; ++i)
            {
                qreal   mid       = (lo + hi) / 2.0;
                QPointF candidate = p0 + (center - p0) * mid;
                if (stillCrosses(candidate))
                    lo = mid;
                else
                {
                    hi   = mid;
                    best = candidate;
                }
            }
        }

        v->setPt(best);
    }

    // second pass: edges whose endpoints are now both inside/on-border but which still cross a
    // tile boundary edge because the tile is concave between them
    QVector<EdgePtr> origEdges = edges;   // snapshot - endpoint positions are moved below

    for (const auto & edge : std::as_const(origEdges))
    {
        QPointF p1 = edge->v1->pt;
        QPointF p2 = edge->v2->pt;

        if (!crossesBoundary(p1,p2))
            continue;

        bool p1Outside = !poly.containsPoint(p1,Qt::OddEvenFill) && !Geo::point_on_poly_edge(p1,poly);
        bool p2Outside = !poly.containsPoint(p2,Qt::OddEvenFill) && !Geo::point_on_poly_edge(p2,poly);
        if (p1Outside || p2Outside)
            continue;   // handled by (or left to) the first pass above

        // bisect 'moving' toward center, holding 'fixed' in place; returns the fraction of the
        // way to center needed to clear the crossing, or -1 if even center doesn't clear it
        auto minimalMove = [&](QPointF moving, QPointF fixed) -> qreal
        {
            if (crossesBoundary(center,fixed))
                return -1.0;

            qreal lo = 0.0;   // moving's original position - known to still cross
            qreal hi = 1.0;   // center - known clear
            for (int i = 0; i < 40; ++i)
            {
                qreal   mid       = (lo + hi) / 2.0;
                QPointF candidate = moving + (center - moving) * mid;
                if (crossesBoundary(candidate,fixed))
                    lo = mid;
                else
                    hi = mid;
            }
            return hi;
        };

        qreal t1 = minimalMove(p1,p2);
        qreal t2 = minimalMove(p2,p1);

        if (t1 < 0 && t2 < 0)
        {
            qWarning() << "Map::cropOutside3: internal edge still crosses tile boundary even at tile center - leaving as is";
            continue;
        }

        if (t2 < 0 || (t1 >= 0 && t1 <= t2))
            edge->v1->setPt(p1 + (center - p1) * t1);
        else
            edge->v2->setPt(p2 + (center - p2) * t2);
    }

    _invalidateGrid();
    _invalidateEdgeGrid();

    MapCleanser mc(this);
    mc.cleanseVertices();
}


// Applying a motion made up only of uniform scales and translations,
// Angles don't change.  So we can just transform each vertex.
void Map::transform(const QTransform & T)
{
    for (const auto & vert : std::as_const(vertices))
    {
        vert->setPt(T.map(vert->pt));
    }
    for (const auto & edge : std::as_const(edges))
    {
        if (edge->getType() == EDGETYPE_CURVE)
        {
            QPointF pt = T.map(edge->getArcCenter());
            edge->chgangeToCurvedEdge(pt,edge->getCurveType());
        }
    }
    _invalidateGrid();
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
    // FIXME - detection is still chord-based, so a point genuinely on a
    // curved edge's arc (but off its chord) won't be found here. Tried a
    // circle+pointWithinArc test instead, but that misclassified points
    // across unrelated same-circle arcs (e.g. a circle split into separate
    // quadrant edges) and corrupted one - reverted pending a correct test.
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

            // Create the new edge instance, preserving curve data so the
            // split doesn't truncate an arc into a straight sub-edge.
            EdgePtr nedge = e->isCurve()
                ? make_shared<Edge>(vert, v2, e->getArcCenter(), e->getCurveType())
                : make_shared<Edge>(vert, v2);

            // We don't need to fix up v1 -- it can still point to the same edge.
            // Fix up v2.
            // Fix up the edge object -- it now points to the intervening edge.
            // e is an already-inserted edge, so it may already be filed in
            // edgeAdjacency under its old v2 (=this v2). Move that single
            // entry rather than invalidating the whole index - splits are
            // common enough (overlapping tile placements) that a blanket
            // invalidate here was forcing a full O(E) rebuild almost every
            // call, which is worse than the original linear scan.
            if (adjacencyValid)
            {
                edgeAdjacency[v2.get()].removeOne(e);
                edgeAdjacency[vert.get()].push_back(e);
            }
            // e's bbox shrinks once v2 is reassigned - remove it from its
            // old cells before mutating, then re-file under its new bbox.
            if (edgeGridValid)
                _removeEdgeFromGrid(e);
            e->setV2(vert);
            if (edgeGridValid)
                _addEdgeToGrid(e);

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
    // A grid built while the map was still small (its cell size reflects
    // the average edge length seen at that point) can go stale as many
    // more edges are merged in - re-derive it once the map has grown well
    // past its size at the last rebuild, so edgeGridCell keeps tracking
    // reality instead of a one-time early snapshot. Cheap in the common
    // case (a size compare), and each rebuild is O(current edges), fired
    // at exponentially growing sizes, so the amortized cost stays small.
    if (edgeGridValid && edges.size() > 4 * qMax(qsizetype(1), edgeGridBuiltAtCount))
        edgeGridValid = false;

    if (!edgeGridValid)
        _rebuildEdgeGrid();

    QStack<Isect> isects;

    // Broad phase: only edges whose bounding box overlaps the cutter's can
    // possibly intersect it. Querying exactly the cutter's own cell range
    // is sufficient (no neighbor padding needed): if two bboxes overlap,
    // their overlap region contains a point that lies in both boxes, so
    // some cell in that overlap falls within both bboxes' own cell ranges
    // - and every edge is filed in every cell its bbox spans, so the
    // cutter's range alone is guaranteed to hit it.
    qint64 cx0, cx1, cy0, cy1;
    _edgeCellRange(cutter, cx0, cx1, cy0, cy1);

    QSet<Edge*> seen;
    QVector<EdgePtr> candidates;
    for (qint64 cx = cx0; cx <= cx1; cx++)
    {
        for (qint64 cy = cy0; cy <= cy1; cy++)
        {
            auto it = edgeGrid.constFind(_gridKey(cx,cy));
            if (it == edgeGrid.constEnd())
                continue;

            for (const auto & candidate : it.value())
            {
                if (candidate == cutter)
                    continue;
                if (seen.contains(candidate.get()))
                    continue;
                seen.insert(candidate.get());
                candidates.push_back(candidate);
            }
        }
    }

    for (auto & edge : std::as_const(candidates))
    {

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
            // edge is always an already-inserted edge (it came from
            // findIntersections scanning existing edges), so it's always
            // filed in edgeAdjacency under its old v2 (when the index is
            // valid) - move that entry rather than invalidating the whole
            // index. edge2 is a fresh object; _insertEdgeSimple below will
            // index it normally.
            if (adjacencyValid)
            {
                edgeAdjacency[edge->v2.get()].removeOne(edge);
                edgeAdjacency[vert.get()].push_back(edge);
            }
            if (edgeGridValid)
                _removeEdgeFromGrid(edge);
            edge->setV2(vert);
            if (edgeGridValid)
                _addEdgeToGrid(edge);
            edge2->setV1(vert);
            _insertEdgeSimple(edge2);
        }

        if (vert != cutter->v1 && vert != cutter->v2)
        {
            EdgePtr cutter2 = make_shared<Edge>(cutter); // the other part after it is split
            // Unlike edge above, cutter may or may not already be inserted
            // at this point (not yet, on the first isect for this cutter;
            // possibly already inserted, from a prior isect in this same
            // loop - see the isect.cutter reassignment below). removeOne's
            // return tells us which case we're in: only re-file the entry
            // if there actually was one to move. If cutter isn't inserted
            // yet, _insertEdgeSimple will index it fresh, normally, below.
            // adjacencyValid and edgeGridValid are always invalidated and
            // incrementally maintained together (see the _invalidateAdjacency
            // sites, all paired with _invalidateEdgeGrid), so this same
            // removeOne result also tells us whether cutter was already
            // filed in the edge grid - reuse it rather than re-deriving it,
            // to avoid double-filing cutter once below via _insertEdgeSimple.
            bool cutterWasIndexed = adjacencyValid && edgeAdjacency[cutter->v2.get()].removeOne(cutter);
            if (cutterWasIndexed)
            {
                edgeAdjacency[vert.get()].push_back(cutter);
            }
            if (edgeGridValid && cutterWasIndexed)
                _removeEdgeFromGrid(cutter);
            cutter->setV2(vert);
            if (edgeGridValid && cutterWasIndexed)
                _addEdgeToGrid(cutter);
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
    _invalidateAdjacency();
    _invalidateEdgeGrid();
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

// Rebuild the spatial grid from the current vertex list.
// Called lazily whenever gridValid is false.
void Map::_rebuildGrid() const
{
    vertexGrid.clear();
    int idx = 0;
    for (const auto & v : std::as_const(vertices))
    {
        qint64 cx = static_cast<qint64>(std::floor(v->pt.x() / GRID_CELL));
        qint64 cy = static_cast<qint64>(std::floor(v->pt.y() / GRID_CELL));
        vertexGrid[_gridKey(cx,cy)].push_back({idx, v});
        idx++;
    }
    gridValid = true;
}

// True if v is already present in `vertices` - answered via vertexGrid
// (rebuilt lazily if stale) in O(1) average instead of an O(V) contains()
// scan. Unlike _getOrCreateVertex's tolerance-window search, this is an
// exact identity check (used by XmlInsertDirect to dedup XML vertex
// references), so v's own cell is the only one that can hold it - no need
// for a 3x3 neighborhood scan.
bool Map::_isVertexIndexed(const VertexPtr & v) const
{
    if (!gridValid)
        _rebuildGrid();

    qint64 cx = static_cast<qint64>(std::floor(v->pt.x() / GRID_CELL));
    qint64 cy = static_cast<qint64>(std::floor(v->pt.y() / GRID_CELL));

    auto it = vertexGrid.constFind(_gridKey(cx,cy));
    if (it == vertexGrid.constEnd())
        return false;

    for (const auto & entry : it.value())
    {
        if (entry.second == v)
            return true;
    }
    return false;
}

// Get a Map Vertex given that we're asserting the vertex
// doesn't lie on an edge in the map.
//
// Was previously a full linear scan of all vertices (O(V) per call), which
// dominates cost in mergeMap/mergeMany since this runs on every edge of
// every placement merged into a growing map. Replaced with a spatial hash
// grid: a candidate match can only be in pt's own cell or an immediately
// adjacent one, since GRID_CELL > sqrt(Loose::TOL).
VertexPtr Map::_getOrCreateVertex(const QPointF & pt)
{
    if (!gridValid)
        _rebuildGrid();

    qint64 cx = static_cast<qint64>(std::floor(pt.x() / GRID_CELL));
    qint64 cy = static_cast<qint64>(std::floor(pt.y() / GRID_CELL));

    // Collect every candidate in the 3x3 neighborhood (don't stop at the
    // first hit) and keep the one with the lowest insertion index, so that
    // when several near-coincident vertices all match within tolerance we
    // reproduce exactly what the old "scan vertices from index 0, return
    // first match" code would have returned - not whichever cell happens
    // to be visited first.
    VertexPtr best;
    int       bestIdx = -1;

    for (qint64 dx = -1; dx <= 1; dx++)
    {
        for (qint64 dy = -1; dy <= 1; dy++)
        {
            auto it = vertexGrid.constFind(_gridKey(cx+dx, cy+dy));
            if (it == vertexGrid.constEnd())
                continue;

            for (const auto & entry : it.value())
            {
                if (Geo::dist2(pt,entry.second->pt) < Loose::TOL)
                {
                    if (!best || entry.first < bestIdx)
                    {
                        best    = entry.second;
                        bestIdx = entry.first;
                    }
                }
            }
        }
    }

    if (best)
    {
        return best;
    }

    int       newIdx = vertices.size();   // matches its future position in vertices
    VertexPtr vert    = make_shared<Vertex>(pt);
    vertices.push_back(vert);
    vertexGrid[_gridKey(cx,cy)].push_back({newIdx, vert});
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

// Rebuild the edge adjacency index from the current edge list.
// Called lazily whenever adjacencyValid is false.
void Map::_rebuildAdjacency() const
{
    edgeAdjacency.clear();
    for (const auto & e : std::as_const(edges))
    {
        edgeAdjacency[e->v1.get()].push_back(e);
        edgeAdjacency[e->v2.get()].push_back(e);
    }
    adjacencyValid = true;
}

// Conservative bounding box: for curves this bounds the full circle rather
// than just the arc span, so a bulging arc can never be missed by the
// broad-phase grid lookup - the exact math in findIntersections still only
// considers the true arc.
void Map::_edgeBBox(const EdgePtr & e, qreal & minX, qreal & maxX, qreal & minY, qreal & maxY)
{
    minX = qMin(e->v1->pt.x(), e->v2->pt.x());
    maxX = qMax(e->v1->pt.x(), e->v2->pt.x());
    minY = qMin(e->v1->pt.y(), e->v2->pt.y());
    maxY = qMax(e->v1->pt.y(), e->v2->pt.y());

    if (e->isCurve())
    {
        QPointF c = e->getArcCenter();
        qreal   r = e->getRadius();
        minX = qMin(minX, c.x() - r);
        maxX = qMax(maxX, c.x() + r);
        minY = qMin(minY, c.y() - r);
        maxY = qMax(maxY, c.y() + r);
    }
}

void Map::_edgeCellRange(const EdgePtr & e, qint64 & cx0, qint64 & cx1, qint64 & cy0, qint64 & cy1) const
{
    qreal minX, maxX, minY, maxY;
    _edgeBBox(e, minX, maxX, minY, maxY);

    cx0 = static_cast<qint64>(std::floor(minX / edgeGridCell));
    cx1 = static_cast<qint64>(std::floor(maxX / edgeGridCell));
    cy0 = static_cast<qint64>(std::floor(minY / edgeGridCell));
    cy1 = static_cast<qint64>(std::floor(maxY / edgeGridCell));
}

void Map::_addEdgeToGrid(const EdgePtr & e) const
{
    qint64 cx0, cx1, cy0, cy1;
    _edgeCellRange(e, cx0, cx1, cy0, cy1);

    for (qint64 cx = cx0; cx <= cx1; cx++)
        for (qint64 cy = cy0; cy <= cy1; cy++)
            edgeGrid[_gridKey(cx,cy)].push_back(e);
}

void Map::_removeEdgeFromGrid(const EdgePtr & e) const
{
    qint64 cx0, cx1, cy0, cy1;
    _edgeCellRange(e, cx0, cx1, cy0, cy1);

    for (qint64 cx = cx0; cx <= cx1; cx++)
    {
        for (qint64 cy = cy0; cy <= cy1; cy++)
        {
            auto it = edgeGrid.find(_gridKey(cx,cy));
            if (it != edgeGrid.end())
                it->removeOne(e);
        }
    }
}

// Rebuild the spatial edge grid from the current edge list. Called lazily
// whenever edgeGridValid is false (see the invalidation sites alongside
// _invalidateAdjacency - the same map-level mutations that require a full
// adjacency rebuild also require a full grid rebuild, since both are only
// otherwise kept in sync incrementally), and also periodically as the map
// grows well past the size it had at the last rebuild (see findIntersections).
//
// Cell size is derived from the *average edge length* currently in the
// map, not from the map's overall bounding box / edge count: mergeMap
// grows a map by merging in many translated placements of the same
// source geometry, so the map's bbox can grow by orders of magnitude
// over the course of a build while individual edge lengths stay roughly
// constant (translation doesn't change length) - sizing off the bbox let
// a single early snapshot (often taken while the map was still nearly
// empty, giving a near-zero bbox) permanently wedge the grid into a
// degenerate cell size, e.g. clamped down near the 1e-6 floor, so that
// every edge added afterward had to be filed into hundreds of thousands
// of cells. Average edge length doesn't have that failure mode: it's
// meaningful even from a handful of edges and stays representative for
// the rest of the build.
void Map::_rebuildEdgeGrid() const
{
    edgeGrid.clear();

    if (edges.isEmpty())
    {
        // Nothing to size a grid from yet, and nothing to bucket either -
        // this happens on the very first findIntersections call, before
        // this map has any edges at all. Leave a sane placeholder cell
        // size; it'll be superseded by a real measurement as soon as the
        // periodic re-derivation check in findIntersections fires.
        edgeGridCell  = 1.0;
        edgeGridBuiltAtCount = 0;
        edgeGridValid = true;
        return;
    }

    qreal totalLen = 0.0;
    for (const auto & e : std::as_const(edges))
        totalLen += QLineF(e->v1->pt, e->v2->pt).length();
    qreal avgLen = totalLen / edges.size();

    // A cell a couple of edge-lengths across keeps most edges inside a
    // small handful of cells; floor it well above Loose::TOL scale so a
    // degenerate (near-zero-length) average edge can't collapse this back
    // into the same trap.
    edgeGridCell = qMax(0.01, avgLen * 2.0);

    for (const auto & e : std::as_const(edges))
        _addEdgeToGrid(e);

    edgeGridBuiltAtCount = edges.size();
    edgeGridValid = true;
}

EdgePtr Map::edgeExists(const EdgePtr &edge) const
{
    return edgeExists(edge->v1,edge->v2);
}

// Was previously a full linear scan of all edges (O(E) per call), which
// dominates cost in insertEdge/mergeMap for the same reason
// _getOrCreateVertex did before it was fixed. Edge::sameAs(v1,v2) only
// ever matches an edge that has v1 as one of its own endpoints, so we only
// need to check v1's own (small, bounded) incident-edge list rather than
// every edge in the map - sameAs itself is untouched, just given a much
// smaller candidate set to run over.
EdgePtr Map::edgeExists(const VertexPtr & v1, const VertexPtr & v2) const
{
    if (!adjacencyValid)
        _rebuildAdjacency();

    EdgePtr found;
    auto it = edgeAdjacency.constFind(v1.get());
    if (it != edgeAdjacency.constEnd())
    {
        for (const auto & edge : it.value())
        {
            if (edge->sameAs(v1,v2))
            {
                found = edge;
                break;
            }
        }
    }

    return found;
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
