#include <QMessageBox>

#include "gui/map_editor/map_editor.h"
#include "gui/map_editor/map_editor_db.h"
#include "gui/map_editor/map_editor_mouseactions.h"
#include "gui/map_editor/map_selection.h"
#include "gui/top/system_view_controller.h"
#include "gui/viewers/map_editor_view.h"
#include "model/motifs/motif.h"
#include "model/prototypes/design_element.h"
#include "model/settings/configuration.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/map.h"
#include "sys/geometry/map_cleanser.h"
#include "sys/geometry/map_verifier.h"
#include "sys/geometry/neighbours.h"
#include "sys/geometry/transform.h"
#include "sys/geometry/vertex.h"
#include "sys/sys.h"

typedef std::weak_ptr<class Edge>   WeakEdgePtr;

MapMouseAction::MapMouseAction(QPointF spt)
{
    desc      = "MapMouseAction";
    selector  = Sys::mapEditorView->getSelector();
    db        = Sys::mapEditorView->getDb();
    last_drag = spt;

    connect(this, &MapMouseAction::sig_updateView, Sys::viewController, &SystemViewController::slot_updateView);

    forceRedraw();
}

void MapMouseAction::forceRedraw()
{
    emit sig_updateView();
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
    DELPtr delp  = layer.getDel();
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

MoveVertex::MoveVertex(NeighbourMap * nmap, VertexPtr vp, QPointF spt ) : MapMouseAction(spt)
{
    desc       = "MoveVertex";
    _vp        = vp;
    this->nmap = nmap;
}

void MoveVertex::updateDragging(QPointF spt)
{
    QPointF w    = Sys::mapEditorView->viewTinv.map(spt);
    if (!selector->insideBoundary(w))
    {
        qDebug() << "outside boundary";
        return;
    }
    //qDebug() << "moveVertex: update spt=" << spt << "w=" << w;
    _vp->setPt(w);
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
    NeighboursPtr      n = nmap->getNeighbours(_vp);
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
        line =  Sys::mapEditorView->viewT.map(line);
        if (Geo::distToLine(spt, line) < 7.0)
        {
            QPointF pt = Geo::snapTo(spt,line);
            _vp->setPt(Sys::mapEditorView->viewTinv.map(pt));
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
            _vp->setPt(endsel->getPoint());
        }
        else
        {
            endsel = selector->findALine(endset);
            if (endsel)
            {
                qDebug() << "end is on line"  << sMapSelection[endsel->getType()];
                _vp->setPt(endsel->getPointNear(endsel,Sys::mapEditorView->viewTinv.map(spt)));
            }
            else
            {
                qDebug() << "no end selection - point is point";
                _vp->setPt(Sys::mapEditorView->viewTinv.map(spt));
            }
        }
    }

	// tidy up
    MapCleanser mc(map);
    mc.cleanse(joinupColinearEdges | divideupIntersectingEdges,Sys::config->mapedMergeSensitivity);   // deal with lines crossing existing lines

    MapVerifier mv(map);
    mv.verify();

    MapMouseAction::endDragging(spt);
}

void MoveVertex::draw(QPainter* painter)
{
    qreal radius = 5.0;
    painter->setPen(QPen(Qt::blue,1));
    painter->setBrush(Qt::blue);
    painter->drawEllipse(Sys::mapEditorView->viewT.map(_vp->pt), radius, radius);
}

/////////
///
///  Move Edge
///
/////////

MoveEdge::MoveEdge(NeighbourMap *nmap, EdgePtr edge, QPointF spt ) : MapMouseAction(spt)
{
    desc       = "MoveEdge";
    _edge      = edge;
    this->nmap = nmap;
}

void MoveEdge::updateDragging(QPointF spt)
{
    QPointF delta  = Sys::mapEditorView->viewTinv.map(spt)  - Sys::mapEditorView->viewTinv.map(last_drag);

    QPointF a = _edge->v1->pt;
    QPointF b = _edge->v2->pt;
    a += delta;
    b += delta;
    _edge->v1->setPt(a);
    _edge->v2->setPt(b);

    MapMouseAction::updateDragging(spt);
}

