#include "makers/tilingmouseactions.h"
#include "makers/tilingmaker.h"
#include "geometry/Point.h"
#include "base/utilities.h"

////////////////////////////////////////////////////////////////////////////
//
// Mouse Actions
//
////////////////////////////////////////////////////////////////////////////

MouseAction::MouseAction(TilingMaker * tilingMaker, TilingSelectionPtr sel, QPointF spt)
{
    desc       = "MouseAction";
    tm         = tilingMaker;
    selection  = sel;
    drag_color = QColor(206,179,102,230);

    wLastDrag = tm->screenToWorld(spt);
    tm->forceRedraw();
}

void MouseAction::updateDragging(QPointF spt)
{
    wLastDrag = tm->screenToWorld(spt);
    tm->forceRedraw();
}

void MouseAction::draw(GeoGraphics * g2d)
{
    Q_UNUSED(g2d)
}

void MouseAction::endDragging(QPointF spt)
{
    Q_UNUSED(spt)
    tm->forceRedraw();
}

/////////
///
///  MovePolygon
///
/////////

MovePolygon::MovePolygon(TilingMaker * tilingMaker, TilingSelectionPtr sel, QPointF spt )
    : MouseAction(tilingMaker,sel,spt)
{
    desc = "MovePolygon";
}

void MovePolygon::updateDragging(QPointF spt)
{
    if  (selection && selection->getType() == INTERIOR)
    {
        //qDebug() << "move: update";

        QPointF wpt         = tm->screenToWorld(spt);
        PlacedFeaturePtr pf = selection->getPlacedFeature();
        QPointF diff        = wpt - wLastDrag;

        QTransform a = pf->getTransform();
        QTransform b = QTransform::fromTranslate(diff.x(),diff.y());
        QTransform  t = a * b;
        pf->setTransform(t);

        MouseAction::updateDragging(spt);

        emit tm->sig_refreshMenu();
    }
    else
        qDebug() << "move: no selection";
}

/////////
///
///  CopyMovePolygon
///
/////////

CopyMovePolygon::CopyMovePolygon(TilingMaker * tilingMaker, TilingSelectionPtr sel, QPointF spt )
    : MovePolygon(tilingMaker, sel = tilingMaker->addFeatureSelectionPointer(sel),spt)
{
    initial_transform = sel->getTransform();
    desc = "CopyMovePolygon";
}

void CopyMovePolygon::endDragging(QPointF spt )
{
    QPointF initial_pos = tm->worldToScreen(initial_transform.map(Point::ORIGIN));
    QPointF final_pos   = tm->worldToScreen(selection->getTransform().map(Point::ORIGIN));
    if (Point::dist2(initial_pos,final_pos ) < 49.0 )
    {
        tm->removeFeature(selection);
        selection.reset();
        emit tm->sig_buildMenu();
    }
    MouseAction::endDragging(spt);
}

/////////
///
///  DrawTranslation
///
/////////

DrawTranslation::DrawTranslation(TilingMaker * tilingMaker, TilingSelectionPtr sel, QPointF spt )
    : MouseAction(tilingMaker,sel,spt)
{
    tm->addToTranslate(sel->getPlacedPoint(), false);
    desc = "DrawTranslation";
}

void DrawTranslation::updateDragging(QPointF spt )
{
    TilingSelectionPtr sel = tm->findSelection(spt);
    if( sel)
    {
        QPointF wpos = sel->getPlacedPoint();
        tm->wTrans1_end = wpos;
    }

    MouseAction::updateDragging(spt);
}

void DrawTranslation::draw(GeoGraphics * g2d )
{
    qreal arrow_length = Transform::distFromInvertedZero(g2d->getTransform(),12.0);
    qreal arrow_width  = Transform::distFromInvertedZero(g2d->getTransform(),6.0);
    QPen pen(drag_color);
    g2d->drawLine(tm->wTrans1_start, wLastDrag,pen);
    g2d->drawArrow(tm->wTrans1_start,wLastDrag, arrow_length, arrow_width, pen, QBrush(drag_color));
}

