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
#include "viewers/grid_view.h"
#include "viewers/view.h"
#include "viewers/viewcontrol.h"

using std::make_shared;

////////////////////////////////////////////////////////////////////////////
//
// Mouse Actions
//
////////////////////////////////////////////////////////////////////////////

TilingMouseAction::TilingMouseAction(TileSelectorPtr sel, QPointF spt)
{
    desc       = "MouseAction";
    qDebug() << desc;

    tilingMakerView = TilingMakerView::getInstance();
    tilingMaker     = TilingMaker::getInstance();
    selection       = sel;
    wLastDrag       = tilingMakerView->screenToWorld(spt);
    drag_color      = QColor(206,179,102,230);

    tilingMakerView->forceRedraw();
}

void TilingMouseAction::updateDragging(QPointF spt)
{
    wLastDrag = tilingMakerView->screenToWorld(spt);
    tilingMakerView->forceRedraw();
}

void TilingMouseAction::draw(GeoGraphics * g2d)
{
    Q_UNUSED(g2d)
}

void TilingMouseAction::endDragging(QPointF spt)
{
    Q_UNUSED(spt)
    tilingMaker->resetOverlaps();       // sets UNDEFINED
    tilingMakerView->forceRedraw();
}

void TilingMouseAction::flash(QColor color)
{
    auto view = ViewControl::getInstance();

    view->setViewBackgroundColor(color);
    view->repaint();

    QTime dieTime= QTime::currentTime().addMSecs(350);
    while (QTime::currentTime() < dieTime)
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }

    view->setViewBackgroundColor(view->getViewSettings().getBkgdColor(VIEW_TILING_MAKER));
    view->repaint();
}

/////////
///
///  MovePolygon
///
/////////

MovePolygon::MovePolygon(TileSelectorPtr sel, QPointF spt) : TilingMouseAction(sel,spt)
{
    desc = "MovePolygon";
    qDebug() << desc;
}

void MovePolygon::updateDragging(QPointF spt)
{
    if  (selection && selection->getType() == INTERIOR)
    {
        //qDebug() << "MovePolygon: update";

        QPointF wpt         = tilingMakerView->screenToWorld(spt);
        PlacedTilePtr pf = selection->getPlacedTile();
        QPointF diff        = wpt - wLastDrag;

        QTransform a = pf->getTransform();
        QTransform b = QTransform::fromTranslate(diff.x(),diff.y());
        QTransform t = a * b;
        pf->setTransform(t);

        TilingMouseAction::updateDragging(spt);

        emit tilingMaker->sig_refreshMenu();
    }
    //else
        //qDebug() << "MovePolygon: no selection";
}

/////////
///
///  CopyMovePolygon
///
/////////

CopyMovePolygon::CopyMovePolygon(TileSelectorPtr sel, QPointF spt )
    : MovePolygon(sel = TilingMaker::getInstance()->addTileSelectionPointer(sel),spt)
{
    PlacedTilePtr pfp = sel->getPlacedTile();
    initial_transform = pfp->getTransform();
    desc = "CopyMovePolygon";
    qDebug() << desc;
}

void CopyMovePolygon::endDragging(QPointF spt )
{
    QPointF initial_pos = tilingMakerView->worldToScreen(initial_transform.map(Point::ORIGIN));
    QPointF final_pos   = tilingMakerView->worldToScreen(selection->getPlacedTile()->getTransform().map(Point::ORIGIN));
    TilingMouseAction::endDragging(spt);
    if (Point::dist2(initial_pos,final_pos ) < 49.0 )
    {
        tilingMaker->deleteTile(selection);
        selection.reset();
    }
    tilingMaker->setTilingMakerMouseMode(TM_NO_MOUSE_MODE);
}

/////////
///
///  DrawTranslation
///
/////////