void MoveEdge::endDragging(QPointF spt)
{
    MapPtr map = db->getEditMap();
    if (map)
    {
        MapCleanser mc(map);
        mc.cleanse(divideupIntersectingEdges,Sys::config->mapedMergeSensitivity);     // deal with lines crossing existing lines
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
            start = new QPointF(sel->getPointNear(sel,Sys::mapEditorView->viewTinv.map(spt)));
            qDebug() << "start is on line"  << sMapSelection[sel->getType()] << *start;
        }
        else
        {
            start = new QPointF(Sys::mapEditorView->viewTinv.map(spt));
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

    end = new QPointF(Sys::mapEditorView->viewTinv.map(spt));

    QLineF newline(*start,*end);

    intersectPoints.clear();
    for (const LineInfo & linfo : std::as_const(selector->lines))
    {
        if (linfo.type == LINE_EDGE && linfo.edge->isCurve())
        {
            QPointF isect1;
            QPointF isect2;
            int count = Geo::findLineCircleIntersections(linfo.edge->getArcCenter(),linfo.edge->getRadius(),newline,isect1,isect2);

            if (count && linfo.edge->pointWithinArc(isect1))
                intersectPoints.push_back(isect1);

            if (count == 2 && linfo.edge->pointWithinArc(isect2))
                intersectPoints.push_back(isect2);
        }
        else
        {
            QPointF intersect;
            QLineF::IntersectType itype = newline.intersects(linfo.line,&intersect);
            if (itype == QLineF::BoundedIntersection)
            {
                intersectPoints.push_back(intersect);
            }
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
            end = new QPointF(endsel->getPointNear(endsel,Sys::mapEditorView->viewTinv.map(spt)));
            qDebug() << "end is on line"  << sMapSelection[endsel->getType()] << *end;
        }
        else
        {
            end = new QPointF(Sys::mapEditorView->viewTinv.map(spt));
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
            map->insertEdge(startv,endv); // deals with new line crossing existing lines
        }
    }
    else
    {
        QMessageBox box((QWidget*)Sys::sysview);
        box.setIcon(QMessageBox::Warning);
        box.setText("Cannot save line\nPlease load a map or create a new map first");
        box.exec();
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
        painter->drawEllipse(Sys::mapEditorView->viewT.map(pt),radius, radius);
    }
    if (start)
    {
        painter->drawEllipse(Sys::mapEditorView->viewT.map(*start), radius, radius);
    }
    if (end)
    {
        painter->drawEllipse(Sys::mapEditorView->viewT.map(*end), radius, radius);
    }
    if (start && end)
    {
        painter->drawLine(Sys::mapEditorView->viewT.map(*start), Sys::mapEditorView->viewT.map(*end));
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
            end = new QPointF(endsel->getPointNear(endsel,Sys::mapEditorView->viewTinv.map(spt)));
            qDebug() << "end is on line"  << sMapSelection[endsel->getType()] << *end;
        }
        else
        {
            end = new QPointF(Sys::mapEditorView->viewTinv.map(spt));
            qDebug() << "no end selection - point is point" << *end;
        }
    }

    if (end && (*start != *end))
    {
        db->constructionLines.push_back(QLineF(*start,*end));
        Sys::mapEditor->stash();
    }

    forceRedraw();
}

/////////
///
///  ExtendLineP2
///
/////////

ExtendLineP2::ExtendLineP2(SelectionSet & set, QPointF spt) : MapMouseAction(spt)
{
    desc = "ExtendLineP2";

    MapSelectionPtr sel = selector->findALine(set);
    if (sel)
    {
        if (sel->getType() == MAP_EDGE)
        {
            startEdge   = sel->getEdge();
            startLine   = sel->getLine();
            currentLine = startLine;
        }
        else
        {
            Q_ASSERT(sel->getType() == MAP_LINE);
            startLine   = sel->getLine();
            currentLine = startLine;
        }
    }

    startDrag = spt;
}

ExtendLineP2::~ExtendLineP2()
{
}

void ExtendLineP2::updateDragging(QPointF spt)
{
    QPointF wpt = Sys::mapEditorView->viewTinv.map(spt);
    if (!selector->insideBoundary(wpt))
    {
        qDebug() << "Extend line - cancelled";
        currentLine = startLine;
        MapMouseAction::endDragging(spt);
        return;
    }
    
    qreal delta = Geo::dist(startDrag,spt);
    delta = delta / Transform::scalex(Sys::mapEditorView->viewT);
    qreal len   = startLine.length();
    qreal len2  = len + delta;
    currentLine = startLine;
    currentLine.setLength(len2);

    intersectPoints.clear();
    QPointF intersect;
    for (const auto & linfo : std::as_const(selector->lines))
    {
        QLineF::IntersectType itype = currentLine.intersects(linfo.line,&intersect);
        if (itype == QLineF::BoundedIntersection)
        {
            intersectPoints.push_back(intersect);
        }
    }

    MapMouseAction::updateDragging(spt);
}

