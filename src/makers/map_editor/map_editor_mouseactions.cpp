#include "makers/map_editor/map_editor_mouseactions.h"
#include "makers/map_editor/map_editor_db.h"
#include "makers/map_editor/map_selection.h"
#include "geometry/vertex.h"
#include "geometry/edge.h"
#include "geometry/map.h"
#include "geometry/geo.h"
#include "geometry/transform.h"
#include "geometry/neighbours.h"
#include "misc/sys.h"
#include "motifs/motif.h"
#include "mosaic/design_element.h"
#include "viewers/map_editor_view.h"
#include "viewers/view.h"

typedef std::weak_ptr<class Edge>   WeakEdgePtr;

MapMouseAction::MapMouseAction(QPointF spt)
{
    desc      = "MapMouseAction";
    meView    = MapEditorView::getInstance();
    view      = Sys::view;
    selector  = meView->getSelector();
    db        = meView->getDb();
    last_drag = spt;

    forceRedraw();
}

void MapMouseAction::forceRedraw()
{
    view->update();
}

void MapMouseAction::updateDragging(QPointF spt)
{
    last_drag = spt;
    forceRedraw();
}

void MapMouseAction::draw(QPainter *painter)
{
    Q_UNUSED(painter)
}

void MapMouseAction::endDragging(QPointF spt)
{
    Q_UNUSED(spt)

    MapEditorLayer & layer = db->getEditLayer();
    DesignElementPtr delp  = layer.getDel();
    if (delp)
    {
        Q_ASSERT(db->isMotif(layer.getLayerMapType()));
        MotifPtr motif = delp->getMotif();
        if (motif)
        {
            //map has been changed
            motif->setMotifType(MOTIF_TYPE_EXPLICIT_MAP);
        }
    }

    forceRedraw();
}

/////////
///
///  Move Vertex
///
/////////

MoveVertex::MoveVertex(VertexPtr vp, QPointF spt ) : MapMouseAction(spt)
{
    desc    = "MoveVertex";
    _vp     = vp;
}

void MoveVertex::updateDragging(QPointF spt)
{
    QPointF w    = meView->viewTinv.map(spt);
    if (!selector->insideBoundary(w))
    {
        qDebug() << "outside boundary";
        return;
    }
    //qDebug() << "moveVertex: update spt=" << spt << "w=" << w;
    _vp->pt = w;
    MapMouseAction::updateDragging(spt);
}

void MoveVertex::endDragging( QPointF spt)
{
    MapPtr map = db->getEditMap();
    if (!map)
    {
        return;
    }

    // if new vertex is over an old vertex, delete this vertex, and
    // replace old vertex in edges
    MapSelectionPtr  sel = selector->findVertex(spt,_vp);
    NeighboursPtr      n = map->getNeighbours(_vp);
    SelectionSet     set = selector->findEdges(spt,n);
    MapSelectionPtr sel2 = selector->findAnEdge(set);
    if (sel)
    {
        VertexPtr existing = sel->getVertex();
        for (auto & wedge : std::as_const(*n))
        {
            EdgePtr ep = wedge.lock();
            Q_ASSERT(map->contains(ep));
            if (ep->v1 == _vp)
            {
                ep->setV1(existing);    // substitute
            }
            else
            {
                Q_ASSERT(ep->v2 == _vp);
                ep->setV2(existing);    // substitue
            }
        }
        map->removeVertexSimple(_vp);   // delete
        qDebug() << "SNAPTO vertex";
    }
    else if (sel2)
    {
        QLineF line =  sel2->getLine();
        line =  meView->viewT.map(line);
        if (Geo::distToLine(spt, line) < 7.0)
        {
            QPointF pt = Geo::snapTo(spt,line);
            _vp->pt = meView->viewTinv.map(pt);
            qDebug() << "SNAPTO edge";
        }
    }
    else
    {
        SelectionSet endset = selector->findSelectionsUsingDB(spt);

        MapSelectionPtr endsel = selector->findAPoint(endset);
        if (endsel)
        {
            qDebug() << "end is point";
            _vp->pt = endsel->getPoint();
        }
        else
        {
            endsel = selector->findALine(endset);
            if (endsel)
            {
                qDebug() << "end is on line"  << sMapSelection[endsel->getType()];
                _vp->pt = endsel->getPointNear(endsel,meView->viewTinv.map(spt));
            }
            else
            {
                qDebug() << "no end selection - point is point";
                _vp->pt = meView->viewTinv.map(spt);
            }
        }
    }

	// tidy up
    map->cleanse(joinupColinearEdges | divideupIntersectingEdges);   // deal with lines crossing existing lines
    map->resetNeighbourMap();
    map->verify();

    MapMouseAction::endDragging(spt);
}

