#include "makers/mapmouseactions.h"
#include "makers/mapeditor.h"
#include "geometry/Vertex.h"
#include "geometry/Map.h"
#include "geometry/Point.h"
#include "geometry/Intersect.h"
#include "base/utilities.h"

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
    Q_UNUSED(painter);
}

void MapMouseAction::endDragging(QPointF spt)
{
    Q_UNUSED(spt);
    me->forceRedraw();
}

/////////
///
///  Move Vertex
///
/////////

MoveVertex::MoveVertex(MapEditor * me, VertexPtr vp, QPointF spt ) : MapMouseAction(me,spt)
{
    desc = "MoveVertex";
    _vp  = vp;
}

void MoveVertex::updateDragging(QPointF spt)
{
    QPointF w    = me->viewTinv.apply(spt);
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

    MapSelectionPtr sel  = me->findVertex(spt,_vp);
    SelectionSet    set  = me->findEdges(spt,_vp->getEdges());
    MapSelectionPtr sel2 = me->findAnEdge(set);
    if (sel)
    {
        VertexPtr existing = sel->getVertex();
        QVector<EdgePtr> eps = _vp->getEdges();
        for (auto it = eps.begin(); it != eps.end(); it++)
        {
            EdgePtr ep = *it;
            Q_ASSERT(map->contains(ep));
            if (ep->getV1() == _vp)
            {
                ep->setV1(existing);     // substitute
            }
            else
            {
                Q_ASSERT(ep->getV2() == _vp);
                ep->setV2(existing);     // substitue
            }
            existing->insertEdge(ep);    // connect
        }
        map->removeVertexSimple(_vp);    // delete
        qDebug() << "SNAPTO vertex";
        edited = true;
    }
    else if (sel2)
    {
        QLineF line =  sel2->getLine();
        line =  me->viewT.apply(line);
        if (Point::distToLine(spt, line) < 7.0)
        {
            QPointF pt = Utils::snapTo(spt,line);
            _vp->setPosition(me->viewTinv.apply(pt));
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
                _vp->setPosition(endsel->getPointNear(endsel,me->viewTinv.apply(spt)));
                edited = true;
            }
            else
            {
                qDebug() << "no end selection - point is point";
                _vp->setPosition(me->viewTinv.apply(spt));
                edited = false;
            }
        }
    }

    if (edited)
    {
        // tidy up
        map->sortVertices();
        map->sortEdges();
        map->cleanNeighbours();

        // deal with lines crossing existing lines
        map->joinColinearEdges();
        map->divideIntersectingEdges();

        map->verify("modifed fig map",false,true);
    }

    MapMouseAction::endDragging(spt);
    me->buildEditorDB();
}

void MoveVertex::draw(QPainter* painter)
{
    qreal radius = 5.0;
    painter->setPen(QPen(Qt::blue,1));
    painter->setBrush(Qt::blue);
    painter->drawEllipse(me->viewT.apply(_vp->getPosition()), radius, radius);
}

/////////
///
///  Move Edge
///
/////////

MoveEdge::MoveEdge(MapEditor * me, EdgePtr edge, QPointF spt ) : MapMouseAction(me,spt)
{
    desc  = "MoveEdge";
    _edge = edge;
}

void MoveEdge::updateDragging(QPointF spt)
{
    QPointF delta  = me->viewTinv.apply(spt)  - me->viewTinv.apply(last_drag);

    QPointF a = _edge->getV1()->getPosition();
    QPointF b = _edge->getV2()->getPosition();
    a += delta;
    b += delta;
    _edge->getV1()->setPosition(a);
    _edge->getV2()->setPosition(b);

    MapMouseAction::updateDragging(spt);
}

void MoveEdge::endDragging(QPointF spt)
{
    MapPtr map = me->map;

    map->sortVertices();
    map->sortEdges();
    map->cleanNeighbours();

    // deal with lines crossing existing lines
    map->divideIntersectingEdges();

    MapMouseAction::updateDragging(spt);
    me->buildEditorDB();
}

/////////
///
///  Draw Line
///
/////////

DrawLine::DrawLine(MapEditor * me, SelectionSet & set, QPointF spt) : MapMouseAction(me,spt)
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
            start = new QPointF(sel->getPointNear(sel,me->viewTinv.apply(spt)));
            qDebug() << "start is on line"  << sMapSelection[sel->getType()] << *start;
        }
        else
        {
            start = new QPointF(me->viewTinv.apply(spt));
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

    end = new QPointF(me->viewTinv.apply(spt));

    QLineF newline(*start,*end);

    intersectPoints.clear();
    QPointF intersect;
    for (auto it = me->lines.begin(); it != me->lines.end(); it++)
    {
        lineInfo & linfo = *it;
        QLineF::IntersectType itype = newline.intersect(linfo._line,&intersect);
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
            end = new QPointF(endsel->getPointNear(endsel,me->viewTinv.apply(spt)));
            qDebug() << "end is on line"  << sMapSelection[endsel->getType()] << *end;
        }
        else
        {
            end = new QPointF(me->viewTinv.apply(spt));
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
    map->divideIntersectingEdges();
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
        painter->drawEllipse(me->viewT.apply(pt),radius, radius);
    }
    if (start)
    {
        painter->drawEllipse(me->viewT.apply(*start), radius, radius);
    }
    if (end)
    {
        painter->drawEllipse(me->viewT.apply(*end), radius, radius);
    }
    if (start && end)
    {
        painter->drawLine(me->viewT.apply(*start), me->viewT.apply(*end));
    }
}

/////////
///
///  Construction Line
///
/////////