void ExtendLineP2::endDragging( QPointF spt)
{
    QPointF wpt = Sys::mapEditorView->viewTinv.map(spt);
    if (!selector->insideBoundary(wpt))
    {
        qDebug() << "Extend line - cancelled";
        MapMouseAction::endDragging(spt);
        return;
    }
    
    qreal delta = Geo::dist(startDrag,spt);
    delta = delta / Transform::scalex(Sys::mapEditorView->viewT);
    qreal len   = startLine.length();
    qreal len2  = len + delta;
    currentLine = startLine;
    currentLine.setLength(len2);

    // this current line now needs to be made into an edge which is put into the map
    // if it comes from a map or put into the set of construction lines

    QPointF currentP2      = Sys::mapEditorView->viewT.map(currentLine.p2());
    SelectionSet endset    = selector->findSelectionsUsingDB(currentP2);
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
            currentLine.setP2(endsel->getPointNear(endsel,Sys::mapEditorView->viewTinv.map(currentP2)));
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
            VertexPtr v1 = startEdge->v2;
            VertexPtr v2 = map->insertVertex(currentLine.p2());
            map->insertEdge(v1,v2);

            MapCleanser mc(map);
            mc.cleanse(divideupIntersectingEdges,Sys::config->mapedMergeSensitivity);     // deal with lines crossing existing lines
            mc.cleanse(joinupColinearEdges,Sys::config->mapedMergeSensitivity);
        }
    }
    else
    {
        // replacing a construction line
        db->constructionLines.removeAll(startLine);
        db->constructionLines.push_back(currentLine);
        Sys::mapEditor->stash();
    }

    MapMouseAction::endDragging(spt);
}

void ExtendLineP2::draw(QPainter * painter)
{
    painter->setPen(QPen(Qt::yellow,3));
    painter->drawLine(Sys::mapEditorView->viewT.map(currentLine));

    painter->setPen(QPen(Qt::yellow,3));
    painter->setBrush(Qt::yellow);
    qreal radius = 3.0;
    for (auto & pt : intersectPoints)
    {
        painter->drawEllipse(Sys::mapEditorView->viewT.map(pt),radius, radius);
    }

    painter->setPen(QPen(Qt::red,3));
    painter->drawEllipse(Sys::mapEditorView->viewT.map(currentLine.p1()),radius, radius);
    painter->setPen(QPen(Qt::green,3));
    painter->drawEllipse(Sys::mapEditorView->viewT.map(currentLine.p2()),radius, radius);
}


/////////
///
///  ExtendLineP1
///
/////////

ExtendLineP1::ExtendLineP1(SelectionSet & set, QPointF spt) : MapMouseAction(spt)
{
    desc = "ExtendLineP2";

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
            startLine   = sel->getLine();
            currentLine = startLine;
        }
    }

    startDrag = spt;
}

ExtendLineP1::~ExtendLineP1()
{
}

void ExtendLineP1::updateDragging(QPointF spt)
{
    QPointF wpt = Sys::mapEditorView->viewTinv.map(spt);
    if (!selector->insideBoundary(wpt))
    {
        qDebug() << "Extend line - cancelled";
        currentLine = startLine;
        MapMouseAction::endDragging(spt);
        return;
    }

    qreal delta = Geo::dist(startDrag,spt);
    delta = delta / Transform::scalex(Sys::mapEditorView->viewT);
    qreal len   = startLine.length();
    qreal len2  = len + delta;
    currentLine = startLine;
    // modify p1
    const qreal oldLength = currentLine.length();
    QPointF pt1 = QPointF(currentLine.x2() - len2 * (currentLine.dx() / oldLength),
                          currentLine.y2() - len2 * (currentLine.dy() / oldLength));
    currentLine.setP1(pt1);

    intersectPoints.clear();
    QPointF intersect;
    for (const auto & linfo : std::as_const(selector->lines))
    {
        QLineF::IntersectType itype = currentLine.intersects(linfo.line,&intersect);
        if (itype == QLineF::BoundedIntersection)
        {
            intersectPoints.push_back(intersect);
        }
    }

    MapMouseAction::updateDragging(spt);
}