DrawTranslation::DrawTranslation(TileSelectorPtr sel, QPointF spt, QPen apen ) : TilingMouseAction(sel,spt)
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
        vector.setP2(tilingMakerView->screenToWorld(spt));
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
        TileSelectorPtr sel = tilingMakerView->findSelection(spt);
        if (sel)
        {
            vector.setP2(sel->getPlacedPoint());
            tilingMaker->addToTranslate(vector);
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

JoinEdge::JoinEdge(TileSelectorPtr sel, QPointF spt) : TilingMouseAction(sel,spt)
{
    desc = "JoinEdge";
    snapped = false;
    qDebug().noquote() << desc << "start";
}

void JoinEdge::updateDragging(QPointF spt)
{
    if (snapped) return;

    QPointF wpt = tilingMakerView->screenToWorld(spt);

    if (!snapTo(spt))
    {
        QPointF diff        = wpt - wLastDrag;
        PlacedTilePtr pf = selection->getPlacedTile();
        QTransform T        = pf->getTransform() * QTransform::fromTranslate(diff.x(), diff.y());
        pf->setTransform(T);
        emit tilingMaker->sig_refreshMenu();
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
        //qDebug() << "JoinEdge::no snap";
        return false;
    }
    
    TileSelectorPtr tosel = tilingMakerView->findEdge(spt, selection);
    if (!tosel || tosel->getType() != EDGE)
    {
        //qDebug() << "JoinEdge::no snap";
        return false;
    }

    snapped = true;

    qDebug().noquote() << "JoinEdge::SNAP" << tosel->getTypeString();
    flash(QColor(0,0,255,100));

    QLineF pline        = selection->getPlacedLine();
    QLineF qline        = tosel->getPlacedLine();

    PlacedTilePtr from  = selection->getPlacedTile();
    QTransform fromT    = from->getTransform();

    QTransform T        = matchTwoSegments(pline.p1(), pline.p2(), qline.p2(), qline.p1());
    QTransform carry    = fromT * T;
    from->setTransform(carry);

    emit tilingMaker->sig_refreshMenu();
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
    QTransform to_p1q1  = matchLineSegment(p1,q1);
    QTransform to_p2q2  = matchLineSegment(p2,q2);
    QTransform T1       = to_p1q1.inverted();
    QTransform T2       = T1 * to_p2q2;
    return T2;
}

/////////
///
///  JoinMidPoint
///
/////////

JoinMidPoint::JoinMidPoint(TileSelectorPtr sel, QPointF spt ) : JoinEdge(sel,spt)
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
        //qDebug() << "JoinMidPoint:: no snap - no selection";
        return false;
    }
    
    TileSelectorPtr tosel = tilingMakerView->findMidPoint(spt, selection);
    if (!tosel)
    {
        //qDebug() << "JoinMidPoint:: no snap - no tosel";
        return false;
    }

    // this is a snap
    snapped = true;

    qDebug().noquote() << "JoinMidPoint::SNAP to" << tosel->getTypeString();
    flash(QColor(0,255,0,100));

    PlacedTilePtr    fromTile  = selection->getPlacedTile();
    QTransform       fromTrans = fromTile->getTransform();

    // the approach is to a) rotate until edges are parallel and then 2) move until centers are coincident
    // this does not resize the fromsel
    auto fromEdge   = selection->getPlacedEdge();
    auto toEdge     = tosel->getPlacedEdge();
    qreal fromAngle = qRadiansToDegrees(fromEdge->angle());
    qreal toAngle   = qRadiansToDegrees(toEdge->angle()) - 180.0;    // to and from are in opposite directions by definition
    qreal angle     = toAngle - fromAngle;

    //qDebug() << "from angle=" << fromAngle << "toAngle" << toAngle << "angle" << angle;

    if (!Loose::zero(angle))
    {
        // rotate the from placed tile
        //qDebug() << "rotating" << angle;
        QTransform t0;
        t0.rotate(angle);
        fromTrans *= t0;
        fromTile->setTransform(fromTrans);
    }

    // now move the placed Tile
    fromEdge      = selection->getPlacedEdge();  // it could have moved
    QPointF delta = toEdge->getLine().center() - fromEdge->getLine().center();
    //qDebug() << "delta" << delta;
    fromTrans    *= QTransform::fromTranslate(delta.x(),delta.y());
    fromTile->setTransform(fromTrans);

    emit tilingMaker->sig_refreshMenu();

    return true;
}

