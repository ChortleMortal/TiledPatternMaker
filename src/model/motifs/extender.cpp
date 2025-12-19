#include <QDebug>
#include "model/motifs/extender.h"
#include "model/motifs/motif.h"
#include "sys/geometry/map.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/map_verifier.h"
#include "sys/geometry/neighbour_map.h"
#include "sys/geometry/vertex.h"
#include "sys/geometry/neighbours.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/transform.h"
#include "sys/geometry/intersect.h"
#include "model/motifs/radial_motif.h"
#include "model/tilings/tile.h"

Extender::Extender(MotifPtr motif) : _extendedBoundary(motif->getN())
{
    _motif = motif;

    _extendRays         = false;
    _extendTipsToBound  = false;
    _extendTipsToTile   = false;
    _connectRays        = 0;
    _embedBoundary      = false;
    _embedTile          = false;
}

Extender::Extender(const Extender & other) : _extendedBoundary(other._extendedBoundary)
{
    _motif              = other._motif;
    _extendRays         = other._extendRays;
    _extendTipsToBound  = other._extendTipsToBound;
    _extendTipsToTile   = other._extendTipsToTile;
    _connectRays        = other._connectRays;
    _embedBoundary      = other._embedBoundary;
    _embedTile          = other._embedTile;
}

bool Extender::equals(ExtenderPtr other)
{
    if (!_extendedBoundary.equals(other->_extendedBoundary))
        return false;

    if (_extendRays != other->_extendRays)
        return false;

    if (_extendTipsToBound != other->_extendTipsToBound)
        return false;

    if (_extendTipsToTile  != other->_extendTipsToTile)
        return false;

    if (_connectRays != other->_connectRays)
        return false;

    if (_embedBoundary != other->_embedBoundary)
        return false;

    if (_embedTile != other->_embedTile)
        return false;

    return true;
}

bool Extender::isUnity()
{
    // unity extnders have no value and can be deleted on load/save

    if (_extendRays)        return false;
    if (_extendTipsToBound) return false;
    if (_extendTipsToTile)  return false;
    if (_connectRays)       return false;
    if (_embedBoundary)     return false;
    if (_embedTile)         return false;

    auto motif = _motif.lock();
    if (!motif)
        return true;

    return _extendedBoundary.isUnity(motif);
}

void Extender::buildExtendedBoundary()
{
    auto motif = _motif.lock();
    if (!motif)
    {
        qWarning() << "Extender's motif has gone";
        return;
    }

    if (motif->isRadial())
    {
        _extendedBoundary.buildRadial();
    }
    else
    {
        _extendedBoundary.buildExplicit(motif->getTile());
    }
}


void Extender::extendRayToBoundary(RadialRay & ray)
{
    if (!ray.valid()) return;

    const ExtendedBoundary & eb = getExtendedBoundary();

    if (eb.isCircle())
    {
        qWarning() << "Extender does not support extension to circle";
        return;
    }

    const QPolygonF & eboundary = eb.getPoly();
    //qDebug() << eboundary;

    qreal radius = eb.getScale();
    QGraphicsEllipseItem circle(-radius,-radius,radius * 2.0, radius * 2.0);

    QLineF l1 = ray.getRay();
    l1  = Geo::extendLine(l1,10.0);      // extends
    if (!eb.isCircle())
    {
        l1  = Geo::clipLine(l1,eboundary);   // clips
        QPointF pt = l1.p2();
        ray.addTip(pt);
    }
    else
    {
        QPointF a;
        QPointF b;
        int points = Geo::circleLineIntersectionPoints(circle,radius,l1,a,b);
        if (points == 2)
        {
            qreal dista = Geo::dist2(a,ray.getTip());
            qreal distb = Geo::dist2(b,ray.getTip());
            if (dista < distb)
                ray.addTip(a);
            else
                ray.addTip(b);
        }
        else if (points == 1)
        {
            if (a.isNull())
                ray.addTip(b);
            else
                ray.addTip(a);
        }
    }
    if (_motif.lock()->getMotifDebug() & 0x01) ray.debug();
}