void DrawTranslation::endDragging(QPointF spt)
{
    TilingSelectionPtr sel = tm->findSelection(spt);
    if (sel)
    {
        tm->addToTranslate(sel->getPlacedPoint(), true );
    }
    MouseAction::endDragging(spt);
}

/////////
///
///  JoinEdge
///
/////////

JoinEdge::JoinEdge(TilingMaker * tilingMaker, TilingSelectionPtr sel, QPointF spt ) : MouseAction(tilingMaker,sel,spt)
{
    desc = "JoinEdge";
    snapped = false;
    qDebug() << desc;
}

bool JoinEdge::snapTo(QPointF spt)
{
    // snap to an edge
    TilingSelectionPtr tosel;
    if (!selection || !(tosel = tm->findEdge(spt, selection)))
    {
        qDebug() << "no snap";
        return false;
    }

    qDebug() << "SNAP EDGE";

    PlacedFeaturePtr pf = selection->getPlacedFeature();
    QLineF pline        = selection->getModelLine();

    QTransform To       = tosel->getTransform();
    QLineF qline        = tosel->getModelLine();

    QTransform t        = matchTwoSegments(pline.p1(), pline.p2(), qline.p2(), qline.p1());
    QTransform carry    = t * To;

    pf->setTransform(carry);

    snapped = true;

    emit tm->sig_refreshMenu();

    return true;
}

void JoinEdge::updateDragging(QPointF spt)
{
    if (snapped) return;

    QPointF wpt = tm->screenToWorld(spt);

    if (!snapTo(spt))
    {
        QPointF diff        = wpt - wLastDrag;
        PlacedFeaturePtr pf = selection->getPlacedFeature();
        QTransform T        = selection->getTransform() * QTransform::fromTranslate(diff.x(), diff.y());
        pf->setTransform(T);
        emit tm->sig_refreshMenu();
    }
    MouseAction::updateDragging(spt);
}

