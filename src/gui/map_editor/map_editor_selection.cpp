#include <QDebug>
#include "gui/map_editor/map_editor_db.h"
#include "gui/map_editor/map_editor_selection.h"
#include "gui/map_editor/map_selection.h"
#include "gui/viewers/map_editor_view.h"
#include "model/motifs/motif.h"
#include "model/prototypes/tile_motif.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "sys/geometry/circle.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/loose.h"
#include "sys/geometry/neighbours.h"
#include "sys/geometry/transform.h"
#include "sys/geometry/vertex.h"

using std::make_shared;

const bool debugSelection = false;

MapEditorSelection::MapEditorSelection(MapEditor *me)
{
    editor = me;
    db     = editor->getDb();
}

SelectionSet  MapEditorSelection::findSelectionsUsingDB(const QPointF & spt)
{
    SelectionSet  set;
    MapSelectionPtr sel;

    // find point near spt
    for (auto & pi : std::as_const(db->getPoints()))
    {
        if ( (pi.type == PointInfo::Type::Vertex || pi.type == PointInfo::Type::VertexMid) && !db->showMap)
            continue;

        if ( (pi.type == PointInfo::Type::Line || pi.type == PointInfo::Type::LineMid || pi.type == PointInfo::Type::Circle) && !db->showConstructionLines)
            continue;

        if ( (pi.type == PointInfo::Type::Center) && !db->showTileCentre)
            continue;

        QPointF    apt = pi.pt;
        QPointF      a = Sys::mapEditorView->viewT.map(apt);
        if (Geo::isNear(spt,a))
        {
            if (debugSelection) qDebug() << "FOUND point" << apt;;
            if (pi.type == PointInfo::Type::Vertex)
                set.push_back(make_shared<MapSelection>(pi.vert));
            else
                set.push_back(make_shared<MapSelection>(apt));
        }
    }

    // find line near spt
    for (auto & linfo : std::as_const(db->getLines()))
    {
        if (linfo.type == LineInfo::Type::LINE_EDGE && !db->showMap)
            continue;
        if (linfo.type == LineInfo::Type::LINE_CONSTRUCTION && !db->showConstructionLines)
            continue;

        QLineF     line  = linfo.line;
        QLineF wline     = Sys::mapEditorView->viewT.map(line);
        if (Geo::dist2ToLine(spt, wline.p1(), wline.p2()) < 49.0)
        {
            if (debugSelection) qDebug() << "FOUND line" << line;
            if (linfo.type == LineInfo::Type::LINE_EDGE)
                set.push_back(make_shared<MapSelection>(linfo.edge));
            else if (linfo.type == LineInfo::Type::LINE_CONSTRUCTION)
                set.push_back(make_shared<MapSelection>(line,true));
            else
                set.push_back(make_shared<MapSelection>(line));
        }
    }

    // find point on circle near spt
    QVector<TimoMap *> vec = editor-> getDrawTimoMaps();
    for (TimoMap * tmap : std::as_const(vec))
    {
        TileMotifSet timos = tmap->getTileMotifs();
        for (auto & timo : timos)
        {
            MotifPtr motif = timo->getMotif();
            for (ExtenderPtr ep : motif->getExtenders())
            {
                const ExtendedBoundary & eb = ep->getExtendedBoundary();
                if (eb.isCircle())
                {
                    qreal bscale    = eb.getRadius();
                    qreal scale     = Transform::scalex(Sys::mapEditorView->viewT) * bscale;
                    qreal radius    = 1.0 * scale;
                    QPointF center  = QPointF(0.0,0.0);
                    QPointF scenter = Sys::mapEditorView->viewT.map(center);

                    QPointF a;
                    QPointF b;
                    QLineF line(center, spt); // line from center
                    CirclePtr c = make_shared<Circle>(center,bscale);
                    int count = Geo::findLineCircleIntersections(scenter,radius,line,a,b);

                    if (count == 1)
                    {
                        // there should be only one point
                        if (Geo::isNear(a,spt))
                        {
                            QPointF aa = Sys::mapEditorView->viewTinv.map(a);
                            if (debugSelection) qDebug() << "FOUND point on circle" << aa;
                            set.push_back(make_shared<MapSelection>(c,aa));
                        }
                    }
                    else if (count == 2)
                    {
                        if (Geo::isNear(spt,a))
                        {
                            QPointF aa = Sys::mapEditorView->viewTinv.map(a);
                            if (debugSelection) qDebug() << "FOUND 2-pt circle intersect a" << aa;
                            set.push_back(make_shared<MapSelection>(c, aa));
                        }
                        if (Geo::isNear(spt,b))
                        {
                            QPointF bb = Sys::mapEditorView->viewTinv.map(b);
                            if (debugSelection) qDebug() << "FOUND 2-pt circle intersect b" << bb;
                            set.push_back(make_shared<MapSelection>(c, bb));
                        }
                    }
                }
            }
        }
    }

    // find construction circle using point
    sel = findConstructionCircle(spt);
    if (sel)
    {
        set.push_back(sel);
    }

    return set;
}

