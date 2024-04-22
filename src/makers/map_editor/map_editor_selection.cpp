#include <QDebug>

#include "makers/map_editor/map_editor_selection.h"
#include "motifs/motif.h"
#include "geometry/circle.h"
#include "geometry/crop.h"
#include "geometry/dcel.h"
#include "geometry/edge.h"
#include "geometry/loose.h"
#include "geometry/neighbours.h"
#include "geometry/geo.h"
#include "geometry/transform.h"
#include "geometry/vertex.h"
#include "makers/map_editor/map_editor_db.h"
#include "makers/map_editor/map_selection.h"
#include "mosaic/design_element.h"
#include "settings/configuration.h"
#include "tile/tile.h"
#include "viewers/map_editor_view.h"

using std::make_shared;

const bool debugSelection = false;
MapEditorSelection::MapEditorSelection(MapEditorDb * db)
{
    this->db = db;
    meView   = Sys::mapEditorView;
    config   = Sys::config;
}

void MapEditorSelection::buildEditorDB()
{
    points.clear();
    lines.clear();
    circles.clear();

    if (config->mapEditorMode == MAPED_MODE_MAP)
    {
        for (const MapEditorLayer * layer : db->getDrawLayers())
        {
            DesignElementPtr delp = layer->getDel();
            if (delp)
            {
                Q_ASSERT(db->isMotif(layer->getLayerMapType()));
                // add motif
                buildMotifDB(delp);
            }

            for (MapPtr & map : db->getDrawMaps())
	        {
	            // add points from map vertices
                for (const VertexPtr & vert : std::as_const(map->getVertices()))
	            {
                    PointInfo pi(PT_VERTEX,vert,"vertex");
	                points.push_back(pi);
	            }

                for (const EdgePtr & edge : std::as_const(map->getEdges()))
	            {
	                // add lines from map edges
                    LineInfo li(LINE_EDGE,edge,"edge");
	                lines.push_back(li);

	                // add points from map edges mid-points
	                QPointF midPt = edge->getLine().pointAt(0.5);
                    PointInfo pi(PT_VERTEX_MID,midPt,"mid-point edge");
                    points.push_back(pi);
	            }
	        }
    	}
    }
    else
    {
        Q_ASSERT(config->mapEditorMode == MAPED_MODE_DCEL);
        DCELPtr dcel = db->getActiveDCEL();
        if  (dcel)
        {
            // add points from map vertices
            const QVector<VertexPtr>  & vertices = dcel->getVertices();
            for (const VertexPtr & v : std::as_const(vertices))
            {
                PointInfo pi(PT_VERTEX,v,"vertex");
                points.push_back(pi);
            }

            for (const EdgePtr & e : std::as_const(dcel->getEdges()))
            {
                // add lines from map edges
                LineInfo li(LINE_EDGE,e,"edge");
                lines.push_back(li);

                // add points from map edges mid-points
                QPointF midPt = e->getLine().pointAt(0.5);
                PointInfo pi(PT_VERTEX_MID,midPt,"mid-point edge");
                points.push_back(pi);
            }
        }
    }

    // add construction lines
    for (const QLineF & line : std::as_const(db->constructionLines))
    {
        LineInfo li(LINE_CONSTRUCTION,line,"construction line");
        lines.push_back(li);

        QPointF a = line.p1();
        PointInfo pi(PT_LINE,a,"end-point construction line");
        points.push_back(pi);

        QPointF b = line.p2();
        PointInfo pi2(PT_LINE,b,"end-point construction line");
        points.push_back(pi2);

        QPointF midPt = line.pointAt(0.5);
        PointInfo pi3(PT_LINE_MID,midPt,"mid-point construction line");
        points.push_back(pi);
    }

    // add construction circles
    for (const CirclePtr & circle :std::as_const(db->constructionCircles))
    {
        circles.push_back(circle);
    }

    // add crop circle
    CropPtr crop = db->getCrop();
    if (crop && crop->getCropType() == CROP_CIRCLE)
    {
        CirclePtr circle = make_shared<Circle>(crop->getCircle());
        circles.push_back(circle);
    }

    // build circle-line intersects
    for (const CirclePtr & c : std::as_const(circles))
    {
        for (const LineInfo & linfo : std::as_const(lines))
        {
            if (linfo.type == LINE_EDGE && !db->showMap)
            {
                continue;
            }

            QLineF line  = linfo.line;

            // try for circle
            QPointF a;
            QPointF b;
            int count = Geo::findLineCircleIntersections(c->centre, c->radius, line, a, b);
            if (count == 1)
            {
                // this is a tangent line
                PointInfo pi(PT_CIRCLE,a,"circle tangent");
                points.push_back(pi);
            }
            else if (count == 2)
            {
                if (Geo::pointOnLine(line,a))
                {
                    PointInfo pi(PT_CIRCLE,a,"circle intersect a");
                    points.push_back(pi);
                }

                if (Geo::pointOnLine(line,b))
                {
                    PointInfo pi2(PT_CIRCLE,b,"circle intersect b");
                    points.push_back(pi2);
                }
            }
        }
    }

    // build circle-circle intersects
    for (int i=0; i < circles.size(); i++)
    {
        CirclePtr c1 = circles[i];
        for (int j=i+1; j < circles.size(); j++)
        {
            CirclePtr c2 = circles[j];
            QPointF p1;
            QPointF p2;
            int count = Geo::circleCircleIntersectionPoints(*c1.get(),*c2.get(),p1,p2);
            if (count == 1)
            {
                qDebug() << "circle-circle touch point=" << p1;
                PointInfo pi(PT_CIRCLE_1,p1,"circle touchPoint");
                points.push_back(pi);
            }
            else if (count == 2)
            {
                qDebug() << "circle-circle touch points=" << p1 << p2;
                PointInfo pi(PT_CIRCLE_1,p1,"circle touchPoint A");
                PointInfo pi2(PT_CIRCLE_2,p2,"circle touchPoint B");
                points.push_back(pi);
                points.push_back(pi2);
            }
        }
    }

    // build construction line intersect points
    for (const QLineF & cline : std::as_const(db->constructionLines))
    {
        for (const LineInfo & linfo : std::as_const(lines))
        {
            if (linfo.type == LINE_CONSTRUCTION && cline == linfo.line)
            {
                continue;
            }
            if (linfo.type == LINE_EDGE && !db->showMap)
            {
                continue;
            }
            if (linfo.type == LINE_EDGE && linfo.edge->isCurve())
            {
                QPointF isect1;
                QPointF isect2;
                int count = Geo::findLineCircleIntersections(linfo.edge->getArcCenter(),linfo.edge->getRadius(),cline,isect1,isect2);

                if (count && linfo.edge->pointWithinArc(isect1))
                {
                    PointInfo pi(PT_LINE,isect1,"construction line intersect");
                    points.push_back(pi);
                }

                if (count == 2 && linfo.edge->pointWithinArc(isect2))
                {
                    PointInfo pi(PT_LINE,isect2,"construction line intersect");
                    points.push_back(pi);
                }
            }
            else
            {
                QLineF line  = linfo.line;
                QPointF intersect;
                QLineF::IntersectType itype = cline.intersects(line,&intersect);
                if (itype == QLineF::BoundedIntersection)
                {
                    // hapily this does into include intersecting ends (which previously I thought was a Qt bug)
                    PointInfo pi(PT_LINE,intersect,"construction line intersect");
                    points.push_back(pi);
                }
            }
        }
    }
}

