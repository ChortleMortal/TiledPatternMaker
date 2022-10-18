#include <QDebug>
#include <QApplication>

#include "makers/tiling_maker/tiling_mouseactions.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "makers/tiling_maker/tile_selection.h"
#include "geometry/point.h"
#include "geometry/transform.h"
#include "geometry/edge.h"
#include "geometry/vertex.h"
#include "misc/utilities.h"
#include "misc/geo_graphics.h"
#include "tile/placed_tile.h"
#include "tile/tile.h"
#include "viewers/grid.h"

using std::make_shared;

////////////////////////////////////////////////////////////////////////////
//
// Mouse Actions
//
////////////////////////////////////////////////////////////////////////////

TilingMouseAction::TilingMouseAction(TilingMaker * tilingMaker, TilingSelectorPtr sel, QPointF spt)
{
    desc       = "MouseAction";
    qDebug() << desc;

    tm         = tilingMaker;
    selection  = sel;
    wLastDrag  = tm->screenToWorld(spt);
    drag_color = QColor(206,179,102,230);

    tm->forceRedraw();
}

void TilingMouseAction::updateDragging(QPointF spt)
{
    wLastDrag = tm->screenToWorld(spt);
    tm->forceRedraw();
}

void TilingMouseAction::draw(GeoGraphics * g2d)
{
    Q_UNUSED(g2d)
}

void TilingMouseAction::endDragging(QPointF spt)
{
    Q_UNUSED(spt)
    tm->forceRedraw();
}

/////////
///
///  MovePolygon
///
/////////

MovePolygon::MovePolygon(TilingMaker * tilingMaker, TilingSelectorPtr sel, QPointF spt )
    : TilingMouseAction(tilingMaker,sel,spt)
{
    desc = "MovePolygon";
    qDebug() << desc;
}

void MovePolygon::updateDragging(QPointF spt)
{
    if  (selection && selection->getType() == INTERIOR)
    {
        qDebug() << "MovePolygon: update";

        QPointF wpt         = tm->screenToWorld(spt);
        PlacedTilePtr pf = selection->getPlacedTile();
        QPointF diff        = wpt - wLastDrag;

        QTransform a = pf->getTransform();
        QTransform b = QTransform::fromTranslate(diff.x(),diff.y());
        QTransform t = a * b;
        pf->setTransform(t);

        TilingMouseAction::updateDragging(spt);

        emit tm->sig_refreshMenu();
    }
    else
        qDebug() << "MovePolygon: no selection";
}

/////////
///
///  CopyMovePolygon
///
/////////

CopyMovePolygon::CopyMovePolygon(TilingMaker * tilingMaker, TilingSelectorPtr sel, QPointF spt )
    : MovePolygon(tilingMaker, sel = tilingMaker->addTileSelectionPointer(sel),spt)
{
    PlacedTilePtr pfp = sel->getPlacedTile();
    initial_transform = pfp->getTransform();
    desc = "CopyMovePolygon";
    qDebug() << desc;
}

void CopyMovePolygon::endDragging(QPointF spt )
{
    QPointF initial_pos = tm->worldToScreen(initial_transform.map(Point::ORIGIN));
    QPointF final_pos   = tm->worldToScreen(selection->getPlacedTile()->getTransform().map(Point::ORIGIN));
    TilingMouseAction::endDragging(spt);
    if (Point::dist2(initial_pos,final_pos ) < 49.0 )
    {
        tm->deleteTile(selection);
        selection.reset();
    }
    tm->setTilingMakerMouseMode(TM_NO_MOUSE_MODE);
}

/////////
///
///  DrawTranslation
///
/////////

DrawTranslation::DrawTranslation(TilingMaker * tilingMaker, TilingSelectorPtr sel, QPointF spt, QPen apen )
    : TilingMouseAction(tilingMaker,sel,spt)
{
    desc = "DrawTranslation";
    qDebug() << desc;
    this->apen = apen;
    apen.setColor(drag_color);
    if (sel)
    {
        state = ADTT_STARTED;
        vector.setP1(sel->getPlacedPoint());
    }
    else
    {
        state = ADTT_NOSTATE;
    }
}