void MoveVertex::draw(QPainter* painter)
{
    qreal radius = 5.0;
    painter->setPen(QPen(Qt::blue,1));
    painter->setBrush(Qt::blue);
    painter->drawEllipse(meView->viewT.map(_vp->pt), radius, radius);
}

/////////
///
///  Move Edge
///
/////////

MoveEdge::MoveEdge(EdgePtr edge, QPointF spt ) : MapMouseAction(spt)
{
    desc  = "MoveEdge";
    _edge = edge;
}

void MoveEdge::updateDragging(QPointF spt)
{
    QPointF delta  = meView->viewTinv.map(spt)  - meView->viewTinv.map(last_drag);

    QPointF a = _edge->v1->pt;
    QPointF b = _edge->v2->pt;
    a += delta;
    b += delta;
    _edge->v1->pt = a;
    _edge->v2->pt = b;

    MapMouseAction::updateDragging(spt);
}

void MoveEdge::endDragging(QPointF spt)
{
    MapPtr map = db->getEditMap();
    if (map)
    {
        map->cleanse(divideupIntersectingEdges);     // deal with lines crossing existing lines
        map->resetNeighbourMap();
        MapMouseAction::endDragging(spt);
    }
}

/////////
///
///  Draw Line
///
/////////

DrawLine::DrawLine(SelectionSet & set, QPointF spt) : MapMouseAction(spt)
{
    start = nullptr;
    end   = nullptr;
    desc = "DrawLine";

    MapSelectionPtr sel = selector->findAPoint(set);
    if (sel)
    {
        start = new QPointF(sel->getPoint());
        qDebug() << "start is point" << *start;
    }
    else
    {
        sel = selector->findALine(set);
        if (sel)
        {
            start = new QPointF(sel->getPointNear(sel,meView->viewTinv.map(spt)));
            qDebug() << "start is on line"  << sMapSelection[sel->getType()] << *start;
        }
        else
        {
            start = new QPointF(meView->viewTinv.map(spt));
            qDebug() << "no start selection - point is point" << *start;
        }
    }
}

DrawLine::~DrawLine()
{
    if (start) delete start;
    if (end)   delete end;
}

void DrawLine::updateDragging(QPointF spt)
{
    if (end)
    {
        delete end;
    }

    end = new QPointF(meView->viewTinv.map(spt));

    QLineF newline(*start,*end);

    intersectPoints.clear();
    QPointF intersect;
    for (const auto & linfo : std::as_const(selector->lines))
    {
        QLineF::IntersectType itype = newline.intersects(linfo._line,&intersect);
        if (itype == QLineF::BoundedIntersection)
        {
            intersectPoints.push_back(intersect);
        }
    }

    MapMouseAction::updateDragging(spt);
}

void DrawLine::endDragging( QPointF spt)
{
    SelectionSet endset = selector->findSelectionsUsingDB(spt);

    MapSelectionPtr endsel = selector->findAPoint(endset);
    if (endsel)
    {
        end = new QPointF(endsel->getPoint());
        qDebug() << "end is point" << *end;
    }
    else
    {
        endsel = selector->findALine(endset);
        if (endsel)
        {
            end = new QPointF(endsel->getPointNear(endsel,meView->viewTinv.map(spt)));
            qDebug() << "end is on line"  << sMapSelection[endsel->getType()] << *end;
        }
        else
        {
            end = new QPointF(meView->viewTinv.map(spt));
            qDebug() << "no end selection - point is point" << *end;
        }
    }

    MapPtr map = db->getEditMap();
    if (map)
    {
        if (end && (*start != *end))
        {
            if (!startv)
            {
                startv = map->insertVertex(*start);
            }
            if (!endv)
            {
                endv = map->insertVertex(*end);
            }
            map->insertEdge(startv,endv);
        }

        // deal with new line crossing existing lines
        map->cleanse(divideupIntersectingEdges);     // deal with lines crossing existing lines
    }
    MapMouseAction::endDragging(spt);
}

void DrawLine::draw(QPainter * painter)
{
    qreal radius = 3.0;
    painter->setPen(QPen(Qt::green,3));
    painter->setBrush(Qt::green);

    for (auto & pt : std::as_const(intersectPoints))
    {
        painter->drawEllipse(meView->viewT.map(pt),radius, radius);
    }
    if (start)
    {
        painter->drawEllipse(meView->viewT.map(*start), radius, radius);
    }
    if (end)
    {
        painter->drawEllipse(meView->viewT.map(*end), radius, radius);
    }
    if (start && end)
    {
        painter->drawLine(meView->viewT.map(*start), meView->viewT.map(*end));
    }
}