void MapEditorSelection::buildMotifDB(DesignElementPtr delp)
{
    Q_ASSERT(delp);

    MotifPtr  motif = delp->getMotif();
    TilePtr tile = delp->getTile();

    QTransform placement  = meView->getPlacement(tile);
    const ExtendedBoundary & eb = motif->getExtendedBoundary();
    if (db->showBoundaries)
    {
        // add points from ext boundary
        const QPolygonF & rextBoundary = eb.getPoly();
        QPolygonF extBoundary  = placement.map(rextBoundary);

        if (extBoundary.size())
        {
            if (tile)
            {
                QPointF center = tile->getCenter();
                QTransform t0  = QTransform::fromTranslate(center.x(),center.y());
                extBoundary    = t0.map(extBoundary);
            }

            for (auto & apt : std::as_const(extBoundary))
            {
                PointInfo pi(PT_LINE,apt,"boundary point");
                points.push_back(pi);
            }

            QVector<QLineF> edges = Geo::polyToLines(extBoundary);
            for (auto & line : std::as_const(edges))
            {
                // add lines from ext boundary
                LineInfo li(LINE_FIXED,line,"boundary line");
                lines.push_back(li);

                // add points from ext bopundary mid-points
                QPointF midPt = line.pointAt(0.5);
                PointInfo pi(PT_LINE_MID,midPt,"mid-point boundary");
                points.push_back(pi);
            }

            if (!extBoundary.isClosed())
            {
                QLineF line(extBoundary.last(),extBoundary.first());
                LineInfo li(LINE_FIXED,line,"boundary line");
                lines.push_back(li);

                QPointF midPt = line.pointAt(0.5);
                PointInfo pi(PT_LINE_MID,midPt,"mid-point boundary");
                points.push_back(pi);
            }
        }

        // add points from radial motif boundary
        QPolygonF motifBoundary = motif->getMotifBoundary();
        motifBoundary            = placement.map(motifBoundary);
        if (motifBoundary.size())
        {
            for (auto & apt : std::as_const(motifBoundary))
            {
                PointInfo pi(PT_LINE,apt,"radial motif boundary point");
                points.push_back(pi);
            }

            QVector<QLineF> edges = Geo::polyToLines(motifBoundary);
            for (auto & line : std::as_const(edges))
            {
                // add line from radial motif boundary
                LineInfo li(LINE_FIXED,line,"radial motif boundary line");
                lines.push_back(li);

                // add point from raidal motif boundary
                QPointF midPt = line.pointAt(0.5);
                PointInfo pi(PT_LINE_MID,midPt,"mid-point radial motif boundary");
                points.push_back(pi);
            }

            if (!motifBoundary.isClosed())
            {
                QLineF line(motifBoundary.last(),motifBoundary.first());
                LineInfo li(LINE_FIXED,line,"boundary line");
                lines.push_back(li);

                QPointF midPt = line.pointAt(0.5);
                PointInfo pi(PT_LINE_MID,midPt,"mid-point boundary");
                points.push_back(pi);
            }
        }

        // add external boundary circle
        if (eb.isCircle())
        {
            CirclePtr c = make_shared<Circle>(QPointF(0,0), eb.getScale());
            circles.push_back(c);
        }
    }

    // add points from tile
    if (tile)
    {
        QPolygonF poly = tile->getPolygon();
        if (poly.size())
        {
            poly = placement.map(poly);

            for (auto & apt : std::as_const(poly))
            {
                PointInfo pi(PT_LINE,apt,"tile point");
                points.push_back(pi);
            }

            const EdgePoly & edges0 = tile->getEdgePoly();
            EdgePoly edges          = edges0.map(placement);
            for (auto & edge : std::as_const(edges))
            {
                // add lines from tile edges
                QLineF line = edge->getLine();
                LineInfo li(LINE_FIXED,line,"tile line");
                lines.push_back(li);

                // add point from tile edge mid-points
                QPointF midPt = line.pointAt(0.5);
                PointInfo pi(PT_LINE_MID,midPt,"mid-point tile");
                points.push_back(pi);
            }

            if (!poly.isClosed())
            {
                QLineF line(poly.last(),poly.first());
                LineInfo li(LINE_FIXED,line,"boundary line");
                lines.push_back(li);

                QPointF midPt = line.pointAt(0.5);
                PointInfo pi(PT_LINE_MID,midPt,"mid-point boundary");
                points.push_back(pi);
            }
        }
    }
}