void DrawTranslation::updateDragging(QPointF spt )
{
    if (state == ADTT_STARTED)
    {
        state = ADTT_DRAGGING;
    }
    if (state == ADTT_DRAGGING)
    {
        vector.setP2(tm->screenToWorld(spt));
    }
    TilingMouseAction::updateDragging(spt);
}

void DrawTranslation::draw(GeoGraphics * g2d )
{
    if (state == ADTT_DRAGGING)
    {
        qreal arrow_length = Transform::distFromInvertedZero(g2d->getTransform(),12.0);
        qreal arrow_width  = Transform::distFromInvertedZero(g2d->getTransform(),6.0);
        g2d->drawLine(vector.p1(), vector.p2(),apen);
        g2d->drawArrow(vector.p1(),vector.p2(), arrow_length, arrow_width, drag_color);
    }
}

void DrawTranslation::endDragging(QPointF spt)
{
    if (state == ADTT_DRAGGING)
    {
        TilingSelectorPtr sel = tm->findSelection(spt);
        if (sel)
        {
            vector.setP2(sel->getPlacedPoint());
            tm->addToTranslate(vector);
        }
    }
    state = ADTT_NOSTATE;
    TilingMouseAction::endDragging(spt);
}

/////////
///
///  JoinEdge
///
/////////

JoinEdge::JoinEdge(TilingMaker * tilingMaker, TilingSelectorPtr sel, QPointF spt ) : TilingMouseAction(tilingMaker,sel,spt)
{
    desc = "JoinEdge";
    snapped = false;
    qDebug().noquote() << desc << "start";
}

void JoinEdge::updateDragging(QPointF spt)
{
    if (snapped) return;

    QPointF wpt = tm->screenToWorld(spt);

    if (!snapTo(spt))
    {
        QPointF diff        = wpt - wLastDrag;
        PlacedTilePtr pf = selection->getPlacedTile();
        QTransform T        = pf->getTransform() * QTransform::fromTranslate(diff.x(), diff.y());
        pf->setTransform(T);
        emit tm->sig_refreshMenu();
    }
    TilingMouseAction::updateDragging(spt);
}

void JoinEdge::endDragging(QPointF spt)
{
    qDebug() << "JoinEdge::endDragging";
    if (!snapped)
    {
        snapTo(spt);
    }
    TilingMouseAction::endDragging(spt);
}

bool JoinEdge::snapTo(QPointF spt)
{
    // snap to an edge
    if (!selection)
    {
        qDebug() << "JoinEdge::no snap";
        return false;
    }

    TilingSelectorPtr tosel = tm->findEdge(spt, selection);
    if (!tosel || tosel->getType() != EDGE)
    {
        qDebug() << "JoinEdge::no snap";
        return false;
    }

    qDebug() << "JoinEdge::SNAP" << tosel->getTypeString();

    QLineF pline          = selection->getPlacedLine();
    QLineF qline          = tosel->getPlacedLine();

    PlacedTilePtr from = selection->getPlacedTile();
    QTransform fromT      = from->getTransform();
    QTransform T          = matchTwoSegments(pline.p1(), pline.p2(), qline.p2(), qline.p1());
    QTransform carry      = fromT * T;
    from->setTransform(carry);

    snapped = true;
    emit tm->sig_refreshMenu();
    return true;
}

// Provide the transform matrix to carry the unit interval
// on the positive X axis to the line segment from p to q.
QTransform JoinEdge::matchLineSegment(QPointF p, QPointF q)
{
    QTransform m(q.x() - p.x(), q.y() - p.y() , p.y() - q.y(), q.x() - p.x(), p.x(), p.y());
    return  m;
}

// get the transform that carries p1->q1 to p2->q2.
QTransform JoinEdge::matchTwoSegments(QPointF p1, QPointF q1, QPointF p2, QPointF q2)
{
    QTransform to_p1q1 = matchLineSegment(p1,q1);
    QTransform to_p2q2 = matchLineSegment(p2,q2);
    QTransform T1 = to_p1q1.inverted();
    QTransform T2 = T1 * to_p2q2;
    return T2;
}

