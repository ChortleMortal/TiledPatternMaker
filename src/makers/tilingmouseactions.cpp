#include "makers/tilingmouseactions.h"
#include "makers/TilingDesigner.h"

////////////////////////////////////////////////////////////////////////////
//
// Mouse interactions.
//
////////////////////////////////////////////////////////////////////////////

MouseAction::MouseAction(TilingDesigner * td, TilingSelectionPtr sel, QPointF spt)
{
    desc       = "MouseAction";
    this->td   = td;
    selection  = sel;
    drag_color = QColor(206,179,102,230);

    last_drag = td->screenToWorld(spt);
    td->forceRedraw();
}

void MouseAction::updateDragging(QPointF spt)
{
    last_drag = spt;
    td->forceRedraw();
}

void MouseAction::draw(GeoGraphics * g2d)
{
    Q_UNUSED(g2d);
}

void MouseAction::endDragging(QPointF spt)
{
    Q_UNUSED(spt);
    td->forceRedraw();
}

MovePolygon::MovePolygon(TilingDesigner * td, TilingSelectionPtr sel, QPointF spt )
    : MouseAction(td,sel,spt)
{
    desc = "MovePolygon";
}

void MovePolygon::updateDragging(QPointF spt)
{
    if  (selection)
    {
        //qDebug() << "move: update";

        QPointF w           = td-> screenToWorld(spt);
        PlacedFeaturePtr pf = selection->getFeature();
        QPointF diff        = w - last_drag;

        // DAC is precedence correct?
#if 0
        Transform t;
        t = t.translate(diff);
        t = t.compose(pf->getTransform());
#else
        Transform a = pf->getTransform();
        Transform b = Transform::translate(diff);
        Transform t = b.compose(a);
#endif
        pf->setTransform(t);
        //pf->setTransform( Transform.translate( diff ).compose( pf.getTransform() ) );
        MouseAction::updateDragging( w );
        td->forceRedraw();
    }
    else
        qDebug() << "move: no selection";
}

CopyMovePolygon::CopyMovePolygon(TilingDesigner * td, TilingSelectionPtr sel, QPointF spt )
    : MovePolygon(td,td->addFeatureSP(sel),spt)
{
    initial_transform = sel->getFeature()->getTransform();
    desc = "CopyMovePolygon";
}

void CopyMovePolygon::endDragging(QPointF spt )
{
    QPointF initial_pos = td->worldToScreen(initial_transform.apply(Point::ORIGIN));
    QPointF final_pos   = td->worldToScreen(selection->getFeature()->getTransform().apply(Point::ORIGIN));
    if (Point::dist2(initial_pos,final_pos ) < 49.0 )
    {
        td->removeFeature(selection);
        selection.reset();
    }
    MouseAction::endDragging(spt);
}

DrawTranslation::DrawTranslation(TilingDesigner * td, TilingSelectionPtr sel, QPointF spt )
    : MouseAction(td,sel,spt)
{
    td->addToTranslate(sel->getPoint(), false);
    desc = "DrawTranslation";
}

void DrawTranslation::updateDragging(QPointF spt )
{
    TilingSelectionPtr sel = td->findSelection(spt);
    if( sel)
    {
        QPointF pos = sel->getPoint();
        td->trans1_end = pos;
    }

    MouseAction::updateDragging(td->screenToWorld(spt));
}

void DrawTranslation::draw(GeoGraphics * g2d )
{
    qreal arrow_length = g2d->getTransform().distFromInvertedZero(12.0);
    qreal arrow_width  = g2d->getTransform().distFromInvertedZero( 6.0);
    g2d->setColor(drag_color);
    g2d->drawLine(td->trans1_start, last_drag);
    g2d->drawArrow(td->trans1_start, last_drag, arrow_length, arrow_width, true );
}

void DrawTranslation::endDragging(QPointF spt)
{
    TilingSelectionPtr sel = td->findSelection(spt);
    if (sel)
    {
        td->addToTranslate(sel->getPoint(), true );
    }
    MouseAction::endDragging(spt);
}

JoinEdge::JoinEdge(TilingDesigner * td, TilingSelectionPtr sel, QPointF spt ) : MouseAction(td,sel,spt)
{
    desc = "JoinEdge";
}

bool JoinEdge::snapToEdge(QPointF spt)
{
    TilingSelectionPtr sel;
    if (!selection || !(sel = td->findEdge(spt, selection)))
    {
        qDebug() << "no snap";
        return false;
    }

    PlacedFeaturePtr pf = selection->getFeature();
    Transform To        = sel->getFeature()->getTransform();

    QLineF pline    = selection->getLine();
    QLineF qline    = sel->getLine();

    Transform t     = Transform::matchTwoSegments(pline.p1(), pline.p2(), qline.p1(), qline.p2());
    Transform carry = To.compose(t);

    pf->setTransform(carry);

    qDebug() << "SNAP EDGE";
    return true;
}

void JoinEdge::updateDragging(QPointF spt)
{
    QPointF w = td->screenToWorld(spt);
    if  (selection)
    {
        if (!snapToEdge(spt))
        {
            QPointF diff        = w - last_drag;
            PlacedFeaturePtr pf = selection->getFeature();
            pf->setTransform(Transform::translate(diff).compose(pf->getTransform()));
        }
    }
    MouseAction::updateDragging(w);
}

void JoinEdge::endDragging(QPointF spt)
{
    snapToEdge(spt);
    MouseAction::endDragging(spt);
}

CopyJoinEdge::CopyJoinEdge(TilingDesigner * td, TilingSelectionPtr sel, QPointF spt )
    : JoinEdge(td,td->addFeatureSP(sel),spt)
{
    initial_transform = sel->getFeature()->getTransform();
    desc = "CopyJoinEdge";
}

