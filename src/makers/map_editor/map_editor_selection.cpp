#include <QDebug>

#include "makers/map_editor/map_editor_selection.h"
#include "motifs/motif.h"
#include "geometry/circle.h"
#include "geometry/crop.h"
#include "geometry/dcel.h"
#include "geometry/edge.h"
#include "geometry/loose.h"
#include "geometry/neighbours.h"
#include "geometry/point.h"
#include "geometry/transform.h"
#include "geometry/vertex.h"
#include "makers/crop_maker/crop_maker.h"
#include "makers/map_editor/map_editor_db.h"
#include "makers/map_editor/map_selection.h"
#include "misc/utilities.h"
#include "mosaic/design_element.h"
#include "settings/configuration.h"
#include "tile/tile.h"
#include "viewers/map_editor_view.h"

using std::make_shared;

const bool debugSelection = false;

MapEditorSelection::MapEditorSelection(MapEditorDb * db)
{
    this->db = db;
    meView   = MapEditorView::getSharedInstance();
    config   = Configuration::getInstance();
}

void  MapEditorSelection::buildEditorDB()
{
    points.clear();
    lines.clear();
    circles.clear();

    if (config->mapEditorMode == MAPED_MODE_MAP)
    {
        for (auto & layer : db->getDrawLayers())
        {
            DesignElementPtr delp = layer.wdel.lock();
            if (delp)
            {
                Q_ASSERT(db->isMotif(layer.type));
                // add motif
                buildMotifDB(delp);
            }

            for (auto map : db->getDrawMaps())
	        {
	            // add points from map vertices
                for (const auto & vert : qAsConst(map->getVertices()))
	            {
	                pointInfo pi(PT_VERTEX,vert,"vertex");
	                points.push_back(pi);
	            }

                for (auto & edge :qAsConst(map->getEdges()))
	            {
	                // add lines from map edges
	                lineInfo li(LINE_EDGE,edge,"edge");
	                lines.push_back(li);

	                // add points from map edges mid-points
	                QPointF midPt = edge->getLine().pointAt(0.5);
	                pointInfo pi(PT_VERTEX_MID,midPt,"mid-point edge");
                    points.push_back(pi);

                    midPt = edge->getLine().pointAt(0.6);
                    pointInfo pi2(PT_VERTEX_MID2,midPt,"mid-point direction");
                    points.push_back(pi2);
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
            for (auto & v : qAsConst(vertices))
            {
                pointInfo pi(PT_VERTEX,v,"vertex");
                points.push_back(pi);
            }

            for (auto & e : qAsConst(dcel->getEdges()))
            {
                // add lines from map edges
                lineInfo li(LINE_EDGE,e,"edge");
                lines.push_back(li);

                // add points from map edges mid-points
                QPointF midPt = e->getLine().pointAt(0.5);
                pointInfo pi(PT_VERTEX_MID,midPt,"mid-point edge");
                points.push_back(pi);

                midPt = e->getLine().pointAt(0.6);
                pointInfo pi2(PT_VERTEX_MID2,midPt,"mid-point direction");
                points.push_back(pi2);
            }
        }
    }

    // add construction lines
    for (auto line : db->constructionLines)
    {
        lineInfo li(LINE_CONSTRUCTION,line,"construction line");
        lines.push_back(li);

        QPointF a = line.p1();
        pointInfo pi(PT_LINE,a,"end-point construction line");
        points.push_back(pi);

        QPointF b = line.p2();
        pointInfo pi2(PT_LINE,b,"end-point construction line");
        points.push_back(pi2);

        QPointF midPt = line.pointAt(0.5);
        pointInfo pi3(PT_LINE_MID,midPt,"mid-point construction line");
        points.push_back(pi);
    }

    // add construction circles
    for (auto & circle : db->constructionCircles)
    {
        circles.push_back(circle);
    }

    // add crop circle
    auto crop = CropMaker::getInstance()->getActiveCrop();
    if (crop && crop->getCropType() == CROP_CIRCLE)
    {
        auto circle = crop->getCircle();
        circles.push_back(circle);
    }

    // build circle-line intersects
    for (auto & c : circles)
    {
        for (auto & linfo : lines)
        {
            if (linfo._type == LINE_EDGE && !db->showMap)
            {
                continue;
            }

            QLineF line  = linfo._line;

            // try for circle
            QPointF a;
            QPointF b;
            int count = Utils::findLineCircleLineIntersections(c->centre, c->radius, line, a, b);
            if (count == 1)
            {
                // this is a tangent line
                pointInfo pi(PT_CIRCLE,a,"circle tangent");
                points.push_back(pi);
            }
            else if (count == 2)
            {
                if (Utils::pointOnLine(line,a))
                {
                    pointInfo pi(PT_CIRCLE,a,"circle intersect a");
                    points.push_back(pi);
                }

                if (Utils::pointOnLine(line,b))
                {
                    pointInfo pi2(PT_CIRCLE,b,"circle intersect b");
                    points.push_back(pi2);
                }
            }
        }
    }

    // build circle-circle intersects
    for (int i=0; i < circles.size(); i++)
    {
        auto c1 = circles[i];
        for (int j=i+1; j < circles.size(); j++)
        {
            auto c2 = circles[j];
            QPointF p1;
            QPointF p2;
            int count = Utils::circleCircleIntersectionPoints(c1,c2,p1,p2);
            if (count == 1)
            {
                qDebug() << "circle-circle touch point=" << p1;
                pointInfo pi(PT_CIRCLE_1,p1,"circle touchPoint");
                points.push_back(pi);
            }
            else if (count == 2)
            {
                qDebug() << "circle-circle touch points=" << p1 << p2;
                pointInfo pi(PT_CIRCLE_1,p1,"circle touchPoint A");
                pointInfo pi2(PT_CIRCLE_2,p2,"circle touchPoint B");
                points.push_back(pi);
                points.push_back(pi2);
            }
        }
    }

    // build construction line intersect points
    for (auto cline : db->constructionLines)
    {
        for (auto & linfo : lines)
        {
            if (linfo._type == LINE_CONSTRUCTION && cline == linfo._line)
            {
                continue;
            }
            if (linfo._type == LINE_EDGE && !db->showMap)
            {
                continue;
            }
            QLineF line  = linfo._line;
            QPointF intersect;
            QLineF::IntersectType itype = cline.intersects(line,&intersect);
            if (itype == QLineF::BoundedIntersection)
            {
                // hapily this does into include intersecting ends (which previously I thought was a Qt bug)
                pointInfo pi(PT_LINE,intersect,"construction line intersect");
                points.push_back(pi);
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
        const QPolygonF & rextBoundary = eb.get();
        QPolygonF extBoundary  = placement.map(rextBoundary);

        if (extBoundary.size())
        {
            if (tile)
            {
                QPointF center = tile->getCenter();
                QTransform t0  = QTransform::fromTranslate(center.x(),center.y());
                extBoundary    = t0.map(extBoundary);
            }

            for (auto apt : extBoundary)
            {
                pointInfo pi(PT_LINE,apt,"boundary point");
                points.push_back(pi);
            }

            QVector<QLineF> edges = Utils::polyToLines(extBoundary);
            for (auto line : edges)
            {
                // add lines from ext boundary
                lineInfo li(LINE_FIXED,line,"boundary line");
                lines.push_back(li);

                // add points from ext bopundary mid-points
                QPointF midPt = line.pointAt(0.5);
                pointInfo pi(PT_LINE_MID,midPt,"mid-point boundary");
                points.push_back(pi);
            }

            if (!extBoundary.isClosed())
            {
                QLineF line(extBoundary.last(),extBoundary.first());
                lineInfo li(LINE_FIXED,line,"boundary line");
                lines.push_back(li);

                QPointF midPt = line.pointAt(0.5);
                pointInfo pi(PT_LINE_MID,midPt,"mid-point boundary");
                points.push_back(pi);
            }
        }

        // add points from radial motif boundary
        QPolygonF figBoundary = motif->getMotifBoundary();
        figBoundary            = placement.map(figBoundary);
        if (figBoundary.size())
        {
            for (auto apt : figBoundary)
            {
                pointInfo pi(PT_LINE,apt,"radial fig boundary point");
                points.push_back(pi);
            }

            QVector<QLineF> edges = Utils::polyToLines(figBoundary);
            for (auto line : edges)
            {
                // add line from radial motif boundary
                lineInfo li(LINE_FIXED,line,"radial fig boundary line");
                lines.push_back(li);

                // add point from raidal motif boundary
                QPointF midPt = line.pointAt(0.5);
                pointInfo pi(PT_LINE_MID,midPt,"mid-point radial fig boundary");
                points.push_back(pi);
            }

            if (!figBoundary.isClosed())
            {
                QLineF line(figBoundary.last(),figBoundary.first());
                lineInfo li(LINE_FIXED,line,"boundary line");
                lines.push_back(li);

                QPointF midPt = line.pointAt(0.5);
                pointInfo pi(PT_LINE_MID,midPt,"mid-point boundary");
                points.push_back(pi);
            }
        }

        // add external boundary circle
        if (eb.isCircle())
        {
            auto c = make_shared<Circle>(QPointF(0,0), eb.scale);
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

            for (auto apt : poly)
            {
                pointInfo pi(PT_LINE,apt,"tile point");
                points.push_back(pi);
            }

            EdgePoly edges0 = tile->getEdgePoly();
            EdgePoly edges  = edges0.map(placement);
            for (auto edge : edges)
            {
                // add lines from tile edges
                QLineF line = edge->getLine();
                lineInfo li(LINE_FIXED,line,"tile line");
                lines.push_back(li);

                // add point from tile edge mid-points
                QPointF midPt = line.pointAt(0.5);
                pointInfo pi(PT_LINE_MID,midPt,"mid-point tile");
                points.push_back(pi);
            }

            if (!poly.isClosed())
            {
                QLineF line(poly.last(),poly.first());
                lineInfo li(LINE_FIXED,line,"boundary line");
                lines.push_back(li);

                QPointF midPt = line.pointAt(0.5);
                pointInfo pi(PT_LINE_MID,midPt,"mid-point boundary");
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
    for (auto it = points.begin(); it != points.end(); it++)
    {
        pointInfo & pi = *it;

        if ( (pi._type == PT_VERTEX || pi._type == PT_VERTEX_MID) && !db->showMap)
            continue;

        if ( (pi._type == PT_LINE || pi._type == PT_LINE_MID || pi._type == PT_CIRCLE) && db->showConstructionLines)
            continue;

        QPointF    apt = pi._pt;
        QPointF      a = meView->viewT.map(apt);
        if (Point::isNear(spt,a))
        {
            if (debugSelection) qDebug() << "FOUND point" << apt << pi._desc;
            if (pi._type == PT_VERTEX)
                set.push_back(make_shared<MapSelection>(pi._vert));
            else
                set.push_back(make_shared<MapSelection>(apt));
        }
    }

    // find line near spt
    for (auto it = lines.begin(); it != lines.end(); it++)
    {
        lineInfo & linfo = *it;

        if (linfo._type == LINE_EDGE && !db->showMap)
            continue;
        if (linfo._type == LINE_CONSTRUCTION && !db->showConstructionLines)
            continue;

        QLineF     line  = linfo._line;
        QLineF wline     = meView->viewT.map(line);
        if (Point::dist2ToLine(spt, wline.p1(), wline.p2()) < 49.0)
        {
            if (debugSelection) qDebug() << "FOUND line" << line << linfo._desc;
            if (linfo._type == LINE_EDGE)
                set.push_back(make_shared<MapSelection>(linfo._edge));
            else if (linfo._type == LINE_CONSTRUCTION)
                set.push_back(make_shared<MapSelection>(line,true));
            else
                set.push_back(make_shared<MapSelection>(line));
        }
    }

    // find point on circle near spt
    for (auto & layer : db->getDrawLayers())
    {
        DesignElementPtr delp = layer.wdel.lock();
        if (delp)
        {
            Q_ASSERT (db->isMotif(layer.type));
            MotifPtr figp = delp->getMotif();
            const ExtendedBoundary & eb = figp->getExtendedBoundary();
            if (eb.isCircle())
            {
                qreal bscale    = eb.scale;
                qreal scale     = Transform::scalex(meView->viewT) * bscale;
                qreal radius    = 1.0 * scale;
                QPointF center  = QPointF(0.0,0.0);
                QPointF scenter = meView->worldToScreen(center);    // TODO - verify this
                scenter         = meView->viewT.map(scenter);

                QPointF a;
                QPointF b;
                QLineF line(center, spt); // line from center
                auto c = make_shared<Circle>(center,bscale);
                int count = Utils::findLineCircleLineIntersections(scenter,radius,line,a,b);

                if (count == 1)
                {
                    // there should be only one point
                    if (Point::isNear(a,spt))
                    {
                        QPointF aa = meView->viewTinv.map(a);
                        if (debugSelection) qDebug() << "FOUND point on circle" << aa;
                        set.push_back(make_shared<MapSelection>(c,aa));
                    }
                }
                else if (count == 2)
                {
                    if (Point::isNear(spt,a))
                    {
                        QPointF aa = meView->viewTinv.map(a);
                        if (debugSelection) qDebug() << "FOUND 2-pt circle intersect a" << aa;
                        set.push_back(make_shared<MapSelection>(c, aa));
                    }
                    if (Point::isNear(spt,b))
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
    for (auto it = set.begin(); it != set.end(); it++)
    {
        auto sel = *it;
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
    for (auto it = set.begin(); it != set.end(); it++)
    {
        auto sel = *it;
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
    for (auto it = set.begin(); it != set.end(); it++)
    {
        auto sel = *it;
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

    for (auto & vp :qAsConst(map->getVertices()))
    {
        if (vp == exclude)
        {
            continue;
        }
        QPointF pt   = meView->worldToScreen(vp->pt);    // TODO - verify
        QPointF a    = meView->viewT.map(pt);
        //QPointF sa   = worldToScreen(a);
        if (Point::isNear(spt,a))
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

    for (auto  & e : qAsConst(map->getEdges()))
    {
        if (excludes.contains(e))
        {
            continue;
        }
        QPointF a = meView->viewT.map(e->v1->pt);
        QPointF b = meView->viewT.map(e->v2->pt);

        if (Point::distToLine(spt, a , b) < 7.0)
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

    for (auto & e : map->getEdges())
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

        if (Point::distToLine(spt, a , b) < 7.0)
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

    MapEditorLayer layer = db->getEditLayer();
    DesignElementPtr delp = layer.wdel.lock();
    if (!delp)
        return true;    // no boundary

    Q_ASSERT(db->isMotif(layer.type));

    MotifPtr figp  = delp->getMotif();
    TilePtr feap = delp->getTile();

    if (!figp || !feap)
        return true;

    const ExtendedBoundary & eb = figp->getExtendedBoundary();
    if (eb.isCircle())
    {
        qreal radius = eb.scale;
        QGraphicsEllipseItem circle(-radius,-radius,radius * 2.0, radius * 2.0);
        if (circle.contains(wpt))
        {
            return true;
        }
    }

    QPolygonF boundary = eb.get();
    if (boundary.size())
    {
        QPointF center = feap->getCenter();
        QTransform t   = QTransform::fromTranslate(center.x(),center.y());
        boundary       = t.map(boundary);
        b_area = Utils::calcArea(boundary);
    }

    QPolygonF tile = feap->getPolygon();
    if (tile.size())
    {
        f_area = Utils::calcArea(tile);
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
    for (auto & c2 : db->constructionCircles)
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
        c4->tmpDist2   = Point::dist2(spt,center);
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