void ExtendLineP1::endDragging(QPointF spt)
{
    QPointF wpt = Sys::mapEditorView->viewTinv.map(spt);
    if (!selector->insideBoundary(wpt))
    {
        qDebug() << "Extend line - cancelled";
        MapMouseAction::endDragging(spt);
        return;
    }

    qreal delta = Geo::dist(startDrag,spt);
    delta = delta / Transform::scalex(Sys::mapEditorView->viewT);
    qreal len   = startLine.length();
    qreal len2  = len + delta;
    currentLine = startLine;
    // modify p1
    const qreal oldLength = currentLine.length();
    QPointF pt1 = QPointF(currentLine.x2() - len2 * (currentLine.dx() / oldLength),
                          currentLine.y2() - len2 * (currentLine.dy() / oldLength));
    currentLine.setP1(pt1);

    // this current line now needs to be made into an edge which is put into the map
    // if it comes from a map or put into the set of construction lines

    QPointF currentP1      = Sys::mapEditorView->viewT.map(currentLine.p1());
    SelectionSet endset    = selector->findSelectionsUsingDB(currentP1);
    MapSelectionPtr endsel = selector->findAPoint(endset);
    QPointF end;
    if (endsel)
    {
        currentLine.setP1(endsel->getPoint());
        qDebug() << "end is point" << currentLine;
    }
    else
    {
        endsel = selector->findALine(endset);
        if (endsel)
        {
            currentLine.setP1(endsel->getPointNear(endsel,Sys::mapEditorView->viewTinv.map(currentP1)));
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
            VertexPtr v1 = startEdge->v1;
            VertexPtr v2 = map->insertVertex(currentLine.p1());
            map->insertEdge(v1,v2);

            MapCleanser mc(map);
            mc.cleanse(divideupIntersectingEdges,Sys::config->mapedMergeSensitivity);     // deal with lines crossing existing lines
            mc.cleanse(joinupColinearEdges,Sys::config->mapedMergeSensitivity);
        }
    }
    else
    {
        // replacing a construction line
        db->constructionLines.removeAll(startLine);
        db->constructionLines.push_back(currentLine);
        Sys::mapEditor->stash();
    }

    MapMouseAction::endDragging(spt);
}

void ExtendLineP1::draw(QPainter * painter)
{
    painter->setPen(QPen(Qt::yellow,3));
    painter->drawLine(Sys::mapEditorView->viewT.map(currentLine));

    painter->setPen(QPen(Qt::yellow,3));
    painter->setBrush(Qt::yellow);
    qreal radius = 3.0;
    for (auto & pt : intersectPoints)
    {
        painter->drawEllipse(Sys::mapEditorView->viewT.map(pt),radius, radius);
    }

    painter->setPen(QPen(Qt::red,3));
    painter->drawEllipse(Sys::mapEditorView->viewT.map(currentLine.p1()),radius, radius);
    painter->setPen(QPen(Qt::green,3));
    painter->drawEllipse(Sys::mapEditorView->viewT.map(currentLine.p2()),radius, radius);
}

/////////
///
///  EditConstructionCircle
///
/////////

EditConstructionCircle::EditConstructionCircle(CirclePtr circle, QPointF spt) : MapMouseAction(spt)
{
    desc = "EditConstructionCircle";
    start = nullptr;
    end   = nullptr;

    origCircle    = circle;
    currentCircle.set(circle.get());

    QPointF center = Sys::mapEditorView->viewT.map(currentCircle.centre);
    qreal radius   = Transform::scalex(Sys::mapEditorView->viewT) * currentCircle.radius;
    Circle sc(center,radius);
    if (Geo::pointOnCircle(spt,sc,7))
    {
        QPointF mpt = Sys::mapEditorView->viewTinv.map(spt);
        start = new QPointF(mpt);
        ecmode = CM_EDGE;
    }
    else if (Geo::pointInCircle(spt,sc))
    {
        QPointF mpt = Sys::mapEditorView->viewTinv.map(spt);
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

    QPointF mpt = Sys::mapEditorView->viewTinv.map(spt);
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

    QPointF sCenter = Sys::mapEditorView->viewT.map(currentCircle.centre);
    selector->setCurrentSelections(selector->findSelectionsUsingDB(sCenter));
}

void EditConstructionCircle::endDragging( QPointF spt)
{
    updateDragging(spt);
    origCircle->set(&currentCircle);
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

    QPointF center = Sys::mapEditorView->viewT.map(currentCircle.centre);
    painter->setPen(QPen(color,3));
    painter->drawEllipse(center, Transform::scalex(Sys::mapEditorView->viewT) * currentCircle.radius, Transform::scalex(Sys::mapEditorView->viewT) * currentCircle.radius);

    painter->setPen(QPen(color,1));
    qreal len = 9.0;
    painter->drawLine(QPointF(center.x()-len,center.y()),QPointF(center.x()+len,center.y()));
    painter->drawLine(QPointF(center.x(),center.y()-len),QPointF(center.x(),center.y()+len));
}

