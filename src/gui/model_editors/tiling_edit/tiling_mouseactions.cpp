#include <QDebug>
#include <QApplication>

#include "gui/model_editors/tiling_edit/tiling_mouseactions.h"
#include "model/makers/tiling_maker.h"
#include  "gui/model_editors/tiling_edit/tile_selection.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/transform.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/vertex.h"
#include "gui/viewers/geo_graphics.h"
#include "sys/sys.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "gui/viewers/grid_view.h"
#include "gui/top/view.h"
#include "gui/top/view_controller.h"

using std::make_shared;

////////////////////////////////////////////////////////////////////////////
//
// Mouse Actions
//
////////////////////////////////////////////////////////////////////////////

TilingMouseAction::TilingMouseAction(PlacedTileSelectorPtr sel, QPointF spt)
{
    desc       = "MouseAction";
    qDebug() << desc;

    tilingMakerView = Sys::tilingMakerView;
    tilingMaker     = Sys::tilingMaker;
    selection       = sel;
    wLastDrag       = tilingMakerView->screenToModel(spt);
    drag_color      = QColor(206,179,102,230);

    tilingMakerView->forceRedraw();
}

void TilingMouseAction::updateDragging(QPointF spt)
{
    wLastDrag = tilingMakerView->screenToModel(spt);
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

    tilingMaker->pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
}



/////////
///
///  MovePolygon
///
/////////

MovePolygon::MovePolygon(PlacedTileSelectorPtr sel, QPointF spt) : TilingMouseAction(sel,spt)
{
    desc = "MovePolygon";
    qDebug() << desc;
}