void Extender::extendRayToBoundaryPerp(RadialRay & ray)
{
    if (!ray.valid()) return;

    const ExtendedBoundary & eb = getExtendedBoundary();
    if (eb.isCircle())
    {
        qWarning() << "Extender does not support extension to circle";
        return;
    }

    const QPolygonF & eboundary = eb.getPoly();
    EdgePoly ep(eboundary);

    QPointF tip = ray.getTip();

    // draw line normal to the boundary from this point
    // until it intersects the tile

    QPointF apt;
    qreal dist = 1000;
    for (QLineF & line : ep.getLines())
    {
        QLineF pline = perpLine(line,tip);
        QPointF isect;
        if (Intersect::getIntersection(line,pline,isect))
        {
            qreal len = QLineF(tip,isect).length();
            if (len < dist)
            {
                dist = len;
                apt  = isect;
            }
        }
    }

    ray.addTip(apt);
}

void Extender::extendRayToTilePerp(RadialRay & ray, TilePtr tile)
{
    if (!ray.valid()) return;

    QPointF tip = ray.getTip();

    // draw line normal to the tile boundary from this point
    // until it intersects the tile

    QPointF apt;
    qreal dist = 1000;
    for (const EdgePtr & edge : tile->getEdgePoly().get())
    {
        QLineF pline = perpLine(edge->getLine(),tip);
        QPointF isect;
        if (Intersect::getIntersection(edge->getLine(),pline,isect))
        {
            qreal len = QLineF(tip,isect).length();
            if (len < dist)
            {
                dist = len;
                apt  = isect;
            }
        }
    }
    ray.addTip(apt);
}

void Extender::connectRays(uint method, RaySet & set1, RaySet & set2)
{
    auto motif   = _motif.lock();
    auto radial  = std::dynamic_pointer_cast<RadialMotif>(motif);

    if (motif && motif->getMotifDebug() & 0x100)
    {
        Sys::debugMapCreate->insertDebugMark(set1.ray1.getTip(),"s1r1",Qt::red);
        Sys::debugMapCreate->insertDebugMark(set1.ray2.getTip(),"s1r2",Qt::red);
        Sys::debugMapCreate->insertDebugMark(set2.ray1.getTip(),"s2r1",Qt::red);
        Sys::debugMapCreate->insertDebugMark(set2.ray2.getTip(),"s2r2",Qt::red);

        Sys::debugMapCreate->insertDebugLine(set1.ray1.getRay(),Qt::blue);
        Sys::debugMapCreate->insertDebugLine(set1.ray2.getRay(),Qt::blue);
        Sys::debugMapCreate->insertDebugLine(set2.ray1.getRay(),Qt::blue);
        Sys::debugMapCreate->insertDebugLine(set2.ray2.getRay(),Qt::blue);
    }

    switch (method)
    {
    case 1 :
    {
        set1.ray2.addTip(set2.ray1.getTip());
        RaySet set3 = set1;
        set3.transform(radial->getUnitRotationTransform() * radial->getUnitRotationTransform());
        set2.ray2.addTip(set3.ray1.getTip());
    }   break;

    case 2 :
    {
        QPointF tipA = set1.ray2.getTip();
        QPointF tipB = set2.ray1.getTip();
        set2.ray2.addTip(tipA);       // do first
        set1.ray2.addTip(tipB);
    }   break;

    case 3:
        // Test 11.v6
        set2.ray1.addTip(set1.ray2.getTip());   // do first
        set1.ray2.addTip(set2.ray2.getTip());
        break;

    case 4:
        set1.ray1.addTip(set2.ray1.getTip());
        set1.ray2.addTip(set2.ray2.getTip());
    }


}

void Extender::extendMotifMap(QTransform Tr)
{
    newVertices.clear();

    auto motif = _motif.lock();
    if (!motif)
        return;

    MapPtr map = motif->getMotifMap();
    if (!map)
        return; // can't extend soemthing that doesn't exist

    buildExtendedBoundary();

    MapVerifier mv(map);
    mv.verify();

    if (_extendRays)   // extend rays
    {
        extendMotifRays(map);
    }

    if (_extendTipsToBound)  // tips
    {
        extendTipsToBoundary(map,Tr);
    }

    if (_extendTipsToTile)  // points on boundary
    {
        extendBoundaryMap(map,motif->getTile());
    }

    if (_connectRays)  // connect to each other
    {
        connectOuterVertices(map);
    }

    if (_embedBoundary)
    {
        embedBoundary(map);
    }

    if (_embedTile)
    {
        embedTile(map,motif->getTile());
    }

    mv.verify();
}