/////////
///
///  Construction Line
///
/////////

ConstructionLine::ConstructionLine(SelectionSet & set, QPointF spt) : DrawLine(set, spt)
{
    desc = "ConstructionLine";
}

void ConstructionLine::endDragging( QPointF spt)
{
    SelectionSet endset = selector->findSelectionsUsingDB(spt);

    MapSelectionPtr endsel = selector->findAPoint(endset);
    if (endsel)
    {
        end = new QPointF(endsel->getPoint());
        qDebug() << "end is point" << *end;
    }
    else
    {
        endsel = selector->findALine(endset);
        if (endsel)
        {
            end = new QPointF(endsel->getPointNear(endsel,meView->viewTinv.map(spt)));
            qDebug() << "end is on line"  << sMapSelection[endsel->getType()] << *end;
        }
        else
        {
            end = new QPointF(meView->viewTinv.map(spt));
            qDebug() << "no end selection - point is point" << *end;
        }
    }

    if (end && (*start != *end))
    {
        db->constructionLines.push_back(QLineF(*start,*end));
        db->getStash()->stash();
    }

    forceRedraw();
}

/////////
///
///  ExtendLine
///
/////////

ExtendLine::ExtendLine(SelectionSet & set, QPointF spt, bool isP1) : MapMouseAction(spt)
{
    desc = "ExtendLine";

    MapSelectionPtr sel = selector->findALine(set);
    if (sel)
    {
        if (sel->getType() == MAP_EDGE)
        {
            startLine   = sel->getLine();
            currentLine = startLine;
            startEdge   = sel->getEdge();
        }
        else
        {
            Q_ASSERT(sel->getType() == MAP_LINE);
            startLine   =  sel->getLine();
            currentLine = startLine;
        }
    }
    if (!isP1)
    {
        flipDirection();
    }

    startDrag = spt;
}

ExtendLine::~ExtendLine()
{
}

void ExtendLine::updateDragging(QPointF spt)
{
    QPointF wpt = meView->viewTinv.map(spt);
    if (!selector->insideBoundary(wpt))
    {
        qDebug() << "Extend line - cancelled";
        currentLine = startLine;
        MapMouseAction::endDragging(spt);
        //me->setMapedMouseMode(MAPED_MOUSE_NONE);
        return;
    }
    
    qreal delta = Geo::dist(startDrag,spt);
    delta = delta / Transform::scalex(meView->viewT);
    qreal len   = startLine.length();
    qreal len2  = len + delta;
    currentLine = startLine;
    currentLine.setLength(len2);

    intersectPoints.clear();
    QPointF intersect;
    for (const auto & linfo : selector->lines)
    {
        QLineF::IntersectType itype = currentLine.intersects(linfo._line,&intersect);
        if (itype == QLineF::BoundedIntersection)
        {
            intersectPoints.push_back(intersect);
        }
    }

    MapMouseAction::updateDragging(spt);
}

void ExtendLine::endDragging( QPointF spt)
{
    QPointF wpt = meView->viewTinv.map(spt);
    if (!selector->insideBoundary(wpt))
    {
        qDebug() << "Extend line - cancelled";
        MapMouseAction::endDragging(spt);
        //me->setMapedMouseMode(MAPED_MOUSE_NONE);
        return;
    }
    
    qreal delta = Geo::dist(startDrag,spt);
    delta = delta / Transform::scalex(meView->viewT);
    qreal len   = startLine.length();
    qreal len2  = len + delta;
    currentLine = startLine;
    currentLine.setLength(len2);

    QPointF currentP2 = meView->viewT.map(currentLine.p2());

    SelectionSet endset = selector->findSelectionsUsingDB(currentP2);

    MapSelectionPtr endsel = selector->findAPoint(endset);
    QPointF end;
    if (endsel)
    {
        currentLine.setP2(endsel->getPoint());
        qDebug() << "end is point" << currentLine;
    }
    else
    {
        endsel = selector->findALine(endset);
        if (endsel)
        {
            currentLine.setP2(endsel->getPointNear(endsel,meView->viewTinv.map(currentP2)));
            qDebug() << "end is on line"  << sMapSelection[endsel->getType()] << end;
        }
        else
        {
            qDebug() << "no end selection - point is no point" << currentLine;
        }
    }

    if (startEdge)
    {
        MapPtr map = db->getEditMap();
        if (map)
        {
            // extending an edge
#if 0
            VertexPtr v2 = startEdge->v2;
            v2->setPosition(currentLine.p2());
#else
            VertexPtr v1 = startEdge->v2;
            VertexPtr v2 = map->insertVertex(currentLine.p2());
            map->insertEdge(v1,v2);
#endif
            map->cleanse(divideupIntersectingEdges);     // deal with lines crossing existing lines
            map->cleanse(joinupColinearEdges);
        }
    }
    else
    {
        // replacing a construction line
        db->constructionLines.removeAll(startLine);
        db->constructionLines.push_back(currentLine);
        db->getStash()->stash();
    }

    MapMouseAction::endDragging(spt);
    //me->setMapedMouseMode(MAPED_MOUSE_NONE);
}

