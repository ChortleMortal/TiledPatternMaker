#include "makers/tiling_maker/tiling_mouseactions.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "geometry/point.h"
#include "geometry/transform.h"
#include "base/utilities.h"
#include "base/configuration.h"

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
        PlacedFeaturePtr pf = selection->getPlacedFeature();
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
    : MovePolygon(tilingMaker, sel = tilingMaker->addFeatureSelectionPointer(sel),spt)
{
    PlacedFeaturePtr pfp = sel->getPlacedFeature();
    initial_transform = pfp->getTransform();
    desc = "CopyMovePolygon";
    qDebug() << desc;
}

void CopyMovePolygon::endDragging(QPointF spt )
{
    QPointF initial_pos = tm->worldToScreen(initial_transform.map(Point::ORIGIN));
    QPointF final_pos   = tm->worldToScreen(selection->getPlacedFeature()->getTransform().map(Point::ORIGIN));
    if (Point::dist2(initial_pos,final_pos ) < 49.0 )
    {
        tm->deleteFeature(selection);
        selection.reset();
    }
    TilingMouseAction::endDragging(spt);
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
    qDebug() << desc;
    snapped = false;
    qDebug() << desc;
}

void JoinEdge::updateDragging(QPointF spt)
{
    if (snapped) return;

    QPointF wpt = tm->screenToWorld(spt);

    if (!snapTo(spt))
    {
        QPointF diff        = wpt - wLastDrag;
        PlacedFeaturePtr pf = selection->getPlacedFeature();
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

    PlacedFeaturePtr from = selection->getPlacedFeature();
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
    QMatrix m(q.x() - p.x(), q.y() - p.y() , p.y() - q.y(), q.x() - p.x(), p.x(), p.y());
    return  QTransform(m);
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
    qDebug() << desc;
    snapped = false;
    qDebug() << desc;
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
    qDebug() << desc;
    snapped = false;
    qDebug() << desc;
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

    PlacedFeaturePtr from = selection->getPlacedFeature();

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
    : JoinEdge(tilingMaker,tilingMaker->addFeatureSelectionPointer(sel),spt)
{
    initial_transform = sel->getPlacedFeature()->getTransform();
    desc = "CopyJoinEdge";
    qDebug() << desc;
}

void CopyJoinEdge::endDragging(QPointF spt)
{
    QPointF initial_pos = tm->worldToScreen(initial_transform.map(Point::ORIGIN));
    QPointF final_pos   = tm->worldToScreen(selection->getPlacedFeature()->getTransform().map(Point::ORIGIN));
    if (Point::dist2(initial_pos,final_pos) < 49.0)
    {
        tm->deleteFeature(selection);
        selection.reset();

    }
    JoinEdge::endDragging(spt);
}

/////////
///
///  CopyJoinMidPoint
///
/////////

CopyJoinMidPoint::CopyJoinMidPoint(TilingMaker * tilingMaker, TilingSelectorPtr sel, QPointF spt )
    : JoinMidPoint(tilingMaker,tilingMaker->addFeatureSelectionPointer(sel),spt)
{
    initial_transform = sel->getPlacedFeature()->getTransform();
    desc = "CopyJoinMidPoint";
    qDebug() << desc;
}

void CopyJoinMidPoint::endDragging(QPointF spt)
{
    QPointF initial_pos = tm->worldToScreen(initial_transform.map(QPointF()));
    QPointF final_pos   = tm->worldToScreen(selection->getPlacedFeature()->getTransform().map(QPointF()));
    if (Point::dist2(initial_pos,final_pos) < 49.0)
    {
        tm->deleteFeature(selection);
        selection.reset();
    }
    JoinMidPoint::endDragging(spt);
}

/////////
///
///  CopyJoinPoint
///
/////////

CopyJoinPoint::CopyJoinPoint(TilingMaker * tilingMaker, TilingSelectorPtr sel, QPointF spt )
    : JoinPoint(tilingMaker,tilingMaker->addFeatureSelectionPointer(sel),spt)
{
    initial_transform = sel->getPlacedFeature()->getTransform();
    desc = "CopyJoinPoint";
    qDebug() << desc;
}

void CopyJoinPoint::endDragging(QPointF spt)
{
    QPointF initial_pos = tm->worldToScreen(initial_transform.map(QPointF()));
    QPointF final_pos   = tm->worldToScreen(selection->getPlacedFeature()->getTransform().map(QPointF()));
    if (Point::dist2(initial_pos,final_pos) < 49.0)
    {
        tm->deleteFeature(selection);
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
    QPointF wpt = tilingMaker->findSelectionPointOrPoint(spt);
    tm->nearGridPoint(spt,wpt);
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
    VertexPtr firstV   = wAccum.first()->getV1();
    QPointF firstPoint = tm->worldToScreen(firstV->getPosition());
    if ((wAccum.size() > 2) &&  Point::isNear(newPoint,firstPoint))
    {
        // add last point
        qDebug("auto-complete the polygon");
        VertexPtr vnew = make_shared<Vertex>(wpt);
        EdgePtr last = wAccum.last();
        Q_ASSERT(last->getType() == EDGETYPE_POINT);
        last->setV2(firstV);

        QTransform t;
        tm->addNewPlacedFeature(make_shared<PlacedFeature>(make_shared<Feature>(wAccum,0), t));
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
    for (auto edge : wAccum)
    {
        QPointF p1 = tm->worldToScreen(edge->getV1()->getPosition());
        QPointF p2 = tm->worldToScreen(edge->getV2()->getPosition());
        if (Point::isNear(p1,spt))
        {
            underneath = edge->getV1()->getPosition();
            return;
        }
        else if (Point::isNear(p2,spt))
        {
            underneath = edge->getV2()->getPosition();
            return;
        }
        else if (tm->nearGridPoint(spt,underneath))
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
        tm->nearGridPoint(spt,wpt);

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
            g2d->drawLine(wAccum.last()->getV2()->getPosition(),wLastDrag,pen);
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
///  Measurement
///
/////////

Measurement::Measurement()
{
    qDebug() << "Measurement";
    tm = TilingMaker::getInstance();
    active = false;
}

void Measurement::reset()
{
    wStart = QPointF();
    wEnd   = QPointF();
    active = false;
}
void Measurement::setStart(QPointF spt)
{
    wStart = tm->screenToWorld(spt);
}

void Measurement::setEnd(QPointF spt)
{
    wEnd = tm->screenToWorld(spt);

}

QPointF Measurement::startW()
{
    return wStart;
}

QPointF Measurement::endW()
{
    return wEnd;
}

QPointF Measurement::startS()
{
    return tm->worldToScreen(wStart);
}

QPointF Measurement::endS()
{
      return tm->worldToScreen(wEnd);
}

qreal Measurement::lenS()
{
    return QLineF(startS(),endS()).length();
}

qreal Measurement::lenW()
{
    return QLineF(wStart,wEnd).length();
}

/////////
///
///  Measure
///
/////////

Measure::Measure(TilingMaker * tilingMaker, QPointF spt, TilingSelectorPtr sel) : TilingMouseAction(tilingMaker,sel,spt)
{
    desc = "Measure";
    qDebug() << desc;

    Qt::KeyboardModifiers kms =  QApplication::keyboardModifiers();
    if (sel && (kms == (Qt::CTRL | Qt::SHIFT)))
    {
        qreal len = QLineF(m.startS(),spt).length();    // approx len
        QLineF line = sel->getPlacedLine();
        sPerpLine = QLineF(spt, tilingMaker->worldToScreen(line.p2()));
        sPerpLine = normalVectorB(sPerpLine);
        sPerpLine.setLength(len);

        m.setStart(spt);
        m.setEnd(sPerpLine.p2());
    }
    else
    {
        m.setStart(spt);
        m.setEnd(spt);
    }
}

void Measure::updateDragging(QPointF spt)
{
    if (spt != m.startS())
    {
        Qt::KeyboardModifiers kms =  QApplication::keyboardModifiers();
        if (selection && (kms == (Qt::CTRL | Qt::SHIFT)))
        {
            qreal len = QLineF(m.startS(),spt).length();    // approx len
            sPerpLine.setLength(len);
            m.setEnd(sPerpLine.p2());
        }
        else
        {
            m.setEnd(spt);
        }
        m.active = true;
        tm->forceRedraw();
    }
}

void Measure::draw(GeoGraphics * g2d)
{
    if (m.active)
    {
        g2d->drawLineDirect(m.startS(),m.endS(),QPen(drag_color));
        QString msg = QString("%1 (%2)").arg(QString::number(m.lenS(),'f',2),QString::number(m.lenW(),'f',8));
        g2d->drawText(m.endS() + QPointF(10,0),msg);
    }
}

void Measure::endDragging(QPointF spt)
{
    if (m.active)
    {
        Qt::KeyboardModifiers kms =  QApplication::keyboardModifiers();
        if (selection && (kms == (Qt::CTRL | Qt::SHIFT)))
        {
            qreal len = QLineF(m.startS(),spt).length();    // approx len
            sPerpLine.setLength(len);
            m.setEnd(sPerpLine.p2());
        }
        else
        {
            m.setEnd(spt);
        }
        tm->getMeasurementsS().push_back(m);
        m.reset();
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
///  Perspective
///
/////////

Perspective::Perspective(TilingMaker * tilingMaker, QPointF spt )
    : TilingMouseAction(tilingMaker,nullptr,spt)
{
    desc = "Perspective";
    qDebug() << desc;
    EdgePoly & waccum = tilingMaker->getAccumW();
    qDebug() << "click size=" << waccum.size();
    if (waccum.size() == 0)
    {
        addPoint(spt);
    }
}

void Perspective::addPoint(QPointF spos)
{
    qDebug("Perspective::addPoint");

    VertexPtr vnew = make_shared<Vertex>(tm->screenToWorld(spos));

    EdgePoly & accum = tm->getAccumW();
    int size = accum.size();

    if (size == 0)
    {
        accum.push_back(make_shared<Edge>(vnew));
        qDebug() << "point count = 1";
    }
    else if (size == 1)
    {
        EdgePtr last = accum.last();
        if (last->getType() == EDGETYPE_POINT)
        {
            last->setV2(vnew);
            qDebug() << "edge count =" << accum.size();
        }
        else
        {
            accum.push_back(make_shared<Edge>(last->getV2(),vnew));
            qDebug() << "edge count =" << accum.size();
        }
    }
    else if (size == 2)
    {
        EdgePtr last = accum.last();
        accum.push_back(make_shared<Edge>(last->getV2(),vnew));
        qDebug() << "edge count = " << accum.size();
        accum.push_back(make_shared<Edge>(vnew,accum.first()->getV1()));
        qDebug() << "completed with edge count =" << accum.size();
        TilingMouseAction::endDragging(spt);
    }
}

void Perspective::updateDragging(QPointF spt)
{
    TilingMouseAction::updateDragging(spt);
}

void Perspective::endDragging(QPointF spt )
{
    EdgePoly & waccum = tm->getAccumW();
    if (!Point::isNear(spt,tm->worldToScreen(waccum.first()->getV1()->getPosition())))
    {
        addPoint(spt);
    }
    TilingMouseAction::endDragging(spt);
}

void Perspective::draw(GeoGraphics * g2d)
{
    EdgePoly & waccum = tm->getAccumW();
    if (waccum.size() > 0)
    {
        if (!wLastDrag.isNull())
        {
            QPen pen(drag_color);
            g2d->drawLine(waccum.last()->getV2()->getPosition(),wLastDrag,pen);
            g2d->drawCircle(wLastDrag,10,pen,QBrush(drag_color));
        }
    }
}


/////////
///
///  Edit Feature
///
/////////

EditFeature::EditFeature(TilingMaker * tilingMaker, TilingSelectorPtr sel, PlacedFeaturePtr pfp, QPointF spt )
    : TilingMouseAction(tilingMaker,sel,spt)
{
    desc = "EditFeature";
    qDebug() << desc;

    Q_ASSERT(sel->getType() == VERTEX);

    vertexIndex = -1;
    desc        = "EditFeature";
    this->pfp   = pfp;

    QPointF v = sel->getPlacedPoint();

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

void EditFeature::updateDragging(QPointF spt)
{
    QPointF wpt = tm->screenToWorld(spt);
    QTransform T = pfp->getTransform().inverted();
    wpt = T.map(wpt);
    const EdgePoly & ep = pfp->getFeature()->getEdgePoly();
    VertexPtr v = ep[vertexIndex]->getV1();
    v->setPosition(wpt);

    TilingMouseAction::updateDragging(tm->screenToWorld(spt));
    emit tm->sig_refreshMenu();
}

void EditFeature::endDragging(QPointF spt )
{
    QPointF wpt = tm->screenToWorld(spt);
    QTransform T = pfp->getTransform().inverted();
    wpt = T.map(wpt);
    const EdgePoly & ep = pfp->getFeature()->getEdgePoly();
    VertexPtr v = ep[vertexIndex]->getV1();
    v->setPosition(wpt);

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
    pfp   = sel->getPlacedFeature();
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
    edge->setArcCenter(pt, convex);
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