/////////
///
///  JoinMidPoint
///
/////////

JoinMidPoint::JoinMidPoint(TilingMaker * tilingMaker, TilingSelectorPtr sel, QPointF spt )
    :JoinEdge(tilingMaker,sel,spt)
{
    desc = "JoinMidPoint";
    snapped = false;
    qDebug().noquote() << desc << "start";
}

bool JoinMidPoint::snapTo(QPointF spt)
{
    // snap to a point
    if (!selection)
    {
        qDebug() << "JoinMidPoint:: no snap - no selection";
        return false;
    }

    TilingSelectorPtr tosel = tm->findMidPoint(spt, selection);
    if (!tosel)
    {
        qDebug() << "JoinMidPoint:: no snap - no tosel";
        return false;
    }

    // this is a snap
    qDebug().noquote() << "JoinMidPoint::SNAP to" << tosel->getTypeString();

    PlacedTilePtr from  = selection->getPlacedTile();
    QTransform       fromT = from->getTransform();
    qreal initialRotation  = qRadiansToDegrees(Transform::rotation(fromT));
    qDebug() << "initial rotation" << initialRotation;

#if 1
    qreal l1 = selection->getModelLine().angle();
    if (l1 > 180.0) l1 -=180.0;
    Q_ASSERT(l1 >= 0.0);
    qreal l2 = tosel->getModelLine().angle();
    if (l2 > 180.0) l2 -=180.0;
    Q_ASSERT(l2 >= 0.0);
    qreal angle    = l1-l2;

    if (!Loose::zero(angle))
    {
        qInfo() << "angle =" << angle;
        if (angle < 0)
        {
            angle += 360;
            qInfo() << "angle2=" << angle;
            //if (angle > 180)
            //{
            //    angle -= 180.0;
            //    qInfo() << "angle3=" << angle;
            //}
        }
        QTransform t0;
        t0 = t0.rotate(angle);
        fromT *= t0;
        from->setTransform(fromT);
    }
#endif

#if 1
    QPointF fromP   = selection->getPlacedPoint();
    QPointF toP     = tosel->getPlacedPoint();
    QPointF diff    = toP - fromP;
    QTransform t1;
    //if (angle <= 0)
        t1   = QTransform::fromTranslate(diff.x(),diff.y());
    //else
    //  t1   = QTransform::fromTranslate(-diff.x(),-diff.y());

    fromT *= t1;
    from->setTransform(fromT);
#endif

    snapped = true;

    emit tm->sig_refreshMenu();

    return true;
}

/////////
///
///  Join Point
///
/////////

JoinPoint::JoinPoint(TilingMaker * tilingMaker, TilingSelectorPtr sel, QPointF spt )
    : JoinEdge(tilingMaker,sel,spt)
{
    desc = "JoinPoint";
    snapped = false;
    qDebug().noquote() << desc << "start";
}

bool JoinPoint::snapTo(QPointF spt)
{
    // snap to a point
    if (!selection)
    {
        qDebug() << "JoinPoint:: no snap - no selection";
    }

    TilingSelectorPtr tosel = tm->findPoint(spt, selection);
    if (!tosel)
    {
        qDebug() << "JoinPoint:: no snap - no tosel";
        return false;
    }

    qDebug() << "JoinPoint::SNAP - type =" << tosel->getTypeString();

    PlacedTilePtr from = selection->getPlacedTile();

    QPointF fromP         = selection->getPlacedPoint();
    QPointF toP           = tosel->getPlacedPoint();
    QPointF diff          = toP - fromP;

    QTransform t = from->getTransform() * QTransform::fromTranslate(diff.x(),diff.y());
    from->setTransform(t);

    snapped = true;
    emit tm->sig_refreshMenu();
    return true;
}

/////////
///
///  CopyJoinEdge
///
/////////

CopyJoinEdge::CopyJoinEdge(TilingMaker * tilingMaker, TilingSelectorPtr sel, QPointF spt )
    : JoinEdge(tilingMaker,tilingMaker->addTileSelectionPointer(sel),spt)
{
    initial_transform = sel->getPlacedTile()->getTransform();
    desc = "CopyJoinEdge";
    qDebug().noquote() << desc << "start";
}