/////////
///
///  Join Point
///
/////////

JoinPoint::JoinPoint(TileSelectorPtr sel, QPointF spt ) : JoinEdge(sel,spt)
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
    
    TileSelectorPtr tosel = tilingMakerView->findPoint(spt, selection);
    if (!tosel)
    {
        qDebug() << "JoinPoint:: no snap - no tosel";
        return false;
    }

    snapped = true;

    qDebug() << "JoinPoint::SNAP - type =" << tosel->getTypeString();
    flash(QColor(255,0,0,100));

    PlacedTilePtr from = selection->getPlacedTile();

    QPointF fromP         = selection->getPlacedPoint();
    QPointF toP           = tosel->getPlacedPoint();
    QPointF diff          = toP - fromP;

    QTransform t = from->getTransform() * QTransform::fromTranslate(diff.x(),diff.y());
    from->setTransform(t);

    emit tilingMaker->sig_refreshMenu();
    return true;
}

/////////
///
///  CopyJoinEdge
///
/////////

CopyJoinEdge::CopyJoinEdge(TileSelectorPtr sel, QPointF spt )
    : JoinEdge(TilingMaker::getInstance()->addTileSelectionPointer(sel),spt)
{
    initial_transform = sel->getPlacedTile()->getTransform();
    desc = "CopyJoinEdge";
    qDebug().noquote() << desc << "start";
}

void CopyJoinEdge::endDragging(QPointF spt)
{
    QPointF initial_pos = tilingMakerView->worldToScreen(initial_transform.map(Point::ORIGIN));
    QPointF final_pos   = tilingMakerView->worldToScreen(selection->getPlacedTile()->getTransform().map(Point::ORIGIN));
    JoinEdge::endDragging(spt);
    if (Point::dist2(initial_pos,final_pos) < 49.0)
    {
        tilingMaker->deleteTile(selection);
        selection.reset();
    }
}

/////////
///
///  CopyJoinMidPoint
///
/////////

CopyJoinMidPoint::CopyJoinMidPoint(TileSelectorPtr sel, QPointF spt)
    : JoinMidPoint(TilingMaker::getInstance()->addTileSelectionPointer(sel),spt)
{
    initial_transform = sel->getPlacedTile()->getTransform();
    desc = "CopyJoinMidPoint";
    qDebug().noquote() << desc << "start";
}

void CopyJoinMidPoint::endDragging(QPointF spt)
{
    QPointF initial_pos = tilingMakerView->worldToScreen(initial_transform.map(QPointF()));
    QPointF final_pos   = tilingMakerView->worldToScreen(selection->getPlacedTile()->getTransform().map(QPointF()));
    JoinMidPoint::endDragging(spt);
    if (Point::dist2(initial_pos,final_pos) < 49.0)
    {
        tilingMaker->deleteTile(selection);
        selection.reset();
    }
}

/////////
///
///  CopyJoinPoint
///
/////////

CopyJoinPoint::CopyJoinPoint(TileSelectorPtr sel, QPointF spt)
    : JoinPoint(TilingMaker::getInstance()->addTileSelectionPointer(sel),spt)
{
    initial_transform = sel->getPlacedTile()->getTransform();
    desc = "CopyJoinPoint";
    qDebug().noquote() << desc << "start";
}

void CopyJoinPoint::endDragging(QPointF spt)
{
    QPointF initial_pos = tilingMakerView->worldToScreen(initial_transform.map(QPointF()));
    QPointF final_pos   = tilingMakerView->worldToScreen(selection->getPlacedTile()->getTransform().map(QPointF()));
    if (Point::dist2(initial_pos,final_pos) < 49.0)
    {
        tilingMaker->deleteTile(selection);
        emit tilingMaker->sig_buildMenu();
    }
    JoinPoint::endDragging(spt);
}