void ExtendLine::draw(QPainter * painter)
{
    painter->setPen(QPen(Qt::yellow,3));
    painter->drawLine(meView->viewT.map(currentLine));

    painter->setPen(QPen(Qt::yellow,3));
    painter->setBrush(Qt::yellow);
    qreal radius = 3.0;
    for (auto pt : intersectPoints)
    {
        painter->drawEllipse(meView->viewT.map(pt),radius, radius);
    }
}

void ExtendLine::flipDirection()
{
    QPointF a = startLine.p1();
    QPointF b = startLine.p2();
    startLine = QLineF(b,a);

    a = currentLine.p1();
    b = currentLine.p2();
    currentLine = QLineF(b,a);

    if (startEdge)
    {
        VertexPtr va = startEdge->v1;
        VertexPtr vb = startEdge->v2;
        startEdge->setV1(vb);
        startEdge->setV2(va);
    }
}

EditConstructionCircle::EditConstructionCircle(Circle circle, QPointF spt) : MapMouseAction(spt)
{
    desc = "MoveConstructionCircle";
    start = nullptr;
    end   = nullptr;

    origCircle    = circle;
    currentCircle.set(circle);

    QPointF center = meView->viewT.map(currentCircle.centre);
    qreal radius   = Transform::scalex(meView->viewT) * currentCircle.radius;
    Circle sc(center,radius);
    if (Geo::pointOnCircle(spt,sc,7))
    {
        QPointF mpt = meView->viewTinv.map(spt);
        start = new QPointF(mpt);
        ecmode = CM_EDGE;
    }
    else if (Geo::pointInCircle(spt,sc))
    {
        QPointF mpt = meView->viewTinv.map(spt);
        start = new QPointF(mpt);
        ecmode = CM_INSIDE;
    }
    else
    {
        ecmode = CM_OUTSIDE;
    }
}

void EditConstructionCircle::updateDragging(QPointF spt)
{
    if (!start)
    {
        return;
    }

    QPointF mpt = meView->viewTinv.map(spt);
    if (ecmode == CM_EDGE)
    {
        // first find direction
        qreal s = Geo::dist2(*start,currentCircle.centre);
        qreal m = Geo::dist2(mpt,currentCircle.centre);
        bool sub = (s > m);
        // find delta
        qreal delta = Geo::dist(mpt,*start);
        delete start;
        start  = new QPointF(mpt);
        if (sub)
            delta = -delta;
        qreal radius = currentCircle.radius + delta;
        currentCircle.setRadius(radius);
    }
    else if (ecmode == CM_INSIDE)
    {
        QPointF delta = mpt - *start;
        delete start;
        start  = new QPointF(mpt);

        QPointF centre = currentCircle.centre + delta;
        currentCircle.setCenter(centre);
    }

    QPointF sCenter = meView->viewT.map(currentCircle.centre);
    selector->setCurrentSelections(selector->findSelectionsUsingDB(sCenter));
}

void EditConstructionCircle::endDragging( QPointF spt)
{
    updateDragging(spt);
    origCircle.set(currentCircle);
    MapMouseAction::endDragging(spt);
}

void EditConstructionCircle::draw(QPainter * painter)
{
    QColor color;
    if (ecmode == CM_INSIDE)
        color = Qt::blue;
    else if (ecmode == CM_EDGE)
        color = Qt::green;
    else
        return;

    QPointF center = meView->viewT.map(currentCircle.centre);
    painter->setPen(QPen(color,3));
    painter->drawEllipse(center, Transform::scalex(meView->viewT) * currentCircle.radius, Transform::scalex(meView->viewT) * currentCircle.radius);

    painter->setPen(QPen(color,1));
    qreal len = 9.0;
    painter->drawLine(QPointF(center.x()-len,center.y()),QPointF(center.x()+len,center.y()));
    painter->drawLine(QPointF(center.x(),center.y()-len),QPointF(center.x(),center.y()+len));
}

