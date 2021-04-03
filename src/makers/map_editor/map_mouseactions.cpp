#include "makers/map_editor/map_mouseactions.h"
#include "makers/map_editor/map_editor.h"
#include "geometry/vertex.h"
#include "geometry/map.h"
#include "geometry/map_cleanser.h"
#include "geometry/point.h"
#include "geometry/transform.h"
#include "geometry/intersect.h"
#include "base/utilities.h"
#include "tapp/figure.h"

MapMouseAction::MapMouseAction(MapEditor * me, QPointF spt)
{
    desc = "MapMouseAction";
    this->me  = me;

    last_drag = spt;
    me->forceRedraw();
}

void MapMouseAction::updateDragging(QPointF spt)
{
    last_drag = spt;
    me->forceRedraw();
}

void MapMouseAction::draw(QPainter *painter)
{
    Q_UNUSED(painter)
}

void MapMouseAction::endDragging(QPointF spt)
{
    Q_UNUSED(spt)

    FigurePtr figp = me->getFigure();
    if (figp)
    {
        //map has been changed
        figp->setFigType(FIG_TYPE_EXPLICIT);
    }

    me->forceRedraw();
}

/////////
///
///  Move Vertex
///
/////////

MoveVertex::MoveVertex(MapEditor * maped, VertexPtr vp, QPointF spt ) : MapMouseAction(maped,spt)
{
    desc    = "MoveVertex";
    _vp     = vp;
}

void MoveVertex::updateDragging(QPointF spt)
{
    QPointF w    = me->viewTinv.map(spt);
    if (!me->insideBoundary(w))
    {
        qDebug() << "outside boundary";
        return;
    }
    //qDebug() << "moveVertex: update spt=" << spt << "w=" << w;
    _vp->setPosition(w);
    MapMouseAction::updateDragging(spt);
}

void MoveVertex::endDragging( QPointF spt)
{
    MapPtr map  = me->map;

    bool edited = false;

    // if new vertex is over an old vertex, delete this vertex, and
    // replace old vertex in edges
    MapSelectionPtr  sel = me->findVertex(spt,_vp);
    SelectionSet     set = me->findEdges(spt,_vp->getNeighbours());
    MapSelectionPtr sel2 = me->findAnEdge(set);
    if (sel)
    {
        VertexPtr existing = sel->getVertex();
        for (auto ep : qAsConst(_vp->getNeighbours()))
        {
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
            existing->insertNeighbour(ep);        // connect
        }
        map->removeVertexSimple(_vp);   // delete
        qDebug() << "SNAPTO vertex";
        edited = true;
    }
    else if (sel2)
    {
        QLineF line =  sel2->getLine();
        line =  me->viewT.map(line);
        if (Point::distToLine(spt, line) < 7.0)
        {
            QPointF pt = Utils::snapTo(spt,line);
            _vp->setPosition(me->viewTinv.map(pt));
            qDebug() << "SNAPTO edge";
            edited = true;
        }
    }
    else
    {
        SelectionSet endset = me->findSelectionsUsingDB(spt);

        MapSelectionPtr endsel = me->findAPoint(endset);
        if (endsel)
        {
            qDebug() << "end is point";
            _vp->setPosition(endsel->getPoint());
            edited = true;
        }
        else
        {
            endsel = me->findALine(endset);
            if (endsel)
            {
                qDebug() << "end is on line"  << sMapSelection[endsel->getType()];
                _vp->setPosition(endsel->getPointNear(endsel,me->viewTinv.map(spt)));
                edited = true;
            }
            else
            {
                qDebug() << "no end selection - point is point";
                _vp->setPosition(me->viewTinv.map(spt));
            }
        }
    }

    if (edited)
    {

        // tidy up
        MapCleanser cleanser(map);
        cleanser.cleanse(joinupColinearEdges | divideupIntersectingEdges,false);   // deal with lines crossing existing lines

        map->sortVertices();
        map->sortEdges();
        map->buildNeighbours();
        map->verifyMap("modifed fig map");
    }

    MapMouseAction::endDragging(spt);
    me->buildEditorDB();
}