ConstructionLine::ConstructionLine(MapEditor * me, SelectionSet & set, QPointF spt)
    : DrawLine(me, set, spt)
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
            end = new QPointF(endsel->getPointNear(endsel,me->viewTinv.apply(spt)));
            qDebug() << "end is on line"  << sMapSelection[endsel->getType()] << *end;
        }
        else
        {
            end = new QPointF(me->viewTinv.apply(spt));
            qDebug() << "no end selection - point is point" << *end;
        }
    }

    if (end && (*start != *end))
    {
        me->constructionLines.push_back(QLineF(*start,*end));
        me->saveStash();
    }

    MapMouseAction::endDragging(spt);
    me->buildEditorDB();
}

/////////
///
///  ExtendLine
///
/////////

ExtendLine::ExtendLine(MapEditor * me, SelectionSet & set, QPointF spt) : MapMouseAction(me,spt)
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
    QPointF wpt = me->viewTinv.apply(spt);
    if (!me->insideBoundary(wpt))
    {
        qDebug() << "Extend line - cancelled";
        currentLine = startLine;
        MapMouseAction::endDragging(spt);
        me->setMouseMode(MAP_MODE_NONE);
        return;
    }

    qreal delta = Point::dist(startDrag,spt);
    delta = delta / me->viewT.scalex();
    qreal len   = startLine.length();
    qreal len2  = len + delta;
    currentLine = startLine;
    currentLine.setLength(len2);

    intersectPoints.clear();
    QPointF intersect;
    for (auto it = me->lines.begin(); it != me->lines.end(); it++)
    {
        lineInfo & linfo = *it;
        QLineF::IntersectType itype = currentLine.intersect(linfo._line,&intersect);
        if (itype == QLineF::BoundedIntersection)
        {
            intersectPoints.push_back(intersect);
        }
    }

    MapMouseAction::updateDragging(spt);
}

void ExtendLine::endDragging( QPointF spt)
{
    QPointF wpt = me->viewTinv.apply(spt);
    if (!me->insideBoundary(wpt))
    {
        qDebug() << "Extend line - cancelled";
        MapMouseAction::endDragging(spt);
        me->setMouseMode(MAP_MODE_NONE);
        return;
    }

    qreal delta = Point::dist(startDrag,spt);
    delta = delta / me->viewT.scalex();
    qreal len   = startLine.length();
    qreal len2  = len + delta;
    currentLine = startLine;
    currentLine.setLength(len2);

    QPointF currentP2 = me->viewT.apply(currentLine.p2());

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
            currentLine.setP2(endsel->getPointNear(endsel,me->viewTinv.apply(currentP2)));
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
        VertexPtr v2 = startEdge->getV2();
        v2->setPosition(currentLine.p2());
        map->divideIntersectingEdges(); // deal with new line crossing existing lines
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
    me->setMouseMode(MAP_MODE_NONE);
}

void ExtendLine::draw(QPainter * painter)
{
    painter->setPen(QPen(Qt::yellow,3));
    painter->drawLine(me->viewT.apply(currentLine));

    painter->setPen(QPen(Qt::yellow,3));
    painter->setBrush(Qt::yellow);
    qreal radius = 3.0;
    for (auto it = intersectPoints.begin(); it != intersectPoints.end(); it++)
    {
        QPointF pt = *it;
        painter->drawEllipse(me->viewT.apply(pt),radius, radius);
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
        VertexPtr va = startEdge->getV1();
        VertexPtr vb = startEdge->getV2();
        startEdge->setV1(vb);
        startEdge->setV2(va);
    }
}

MoveConstructionCircle::MoveConstructionCircle(MapEditor * me, CirclePtr circle, QPointF spt)
    : MapMouseAction(me,spt)
{
    desc = "MoveConstructionCircle";
    origCircle    = circle;
    currentCircle = *origCircle;

    last_drag    = me->viewTinv.apply(spt);
}

void MoveConstructionCircle::updateDragging(QPointF spt)
{
    QPointF w     = me->viewTinv.apply(spt);
    QPointF delta = w - last_drag;
    currentCircle.centre += delta;
    last_drag = w;

    QPointF sCenter = me->viewT.apply(currentCircle.centre);
    me->currentSelections = me->findSelectionsUsingDB(sCenter);
}

void MoveConstructionCircle::endDragging( QPointF spt)
{
    QPointF w     = me->viewTinv.apply(spt);
    QPointF delta = w - last_drag;
    currentCircle.centre += delta;
    QPointF new_center = me->viewT.apply(currentCircle.centre);

    SelectionSet endset    = me->findSelectionsUsingDB(new_center);
    MapSelectionPtr endsel = me->findAPoint(endset);
    QPointF center;
    if (endsel)
    {
        center = QPointF(endsel->getPoint());
    }
    // if no selection, don't adjust
    currentCircle.centre = center;
    *origCircle = currentCircle;
    me->buildEditorDB();
    MapMouseAction::endDragging(spt);
}

void MoveConstructionCircle::draw(QPainter * painter)
{
    QPointF center = me->viewT.apply(currentCircle.centre);
    painter->setPen(QPen(Qt::blue,3));
    painter->drawEllipse(center, me->viewT.scalex() * currentCircle.radius, me->viewT.scalex() * currentCircle.radius);

    painter->setPen(QPen(Qt::blue,1));
    qreal len = 9.0;
    painter->drawLine(QPointF(center.x()-len,center.y()),QPointF(center.x()+len,center.y()));
    painter->drawLine(QPointF(center.x(),center.y()-len),QPointF(center.x(),center.y()+len));
}