void MovePolygon::updateDragging(QPointF spt)
{
    if  (selection && selection->getType() == INTERIOR)
    {
        //qDebug() << "MovePolygon: update";

        QPointF wpt         = tilingMakerView->screenToModel(spt);
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

CopyMovePolygon::CopyMovePolygon(PlacedTileSelectorPtr sel, QPointF spt )
    : MovePolygon(sel = Sys::tilingMaker->addTileSelectionPointer(sel),spt)
{
    PlacedTilePtr pfp = sel->getPlacedTile();
    initial_transform = pfp->getTransform();
    desc = "CopyMovePolygon";
    qDebug() << desc;
}

void CopyMovePolygon::endDragging(QPointF spt )
{
    QPointF initial_pos = tilingMakerView->modelToScreen(initial_transform.map(Sys::ORIGIN));
    QPointF final_pos   = tilingMakerView->modelToScreen(selection->getPlacedTile()->getTransform().map(Sys::ORIGIN));
    TilingMouseAction::endDragging(spt);
    if (Geo::dist2(initial_pos,final_pos ) < 49.0 )
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

DrawTranslation::DrawTranslation(PlacedTileSelectorPtr sel, QPointF spt, QPen apen ) : TilingMouseAction(sel,spt)
{
    desc = "DrawTranslation";
    qDebug() << desc;
    this->apen = apen;
    apen.setColor(drag_color);
    if (sel)
    {
        state = ADTT_STARTED;
        vector.setP1(sel->getPlacedPoint());
        mOrigin = sel->getPlacedPoint();
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
        vector.setP2(tilingMakerView->screenToModel(spt));
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
        PlacedTileSelectorPtr sel = tilingMakerView->findSelection(spt);
        if (sel)
        {
            vector.setP2(sel->getPlacedPoint());
            tilingMaker->addToTranslate(vector,mOrigin);
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

JoinEdge::JoinEdge(PlacedTileSelectorPtr sel, QPointF spt) : TilingMouseAction(sel,spt)
{
    desc = "JoinEdge";
    snapped = false;
    qDebug().noquote() << desc << "start";
}

void JoinEdge::updateDragging(QPointF spt)
{
    if (snapped) return;

    QPointF wpt = tilingMakerView->screenToModel(spt);

    if (!snapTo(spt))
    {
        QPointF diff        = wpt - wLastDrag;
        if (selection)
        {
            PlacedTilePtr ptp   = selection->getPlacedTile();
            if(ptp)
            {
                QTransform T    = ptp->getTransform() * QTransform::fromTranslate(diff.x(), diff.y());
                ptp->setTransform(T);
                emit tilingMaker->sig_refreshMenu();
            }
        }
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
    
    PlacedTileSelectorPtr tosel = tilingMakerView->findEdge(spt, selection);
    if (!tosel || tosel->getType() != EDGE)
    {
        //qDebug() << "JoinEdge::no snap";
        return false;
    }

    snapped = true;

    qDebug().noquote() << "JoinEdge::SNAP" << tosel->getTypeString();
    Sys::view->flash(QColor(0,0,255,100));

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

JoinMidPoint::JoinMidPoint(PlacedTileSelectorPtr sel, QPointF spt ) : JoinEdge(sel,spt)
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
    
    PlacedTileSelectorPtr tosel = tilingMakerView->findMidPoint(spt, selection);
    if (!tosel)
    {
        //qDebug() << "JoinMidPoint:: no snap - no tosel";
        return false;
    }

    // this is a snap
    snapped = true;

    qDebug().noquote() << "JoinMidPoint::SNAP to" << tosel->getTypeString();
    Sys::view->flash(QColor(0,255,0,100));

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

JoinPoint::JoinPoint(PlacedTileSelectorPtr sel, QPointF spt ) : JoinEdge(sel,spt)
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
    
    PlacedTileSelectorPtr tosel = tilingMakerView->findPoint(spt, selection);
    if (!tosel)
    {
        qDebug() << "JoinPoint:: no snap - no tosel";
        return false;
    }

    snapped = true;

    qDebug().noquote() << "JoinPoint::SNAP - type =" << tosel->getTypeString();
    Sys::view->flash(QColor(255,0,0,100));

    if (selection)
    {
        PlacedTilePtr from    = selection->getPlacedTile();
        QPointF fromP         = selection->getPlacedPoint();
        QPointF toP           = tosel->getPlacedPoint();
        QPointF diff          = toP - fromP;

        QTransform t = from->getTransform() * QTransform::fromTranslate(diff.x(),diff.y());
        from->setTransform(t);

        emit tilingMaker->sig_refreshMenu();
        return true;
    }
    else
    {
        return false;
    }
}

/////////
///
///  CopyJoinEdge
///
/////////

CopyJoinEdge::CopyJoinEdge(PlacedTileSelectorPtr sel, QPointF spt )
    : JoinEdge(Sys::tilingMaker->addTileSelectionPointer(sel),spt)
{
    initial_transform = sel->getPlacedTile()->getTransform();
    desc = "CopyJoinEdge";
    qDebug().noquote() << desc << "start";
}

void CopyJoinEdge::endDragging(QPointF spt)
{
    QPointF initial_pos = tilingMakerView->modelToScreen(initial_transform.map(Sys::ORIGIN));
    QPointF final_pos   = tilingMakerView->modelToScreen(selection->getPlacedTile()->getTransform().map(Sys::ORIGIN));
    if (Geo::dist2(initial_pos,final_pos) < 49.0)
    {
        tilingMaker->deleteTile(selection);
        selection.reset();
    }
    JoinEdge::endDragging(spt);
}

/////////
///
///  CopyJoinMidPoint
///
/////////

CopyJoinMidPoint::CopyJoinMidPoint(PlacedTileSelectorPtr sel, QPointF spt)
    : JoinMidPoint(Sys::tilingMaker->addTileSelectionPointer(sel),spt)
{
    initial_transform = sel->getPlacedTile()->getTransform();
    desc = "CopyJoinMidPoint";
    qDebug().noquote() << desc << "start";
}

void CopyJoinMidPoint::endDragging(QPointF spt)
{
    QPointF initial_pos = tilingMakerView->modelToScreen(initial_transform.map(QPointF()));
    QPointF final_pos   = tilingMakerView->modelToScreen(selection->getPlacedTile()->getTransform().map(QPointF()));
    JoinMidPoint::endDragging(spt);
    if (Geo::dist2(initial_pos,final_pos) < 49.0)
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

CopyJoinPoint::CopyJoinPoint(PlacedTileSelectorPtr sel, QPointF spt)
    : JoinPoint(Sys::tilingMaker->addTileSelectionPointer(sel),spt)
{
    initial_transform = sel->getPlacedTile()->getTransform();
    desc = "CopyJoinPoint";
    qDebug().noquote() << desc << "start";
}

void CopyJoinPoint::endDragging(QPointF spt)
{
    if (selection)
    {
        QPointF initial_pos = tilingMakerView->modelToScreen(initial_transform.map(QPointF()));
        QPointF final_pos   = tilingMakerView->modelToScreen(selection->getPlacedTile()->getTransform().map(QPointF()));
        if (Geo::dist2(initial_pos,final_pos) < 49.0)
        {
            tilingMaker->deleteTile(selection);
            emit tilingMaker->sig_buildMenu();
        }
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
    gridView = Sys::gridViewer;
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

    QPointF newPoint   = tilingMakerView->modelToScreen(wpt);
    VertexPtr firstV   = wAccum.first()->v1;
    QPointF firstPoint = tilingMakerView->modelToScreen(firstV->pt);
    if ((wAccum.size() > 2) &&  Geo::isNear(newPoint,firstPoint))
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
    for (auto & edge :std::as_const(wAccum))
    {
        QPointF p1 = tilingMakerView->modelToScreen(edge->v1->pt);
        QPointF p2 = tilingMakerView->modelToScreen(edge->v2->pt);
        if (Geo::isNear(p1,spt))
        {
            underneath = edge->v1->pt;
            return;
        }
        else if (Geo::isNear(p2,spt))
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

Measure::Measure(QPointF spt, PlacedTileSelectorPtr sel) : TilingMouseAction(sel,spt)
{
    m = new Measurement(tilingMakerView);
    desc = "Measure";
    qDebug() << desc;

    Qt::KeyboardModifiers kms =  QApplication::keyboardModifiers();
    if (sel && (kms == (Qt::CTRL | Qt::SHIFT)))
    {
        qreal len = QLineF(m->startS(),spt).length();    // approx len
        QLineF line = sel->getPlacedLine();
        sPerpLine = QLineF(spt, tilingMakerView->modelToScreen(line.p2()));
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
    QPointF mpt = tilingMakerView->screenToModel(spt);
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
 
 EditTile::EditTile(PlacedTileSelectorPtr sel, PlacedTilePtr pfp, QPointF spt )
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
}

void EditTile::updateDragging(QPointF spt)
{
    if (vertexIndex == -1)
        return;

    QPointF wpt = tilingMakerView->screenToModel(spt);
    QTransform T = pfp->getTransform().inverted();
    wpt = T.map(wpt);
    const EdgePoly & ep = pfp->getTile()->getEdgePoly();
    VertexPtr v = ep[vertexIndex]->v1;
    v->setPt(wpt);  //decompose later

    TilingMouseAction::updateDragging(tilingMakerView->screenToModel(spt));
    emit tilingMaker->sig_refreshMenu();
}

void EditTile::endDragging(QPointF spt )
{
    if (vertexIndex == -1)
        return;

    QPointF wpt = tilingMakerView->screenToModel(spt);
    QTransform T = pfp->getTransform().inverted();
    wpt = T.map(wpt);

    auto tile = pfp->getTile();
    const EdgePoly & ep = tile->getEdgePoly();
    VertexPtr v = ep[vertexIndex]->v1;
    v->setPt(wpt);

    tile->decompose();

    TilingMouseAction::endDragging(spt);
    emit tilingMaker->sig_refreshMenu();
}

/////////
///
///  Edit Edge
///
/////////

EditEdge::EditEdge(PlacedTileSelectorPtr sel, QPointF spt) : TilingMouseAction(sel,spt)
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
    QPointF wpt    = tilingMakerView->screenToModel(spt);
    QPolygonF poly = pfp->getPlacedPoints();
    QPointF pt     = Geo::getClosestPoint(perp,wpt);
    QTransform inv = pfp->getTransform().inverted();
    pt             = inv.map(pt);
    bool convex    = edge->isConvex();
    edge->setCurvedEdge(pt, convex,(edge->getType()==EDGETYPE_CHORD));
    TilingMouseAction::updateDragging(tilingMakerView->screenToModel(spt));
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

TilingConstructionLine::TilingConstructionLine(PlacedTileSelectorPtr sel, QPointF spt)
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