/////////
///
///  CreatePolygon
///
/////////

CreatePolygon::CreatePolygon(QPointF spt) : TilingMouseAction(nullptr,spt)
{
    qDebug() << "CreatePolygon";
    qDebug() << desc;
    gridView = GridView::getInstance();
    QPointF wpt = tilingMakerView->findSelectionPointOrPoint(spt);
    gridView->nearGridPoint(spt,wpt);
    addVertex(wpt);
    desc = "CreatePolygon";
}

void CreatePolygon::addVertex(QPointF wpt)
{
    qDebug("CreatePolygon::addVertex");

    EdgePoly & wAccum = tilingMakerView->getAccumW();
    if (wAccum.size() == 0)
    {
        // add first point
        VertexPtr vnew = make_shared<Vertex>(wpt);
        wAccum.push_back(make_shared<Edge>(vnew));
        qDebug() << "point count = 1";
        tilingMakerView->forceRedraw();
        return;
    }

    QPointF newPoint   = tilingMakerView->worldToScreen(wpt);
    VertexPtr firstV   = wAccum.first()->v1;
    QPointF firstPoint = tilingMakerView->worldToScreen(firstV->pt);
    if ((wAccum.size() > 2) &&  Point::isNear(newPoint,firstPoint))
    {
        // add last point
        qDebug("auto-complete the polygon");
        VertexPtr vnew = make_shared<Vertex>(wpt);
        EdgePtr last = wAccum.last();
        Q_ASSERT(last->getType() == EDGETYPE_POINT);
        last->setV2(firstV);

        auto tiling = tilingMaker->getSelected();
        QTransform t;
        tilingMaker->addNewPlacedTile(make_shared<PlacedTile>(make_shared<Tile>(wAccum,0), t));
        tilingMaker->setTilingMakerMouseMode(TM_NO_MOUSE_MODE);
        emit tilingMaker->sig_buildMenu();
        return;
    }

    if (!tilingMakerView->accumHasPoint(wpt))
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

    EdgePoly & wAccum = tilingMakerView->getAccumW();
    for (auto & edge : wAccum)
    {
        QPointF p1 = tilingMakerView->worldToScreen(edge->v1->pt);
        QPointF p2 = tilingMakerView->worldToScreen(edge->v2->pt);
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
        else if (gridView->nearGridPoint(spt,underneath))
        {
            return;
        }
    }
}

void CreatePolygon::endDragging(QPointF spt )
{
    qDebug("CreatePolygon::endDragging");
    EdgePoly & wAccum = tilingMakerView->getAccumW();
    if (wAccum.size() != 0)
    {
        QPointF wpt = tilingMakerView->findSelectionPointOrPoint(spt);
        gridView->nearGridPoint(spt,wpt);

        addVertex(wpt);
    }
    TilingMouseAction::endDragging(spt);
}