void CopyJoinEdge::endDragging(QPointF spt)
{
    QPointF initial_pos = tm->worldToScreen(initial_transform.map(Point::ORIGIN));
    QPointF final_pos   = tm->worldToScreen(selection->getPlacedTile()->getTransform().map(Point::ORIGIN));
    JoinEdge::endDragging(spt);
    if (Point::dist2(initial_pos,final_pos) < 49.0)
    {
        tm->deleteTile(selection);
        selection.reset();
    }
}

/////////
///
///  CopyJoinMidPoint
///
/////////

CopyJoinMidPoint::CopyJoinMidPoint(TilingMaker * tilingMaker, TilingSelectorPtr sel, QPointF spt )
    : JoinMidPoint(tilingMaker,tilingMaker->addTileSelectionPointer(sel),spt)
{
    initial_transform = sel->getPlacedTile()->getTransform();
    desc = "CopyJoinMidPoint";
    qDebug().noquote() << desc << "start";
}

void CopyJoinMidPoint::endDragging(QPointF spt)
{
    QPointF initial_pos = tm->worldToScreen(initial_transform.map(QPointF()));
    QPointF final_pos   = tm->worldToScreen(selection->getPlacedTile()->getTransform().map(QPointF()));
    JoinMidPoint::endDragging(spt);
    if (Point::dist2(initial_pos,final_pos) < 49.0)
    {
        tm->deleteTile(selection);
        selection.reset();
    }
}

/////////
///
///  CopyJoinPoint
///
/////////

CopyJoinPoint::CopyJoinPoint(TilingMaker * tilingMaker, TilingSelectorPtr sel, QPointF spt )
    : JoinPoint(tilingMaker,tilingMaker->addTileSelectionPointer(sel),spt)
{
    initial_transform = sel->getPlacedTile()->getTransform();
    desc = "CopyJoinPoint";
    qDebug().noquote() << desc << "start";
}

void CopyJoinPoint::endDragging(QPointF spt)
{
    QPointF initial_pos = tm->worldToScreen(initial_transform.map(QPointF()));
    QPointF final_pos   = tm->worldToScreen(selection->getPlacedTile()->getTransform().map(QPointF()));
    if (Point::dist2(initial_pos,final_pos) < 49.0)
    {
        tm->deleteTile(selection);
        emit tm->sig_buildMenu();
    }
    JoinPoint::endDragging(spt);
}

/////////
///
///  CreatePolygon
///
/////////

CreatePolygon::CreatePolygon(TilingMaker * tilingMaker, QPointF spt )
    : TilingMouseAction(tilingMaker,nullptr,spt)
{
    qDebug() << "CreatePolygon";
    qDebug() << desc;
    grid = Grid::getSharedInstance();
    QPointF wpt = tilingMaker->findSelectionPointOrPoint(spt);
    grid->nearGridPoint(spt,wpt);
    addVertex(wpt);
    desc = "CreatePolygon";
}

void CreatePolygon::addVertex(QPointF wpt)
{
    qDebug("CreatePolygon::addVertex");

    EdgePoly & wAccum = tm->getAccumW();
    if (wAccum.size() == 0)
    {
        // add first point
        VertexPtr vnew = make_shared<Vertex>(wpt);
        wAccum.push_back(make_shared<Edge>(vnew));
        qDebug() << "point count = 1";
        tm->forceRedraw();
        return;
    }

    QPointF newPoint   = tm->worldToScreen(wpt);
    VertexPtr firstV   = wAccum.first()->v1;
    QPointF firstPoint = tm->worldToScreen(firstV->pt);
    if ((wAccum.size() > 2) &&  Point::isNear(newPoint,firstPoint))
    {
        // add last point
        qDebug("auto-complete the polygon");
        VertexPtr vnew = make_shared<Vertex>(wpt);
        EdgePtr last = wAccum.last();
        Q_ASSERT(last->getType() == EDGETYPE_POINT);
        last->setV2(firstV);

        auto tiling = tm->getSelected();
        QTransform t;
        tm->addNewPlacedTile(make_shared<PlacedTile>(tiling.get(),make_shared<Tile>(wAccum,0), t));
        tm->setTilingMakerMouseMode(TM_NO_MOUSE_MODE);
        emit tm->sig_buildMenu();
        return;
    }

    if (!tm->accumHasPoint(wpt))
    {
        // is a new point
        VertexPtr vnew = make_shared<Vertex>(wpt);
        EdgePtr last = wAccum.last();
        Q_ASSERT(last->getType() == EDGETYPE_POINT);
        last->setV2(vnew);

        // start of next edge
        wAccum.push_back(make_shared<Edge>(vnew));

        qDebug() << "edge count =" << wAccum.size();
    }
    else
    {
        qDebug() << "addVertex: ignored (exisiting point)";
    }
}