SelectionSet  MapEditorSelection::findSelectionsUsingDB(const QPointF & spt)
{
    SelectionSet  set;
    MapSelectionPtr sel;

    // find point near spt
    for (auto & pi : std::as_const(points))
    {
        if ( (pi.type == PT_VERTEX || pi.type == PT_VERTEX_MID) && !db->showMap)
            continue;

        if ( (pi.type == PT_LINE || pi.type == PT_LINE_MID || pi.type == PT_CIRCLE) && !db->showConstructionLines)
            continue;

        QPointF    apt = pi.pt;
        QPointF      a = meView->viewT.map(apt);
        if (Geo::isNear(spt,a))
        {
            if (debugSelection) qDebug() << "FOUND point" << apt << pi.desc;
            if (pi.type == PT_VERTEX)
                set.push_back(make_shared<MapSelection>(pi.vert));
            else
                set.push_back(make_shared<MapSelection>(apt));
        }
    }

    // find line near spt
    for (auto & linfo : std::as_const(lines))
    {
        if (linfo.type == LINE_EDGE && !db->showMap)
            continue;
        if (linfo.type == LINE_CONSTRUCTION && !db->showConstructionLines)
            continue;

        QLineF     line  = linfo.line;
        QLineF wline     = meView->viewT.map(line);
        if (Geo::dist2ToLine(spt, wline.p1(), wline.p2()) < 49.0)
        {
            if (debugSelection) qDebug() << "FOUND line" << line << linfo.desc;
            if (linfo.type == LINE_EDGE)
                set.push_back(make_shared<MapSelection>(linfo.edge));
            else if (linfo.type == LINE_CONSTRUCTION)
                set.push_back(make_shared<MapSelection>(line,true));
            else
                set.push_back(make_shared<MapSelection>(line));
        }
    }

    // find point on circle near spt
    for (const MapEditorLayer * layer : db->getDrawLayers())
    {
        DesignElementPtr delp = layer->getDel();
        if (delp)
        {
            Q_ASSERT (db->isMotif(layer->getLayerMapType()));
            MotifPtr motif = delp->getMotif();
            const ExtendedBoundary & eb = motif->getExtendedBoundary();
            if (eb.isCircle())
            {
                qreal bscale    = eb.getScale();
                qreal scale     = Transform::scalex(meView->viewT) * bscale;
                qreal radius    = 1.0 * scale;
                QPointF center  = QPointF(0.0,0.0);
                QPointF scenter = meView->modelToScreen(center);    // TODO - verify this
                scenter         = meView->viewT.map(scenter);

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
                        QPointF aa = meView->viewTinv.map(a);
                        if (debugSelection) qDebug() << "FOUND point on circle" << aa;
                        set.push_back(make_shared<MapSelection>(c,aa));
                    }
                }
                else if (count == 2)
                {
                    if (Geo::isNear(spt,a))
                    {
                        QPointF aa = meView->viewTinv.map(a);
                        if (debugSelection) qDebug() << "FOUND 2-pt circle intersect a" << aa;
                        set.push_back(make_shared<MapSelection>(c, aa));
                    }
                    if (Geo::isNear(spt,b))
                    {
                        QPointF bb = meView->viewTinv.map(b);
                        if (debugSelection) qDebug() << "FOUND 2-pt circle intersect b" << bb;
                        set.push_back(make_shared<MapSelection>(c, bb));
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
        if (sel->getType() == MAP_POINT || sel->getType() == MAP_VERTEX)
        {
            return sel;
        }
        else if ( sel->getType() == MAP_CIRCLE && sel->hasCircleIntersect())
        {
            return sel;
        }
    }
    MapSelectionPtr msp;
    return msp;
}

MapSelectionPtr MapEditorSelection::findALine(SelectionSet &set)
{
    for (auto & sel : std::as_const(set))
    {
        if (sel->getType() == MAP_LINE || sel->getType() == MAP_EDGE)
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
        if (sel->getType() == MAP_EDGE)
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

    MapPtr map = db->getEditMap();
    if (!map)
        return sel;

    for (const auto & vp : std::as_const(map->getVertices()))
    {
        if (vp == exclude)
        {
            continue;
        }
        QPointF pt   = meView->modelToScreen(vp->pt);    // TODO - verify
        QPointF a    = meView->viewT.map(pt);
        //QPointF sa   = worldToScreen(a);
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
        QPointF a = meView->viewT.map(e->v1->pt);
        QPointF b = meView->viewT.map(e->v2->pt);
        
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

    MapPtr map = db->getEditMap();
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
        QPointF a = meView->viewT.map(e->v1->pt);
        QPointF b = meView->viewT.map(e->v2->pt);
        
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
    // TODO - optimize me

    qreal b_area = 0;
    qreal f_area = 0;

    MapEditorLayer & layer = db->getEditLayer();
    DesignElementPtr delp  = layer.getDel();
    if (!delp)
        return true;    // no boundary

    Q_ASSERT(db->isMotif(layer.getLayerMapType()));

    MotifPtr motif = delp->getMotif();
    TilePtr tilep  = delp->getTile();

    if (!motif || !tilep)
        return true;

    const ExtendedBoundary & eb = motif->getExtendedBoundary();
    if (eb.isCircle())
    {
        qreal radius = eb.getScale();
        QGraphicsEllipseItem circle(-radius,-radius,radius * 2.0, radius * 2.0);
        if (circle.contains(wpt))
        {
            return true;
        }
    }

    QPolygonF boundary = eb.getPoly();
    if (boundary.size())
    {
        QPointF center = tilep->getCenter();
        QTransform t   = QTransform::fromTranslate(center.x(),center.y());
        boundary       = t.map(boundary);
        b_area         = Geo::calcArea(boundary);
    }

    QPolygonF tile = tilep->getPolygon();
    if (tile.size())
    {
        f_area = Geo::calcArea(tile);
    }

    if (Loose::zero(b_area) && Loose::zero(f_area))
    {
        return true;
    }
    QPolygonF & poly = (b_area > f_area) ? boundary : tile;

    if (poly.contains(wpt) || poly.containsPoint(wpt, Qt::OddEvenFill))
    {
        return true;
    }

    return false;
}

MapSelectionPtr MapEditorSelection::findConstructionCircle(const QPointF & spt)
{
    // if pt is in more than one circle, closes to centre is selected
    MapSelectionPtr sel;

    QVector<CirclePtr> selected;
    for (const CirclePtr & c2 : std::as_const(db->constructionCircles))
    {
        QPointF center = meView->viewT.map(c2->centre);
        qreal radius   = Transform::scalex(meView->viewT) * c2->radius;
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
        QPointF center = meView->viewT.map(c4->centre);
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