void MoveVertex::draw(QPainter* painter)
{
    qreal radius = 5.0;
    painter->setPen(QPen(Qt::blue,1));
    painter->setBrush(Qt::blue);
    painter->drawEllipse(me->viewT.map(_vp->pt), radius, radius);
}

/////////
///
///  Move Edge
///
/////////

MoveEdge::MoveEdge(MapEditor * maped, EdgePtr edge, QPointF spt ) : MapMouseAction(maped,spt)
{
    desc  = "MoveEdge";
    _edge = edge;
}

void MoveEdge::updateDragging(QPointF spt)
{
    QPointF delta  = me->viewTinv.map(spt)  - me->viewTinv.map(last_drag);

    QPointF a = _edge->v1->pt;
    QPointF b = _edge->v2->pt;
    a += delta;
    b += delta;
    _edge->v1->setPosition(a);
    _edge->v2->setPosition(b);

    MapMouseAction::updateDragging(spt);
}

void MoveEdge::endDragging(QPointF spt)
{
    MapPtr map = me->map;

    MapCleanser cleanser(map);
    cleanser.cleanse(divideupIntersectingEdges,false);     // deal with lines crossing existing lines

    map->sortVertices();
    map->sortEdges();
    map->buildNeighbours();

    MapMouseAction::endDragging(spt);
    me->buildEditorDB();
}

/////////
///
///  Draw Line
///
/////////

