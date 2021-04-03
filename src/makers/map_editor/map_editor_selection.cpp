#include "makers/map_editor/map_editor_selection.h"
#include "geometry/dcel.h"
#include "geometry/point.h"
#include "geometry/transform.h"
#include "makers/map_editor/map_selection.h"
#include "base/utilities.h"
#include "tapp/figure.h"
#include "geometry/loose.h"
#include "base/configuration.h"

const bool debugSelection = false;

MapEditorSelection::MapEditorSelection() : MapEditorView ()
{
}

void  MapEditorSelection::buildEditorDB()
{
    points.clear();
    lines.clear();
    circles.clear();

    eMapEditorMode mode = config->mapEditorMode;

    if (map)
    {
        // add points from map vertices
        for (const auto & vert : map->vertices)
        {
            pointInfo pi(PT_VERTEX,vert,"vertex");
            points.push_back(pi);
        }

        for (auto edge : map->edges)
        {
            // add lines from map edges
            lineInfo li(LINE_EDGE,edge,"edge");
            lines.push_back(li);

            // add points from map edges mid-points
            QPointF midPt = edge->getLine().pointAt(0.5);
            pointInfo pi(PT_VERTEX_MID,midPt,"mid-point edge");
            points.push_back(pi);
        }
    }


    if (mode == MAPED_MODE_FIGURE)
    {
        QPolygonF p    = figp->getExtBoundary();
        QPointF center = feap->getCenter();
        QTransform t   = QTransform::fromTranslate(center.x(),center.y());
        p              = t.map(p);

        // add points from ext boundary
        for (auto it = p.begin(); it != p.end(); it++)
        {
            QPointF apt  = *it;
            pointInfo pi(PT_LINE,apt,"boundary point");
            points.push_back(pi);
        }

        if (p.size())
        {
            QVector<QLineF> edges = Utils::polyToLines(p);
            for (auto it = edges.begin(); it != edges.end(); it++)
            {
                // add lines from ext boundary
                QLineF line  = *it;
                lineInfo li(LINE_FIXED,line,"boundary line");
                lines.push_back(li);

                // add points from ext bopundary mid-points
                QPointF midPt = line.pointAt(0.5);
                pointInfo pi(PT_LINE_MID,midPt,"mid-point boundary");
                points.push_back(pi);
            }

            if (!p.isClosed())
            {
                QLineF line(p.last(),p.first());
                lineInfo li(LINE_FIXED,line,"boundary line");
                lines.push_back(li);

                QPointF midPt = line.pointAt(0.5);
                pointInfo pi(PT_LINE_MID,midPt,"mid-point boundary");
                points.push_back(pi);
            }
        }

        p = figp->getRadialFigBoundary();

        // add points from radial figure boundary
        for (auto it = p.begin(); it != p.end(); it++)
        {

            QPointF apt  = *it;
            pointInfo pi(PT_LINE,apt,"radial fig boundary point");
            points.push_back(pi);
        }

        if (p.size())
        {
            QVector<QLineF> edges = Utils::polyToLines(p);
            for (auto it = edges.begin(); it != edges.end(); it++)
            {
                // add line from radial figure boundary
                QLineF line  = *it;
                lineInfo li(LINE_FIXED,line,"rradial fig boundary line");
                lines.push_back(li);

                // add point from raidal figure boundary
                QPointF midPt = line.pointAt(0.5);
                pointInfo pi(PT_LINE_MID,midPt,"mid-point radial fig boundary");
                points.push_back(pi);
            }

            if (!p.isClosed())
            {
                QLineF line(p.last(),p.first());
                lineInfo li(LINE_FIXED,line,"boundary line");
                lines.push_back(li);

                QPointF midPt = line.pointAt(0.5);
                pointInfo pi(PT_LINE_MID,midPt,"mid-point boundary");
                points.push_back(pi);
            }
        }


        QPolygonF pts = feap->getPoints();

        // add points from feature
        for (auto it = pts.begin(); it != pts.end(); it++)
        {
            QPointF apt  = *it;
            pointInfo pi(PT_LINE,apt,"feature point");
            points.push_back(pi);
        }

        if (pts.size())
        {
            EdgePoly edges = feap->getEdgePoly();
            for (auto it = edges.begin(); it != edges.end(); it++)
            {
                EdgePtr edge = *it;
                // add lines from feature edges
                QLineF line = edge->getLine();
                lineInfo li(LINE_FIXED,line,"feature line");
                lines.push_back(li);

                // add point from feature edge mid-points
                QPointF midPt = line.pointAt(0.5);
                pointInfo pi(PT_LINE_MID,midPt,"mid-point feature");
                points.push_back(pi);
            }
            if (!pts.isClosed())
            {
                QLineF line(pts.last(),pts.first());
                lineInfo li(LINE_FIXED,line,"boundary line");
                lines.push_back(li);

                QPointF midPt = line.pointAt(0.5);
                pointInfo pi(PT_LINE_MID,midPt,"mid-point boundary");
                points.push_back(pi);
            }
        }
    }

    if (mode == MAPED_MODE_DCEL)
    {
        DCELPtr dp = dcel.lock();
        if  (dp)
        {
            // add points from map vertices
            for (auto & v : qAsConst(dp->vertices))
            {
                pointInfo pi(PT_VERTEX,v->vert,"vertex");
                points.push_back(pi);
            }

            for (auto & e : qAsConst(dp->edges))
            {
                // add lines from map edges
                lineInfo li(LINE_EDGE,e->edge,"edge");
                lines.push_back(li);

                // add points from map edges mid-points
                QPointF midPt = e->edge->getLine().pointAt(0.5);
                pointInfo pi(PT_VERTEX_MID,midPt,"mid-point edge");
                points.push_back(pi);
            }
        }
    }

    // build construction lines
    for (auto it = constructionLines.begin(); it != constructionLines.end(); it++)
    {
        QLineF line  = *it;
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

    // add circles
    if (mode == MAPED_MODE_FIGURE)
    {
        if (figp->hasExtCircleBoundary())
        {
            CirclePtr c = make_shared<Circle>(QPointF(0,0), figp->getExtBoundaryScale());
            circles.push_back(c);
        }
    }

    for (auto it = constructionCircles.begin(); it != constructionCircles.end(); it++)
    {
        CirclePtr c = *it;
        circles.push_back(c);
    }


    // build circle-line intersects
    for (auto cit = circles.begin(); cit != circles.end(); cit++)
    {
        CirclePtr c = *cit;
        for (auto it = lines.begin(); it != lines.end(); it++)
        {
            lineInfo & linfo = *it;
            if (linfo._type == LINE_EDGE && hideMap)
            {
                continue;
            }

            QLineF     line  = linfo._line;

            // try for circle
            QPointF a;
            QPointF b;
            int count = Utils::findLineCircleLineIntersections(c->centre, c->radius, line, a, b);
            if (count == 1)
            {
                // this is a tangent line
                pointInfo pi(PT_LINE,a,"circle tangent");
                points.push_back(pi);
            }
            else if (count == 2)
            {
                if (Utils::pointOnLine(line,a))
                {
                    pointInfo pi(PT_LINE,a,"circle intersect a");
                    points.push_back(pi);
                }

                if (Utils::pointOnLine(line,b))
                {
                    pointInfo pi2(PT_LINE,b,"circle intersect b");
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
                pointInfo pi(PT_CIRCLE_2,p1,"circle touchPoint A");
                pointInfo pi2(PT_CIRCLE_2,p2,"circle touchPoint B");
                points.push_back(pi);
                points.push_back(pi2);
            }
        }
    }

    // build construction line intersect points
    for (auto it = constructionLines.begin(); it != constructionLines.end(); it++)
    {
        QLineF cline  = *it;
        for (auto it = lines.begin(); it != lines.end(); it++)
        {
            lineInfo & linfo = *it;
            if (linfo._type == LINE_CONSTRUCTION && cline == linfo._line)
            {
                continue;
            }
            if (linfo._type == LINE_EDGE && hideMap)
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


SelectionSet  MapEditorSelection::findSelectionsUsingDB(const QPointF & spt)
{
    SelectionSet  set;
    MapSelectionPtr sel;

    // find point near spt
    for (auto it = points.begin(); it != points.end(); it++)
    {
        pointInfo & pi = *it;

        if ( (pi._type == PT_VERTEX || pi._type == PT_VERTEX_MID) && hideMap)
            continue;

        if ( (pi._type == PT_LINE || pi._type == PT_LINE_MID) && hideConstructionLines)
            continue;

        QPointF    apt = pi._pt;
        QPointF      a = viewT.map(apt);
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

        if (linfo._type == LINE_EDGE && hideMap)
            continue;
        if (linfo._type == LINE_CONSTRUCTION && hideConstructionLines)
            continue;

        QLineF     line  = linfo._line;
        QLineF wline     = viewT.map(line);
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
    if (figp && figp->hasExtCircleBoundary())
    {
        qreal bscale    = figp->getExtBoundaryScale();
        qreal scale     = Transform::scalex(viewT) * bscale;
        qreal radius    = 1.0 * scale;
        QPointF center  = QPointF(0.0,0.0);
        QPointF scenter = worldToScreen(center);    // TODO - verify this
        scenter         = viewT.map(scenter);

        QPointF a;
        QPointF b;
        QLineF line(center, spt); // line from center
        CirclePtr c = make_shared<Circle>(center,bscale);
        int count = Utils::findLineCircleLineIntersections(scenter,radius,line,a,b);

        if (count == 1)
        {
            // there should be only one point
            if (Point::isNear(a,spt))
            {
                QPointF aa = viewTinv.map(a);
                if (debugSelection) qDebug() << "FOUND point on circle" << aa;
                set.push_back(make_shared<MapSelection>(c,aa));
            }
        }
        else if (count == 2)
        {
            if (Point::isNear(spt,a))
            {
                QPointF aa = viewTinv.map(a);
                if (debugSelection) qDebug() << "FOUND 2-pt circle intersect a" << aa;
                set.push_back(make_shared<MapSelection>(c, aa));
            }
            if (Point::isNear(spt,b))
            {
                QPointF bb = viewTinv.map(b);
                if (debugSelection) qDebug() << "FOUND 2-pt circle intersect b" << bb;
                set.push_back(make_shared<MapSelection>(c, bb));
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

    if (config->mapEditorMode == MAPED_MODE_NONE)
        return sel;

    for (auto vp : map->vertices)
    {
        if (vp == exclude)
        {
            continue;
        }
        QPointF pt   = worldToScreen(vp->pt);    // TODO - verify
        QPointF a    = viewT.map(pt);
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

SelectionSet MapEditorSelection::findEdges(QPointF spt, const QVector<EdgePtr> & excludes)
{
    SelectionSet set;

    if (config->mapEditorMode == MAPED_MODE_NONE)
        return set;

    for (auto e : map->edges)
    {
        if (excludes.contains(e))
        {
            continue;
        }
        QPointF a = viewT.map(e->v1->pt);
        QPointF b = viewT.map(e->v2->pt);

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

    if (config->mapEditorMode == MAPED_MODE_NONE)
        return true;    // no boundary

    qreal b_area = 0;
    qreal f_area = 0;

    if (figp->hasExtCircleBoundary())
    {
        qreal radius = figp->getExtBoundaryScale();
        QGraphicsEllipseItem circle(-radius,-radius,radius * 2.0, radius * 2.0);
        if (circle.contains(wpt))
        {
            return true;
        }
    }

    QPolygonF boundary = figp->getExtBoundary();
    if (boundary.size())
    {
        QPointF center = feap->getCenter();
        QTransform t   = QTransform::fromTranslate(center.x(),center.y());
        boundary       = t.map(boundary);
        b_area = Utils::calcArea(boundary);
    }

    QPolygonF feature = feap->getPolygon();
    if (feature.size())
    {
        f_area = Utils::calcArea(feature);
    }

    if (Loose::zero(b_area) && Loose::zero(f_area))
    {
        return true;
    }
    QPolygonF & poly = (b_area > f_area) ? boundary : feature;

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

    if (config->mapEditorMode == MAPED_MODE_NONE)
        return sel;

    QVector<CirclePtr> selected;
    for (auto it = constructionCircles.begin(); it != constructionCircles.end(); it++)
    {
        CirclePtr   c2 = *it;
        QPointF center = viewT.map(c2->centre);
        qreal radius   = Transform::scalex(viewT) * c2->radius;
        QGraphicsEllipseItem gcircle(center.x()-radius,center.y()-radius, radius * 2.0, radius * 2.0);
        if (gcircle.contains(spt))
        {
            qDebug() << "spt IS in circle";
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
        qDebug() << "single circle selection";
        CirclePtr c3 = selected.first();
        sel = make_shared<MapSelection>(c3,true);
        return sel;
    }

    // this finds closest to center
    qreal closestDist = 1000000.0;
    for (int i=0; i < selected.size(); i++)
    {
        CirclePtr c4   = selected[i];
        QPointF center = viewT.map(c4->centre);
        c4->tmpDist2   = Point::dist2(spt,center);
        if (c4->tmpDist2 < closestDist)
        {
            closestDist = c4->tmpDist2;
        }
    }

    // if there is more than one (i.e. concentric circles - take the smallest radius)
    CirclePtr c;
    qreal smallestRadius = 1000000.0;
    for (int i=0; i < selected.size(); i++)
    {
        CirclePtr c4   = selected[i];
        if (Loose::equals(c4->tmpDist2,closestDist))
        {
            if (c4->radius < smallestRadius)
            {
                smallestRadius = c4->radius;
                c = c4;
            }
        }
    }

    qDebug() << "best circle selection of" << selected.size();
    sel = make_shared<MapSelection>(c,true);
    return sel;
}