void JoinEdge::endDragging(QPointF spt)
{
    if (!snapped)
    {
        snapTo(spt);
    }
    MouseAction::endDragging(spt);
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

JoinMidPoint::JoinMidPoint(TilingMaker * tilingMaker, TilingSelectionPtr sel, QPointF spt )
    :JoinEdge(tilingMaker,sel,spt)
{
    desc = "JoinMidPoint";
    snapped = false;
    qDebug() << desc;
}

/////////
///
///  Join Point
///
/////////

JoinPoint::JoinPoint(TilingMaker * tilingMaker, TilingSelectionPtr sel, QPointF spt )
    : JoinEdge(tilingMaker,sel,spt)
{
    desc = "JoinPoint";
    snapped = false;
    qDebug() << desc;
}

bool JoinPoint::snapTo(QPointF spt)
{
    // snap to a point
    TilingSelectionPtr tosel;
    if (!selection || !(tosel = tm->findPoint(spt, selection)))
    {
        qDebug() << "no snap";
        return false;
    }

    qDebug() << "SNAP POINT";

    PlacedFeaturePtr pf = selection->getPlacedFeature();
    QPointF from        = selection->getModelPoint();

    QTransform T2       = tosel->getTransform();
    QPointF to          = tosel->getModelPoint();
    qDebug() << "to  " << to;

    QPointF diff = to - from;
    QTransform t = QTransform::fromTranslate(diff.x(),diff.y());
    pf->setTransform(t * T2);

    snapped = true;
    emit tm->sig_refreshMenu();

    return true;
}

/////////
///
///  CopyJoinEdge
///
/////////

CopyJoinEdge::CopyJoinEdge(TilingMaker * tilingMaker, TilingSelectionPtr sel, QPointF spt )
    : JoinEdge(tilingMaker,tilingMaker->addFeatureSelectionPointer(sel),spt)
{
    initial_transform = sel->getTransform();
    desc = "CopyJoinEdge";
    qDebug() << desc;
}

void CopyJoinEdge::endDragging(QPointF spt)
{
    QPointF initial_pos = tm->worldToScreen(initial_transform.map(Point::ORIGIN));
    QPointF final_pos   = tm->worldToScreen(selection->getTransform().map(Point::ORIGIN));
    if (Point::dist2(initial_pos,final_pos) < 49.0)
    {
        tm->removeFeature(selection);
        selection.reset();
        tm->sig_buildMenu();

    }
    JoinEdge::endDragging(spt);
}

/////////
///
///  CopyJoinMidPoint
///
/////////

CopyJoinMidPoint::CopyJoinMidPoint(TilingMaker * tilingMaker, TilingSelectionPtr sel, QPointF spt )
    : JoinMidPoint(tilingMaker,tilingMaker->addFeatureSelectionPointer(sel),spt)
{
    initial_transform = sel->getTransform();
    desc = "CopyJoinMidPoint";
    qDebug() << desc;
}

void CopyJoinMidPoint::endDragging(QPointF spt)
{
    QPointF initial_pos = tm->worldToScreen(initial_transform.map(QPointF()));
    QPointF final_pos   = tm->worldToScreen(selection->getTransform().map(QPointF()));
    if (Point::dist2(initial_pos,final_pos) < 49.0)
    {
        tm->removeFeature(selection);
        selection.reset();
        emit tm->sig_buildMenu();
    }
    JoinMidPoint::endDragging(spt);
}

/////////
///
///  CopyJoinPoint
///
/////////

CopyJoinPoint::CopyJoinPoint(TilingMaker * tilingMaker, TilingSelectionPtr sel, QPointF spt )
    : JoinPoint(tilingMaker,tilingMaker->addFeatureSelectionPointer(sel),spt)
{
    initial_transform = sel->getTransform();
    desc = "CopyJoinPoint";
    qDebug() << desc;
}

void CopyJoinPoint::endDragging(QPointF spt)
{
    QPointF initial_pos = tm->worldToScreen(initial_transform.map(QPointF()));
    QPointF final_pos   = tm->worldToScreen(selection->getTransform().map(QPointF()));
    if (Point::dist2(initial_pos,final_pos) < 49.0)
    {
        tm->removeFeature(selection);
        selection.reset();
        emit tm->sig_buildMenu();
    }
    JoinPoint::endDragging(spt);
}

/////////
///
///  DrawPolygon
///
/////////

CreatePolygon::CreatePolygon(TilingMaker * tilingMaker, QPointF spt )
    : MouseAction(tilingMaker,nullptr,spt)
{
    qDebug() << "CreatePolygon";
    QPointF wpt = tilingMaker->findSelectionPointOrPoint(spt);
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

    QPointF newPoint = tm->worldToScreen(wpt);
    QPointF firstPoint = tm->worldToScreen(wAccum.first()->getV1()->getPosition());
    if ((wAccum.size() > 2) &&  Point::isNear(newPoint,firstPoint))
    {
        // add last point
        qDebug("auto-complete the polygon");
        VertexPtr vnew = make_shared<Vertex>(wpt);
        EdgePtr last = wAccum.last();
        Q_ASSERT(last->getType() == EDGE_POINT);
        last->setV2(vnew);

        QTransform t;
        tm->addToAllPlacedFeatures(make_shared<PlacedFeature>(make_shared<Feature>(wAccum,0), t));
        tm->setMouseMode(NO_MOUSE_MODE);
        tm->sig_buildMenu();
        return;
    }

    if (!tm->accumHasPoint(wpt))
    {
        // is a new point
        VertexPtr vnew = make_shared<Vertex>(wpt);
        EdgePtr last = wAccum.last();
        Q_ASSERT(last->getType() == EDGE_POINT);
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
    TilingSelectionPtr sel;
    sel = tm->findVertex(spt);
    if (sel)
        MouseAction::updateDragging(tm->worldToScreen(sel->getPlacedPoint()));
    else
        MouseAction::updateDragging(spt);
}

void CreatePolygon::endDragging(QPointF spt )
{
    qDebug("CreatePolygon::endDragging");
    EdgePoly & wAccum = tm->getAccumW();
    if (wAccum.size() != 0)
    {
        QPointF wpt = tm->findSelectionPointOrPoint(spt);
        addVertex(wpt);
    }
    MouseAction::endDragging(spt);
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
    }
}

/////////
///
///  Measurement
///
/////////

Measurement::Measurement()
{
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

Measure::Measure(TilingMaker * tilingMaker, QPointF spt, TilingSelectionPtr sel) : MouseAction(tilingMaker,sel,spt)
{
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
    desc = "Measure";
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
        QString msg = QString("%1 (%2)").arg(QString::number(m.lenS(),'f',2)).arg(QString::number(m.lenW(),'f',8));
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
    MouseAction::endDragging(spt);
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

Position::Position(TilingMaker * tilingMaker, QPointF spt) : MouseAction(tilingMaker,nullptr,spt)
{
    this->spt = spt;
    desc = "Position";
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

    QString msg = QString("(%1,%2)(%3,%4)").arg(QString::number(sx,'f',2)) \
                                           .arg(QString::number(sy,'f',2)) \
                                           .arg(QString::number(mx,'f',8)) \
                                           .arg(QString::number(my,'f',8));
    g2d->drawCircle(mpt,3,QPen(Qt::magenta),QBrush(Qt::magenta));
    g2d->drawText(spt + QPointF(10,0),msg);
 }

/////////
///
///  Perspective
///
/////////

Perspective::Perspective(TilingMaker * tilingMaker, QPointF spt )
    : MouseAction(tilingMaker,nullptr,spt)
{
    desc = "Perspective";
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
        if (last->getType() == EDGE_POINT)
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
        MouseAction::endDragging(spt);
    }
}

void Perspective::updateDragging(QPointF spt)
{
    MouseAction::updateDragging(spt);
}

void Perspective::endDragging(QPointF spt )
{
    EdgePoly & waccum = tm->getAccumW();
    if (!Point::isNear(spt,tm->worldToScreen(waccum.first()->getV1()->getPosition())))
    {
        addPoint(spt);
    }
    MouseAction::endDragging(spt);
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

EditFeature::EditFeature(TilingMaker * tilingMaker, TilingSelectionPtr sel, PlacedFeaturePtr pfp, QPointF spt )
    : MouseAction(tilingMaker,sel,spt)
{
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
    EdgePoly & ep = pfp->getFeature()->getEdgePoly();
    VertexPtr v = ep[vertexIndex]->getV1();
    v->setPosition(wpt);

    MouseAction::updateDragging(tm->screenToWorld(spt));
    emit tm->sig_refreshMenu();
}

void EditFeature::endDragging(QPointF spt )
{
    QPointF wpt = tm->screenToWorld(spt);
    QTransform T = pfp->getTransform().inverted();
    wpt = T.map(wpt);
    EdgePoly & ep = pfp->getFeature()->getEdgePoly();
    VertexPtr v = ep[vertexIndex]->getV1();
    v->setPosition(wpt);

    MouseAction::endDragging(spt);
    emit tm->sig_refreshMenu();
}

/////////
///
///  Edit Edge
///
/////////

EditEdge::EditEdge(TilingMaker * tilingMaker, TilingSelectionPtr sel, QPointF spt)
    : MouseAction(tilingMaker,sel,spt)
{
    desc  = "EditEdge";
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
    if (poly.containsPoint(wpt,Qt::OddEvenFill))
    {
        edge->setArcCenter(pt, true);
    }
    else
    {
        edge->setArcCenter(pt, false);
    }

    MouseAction::updateDragging(tm->screenToWorld(spt));
    emit tm->sig_refreshMenu();
}

void EditEdge::endDragging(QPointF spt )
{
    MouseAction::endDragging(spt);
}