void Extender::extendMotifRays(MapPtr motifMap)
{
    Q_ASSERT(_extendRays);

    qDebug().noquote() << "Extender::extendMotifRays" << motifMap->summary();

    auto motif = _motif.lock();
    if (!motif)
    {
        qWarning() << "Extender::extendMotifRays - Extender has no motif";
        return;
    }

    buildExtendedBoundary();

    const ExtendedBoundary & eb = getExtendedBoundary();

    qreal radius = eb.getScale();
    QGraphicsEllipseItem circle(-radius,-radius,radius * 2.0, radius * 2.0);

    const QPolygonF & eboundary = eb.getPoly();
    const QPolygonF & fboundary = motif->getMotifBoundary();

    // extend the lines to extended boundary and clip
    // extendLine and clipLine both assume that p1 is closest to the center
    // and p0 is closest to the edge

    // the shortest line is the one to insert
    QVector<EdgePtr> edges = motifMap->getEdges();
    for (auto & edge  : std::as_const(edges))
    {
        VertexPtr v1 = edge->v1;   // outer
        VertexPtr v2 = edge->v2;   // inner
        QLineF l1(v2->pt,v1->pt);

        QPointF intersect;
        if (Geo::intersectPoly(l1,fboundary,intersect))
        {
            // l1 intersects boundary
            if (!eb.isCircle())
            {
                // polygonal boundary
                // dont extend lines which already touch boundary
                QPointF intersect2;
                if (Geo::intersectPoly(l1,eboundary,intersect2))
                {
                    continue;
                }
                // extend lines
                QLineF l2  = Geo::extendLine(l1,10.0);      // extends
                l2         = Geo::clipLine(l2,eboundary);   // clips
                QPointF pt = l2.p2();                         // outer
                // test if this new point is already a vertex
                VertexPtr newv = motifMap->insertVertex(pt);
                newVertices.push_back(newv);
                motifMap->insertEdge(v1,newv);
            }
            else
            {
                // circular boundary
                QPointF a;
                QPointF b;
                int points = Geo::circleLineIntersectionPoints(circle,radius,l1,a,b);
                if (points)
                {
                    VertexPtr newv;
                    if (!a.isNull())
                    {
                        newv = motifMap->insertVertex(a);
                        newVertices.push_back(newv);
                        motifMap->insertEdge(v1,newv);
                    }
                    if (!b.isNull())
                    {
                        newv = motifMap->insertVertex(b);
                        newVertices.push_back(newv);
                        motifMap->insertEdge(v1,newv);
                    }
                }
            }
        }
    }
}

void Extender::extendTipsToBoundary(MapPtr motifMap, QTransform unitRotationTr)
{
    auto motif = _motif.lock();
    if (!motif)
    {
        qWarning() << "Extender::extendTipsToBoundary - Extender has no motif";
        return;
    }

    if (motif->isIrregular())
    {
        // FIXME implement Extender::extendFreeMap for irregular
        qWarning() << "Extender::extendTipsToBoundary - FOR IRREGULAR NOT YET SUPPORTEDf";
        return;
    }

    qDebug() << "Tile transform" << Transform::info(unitRotationTr);
    const ExtendedBoundary & eb = getExtendedBoundary();
    int n = motif->getN();
    if (n != eb.getSides())
    {
        qWarning("Cannot extend - no matching boundary");
        return;
    }

    QPointF tip(1,0);
    QPointF e_tip(1,0);

    QTransform t1 = motif->getDELTransform();
    tip = t1.map(tip);
    VertexPtr v1 = motifMap->getVertex(tip);
    Q_ASSERT(v1);

    QTransform t2 = motif->getTile()->getTransform();
    QTransform t3 = eb.getTransform();
    QTransform t4 = t2 * t3;
    e_tip = t4.map(e_tip);
    auto newv = motifMap->insertVertex(e_tip);
    Q_ASSERT(newv);

    newVertices.push_back(newv);
    motifMap->insertEdge(v1,newv);

    for (int idx = 1; idx < n; idx++)
    {
        tip   = unitRotationTr.map(tip);
        v1    = motifMap->getVertex(tip);
        e_tip = unitRotationTr.map(e_tip);
        auto newv = motifMap->insertVertex(e_tip);
        newVertices.push_back(newv);
        motifMap->insertEdge(v1,newv);
    }
}

