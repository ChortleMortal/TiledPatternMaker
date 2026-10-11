#include <QDebug>
#include <QApplication>

#include "gui/model_editors/tiling_edit/tile_selection.h"
#include "gui/model_editors/tiling_edit/tiling_mouseactions.h"
#include "gui/top/system_view.h"
#include "gui/top/view_controller.h"
#include "gui/viewers/geo_graphics.h"
#include "gui/viewers/grid_view.h"
#include "model/makers/tiling_maker.h"
#include "model/makers/motif_maker.h"
#include "model/settings/configuration.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/transform.h"
#include "sys/geometry/vertex.h"
#include "sys/sys.h"

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

    cancel          = false;
    tilingMaker     = Sys::tilingMaker;
    selection       = sel;
    wLastDrag       = Sys::tilingMakerView->screenToModel(spt);
    drag_color      = QColor(206,179,102,230);

    Sys::tilingMakerView->forceRedraw();
}

void TilingMouseAction::updateDragging(QPointF spt)
{
    wLastDrag = Sys::tilingMakerView->screenToModel(spt);
    Sys::tilingMakerView->forceRedraw();
}

void TilingMouseAction::draw(GeoGraphics * g2d)
{
    Q_UNUSED(g2d)
}

void TilingMouseAction::endDragging(QPointF spt)
{
    Q_UNUSED(spt)
    tilingMaker->resetOverlaps();       // sets UNDEFINED
    Sys::tilingMakerView->forceRedraw();

    if (tilingMaker->getPropagate()  && !cancel)
    {
        MotifEvent pevent;
        pevent.event = MotifMaker::Event::TilingChanged;
        pevent.tiling = tilingMaker->getSelectedTiling();
        Sys::motifMaker->sm_takeUp(pevent);
    }
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
    cancel = true;          // is overriden by dragging
}