DrawLine::DrawLine(MapEditor * maped, SelectionSet & set, QPointF spt) : MapMouseAction(maped,spt)
{
    start = nullptr;
    end   = nullptr;
    desc = "DrawLine";

    MapSelectionPtr sel = me->findAPoint(set);
    if (sel)
    {
        start = new QPointF(sel->getPoint());
        qDebug() << "start is point" << *start;
    }
    else
    {
        sel = me->findALine(set);
        if (sel)
        {
            start = new QPointF(sel->getPointNear(sel,me->viewTinv.map(spt)));
            qDebug() << "start is on line"  << sMapSelection[sel->getType()] << *start;
        }
        else
        {
            start = new QPointF(me->viewTinv.map(spt));
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

    end = new QPointF(me->viewTinv.map(spt));

    QLineF newline(*start,*end);

    intersectPoints.clear();
    QPointF intersect;
    for (auto it = me->lines.begin(); it != me->lines.end(); it++)
    {
        lineInfo & linfo = *it;
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
    SelectionSet endset = me->findSelectionsUsingDB(spt);

    MapSelectionPtr endsel = me->findAPoint(endset);
    if (endsel)
    {
        end = new QPointF(endsel->getPoint());
        qDebug() << "end is point" << *end;
    }
    else
    {
        endsel = me->findALine(endset);
        if (endsel)
        {
            end = new QPointF(endsel->getPointNear(endsel,me->viewTinv.map(spt)));
            qDebug() << "end is on line"  << sMapSelection[endsel->getType()] << *end;
        }
        else
        {
            end = new QPointF(me->viewTinv.map(spt));
            qDebug() << "no end selection - point is point" << *end;
        }
    }

    MapPtr map = me->map;

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
    MapCleanser cleanser(map);
    cleanser.cleanse(divideupIntersectingEdges,false);     // deal with lines crossing existing lines

    MapMouseAction::endDragging(spt);
    me->buildEditorDB();
}

void DrawLine::draw(QPainter * painter)
{
    qreal radius = 3.0;
    painter->setPen(QPen(Qt::green,3));
    painter->setBrush(Qt::green);

    for (auto it = intersectPoints.begin(); it !=intersectPoints.end(); it++)
    {
        QPointF pt = *it;
        painter->drawEllipse(me->viewT.map(pt),radius, radius);
    }
    if (start)
    {
        painter->drawEllipse(me->viewT.map(*start), radius, radius);
    }
    if (end)
    {
        painter->drawEllipse(me->viewT.map(*end), radius, radius);
    }
    if (start && end)
    {
        painter->drawLine(me->viewT.map(*start), me->viewT.map(*end));
    }
}

/////////
///
///  Construction Line
///
/////////

ConstructionLine::ConstructionLine(MapEditor * maped, SelectionSet & set, QPointF spt)
    : DrawLine(maped, set, spt)
{
    desc = "ConstructionLine";
}

void ConstructionLine::endDragging( QPointF spt)
{
    SelectionSet endset = me->findSelectionsUsingDB(spt);

    MapSelectionPtr endsel = me->findAPoint(endset);
    if (endsel)
    {
        end = new QPointF(endsel->getPoint());
        qDebug() << "end is point" << *end;
    }
    else
    {
        endsel = me->findALine(endset);
        if (endsel)
        {
            end = new QPointF(endsel->getPointNear(endsel,me->viewTinv.map(spt)));
            qDebug() << "end is on line"  << sMapSelection[endsel->getType()] << *end;
        }
        else
        {
            end = new QPointF(me->viewTinv.map(spt));
            qDebug() << "no end selection - point is point" << *end;
        }
    }

    if (end && (*start != *end))
    {
        me->constructionLines.push_back(QLineF(*start,*end));
        me->saveStash();
    }

    me->buildEditorDB();
    me->forceRedraw();
}

/////////
///
///  ExtendLine
///
/////////

ExtendLine::ExtendLine(MapEditor * maped, SelectionSet & set, QPointF spt) : MapMouseAction(maped,spt)
{
    desc = "ExtendLine";

    MapSelectionPtr sel = me->findALine(set);
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
    startDrag = spt;
}

ExtendLine::~ExtendLine()
{
}

void ExtendLine::updateDragging(QPointF spt)
{
    QPointF wpt = me->viewTinv.map(spt);
    if (!me->insideBoundary(wpt))
    {
        qDebug() << "Extend line - cancelled";
        currentLine = startLine;
        MapMouseAction::endDragging(spt);
        me->setMouseMode(MAPED_MOUSE_NONE);
        return;
    }

    qreal delta = Point::dist(startDrag,spt);
    delta = delta / Transform::scalex(me->viewT);
    qreal len   = startLine.length();
    qreal len2  = len + delta;
    currentLine = startLine;
    currentLine.setLength(len2);

    intersectPoints.clear();
    QPointF intersect;
    for (auto it = me->lines.begin(); it != me->lines.end(); it++)
    {
        lineInfo & linfo = *it;
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
    QPointF wpt = me->viewTinv.map(spt);
    if (!me->insideBoundary(wpt))
    {
        qDebug() << "Extend line - cancelled";
        MapMouseAction::endDragging(spt);
        me->setMouseMode(MAPED_MOUSE_NONE);
        return;
    }

    qreal delta = Point::dist(startDrag,spt);
    delta = delta / Transform::scalex(me->viewT);
    qreal len   = startLine.length();
    qreal len2  = len + delta;
    currentLine = startLine;
    currentLine.setLength(len2);

    QPointF currentP2 = me->viewT.map(currentLine.p2());

    SelectionSet endset = me->findSelectionsUsingDB(currentP2);

    MapSelectionPtr endsel = me->findAPoint(endset);
    QPointF end;
    if (endsel)
    {
        currentLine.setP2(endsel->getPoint());
        qDebug() << "end is point" << currentLine;
    }
    else
    {
        endsel = me->findALine(endset);
        if (endsel)
        {
            currentLine.setP2(endsel->getPointNear(endsel,me->viewTinv.map(currentP2)));
            qDebug() << "end is on line"  << sMapSelection[endsel->getType()] << end;
        }
        else
        {
            qDebug() << "no end selection - point is no point" << currentLine;
        }
    }

    if (startEdge)
    {
        MapPtr map = me->map;
        // extending an edge
        VertexPtr v2 = startEdge->v2;
        v2->setPosition(currentLine.p2());

        MapCleanser cleanser(map);
        cleanser.cleanse(divideupIntersectingEdges,false);     // deal with lines crossing existing lines
    }
    else
    {
        // replacing a construction line
        me->constructionLines.removeAll(startLine);
        me->constructionLines.push_back(currentLine);
        me->saveStash();
    }

    MapMouseAction::endDragging(spt);
    me->buildEditorDB();
    me->setMouseMode(MAPED_MOUSE_NONE);
}

void ExtendLine::draw(QPainter * painter)
{
    painter->setPen(QPen(Qt::yellow,3));
    painter->drawLine(me->viewT.map(currentLine));

    painter->setPen(QPen(Qt::yellow,3));
    painter->setBrush(Qt::yellow);
    qreal radius = 3.0;
    for (auto it = intersectPoints.begin(); it != intersectPoints.end(); it++)
    {
        QPointF pt = *it;
        painter->drawEllipse(me->viewT.map(pt),radius, radius);
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

MoveConstructionCircle::MoveConstructionCircle(MapEditor * maped, CirclePtr circle, QPointF spt)
    : MapMouseAction(maped,spt)
{
    desc = "MoveConstructionCircle";
    origCircle    = circle;
    currentCircle.set(circle);

    last_drag    = me->viewTinv.map(spt);
}

void MoveConstructionCircle::updateDragging(QPointF spt)
{
    QPointF w     = me->viewTinv.map(spt);
    QPointF delta = w - last_drag;
    currentCircle.centre += delta;
    last_drag = w;

    QPointF sCenter = me->viewT.map(currentCircle.centre);
    me->currentSelections = me->findSelectionsUsingDB(sCenter);
}

void MoveConstructionCircle::endDragging( QPointF spt)
{
    QPointF w     = me->viewTinv.map(spt);
    QPointF delta = w - last_drag;
    currentCircle.centre += delta;
    QPointF new_center = me->viewT.map(currentCircle.centre);

    SelectionSet endset    = me->findSelectionsUsingDB(new_center);
    MapSelectionPtr endsel = me->findAPoint(endset);
    QPointF center;
    if (endsel)
    {
        center = QPointF(endsel->getPoint());
    }
    // if no selection, don't adjust
    currentCircle.centre = center;
    origCircle->set(currentCircle);
    me->buildEditorDB();
    MapMouseAction::endDragging(spt);
}

void MoveConstructionCircle::draw(QPainter * painter)
{
    QPointF center = me->viewT.map(currentCircle.centre);
    painter->setPen(QPen(Qt::blue,3));
    painter->drawEllipse(center, Transform::scalex(me->viewT) * currentCircle.radius, Transform::scalex(me->viewT) * currentCircle.radius);

    painter->setPen(QPen(Qt::blue,1));
    qreal len = 9.0;
    painter->drawLine(QPointF(center.x()-len,center.y()),QPointF(center.x()+len,center.y()));
    painter->drawLine(QPointF(center.x(),center.y()-len),QPointF(center.x(),center.y()+len));
}

/////////
///
///  Create Crop
///
/////////

CreateCrop::CreateCrop(MapEditor * maped, QPointF spt) : MapMouseAction(maped,spt)
{
    start = nullptr;
    end   = nullptr;
    desc = "DrawLine";

    me->cropRect = QRectF();
    Q_ASSERT(me->cropRect.isEmpty());

    start = new QPointF(me->viewTinv.map(spt));
}

CreateCrop::~CreateCrop()
{
    if (start) delete start;
    if (end)   delete end;
}

void CreateCrop::updateDragging(QPointF spt)
{
    if (end)
    {
        delete end;
    }

    end = new QPointF(me->viewTinv.map(spt));

    if (start && end)
    {
        crop = QRectF(*start,*end);
    }
}

void CreateCrop::endDragging( QPointF spt)
{
    updateDragging(spt);

    me->cropRect = crop;

    MapMouseAction::endDragging(spt);

    me->setMouseMode(MAPED_MOUSE_NONE);
}

void CreateCrop::draw(QPainter * painter)
{
    if (!crop.isEmpty())
    {
        painter->setPen(QPen(Qt::red,3));
        painter->setBrush(QBrush(0xff0000,Qt::Dense7Pattern));

        QRectF rect = me->viewT.mapRect(crop);
        painter->drawRect(rect);
    }
}