void CopyJoinEdge::endDragging(QPointF spt)
{
    QPointF initial_pos = td->worldToScreen(initial_transform.apply(Point::ORIGIN));
    QPointF final_pos   = td->worldToScreen(selection->getFeature()->getTransform().apply(Point::ORIGIN));
    if (Point::dist2(initial_pos,final_pos) < 49.0)
    {
        td->removeFeature(selection);
        selection.reset();
    }
    JoinEdge::endDragging(spt);
}

DrawPolygon::DrawPolygon(TilingDesigner * td, TilingSelectionPtr sel, QPointF spt )
    : MouseAction(td,sel,spt)
{
    addVertex(sel);
    desc = "DrawPolygon";
}

void DrawPolygon::addVertex(TilingSelectionPtr sel)
{
    qDebug("addVertex");

    if (sel)
    {
        selection = sel;
        QPointF pos = sel->getPoint();
        QPointF spos = td->worldToScreen(pos);
        if (td->accum.size())
        {
            QPointF zpos = td->worldToScreen(td->accum[0]);
            if ((td->accum.size() > 2) &&  Point::isNear(spos, zpos))
            {
                qDebug("auto-complete the polygon");
                QPolygonF pgon(td->accum);
                Transform t;
                td->addFeaturePF(make_shared<PlacedFeature>(make_shared<Feature>(pgon), t));
                selection.reset();
                td->accum.clear();
                return;
            }
        }
        if (!td->accumHasPoint(spos))
        {
            qDebug("add the point");
            td->accum.push_back(pos);
            td->forceRedraw();
        }
        else
            qDebug() << "addVertex: ignored (exisiting point)";
    }
    else
        qDebug() << "addVertex: ignored (no selection)";
}

void DrawPolygon::updateDragging(QPointF spt)
{
    TilingSelectionPtr sel;
    sel = td->findSelection(spt);
    if (sel)
        MouseAction::updateDragging(sel->getPoint());
    else
        MouseAction::updateDragging(td->screenToWorld(spt));
}

void DrawPolygon::endDragging(QPointF spt )
{
    addVertex(td->findSelection(spt));
    MouseAction::endDragging(spt);
}

void DrawPolygon::draw(GeoGraphics * g2d)
{
    if ( td->accum.size() > 0 )
    {
        if (!last_drag.isNull())
        {
            qreal radius = g2d->getTransform().distFromInvertedZero( 5.0 );
            g2d->setColor(drag_color);
            g2d->drawLine(td->accum.last(), last_drag );
            g2d->drawCircle( last_drag, radius, true );
        }
    }
}

Measurement::Measurement()
{
    td = TilingDesigner::getInstance();
    active = false;
}

void Measurement::reset()
{
    _start = QPointF();
    _end   = QPointF();
    active = false;
}
void Measurement::setStart(QPointF spt)
{
    _start = td->screenToWorld(spt);
}

void Measurement::setEnd(QPointF spt)
{
    _end = td->screenToWorld(spt);

}

QPointF Measurement::start()
{
    return _start;
}

QPointF Measurement::end()
{
    return _end;
}

QPointF Measurement::startS()
{
    return td->worldToScreen(_start);
}

QPointF Measurement::endS()
{
      return td->worldToScreen(_end);
}

qreal Measurement::lenS()
{
    return QLineF(startS(),endS()).length();
}

qreal Measurement::len()
{
    return QLineF(_start,_end).length();
}

Measure::Measure(TilingDesigner * td, QPointF spt, TilingSelectionPtr sel) : MouseAction(td,sel,spt)
{
    Qt::KeyboardModifiers kms =  QApplication::keyboardModifiers();
    if (sel && (kms == (Qt::CTRL | Qt::SHIFT)))
    {
        qreal len = QLineF(m.startS(),spt).length();    // approx len
        QLineF line = sel->getLine();
        sPerpLine = QLineF(spt, td->worldToScreen(line.p2()));
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
        td->forceRedraw();
    }
}

void Measure::draw(GeoGraphics * g2d)
{
    if (m.active)
    {
        g2d->setColor(drag_color);
        g2d->drawLineS(m.startS(),m.endS());
        QString msg = QString("%1 (%2)").arg(QString::number(m.lenS(),'f',2)).arg(QString::number(m.len(),'f',8));
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
        td->measurements.push_back(m);
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
Position::Position(TilingDesigner * td, QPointF spt) : MouseAction(td,nullptr,spt)
{
    this->spt = spt;
    desc = "Position";
    td->forceRedraw();
}

void Position::updateDragging(QPointF spt)
{
    qDebug() << "spt=" << spt;
    this->spt = spt;
    td->forceRedraw();
}

void Position::draw(GeoGraphics * g2d)
{
    qreal sx    = spt.x();
    qreal sy    = spt.y();
    QPointF mpt = td->screenToWorld(spt);
    qreal mx    = mpt.x();
    qreal my    = mpt.y();

    QString msg = QString("(%1,%2)(%3,%4)").arg(QString::number(sx,'f',2)) \
                                           .arg(QString::number(sy,'f',2)) \
                                           .arg(QString::number(mx,'f',8)) \
                                           .arg(QString::number(my,'f',8));
    qreal radius = g2d->getTransform().distFromInvertedZero(3.0);
    g2d->drawCircle(mpt, radius, true);

    g2d->drawText(spt + QPointF(10,0),msg);
 }

void Position::endDragging(QPointF spt)
{
    MouseAction::endDragging(spt);
}