MapSelectionPtr MapEditorSelection::findAPoint(SelectionSet & set)
{
    for (auto & sel : std::as_const(set))
    {
        if (sel->getType() == MapSelection::Type::MAP_POINT || sel->getType() == MapSelection::Type::MAP_VERTEX)
        {
            return sel;
        }
        else if ( sel->getType() == MapSelection::Type::MAP_CIRCLE && sel->hasCircleIntersect())
        {
            return sel;
        }
    }
    MapSelectionPtr msp;
    return msp;
}

QPointF MapEditorSelection::snapPoint(SelectionSet & set, QPointF spt, bool * snapped)
{
    if (snapped) *snapped = true;

    MapSelectionPtr sel = findAPoint(set);
    if (sel)
    {
        return sel->getPoint();
    }

    sel = findALine(set);
    if (sel)
    {
        return sel->getPointNear(sel,Sys::mapEditorView->viewTinv.map(spt));
    }

    if (snapped) *snapped = false;
    return Sys::mapEditorView->viewTinv.map(spt);
}

MapSelectionPtr MapEditorSelection::findALine(SelectionSet &set)
{
    for (auto & sel : std::as_const(set))
    {
        if (sel->getType() == MapSelection::Type::MAP_LINE || sel->getType() == MapSelection::Type::MAP_EDGE)
        {
            return sel;
        }
    }
    MapSelectionPtr msp;
    return msp;
}

MapSelectionPtr MapEditorSelection::findAnEdge(SelectionSet &set)
{
    for (auto & sel : std::as_const(set))
    {
        if (sel->getType() == MapSelection::Type::MAP_EDGE)
        {
            return sel;
        }
    }
    MapSelectionPtr msp;
    return msp;
}

MapSelectionPtr MapEditorSelection::findVertex(QPointF spt , VertexPtr exclude)
{
    MapSelectionPtr sel;

    MapPtr map = editor->getEditMap();
    if (!map)
        return sel;

    for (const auto & vp : std::as_const(map->getVertices()))
    {
        if (vp == exclude)
        {
            continue;
        }
        QPointF a    = Sys::mapEditorView->viewT.map(vp->pt);
        if (Geo::isNear(spt,a))
        {
            if (debugSelection) qDebug() << "FOUND vertex";
            return make_shared<MapSelection>(vp);
        }
    }
    //qDebug() << "no vertex" << spt;
    return sel;
}

void MapEditorSelection::findEdges(MapPtr map, QPointF spt, const QVector<EdgePtr> & excludes, SelectionSet & set)
{
    if (!map)
        return;

    for (const auto & e : std::as_const(map->getEdges()))
    {
        if (excludes.contains(e))
        {
            continue;
        }
        QPointF a = Sys::mapEditorView->viewT.map(e->v1->pt);
        QPointF b = Sys::mapEditorView->viewT.map(e->v2->pt);
        
        if (Geo::distToLine(spt, a , b) < 7.0)
        {
            if (debugSelection) qDebug() << "FOUND EDGE";
            set.push_back(make_shared<MapSelection>(e));
        }
    }
}

SelectionSet MapEditorSelection::findEdges(QPointF spt, const NeighboursPtr excludes)
{
    SelectionSet set;

    MapPtr map = editor->getEditMap();
    if (!map)
        return set;

    for (const auto & e : std::as_const(map->getEdges()))
    {
        bool found = false;
        for (auto pos = excludes->begin(); pos != excludes->end(); pos++)
        {
            WeakEdgePtr wep = *pos;
            EdgePtr ep= wep.lock();
            if (e == ep)
            {
                found = true;
                break;
            }
        }
        if (found)
        {
            continue;
        }
        QPointF a = Sys::mapEditorView->viewT.map(e->v1->pt);
        QPointF b = Sys::mapEditorView->viewT.map(e->v2->pt);
        
        if (Geo::distToLine(spt, a , b) < 7.0)
        {
            if (debugSelection) qDebug() << "FOUND EDGE";
            set.push_back(make_shared<MapSelection>(e));
        }
    }
    //qDebug() << "not edge";
    return set;
}