void CreatePolygon::updateDragging(QPointF spt)
{
    TilingMouseAction::updateDragging(spt);

    underneath = QPointF();

    EdgePoly & wAccum = tm->getAccumW();
    for (auto & edge : wAccum)
    {
        QPointF p1 = tm->worldToScreen(edge->v1->pt);
        QPointF p2 = tm->worldToScreen(edge->v2->pt);
        if (Point::isNear(p1,spt))
        {
            underneath = edge->v1->pt;
            return;
        }
        else if (Point::isNear(p2,spt))
        {
            underneath = edge->v2->pt;
            return;
        }
        else if (grid->nearGridPoint(spt,underneath))
        {
            return;
        }
    }
}

void CreatePolygon::endDragging(QPointF spt )
{
    qDebug("CreatePolygon::endDragging");
    EdgePoly & wAccum = tm->getAccumW();
    if (wAccum.size() != 0)
    {
        QPointF wpt = tm->findSelectionPointOrPoint(spt);
        grid->nearGridPoint(spt,wpt);

        addVertex(wpt);
    }
    TilingMouseAction::endDragging(spt);
}

void CreatePolygon::draw(GeoGraphics * g2d)
{
    EdgePoly & wAccum = tm->getAccumW();
    if (wAccum.size() > 0)
    {
        if (!wLastDrag.isNull())
        {
            QPen pen(drag_color,3);
            g2d->drawLine(wAccum.last()->v2->pt,wLastDrag,pen);
            g2d->drawCircle(wLastDrag,6,pen,QBrush(drag_color));
        }
        if (!underneath.isNull())
        {
            g2d->drawCircle(underneath,6,QPen(Qt::red,3),QBrush(Qt::red));
        }
    }
}


/////////
///
///  Measure
///
/////////

Measure::Measure(TilingMaker * tilingMaker, QPointF spt, TilingSelectorPtr sel) : TilingMouseAction(tilingMaker,sel,spt)
{
    m = new Measurement(tilingMaker);
    desc = "Measure";
    qDebug() << desc;

    Qt::KeyboardModifiers kms =  QApplication::keyboardModifiers();
    if (sel && (kms == (Qt::CTRL | Qt::SHIFT)))
    {
        qreal len = QLineF(m->startS(),spt).length();    // approx len
        QLineF line = sel->getPlacedLine();
        sPerpLine = QLineF(spt, tilingMaker->worldToScreen(line.p2()));
        sPerpLine = normalVectorB(sPerpLine);
        sPerpLine.setLength(len);

        m->setStart(spt);
        m->setEnd(sPerpLine.p2());
    }
    else
    {
        m->setStart(spt);
        m->setEnd(spt);
    }
}

void Measure::updateDragging(QPointF spt)
{
    if (spt != m->startS())
    {
        Qt::KeyboardModifiers kms =  QApplication::keyboardModifiers();
        if (selection && (kms == (Qt::CTRL | Qt::SHIFT)))
        {
            qreal len = QLineF(m->startS(),spt).length();    // approx len
            sPerpLine.setLength(len);
            m->setEnd(sPerpLine.p2());
        }
        else
        {
            m->setEnd(spt);
        }
        m->active = true;
        tm->forceRedraw();
    }
}