void CreatePolygon::draw(GeoGraphics * g2d)
{
    EdgePoly & wAccum = tilingMakerView->getAccumW();
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

Measure::Measure(QPointF spt, TileSelectorPtr sel) : TilingMouseAction(sel,spt)
{
    m = new Measurement(tilingMakerView);
    desc = "Measure";
    qDebug() << desc;

    Qt::KeyboardModifiers kms =  QApplication::keyboardModifiers();
    if (sel && (kms == (Qt::CTRL | Qt::SHIFT)))
    {
        qreal len = QLineF(m->startS(),spt).length();    // approx len
        QLineF line = sel->getPlacedLine();
        sPerpLine = QLineF(spt, tilingMakerView->worldToScreen(line.p2()));
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
        tilingMakerView->forceRedraw();
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
        tilingMakerView->getMeasurementsS().push_back(m);
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

Position::Position(QPointF spt) : TilingMouseAction(nullptr,spt)
{
    desc = "Position";
    qDebug() << desc;
    this->spt = spt;
    tilingMakerView->forceRedraw();
}

void Position::updateDragging(QPointF spt)
{
    qDebug() << "spt=" << spt;
    this->spt = spt;
    tilingMakerView->forceRedraw();
}

void Position::draw(GeoGraphics * g2d)
{
    qreal sx    = spt.x();
    qreal sy    = spt.y();
    QPointF mpt = tilingMakerView->screenToWorld(spt);
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
 
 EditTile::EditTile(TileSelectorPtr sel, PlacedTilePtr pfp, QPointF spt )
    : TilingMouseAction(sel,spt)
{
    desc        = "EditTile";
    this->pfp   = pfp;
    vertexIndex = -1;
    Q_ASSERT(sel->getType() == VERTEX);
    qDebug() << desc;

    QPointF v   = sel->getPlacedPoint();

    QPolygonF poly = pfp->getPlacedPoints();
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
    QPointF wpt = tilingMakerView->screenToWorld(spt);
    QTransform T = pfp->getTransform().inverted();
    wpt = T.map(wpt);
    const EdgePoly & ep = pfp->getTile()->getEdgePoly();
    VertexPtr v = ep[vertexIndex]->v1;
    v->pt = wpt;

    TilingMouseAction::updateDragging(tilingMakerView->screenToWorld(spt));
    emit tilingMaker->sig_refreshMenu();
}

void EditTile::endDragging(QPointF spt )
{
    QPointF wpt = tilingMakerView->screenToWorld(spt);
    QTransform T = pfp->getTransform().inverted();
    wpt = T.map(wpt);
    const EdgePoly & ep = pfp->getTile()->getEdgePoly();
    VertexPtr v = ep[vertexIndex]->v1;
    v->pt = wpt;

    TilingMouseAction::endDragging(spt);
    emit tilingMaker->sig_refreshMenu();
}

/////////
///
///  Edit Edge
///
/////////

EditEdge::EditEdge(TileSelectorPtr sel, QPointF spt) : TilingMouseAction(sel,spt)
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
    QPointF wpt    = tilingMakerView->screenToWorld(spt);
    QPolygonF poly = pfp->getPlacedPoints();
    QPointF pt     = Utils::getClosestPoint(perp,wpt);
    QTransform inv = pfp->getTransform().inverted();
    pt             = inv.map(pt);
    bool convex    = edge->isConvex();
    edge->setArcCenter(pt, convex,(edge->getType()==EDGETYPE_CHORD));
    TilingMouseAction::updateDragging(tilingMakerView->screenToWorld(spt));
    emit tilingMaker->sig_refreshMenu();
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

TilingConstructionLine::TilingConstructionLine(TileSelectorPtr sel, QPointF spt)
    : TilingMouseAction(sel,spt)
{
    desc = "ConstructionLine";
    qDebug() << desc;
    start = nullptr;
    end   = nullptr;

    start = new QPointF(tilingMakerView->findSelectionPointOrPoint(spt));
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


    end = new QPointF(tilingMakerView->getLayerTransform().inverted().map(spt));

    TilingMouseAction::updateDragging(spt);
}

void TilingConstructionLine::endDragging( QPointF spt)
{
    qDebug() << "spt start"  << spt;
    Qt::KeyboardModifiers kms =  QApplication::keyboardModifiers();
    if (kms == Qt::SHIFT)
    {
        QTransform t = tilingMakerView->getLayerTransform();
        QPointF s = t.map(*start);
        spt = QPointF(spt.x(),s.y());
    }
    else if (kms == Qt::CTRL)
    {
        QTransform t = tilingMakerView->getLayerTransform();
        QPointF s = t.map(*start);
        spt = QPointF(s.x(),spt.y());
    }

    qDebug() << "spt end"  << spt;
    end = new QPointF(tilingMakerView->findSelectionPointOrPoint(spt));

    tilingMakerView->getConstructionLines().push_back(QLineF(*start,*end));
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