// conenct vertices on boundary
void Extender::connectOuterVertices(MapPtr motifMap)
{
    // this algorithm connects vertices on the same edge
    const ExtendedBoundary & eb = getExtendedBoundary();
    QVector<QLineF> blines = Geo::polyToLines(eb.getPoly());

    for (const auto & line : std::as_const(blines))
    {
        VertexPtr v1;
        VertexPtr v2;
        auto it = newVertices.begin();
        while (it != newVertices.end())
        {
            if (!v1)
            {
                if (it == newVertices.end())
                    continue;
                v1  = *it++;
                if (!Geo::pointOnLine(line,v1->pt))
                {
                    v1.reset();
                    continue;
                }
            }
            // we have v1
            if (!v2)
            {
                if (it == newVertices.end())
                    continue;
                v2  = *it++;
                if (!Geo::pointOnLine(line,v2->pt))
                {
                    v2.reset();
                    continue;
                }
            }
            // we have v1 and v2
            motifMap->insertEdge(v1,v2);
        }
    }
}

void Extender::extendBoundaryMap(MapPtr motifMap, TilePtr tile)
{
    Q_ASSERT(_extendTipsToTile);
    Q_ASSERT(tile);

    const ExtendedBoundary & eb = getExtendedBoundary();
    const QVector<QLineF> blines = Geo::polyToLines(eb.getPoly());

    NeighbourMap nmap(motifMap);

    MapPtr newMap = std::make_shared<Map>("Temp");

    const QVector<VertexPtr> & vertices = motifMap->getVertices();
    for (auto & v : vertices)
    {
        NeighboursPtr np = nmap.getNeighbours(v);
        if (np->numNeighbours() == 1)
        {
            // is a tip - which is on a boundary
            // draw line normal to the boundary from this point
            // until it intersects the tile
            for (const QLineF & bline : blines)
            {
                if (Geo::pointOnLine(bline,v->pt))
                {
                    QPointF apt;
                    qreal dist = 1000;
                    QLineF pline = perpLine(bline,v->pt);
                    for (const EdgePtr & edge : tile->getEdgePoly().get())
                    {
                        QPointF isect;
                        if (Intersect::getIntersection(edge->getLine(),pline,isect))
                        {
                            qreal len = QLineF(v->pt,isect).length();
                            if (len < dist)
                            {
                                dist = len;
                                apt  = isect;
                            }
                        }
                    }
                    newMap->insertVertex(v);
                    VertexPtr v2 = newMap->insertVertex(apt);
                    newMap->insertEdge(v,v2);
                    break;
                }
            }
        }
    }
    motifMap->mergeMap(newMap);
}

void Extender::embedBoundary(MapPtr motifMap)
{
    Q_ASSERT(_embedBoundary);

    buildExtendedBoundary();

    const ExtendedBoundary & eb = getExtendedBoundary();
    MapPtr ebmap = std::make_shared<Map>("Temp",eb.getPoly());
    motifMap->mergeMap(ebmap);
}

void Extender::embedTile(MapPtr motifMap,TilePtr tile)
{
    Q_ASSERT(_embedTile);

    for (QLineF & line : tile->getLines())
    {
        VertexPtr v1 = motifMap->insertVertex(line.p1());
        VertexPtr v2 = motifMap->insertVertex(line.p2());
        motifMap->insertEdge(v1,v2);
    }
}

qreal Extender::len(VertexPtr v1, VertexPtr v2)
{
    return QLineF(v1->pt,v2->pt).length();
}

// From Chat GPT
QLineF Extender::perpLine(QLineF line, QPointF pt)
{
    // Define the points of the original line
    QPointF point1 = line.p1();
    QPointF point2 = line.p2();

    // Calculate the slope of the original line
    qreal dx = point2.x() - point1.x();
    qreal dy = point2.y() - point1.y();
    qreal originalSlope = dy / dx;

    // Calculate the perpendicular slope
    qreal perpendicularSlope = -1 / originalSlope;

    // Define the length of the perpendicular line segment
    qreal length = 100;

    // Calculate the endpoints of the perpendicular line
    qreal theta = std::atan(perpendicularSlope);
    QPointF perpPoint1(pt.x() + length * std::cos(theta), pt.y() + length * std::sin(theta));
    QPointF perpPoint2(pt.x() - length * std::cos(theta), pt.y() - length * std::sin(theta));

    // Draw the perpendicular line
    return QLineF(perpPoint1, perpPoint2);
}

void Extender::dump()
{
    qInfo().noquote()
    << "Extend:"
    << "Rays:"                << _extendRays
    << "TipsToBoundary:"      << _extendTipsToBound
    << "TipsToTile:"          << _extendTipsToTile
    << "ConnectRaya:"         << _connectRays
    << "EmbedBoundary:"       << _embedBoundary
    << "EmbedTile:"           << _embedTile;
}