void Measure::draw(GeoGraphics * g2d)
{
    if (m->active)
    {
        g2d->drawLineDirect(m->startS(),m->endS(),QPen(drag_color));
        QString msg = QString("%1 (%2)").arg(QString::number(m->lenS(),'f',2),QString::number(m->lenW(),'f',8));
        g2d->drawText(m->endS() + QPointF(10,0),msg);
    }
}

void Measure::endDragging(QPointF spt)
{
    if (m->active)
    {
        Qt::KeyboardModifiers kms =  QApplication::keyboardModifiers();
        if (selection && (kms == (Qt::CTRL | Qt::SHIFT)))
        {
            qreal len = QLineF(m->startS(),spt).length();    // approx len
            sPerpLine.setLength(len);
            m->setEnd(sPerpLine.p2());
        }
        else
        {
            m->setEnd(spt);
        }
        tm->getMeasurementsS().push_back(m);
        //m->reset();
    }
    TilingMouseAction::endDragging(spt);
}

QLineF Measure::normalVectorA(QLineF line)
{
    return QLineF(line.p1(), line.p1() + QPointF(line.dy(), -line.dx()));
}

QLineF Measure::normalVectorB(QLineF line)
{
    return QLineF(line.p1(), line.p1() - QPointF(line.dy(), -line.dx()));
}

/////////
///
///  Position
///
/////////

Position::Position(TilingMaker * tilingMaker, QPointF spt) : TilingMouseAction(tilingMaker,nullptr,spt)
{
    desc = "Position";
    qDebug() << desc;
    this->spt = spt;
    tilingMaker->forceRedraw();
}

void Position::updateDragging(QPointF spt)
{
    qDebug() << "spt=" << spt;
    this->spt = spt;
    tm->forceRedraw();
}

void Position::draw(GeoGraphics * g2d)
{
    qreal sx    = spt.x();
    qreal sy    = spt.y();
    QPointF mpt = tm->screenToWorld(spt);
    qreal mx    = mpt.x();
    qreal my    = mpt.y();

    QString msg = QString("(%1,%2)(%3,%4)").arg(QString::number(sx,'f',2),
                                                QString::number(sy,'f',2),
                                                QString::number(mx,'f',8),
                                                QString::number(my,'f',8));
    g2d->drawCircle(mpt,3,QPen(Qt::magenta),QBrush(Qt::magenta));
    g2d->drawText(spt + QPointF(10,0),msg);
 }


/////////
///
///  Edit EditTile
///
/////////

EditTile::EditTile(TilingMaker * tilingMaker, TilingSelectorPtr sel, PlacedTilePtr pfp, QPointF spt )
    : TilingMouseAction(tilingMaker,sel,spt)
{
    desc        = "EditTile";
    this->pfp   = pfp;
    vertexIndex = -1;
    Q_ASSERT(sel->getType() == VERTEX);
    qDebug() << desc;

    QPointF v   = sel->getPlacedPoint();

    QPolygonF poly = pfp->getPlacedPolygon();
    for (int i=0;  i < poly.size(); i++)
    {
        QPointF v2 = poly[i];
        if (v == v2)
        {
            vertexIndex = i;
            qDebug() << "vertexIndex"  << vertexIndex;
            break;
        }
    }

    Q_ASSERT(vertexIndex != -1);
}

void EditTile::updateDragging(QPointF spt)
{
    QPointF wpt = tm->screenToWorld(spt);
    QTransform T = pfp->getTransform().inverted();
    wpt = T.map(wpt);
    const EdgePoly & ep = pfp->getTile()->getEdgePoly();
    VertexPtr v = ep[vertexIndex]->v1;
    v->pt = wpt;

    TilingMouseAction::updateDragging(tm->screenToWorld(spt));
    emit tm->sig_refreshMenu();
}

void EditTile::endDragging(QPointF spt )
{
    QPointF wpt = tm->screenToWorld(spt);
    QTransform T = pfp->getTransform().inverted();
    wpt = T.map(wpt);
    const EdgePoly & ep = pfp->getTile()->getEdgePoly();
    VertexPtr v = ep[vertexIndex]->v1;
    v->pt = wpt;

    TilingMouseAction::endDragging(spt);
    emit tm->sig_refreshMenu();
}