bool MapEditorSelection::insideBoundary(QPointF wpt)
{
    TileMotifSet timos = editor->getEditTileMotifs();
    if (timos.isEmpty())
        return true;    // no boundary

    bool rv = false;

    for (auto & timo : timos)
    {
        MotifPtr motif = timo->getMotif();
        TilePtr tilep  = timo->getTile();

        if (!motif || !tilep)
        {
            rv = true;
            continue;
        }

        qreal boundary_area = 0;
        qreal tile_area     = 0;
        QPolygonF boundary;

        QPolygonF tile = tilep->getPolygon();
        if (tile.size())
        {
            tile_area = Geo::calcArea(tile);
        }

        for (ExtenderPtr ep : motif->getExtenders())
        {
            const ExtendedBoundary & eb = ep->getExtendedBoundary();
            if (eb.isCircle())
            {
                qreal radius = eb.getRadius();
                QGraphicsEllipseItem circle(-radius,-radius,radius * 2.0, radius * 2.0);
                if (circle.contains(wpt))
                {
                    rv = true;
                    continue;
                }
            }

            QPolygonF aboundary = eb.getPoly();
            if (aboundary.size())
            {
                QPointF center = tilep->getCenter();
                QTransform t   = QTransform::fromTranslate(center.x(),center.y());
                aboundary      = t.map(aboundary);
                qreal area     = Geo::calcArea(aboundary);
                if (area > boundary_area)
                {
                    boundary_area = area;
                    boundary      = aboundary;
                }
            }
        }

        if (Loose::zero(boundary_area) && Loose::zero(tile_area))
        {
            rv = true;
            continue;
        }

        QPolygonF & poly = (boundary_area > tile_area) ? boundary : tile;

        if (poly.contains(wpt) || poly.containsPoint(wpt, Qt::OddEvenFill))
        {
            rv = true;
            continue;
        }
    }
    return rv;
}

MapSelectionPtr MapEditorSelection::findConstructionCircle(const QPointF & spt)
{
    // if pt is in more than one circle, closes to centre is selected
    MapSelectionPtr sel;

    QVector<CirclePtr> selected;
    for (const CirclePtr & c2 : std::as_const(db->constructionCircles))
    {
        QPointF center = Sys::mapEditorView->viewT.map(c2->centre);
        qreal radius   = Transform::scalex(Sys::mapEditorView->viewT) * c2->radius;
        QGraphicsEllipseItem gcircle(center.x()-radius,center.y()-radius, radius * 2.0, radius * 2.0);
        if (gcircle.contains(spt))
        {
            //qDebug() << "spt IS in circle";
            selected.push_back(c2);
        }
    }

    if (selected.size() == 0)
    {
        //qDebug() << "spt NOT in circle";
        return sel;
    }
    if (selected.size() == 1)
    {
        //qDebug() << "single circle selection";
        auto c3 = selected.first();
        sel = make_shared<MapSelection>(c3,true);
        return sel;
    }

    // this finds closest to center
    qreal closestDist = 1000000.0;
    for (auto & c4 : selected)
    {
        QPointF center = Sys::mapEditorView->viewT.map(c4->centre);
        c4->tmpDist2   = Geo::dist2(spt,center);
        if (c4->tmpDist2 < closestDist)
        {
            closestDist = c4->tmpDist2;
        }
    }

    // if there is more than one (i.e. concentric circles - take the smallest radius)
    CirclePtr c;
    qreal smallestRadius = 1000000.0;
    for (auto & c4 : selected)
    {
        if (Loose::equals(c4->tmpDist2,closestDist))
        {
            if (c4->radius < smallestRadius)
            {
                smallestRadius = c4->radius;
                c = c4;
            }
        }
    }

    //qDebug() << "best circle selection of" << selected.size();
    sel = make_shared<MapSelection>(c,true);
    return sel;
}