void MovePolygon::updateDragging(QPointF spt)
{
    if  (selection && selection->getType() == INTERIOR)
    {
        //qDebug() << "MovePolygon: update";

        cancel           = false;

        QPointF wpt      = Sys::tilingMakerView->screenToModel(spt);
        PlacedTilePtr pf = selection->getPlacedTile();
        QPointF diff     = wpt - wLastDrag;

        QTransform a = pf->getPlacement();
        QTransform b = QTransform::fromTranslate(diff.x(),diff.y());
        QTransform t = a * b;
        pf->setPlacement(t);

        if (Sys::config->tm_tileAnchorMode)
            tilingMaker->propagateAnchors(pf);

        TilingMouseAction::updateDragging(spt);

        emit tilingMaker->sig_menuRefresh(TMR_PLACED_TILE);
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
    initial_transform = pfp->getPlacement();
    desc = "CopyMovePolygon";
    qDebug() << desc;
}

void CopyMovePolygon::endDragging(QPointF spt )
{
    QPointF initial_pos = Sys::tilingMakerView->modelToScreen(initial_transform.map(Loose::ORIGIN));
    QPointF final_pos   = Sys::tilingMakerView->modelToScreen(selection->getPlacedTile()->getPlacement().map(Loose::ORIGIN));
    TilingMouseAction::endDragging(spt);
    if (Geo::dist2(initial_pos,final_pos ) < 49.0 )
    {
        tilingMaker->deleteTile(selection);
        selection.reset();
    }
    tilingMaker->setMouseMode(TilingMaker::MouseMode::None);
}

/////////
///
///  DrawTranslation
///
/////////

DrawTranslation::DrawTranslation(PlacedTileSelectorPtr sel, QPointF originPt, QPointF spt, QPen apen ) : TilingMouseAction(sel,spt)
{
    desc = "DrawTranslation";
    qDebug() << desc;
    this->apen = apen;
    apen.setColor(drag_color);
    state = ADTT_STARTED;
    vector.setP1(originPt);
    mOrigin = originPt;
}

void DrawTranslation::updateDragging(QPointF spt )
{
    if (state == ADTT_STARTED)
    {
        state = ADTT_DRAGGING;
    }
    if (state == ADTT_DRAGGING)
    {
        // if the origin is a real point on a placed tile, it renders distorted like any other
        // tile, so track the cursor on that same (distorted) transform. If the origin came from
        // a construction-line snap instead, it's already canvas-space, so track undistorted -
        // matching whichever space the origin itself is drawn in.
        vector.setP2(selection ? Sys::tilingMakerView->screenToModel(spt)
                                : Sys::tilingMakerView->undistortedScreenToModel(spt));
    }
    TilingMouseAction::updateDragging(spt);
}

void DrawTranslation::draw(GeoGraphics * g2d )
{
    if (state == ADTT_DRAGGING)
    {
        if (!selection)
            g2d->push(Sys::tilingMakerView->getUndistortedLayerTransform());

        qreal arrow_length = Transform::distFromInvertedZero(g2d->getTransform(),12.0);
        qreal arrow_width  = Transform::distFromInvertedZero(g2d->getTransform(),6.0);
        g2d->drawLine(vector.p1(), vector.p2(),apen);
        g2d->drawArrow(vector.p1(),vector.p2(), arrow_length, arrow_width, drag_color);

        if (!selection)
            g2d->pop();
    }
}

void DrawTranslation::endDragging(QPointF spt)
{
    if (state == ADTT_DRAGGING)
    {
        PlacedTileSelectorPtr sel = Sys::tilingMakerView->findSelection(spt);
        QPointF endPt;
        bool ok = false;
        if (sel && sel->isPoint())
        {
            endPt = sel->getPlacedPoint(true);
            ok = true;
        }
        else
        {
            sel = PlacedTileSelectorPtr();     // not a tile point - try a construction-line snap instead
            ok  = Sys::tilingMakerView->findConstructionSnapCandidate(spt, endPt);
        }
        if (ok)
        {
            vector.setP2(endPt);
            tilingMaker->addToTranslate(vector,mOrigin,selection,sel);
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
    if (selection)
        startPoint = selection->getPlacedPoint(true);
    qDebug().noquote() << desc << "start";
}

void JoinEdge::updateDragging(QPointF spt)
{
    if (snapped) return;

    QPointF wpt = Sys::tilingMakerView->screenToModel(spt);

    if (!snapTo(spt))
    {
        QPointF diff = wpt - wLastDrag;
        if (selection)
        {
            PlacedTilePtr ptp  = selection->getPlacedTile();
            if(ptp)
            {
                QTransform T = ptp->getPlacement() * QTransform::fromTranslate(diff.x(), diff.y());
                ptp->setPlacement(T);
                emit tilingMaker->sig_menuRefresh(TMR_PLACED_TILE);
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
        //qDebug() << "JoinEdge::no snap - no selection";
        return false;
    }
    
    PlacedTileSelectorPtr tosel = Sys::tilingMakerView->findEdge(spt, selection);
    if (!tosel || tosel->getType() != EDGE)
    {
        //qDebug() << "JoinEdge::no snap - no tosel";
        return false;
    }

    snapped = true;

    qDebug().noquote() << "JoinEdge::SNAPs and scales to " << tosel->getTypeString();
    Sys::systemViewController->flashView(QColor(0,0,255,100));

    QLineF pline        = selection->getPlacedLine(true);
    QLineF qline        = tosel->getPlacedLine(true);

    PlacedTilePtr from  = selection->getPlacedTile();
    QTransform fromT    = from->getPlacement();

    QTransform T        = matchTwoSegments(pline.p1(), pline.p2(), qline.p2(), qline.p1());
    QTransform carry    = fromT * T;
    from->setPlacement(carry);

    emit tilingMaker->sig_menuRefresh(TMR_PLACED_TILE);
    return true;
}

// moves the selected point onto a nearby grid crossing
bool JoinEdge::snapToGrid(QPointF spt)
{
    QPointF toP;
    if (!selection || !Sys::gridViewer->nearGridPoint(spt,toP))
        return false;

    auto view = Sys::tilingMakerView;
    if (Geo::isNear(view->modelToScreen(toP),view->modelToScreen(startPoint)))
        return false;   // the crossing it started on

    snapped = true;
    qDebug().noquote() << desc << "SNAP - grid";
    Sys::systemViewController->flashView(QColor(255,0,0,100));

    moveSelectionTo(toP);
    return true;
}

// translates the selected tile so the selected point lands on toP
void JoinEdge::moveSelectionTo(QPointF toP)
{
    PlacedTilePtr from = selection->getPlacedTile();
    QPointF fromP      = selection->getPlacedPoint(true);
    QPointF diff       = toP - fromP;

    QTransform t = from->getPlacement() * QTransform::fromTranslate(diff.x(),diff.y());
    from->setPlacement(t);

    emit tilingMaker->sig_menuRefresh(TMR_PLACED_TILE);
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

// Provide the transform matrix to carry the unit interval
// on the positive X axis to the line segment from p to q.
QTransform JoinEdge::matchLineSegment(QPointF p, QPointF q)
{
    QTransform m(q.x() - p.x(), q.y() - p.y() , p.y() - q.y(), q.x() - p.x(), p.x(), p.y());
    return  m;
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
    
    PlacedTileSelectorPtr tosel = Sys::tilingMakerView->findMidPoint(spt, selection);
    if (!tosel)
    {
        //qDebug() << "JoinMidPoint:: no snap - no tosel";
        return snapToGrid(spt);
    }

    // this is a snap
    snapped = true;

    qDebug().noquote() << "JoinMidPoint::SNAPs without scaling to" << tosel->getTypeString();
    Sys::systemViewController->flashView(QColor(0,255,0,100));

    // the approach is to
    // 1) rotate until edges are parallel and then
    // 2) move until centers are coincident
    // this does not resize the fromsel
    tilingMaker->snapEdgeToEdge(selection->getPlacedTile(), selection->getModelEdge(),
                                    tosel->getPlacedTile(),     tosel->getModelEdge());

    emit tilingMaker->sig_menuRefresh(TMR_PLACED_TILE);

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
    
    PlacedTileSelectorPtr tosel = Sys::tilingMakerView->findPoint(spt, selection);
    QPointF toP;
    if (tosel)
    {
        toP = tosel->getPlacedPoint(true);
        qDebug().noquote() << "JoinPoint::SNAP - type =" << tosel->getTypeString();
    }
    else if (Sys::tilingMakerView->findConstructionSnapCandidate(spt, toP, selection))
    {
        qDebug().noquote() << "JoinPoint::SNAP - construction line feature";
    }
    else
    {
        return snapToGrid(spt);
    }

    snapped = true;

    Sys::systemViewController->flashView(QColor(255,0,0,100));

    if (selection)
    {
        moveSelectionTo(toP);
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
    initial_transform = sel->getPlacedTile()->getPlacement();
    desc = "CopyJoinEdge";
    qDebug().noquote() << desc << "start";
}

void CopyJoinEdge::endDragging(QPointF spt)
{
    QPointF initial_pos = Sys::tilingMakerView->modelToScreen(initial_transform.map(Loose::ORIGIN));
    QPointF final_pos   = Sys::tilingMakerView->modelToScreen(selection->getPlacedTile()->getPlacement().map(Loose::ORIGIN));
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
    initial_transform = sel->getPlacedTile()->getPlacement();
    desc = "CopyJoinMidPoint";
    qDebug().noquote() << desc << "start";
}

void CopyJoinMidPoint::endDragging(QPointF spt)
{
    QPointF initial_pos = Sys::tilingMakerView->modelToScreen(initial_transform.map(QPointF()));
    QPointF final_pos   = Sys::tilingMakerView->modelToScreen(selection->getPlacedTile()->getPlacement().map(QPointF()));
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
    initial_transform = sel->getPlacedTile()->getPlacement();
    desc = "CopyJoinPoint";
    qDebug().noquote() << desc << "start";
}

void CopyJoinPoint::endDragging(QPointF spt)
{
    if (selection)
    {
        QPointF initial_pos = Sys::tilingMakerView->modelToScreen(initial_transform.map(QPointF()));
        QPointF final_pos   = Sys::tilingMakerView->modelToScreen(selection->getPlacedTile()->getPlacement().map(QPointF()));
        if (Geo::dist2(initial_pos,final_pos) < 49.0)
        {
            tilingMaker->deleteTile(selection);
            //emit tilingMaker->sig_menuRefresh(TMR_TILING_UNIT);
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
    QPointF wpt = Sys::tilingMakerView->findSelectionPointOrPoint(spt);
    Sys::gridViewer->nearGridPoint(spt,wpt);
    addVertex(wpt);
    desc = "CreatePolygon";
}

void CreatePolygon::addVertex(QPointF wpt)
{
    qDebug("CreatePolygon::addVertex");

    EdgeSet & wAccum = Sys::tilingMakerView->getAccumW();
    if (wAccum.size() == 0)
    {
        // add first point
        VertexPtr vnew = make_shared<Vertex>(wpt);
        wAccum.push_back(make_shared<Edge>(vnew));
        qDebug() << "point count = 1";
        Sys::tilingMakerView->forceRedraw();
        return;
    }

    QPointF newPoint   = Sys::tilingMakerView->modelToScreen(wpt);
    VertexPtr firstV   = wAccum.first()->v1;
    QPointF firstPoint = Sys::tilingMakerView->modelToScreen(firstV->pt);
    if ((wAccum.size() > 2) &&  Geo::isNear(newPoint,firstPoint))
    {
        // add last point
        qDebug("auto-complete the polygon");
        VertexPtr vnew = make_shared<Vertex>(wpt);
        EdgePtr last = wAccum.last();
        Q_ASSERT(last->getType() == EDGETYPE_POINT);
        last->setV2(firstV);

        auto tiling = tilingMaker->getSelectedTiling();
        QTransform t;
        tilingMaker->addPlacedTile(make_shared<PlacedTile>(make_shared<Tile>(wAccum), t));
        tilingMaker->setMouseMode(TilingMaker::MouseMode::None);
        //emit tilingMaker->sig_menuRefresh(TMR_TILING_UNIT);
        return;
    }

    if (!Sys::tilingMakerView->accumHasPoint(wpt))
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

    EdgeSet & wAccum = Sys::tilingMakerView->getAccumW();
    for (auto & edge : wAccum)
    {
        QPointF p1 = Sys::tilingMakerView->modelToScreen(edge->v1->pt);
        QPointF p2 = Sys::tilingMakerView->modelToScreen(edge->v2->pt);
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
        else if (Sys::gridViewer->nearGridPoint(spt,underneath))
        {
            return;
        }
    }
}

void CreatePolygon::endDragging(QPointF spt )
{
    qDebug("CreatePolygon::endDragging");
    EdgeSet & wAccum = Sys::tilingMakerView->getAccumW();
    if (wAccum.size() != 0)
    {
        QPointF wpt = Sys::tilingMakerView->findSelectionPointOrPoint(spt);
        Sys::gridViewer->nearGridPoint(spt,wpt);

        addVertex(wpt);
    }
    TilingMouseAction::endDragging(spt);
}

void CreatePolygon::draw(GeoGraphics * g2d)
{
    EdgeSet & wAccum = Sys::tilingMakerView->getAccumW();
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
    m = new Measurement(Sys::tilingMakerView.get());
    desc = "Measure";
    qDebug() << desc;

    Qt::KeyboardModifiers kms =  QApplication::keyboardModifiers();
    if (sel && (kms == (Qt::CTRL | Qt::SHIFT)))
    {
        qreal len = QLineF(m->startS(),spt).length();    // approx len
        QLineF line = sel->getPlacedLine(true);
        sPerpLine = QLineF(spt, Sys::tilingMakerView->modelToScreen(line.p2()));
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
        Sys::tilingMakerView->forceRedraw();
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
        Sys::tilingMakerView->getMeasurementsS().push_back(m);
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
    Sys::tilingMakerView->forceRedraw();
}

void Position::updateDragging(QPointF spt)
{
    qDebug() << "spt=" << spt;
    this->spt = spt;
    Sys::tilingMakerView->forceRedraw();
}

void Position::draw(GeoGraphics * g2d)
{
    qreal sx    = spt.x();
    qreal sy    = spt.y();
    QPointF mpt = Sys::tilingMakerView->screenToModel(spt);
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
///  Edit EditTilePoint
///
/////////
 
 EditTilePoint::EditTilePoint(PlacedTileSelectorPtr sel, PlacedTilePtr pfp, QPointF spt)
    : TilingMouseAction(sel,spt)
{
    desc        = "EditTile";
    this->pfp   = pfp;
    vertexIndex = -1;
    snapped     = false;
    Q_ASSERT(sel->getType() == VERTEX);
    qDebug() << desc;

    QPointF v   = sel->getPlacedPoint(true);

    QPolygonF poly = pfp->getWorldPoints(Sys::getActiveDistortion());
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

void EditTilePoint::updateDragging(QPointF spt)
{
    if (vertexIndex == -1)
        return;

    if (snapped)
        return;

    spt = snapTo(spt);

    QPointF wpt = Sys::tilingMakerView->screenToModel(spt);

    QTransform T = pfp->getWorldTransform(Sys::getActiveDistortion()).inverted();
    wpt = T.map(wpt);
    const EdgePoly & ep = pfp->getTile()->getEdgePoly();
    VertexPtr v = ep.get().at(vertexIndex)->v1;
    v->setPt(wpt);  //decompose later

    TilingMouseAction::updateDragging(Sys::tilingMakerView->screenToModel(spt));
    emit tilingMaker->sig_menuRefresh(TMR_PLACED_TILE);
}

void EditTilePoint::endDragging(QPointF spt )
{
    if (vertexIndex == -1)
        return;

    if (!snapped)
    {
        spt = snapTo(spt);
    }
    else
    {
        spt = sSnapped;
    }

    QPointF wpt = Sys::tilingMakerView->screenToModel(spt);
    QTransform T = pfp->getWorldTransform(Sys::getActiveDistortion()).inverted();
    wpt = T.map(wpt);

    auto tile = pfp->getTile();
    const EdgePoly & ep = tile->getEdgePoly();
    VertexPtr v = ep.get().at(vertexIndex)->v1;
    v->setPt(wpt);

    tile->decompose();

    TilingMouseAction::endDragging(spt);
    emit tilingMaker->sig_menuRefresh(TMR_PLACED_TILE);
}

QPointF EditTilePoint::snapTo(QPointF spt)
{
    // snap to a point
    if (!selection)
    {
        return spt;
    }

    PlacedTileSelectorPtr tosel = Sys::tilingMakerView->findPoint(spt, selection);
    if (!tosel)
    {
        tosel = Sys::tilingMakerView->findMidPoint(spt, selection);
        if (!tosel)
        {
            return spt;
        }
    }

    // this is a snap
    qDebug().noquote() << "EditTilePoint::snapTo" << tosel->getTypeString();

    Sys::systemViewController->flashView(QColor(0,255,0,100));

    QPointF toPt = tosel->getPlacedPoint(true);
    sSnapped     = Sys::tilingMakerView->modelToScreen(toPt);
    snapped = true;
    qDebug() << "EditTilePoint::snapTo" << "from" << spt << "to" << sSnapped;
    return sSnapped;
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

    edge  = sel->getModelEdge();
    start = edge->getArcCenter();
    pfp   = sel->getPlacedTile();
    QTransform T = pfp->getWorldTransform(Sys::getActiveDistortion());

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
    QPointF wpt    = Sys::tilingMakerView->screenToModel(spt);
    QPolygonF poly = pfp->getWorldPoints(Sys::getActiveDistortion());
    QPointF pt     = Geo::getClosestPoint(perp,wpt);
    QTransform inv = pfp->getWorldTransform(Sys::getActiveDistortion()).inverted();
    pt             = inv.map(pt);
    edge->chgangeToCurvedEdge(pt, edge->getCurveType());
    TilingMouseAction::updateDragging(Sys::tilingMakerView->screenToModel(spt));
    emit tilingMaker->sig_menuRefresh(TMR_PLACED_TILE);
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

    start = new QPointF(Sys::tilingMakerView->findConstructionSnapPoint(spt));
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


    end = new QPointF(Sys::tilingMakerView->undistortedScreenToModel(spt));

    TilingMouseAction::updateDragging(spt);
}

void TilingConstructionLine::endDragging( QPointF spt)
{
    qDebug() << "spt start"  << spt;
    Qt::KeyboardModifiers kms =  QApplication::keyboardModifiers();
    if (kms == Qt::SHIFT)
    {
        QPointF s = Sys::tilingMakerView->undistortedModelToScreen(*start);
        spt = QPointF(spt.x(),s.y());
    }
    else if (kms == Qt::CTRL)
    {
        QPointF s = Sys::tilingMakerView->undistortedModelToScreen(*start);
        spt = QPointF(s.x(),spt.y());
    }

    qDebug() << "spt end"  << spt;
    end = new QPointF(Sys::tilingMakerView->findConstructionSnapPoint(spt));

    QPointF startS = Sys::tilingMakerView->undistortedModelToScreen(*start);
    QPointF endS   = Sys::tilingMakerView->undistortedModelToScreen(*end);

    bool reject = Geo::isNear(startS,endS);    // too short to be a real line

    if (!reject)
    {
        // both ends already snapped onto the same existing line - would just create a
        // redundant duplicate/overlap, so reject this one too
        for (const QLineF & existing : std::as_const(Sys::tilingMakerView->getConstructionLines()))
        {
            QLineF existingS = Sys::tilingMakerView->undistortedModelToScreen(existing);
            if (Geo::distToLine(startS,existingS) < 5.0 && Geo::distToLine(endS,existingS) < 5.0)
            {
                qDebug() << "rejected - duplicates an existing construction line";
                reject = true;
                break;
            }
        }
    }

    if (!reject)
    {
        Sys::tilingMakerView->getConstructionLines().push_back(QLineF(*start,*end));
    }

    TilingMouseAction::endDragging(spt);
}

void TilingConstructionLine::draw(GeoGraphics * gg)
{
    qreal radius = 3.0;
    QPen apen(Qt::green,3);
    QBrush abrush(Qt::green);

    gg->push(Sys::tilingMakerView->getUndistortedLayerTransform());

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

    gg->pop();
}

/////////////////////////////////////////
//
//  TilingExtendConstructionLine
//
////////////////////////////////////////

TilingExtendConstructionLine::TilingExtendConstructionLine(int lineIndex, int endpoint, QPointF spt)
    : TilingMouseAction(PlacedTileSelectorPtr(),spt), lineIndex(lineIndex), endpoint(endpoint)
{
    desc = "ExtendConstructionLine";
    qDebug() << desc;

    QVector<QLineF> & lines = Sys::tilingMakerView->getConstructionLines();
    originalLine = lines[lineIndex];

    anchor    = (endpoint == 0) ? originalLine.p2() : originalLine.p1();
    currentPt = (endpoint == 0) ? originalLine.p1() : originalLine.p2();

    QPointF d = currentPt - anchor;
    qreal   len = qSqrt(d.x()*d.x() + d.y()*d.y());
    direction = (len > 0) ? (d / len) : QPointF(1,0);

    snapState = LengthSnapState::None;
}

// projects spt onto the fixed anchor/direction line - this is what keeps the angle unchanged.
// Two kinds of snap candidate are considered, both gated on being near the cursor (screen space):
//  - where the fixed axis actually crosses another construction line (always exactly on-axis,
//    so this is how "extend until it meets/crosses another line" is satisfied)
//  - a generic snap point (tile vertex/edge, another construction line's endpoint/midpoint) that
//    happens to already lie close to the axis
// Either counts as a length-only Match (angle unchanged) if close enough to the axis; a candidate
// that's near the cursor but too far off-axis is flagged Near (rejected, not snapped) so draw()
// can still hint that something was spotted.
QPointF TilingExtendConstructionLine::newEndpoint(QPointF spt)
{
    QPointF m = Sys::tilingMakerView->undistortedScreenToModel(spt);
    QPointF am = m - anchor;
    qreal t = am.x()*direction.x() + am.y()*direction.y();     // length along direction
    if (t < 0.0)
        t = 0.0;
    QPointF projected = anchor + direction * t;

    constexpr qreal matchTolerance   = 5.0;    // perpendicular screen px - counts as on-axis
    constexpr qreal nearTolerance    = 20.0;   // perpendicular screen px - spotted but rejected
    constexpr qreal cursorTolerance2 = 49.0;   // 7px, matches other hit-test conventions

    snapState = LengthSnapState::None;

    bool    haveMatch = false;
    bool    haveNear  = false;
    QPointF matchPt;
    qreal   matchD2 = 0.0;
    qreal   nearD2  = 0.0;

    auto consider = [&](QPointF candidate, qreal perp, const char * source)
    {
        QPointF candidateS = Sys::tilingMakerView->undistortedModelToScreen(candidate);
        qreal d2 = Geo::dist2(spt, candidateS);
        if (d2 > cursorTolerance2)
            return;

        qDebug() << "SNAP -" << source;

        if (perp <= matchTolerance)
        {
            if (!haveMatch || d2 < matchD2)
            {
                haveMatch = true;
                matchPt   = candidate;
                matchD2   = d2;
            }
        }
        else if (perp <= nearTolerance)
        {
            if (!haveNear || d2 < nearD2)
            {
                haveNear = true;
                nearD2   = d2;
            }
        }
    };

    // where the fixed axis crosses any other construction line - perfectly on-axis by construction
    QLineF axisRay(anchor, anchor + direction * 1.0e6);
    const QVector<QLineF> & lines = Sys::tilingMakerView->getConstructionLines();
    for (int i = 0; i < lines.size(); i++)
    {
        if (i == lineIndex)
            continue;
        QPointF ipt;
        if (axisRay.intersects(lines[i], &ipt) == QLineF::BoundedIntersection)
            consider(ipt, 0.0, "axis crosses another construction line");
    }

    // generic snap candidate (tile vertex/edge, another construction line's endpoint/midpoint) -
    // excludes this line itself, since its own anchor/midpoint are meaningless self-references
    QPointF candidate;
    if (Sys::tilingMakerView->findConstructionSnapCandidate(spt, candidate, PlacedTileSelectorPtr(), lineIndex))
    {
        QPointF axisA = Sys::tilingMakerView->undistortedModelToScreen(anchor);
        QPointF axisB = Sys::tilingMakerView->undistortedModelToScreen(anchor + direction);
        qreal perp = qAbs(Geo::signedDistToLine(Sys::tilingMakerView->undistortedModelToScreen(candidate), QLineF(axisA,axisB)));
        consider(candidate, perp, "generic point near axis");
    }

    if (haveMatch)
    {
        snapState = LengthSnapState::Match;
        return matchPt;
    }
    if (haveNear)
    {
        snapState = LengthSnapState::Near;
    }
    return projected;
}

void TilingExtendConstructionLine::updateDragging(QPointF spt)
{
    currentPt = newEndpoint(spt);

    QVector<QLineF> & lines = Sys::tilingMakerView->getConstructionLines();
    QLineF & line = lines[lineIndex];
    if (endpoint == 0)
        line.setP1(currentPt);
    else
        line.setP2(currentPt);

    TilingMouseAction::updateDragging(spt);
}

void TilingExtendConstructionLine::endDragging(QPointF spt)
{
    currentPt = newEndpoint(spt);

    QVector<QLineF> & lines = Sys::tilingMakerView->getConstructionLines();
    QLineF & line = lines[lineIndex];

    QPointF anchorS  = Sys::tilingMakerView->undistortedModelToScreen(anchor);
    QPointF currentS = Sys::tilingMakerView->undistortedModelToScreen(currentPt);
    if (Geo::isNear(anchorS,currentS))
    {
        // degenerate result (dragged back onto the anchor) - leave line unchanged
        line = originalLine;
        currentPt = (endpoint == 0) ? originalLine.p1() : originalLine.p2();
    }
    else if (endpoint == 0)
        line.setP1(currentPt);
    else
        line.setP2(currentPt);

    TilingMouseAction::endDragging(spt);
}

void TilingExtendConstructionLine::draw(GeoGraphics * gg)
{
    QColor color = Qt::yellow;
    if      (snapState == LengthSnapState::Match) color = Qt::green;
    else if (snapState == LengthSnapState::Near)   color = Qt::red;

    QPen apen(color,3);
    QBrush abrush(color);

    gg->push(Sys::tilingMakerView->getUndistortedLayerTransform());
    gg->drawCircle(currentPt, 3.0, apen, abrush);
    gg->pop();
}

/////////////////////////////////////////
//
//  TilingMoveConstructionEndpoint
//
////////////////////////////////////////

TilingMoveConstructionEndpoint::TilingMoveConstructionEndpoint(int lineIndex, int endpoint, QPointF spt)
    : TilingMouseAction(PlacedTileSelectorPtr(),spt), lineIndex(lineIndex), endpoint(endpoint)
{
    desc = "MoveConstructionEndpoint";
    qDebug() << desc;

    originalLine = Sys::tilingMakerView->getConstructionLines()[lineIndex];
    currentPt    = (endpoint == 0) ? originalLine.p1() : originalLine.p2();
    snapped      = false;
}

void TilingMoveConstructionEndpoint::moveTo(QPointF spt)
{
    snapped = Sys::tilingMakerView->findConstructionSnapCandidate(spt, currentPt, PlacedTileSelectorPtr(), lineIndex);
    if (!snapped)
        currentPt = Sys::tilingMakerView->undistortedScreenToModel(spt);

    QLineF & line = Sys::tilingMakerView->getConstructionLines()[lineIndex];
    if (endpoint == 0)
        line.setP1(currentPt);
    else
        line.setP2(currentPt);
}

void TilingMoveConstructionEndpoint::updateDragging(QPointF spt)
{
    moveTo(spt);

    TilingMouseAction::updateDragging(spt);
}

void TilingMoveConstructionEndpoint::endDragging(QPointF spt)
{
    moveTo(spt);

    QLineF & line = Sys::tilingMakerView->getConstructionLines()[lineIndex];
    if (Geo::isNear(Sys::tilingMakerView->undistortedModelToScreen(line.p1()),
                    Sys::tilingMakerView->undistortedModelToScreen(line.p2())))
    {
        // degenerate result (dragged onto the other endpoint) - leave line unchanged
        line = originalLine;
        currentPt = (endpoint == 0) ? originalLine.p1() : originalLine.p2();
    }

    TilingMouseAction::endDragging(spt);
}

void TilingMoveConstructionEndpoint::draw(GeoGraphics * gg)
{
    QColor color = snapped ? Qt::green : Qt::yellow;

    gg->push(Sys::tilingMakerView->getUndistortedLayerTransform());
    gg->drawCircle(currentPt, 3.0, QPen(color,3), QBrush(color));
    gg->pop();
}