/////////
///
///  Edit Edge
///
/////////

EditEdge::EditEdge(TilingMaker * tilingMaker, TilingSelectorPtr sel, QPointF spt)
    : TilingMouseAction(tilingMaker,sel,spt)
{
    desc  = "EditEdge";
    qDebug() << desc;

    Q_ASSERT(selection->getType() == ARC_POINT);

    edge  = sel->getModelEdge();
    start = edge->getArcCenter();
    pfp   = sel->getPlacedTile();
    QTransform T = pfp->getTransform();

    // draw perpendicular line to edge
    QLineF line = edge->getLine();
    QPointF mid = edge->getMidPoint();
    // half-line
    line.setP1(mid);
    line = T.map(line);

    //QLineF perp = Utils::normalVectorP1(line);
    QLineF perp0 = line.normalVector();
    qreal len    = perp0.length();
    perp0.setLength(len * 5);
    perp = QLineF(perp0.p2(),perp0.p1());  // swap ends
    perp.setLength(len*10);
}

void EditEdge::draw(GeoGraphics * g2d)
{
    g2d->drawLine(perp,QPen(Qt::red));
}

void EditEdge::updateDragging(QPointF spt)
{
    QPointF wpt    = tm->screenToWorld(spt);
    QPolygonF poly = pfp->getPlacedPolygon();
    QPointF pt     = Utils::getClosestPoint(perp,wpt);
    QTransform inv = pfp->getTransform().inverted();
    pt             = inv.map(pt);
    bool convex    = edge->isConvex();
    edge->setArcCenter(pt, convex,(edge->getType()==EDGETYPE_CHORD));
    TilingMouseAction::updateDragging(tm->screenToWorld(spt));
    emit tm->sig_refreshMenu();
}

void EditEdge::endDragging(QPointF spt )
{
    TilingMouseAction::endDragging(spt);
}

/////////////////////////////////////////
//
//  TilingConstructionLine
//
////////////////////////////////////////

TilingConstructionLine::TilingConstructionLine(TilingMaker * tilingMaker, TilingSelectorPtr sel, QPointF spt) : TilingMouseAction(tilingMaker,sel,spt)
{
    desc = "ConstructionLine";
    qDebug() << desc;
    start = nullptr;
    end   = nullptr;

    start = new QPointF(tilingMaker->findSelectionPointOrPoint(spt));
}

TilingConstructionLine::~TilingConstructionLine()
{
    if (start) delete start;
    if (end)   delete end;
}

void TilingConstructionLine::updateDragging(QPointF spt)
{
    if (end)
    {
        delete end;
    }


    end = new QPointF(tm->getLayerTransform().inverted().map(spt));

    TilingMouseAction::updateDragging(spt);
}

void TilingConstructionLine::endDragging( QPointF spt)
{
    qDebug() << "spt start"  << spt;
    Qt::KeyboardModifiers kms =  QApplication::keyboardModifiers();
    if (kms == Qt::SHIFT)
    {
        QTransform t = tm->getLayerTransform();
        QPointF s = t.map(*start);
        spt = QPointF(spt.x(),s.y());
    }
    else if (kms == Qt::CTRL)
    {
        QTransform t = tm->getLayerTransform();
        QPointF s = t.map(*start);
        spt = QPointF(s.x(),spt.y());
    }

    qDebug() << "spt end"  << spt;
    end = new QPointF(tm->findSelectionPointOrPoint(spt));

    tm->getConstructionLines().push_back(QLineF(*start,*end));
    TilingMouseAction::endDragging(spt);
}

void TilingConstructionLine::draw(GeoGraphics * gg)
{
    qreal radius = 3.0;
    QPen apen(Qt::green,3);
    QBrush abrush(Qt::green);

    if (start)
    {
        gg->drawCircle(*start, radius, apen, abrush);
    }
    if (end)
    {
        gg->drawCircle(*end, radius, apen, abrush);
    }
    if (start && end)
    {
        //qDebug() << "TilingConstructionLine::draw" << *start << *end;
        gg->drawLine(*start, *end, apen);
    }
}
