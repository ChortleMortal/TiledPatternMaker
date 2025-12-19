#include <QMenu>

#include "gui/widgets/dlg_point_edit.h"
#include "gui/top/system_view.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/vertex.h"
#include "sys/sys.h"

DlgPointEdit::DlgPointEdit(EdgePoly & epoly, int row, int col, QWidget *parent) : AQDoubleSpinBox(parent), _poly(epoly)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    setMinimum(-4096.0);
    setMaximum(4096.0);
    setDecimals(16);
    setSingleStep(0.01);

    connect(this, &DlgPointEdit::customContextMenuRequested, this, &DlgPointEdit::slot_menu);
    connect(this, &DlgPointEdit::editingFinished,            this, &DlgPointEdit::slot_editingFinished);
    connect(this, &DlgPointEdit::valueChanged,               this, &DlgPointEdit::slot_editingFinished);
    connect(this, &DlgPointEdit::sig_updateView,             Sys::viewController, &SystemViewController::slot_updateView);

    this->row = row;
    this->col = col;

    _orig = _poly.get()[row]->v1->pt;
}

void DlgPointEdit::slot_menu(QPointF spt)
{
   qDebug() << "menu spt=" << spt;
   QMenu menu(this);
   menu.addAction("Round down",this, SLOT(slot_down()));
   menu.addAction("Nearest",this, SLOT(slot_nearest()));
   menu.addAction("Round Up",this, SLOT(slot_up()));
   menu.addAction("Undo",this, SLOT(slot_undo()));
   menu.exec(mapToGlobal(spt.toPoint()));
}

void DlgPointEdit::focusInEvent(QFocusEvent * event)
{
    Q_UNUSED(event)
    emit currentPoint(_poly.get()[row]->v1->pt);
}

void  DlgPointEdit::slot_down()
{
    EdgeSet & base = _poly.getBaseRW();
    VertexPtr vp   = base[row]->v1;
    QPointF pos    = vp->pt;
    if (col == 0)
    {
        qreal val = pos.x();
        int down  = qFloor(val);
        pos.setX(static_cast<qreal>(down));
        setValue(pos.x());
    }
    else
    {
        qreal val = pos.y();
        int down   = qFloor(val);
        pos.setY(static_cast<qreal>(down));
        setValue(pos.y());
    }
    vp->setPt(pos);
    _poly.compose();
    emit currentPoint(pos);
    emit sig_updateView();
}

void  DlgPointEdit::slot_nearest()
{
    EdgeSet & base = _poly.getBaseRW();
    VertexPtr vp   = base[row]->v1;
    QPointF pos    = vp->pt;
    if (col == 0)
    {
        qreal val     = pos.x();
        qreal nearest = qFabs(val);
        pos.setX(nearest);
        setValue(pos.x());
    }
    else
    {
        qreal val     = pos.y();
        qreal nearest = qFabs(val);
        pos.setY(nearest);
        setValue(pos.y());
    }
    vp->setPt(pos);
    _poly.compose();
    emit currentPoint(pos);
    emit sig_updateView();
}

void  DlgPointEdit::slot_up()
{
    EdgeSet & base = _poly.getBaseRW();
    VertexPtr vp   = base[row]->v1;
    QPointF pos    = vp->pt;
    if (col == 0)
    {
        qreal val = pos.x();
        int up    = qCeil(val);
        pos.setX(static_cast<qreal>(up));
        setValue(pos.x());
    }
    else
    {
        qreal val = pos.y();
        int up    = qCeil(val);
        pos.setY(static_cast<qreal>(up));
        setValue(pos.y());
    }
    vp->setPt(pos);
    _poly.compose();
    emit currentPoint(pos);
    emit sig_updateView();
}

void DlgPointEdit::slot_undo()
{
    EdgeSet & base = _poly.getBaseRW();
    VertexPtr vp   = base[row]->v1;
    QPointF pos    = vp->pt;
    if (col == 0)
    {
        pos.setX(_orig.x());
        setValue(pos.x());
    }
    else
    {
        pos.setY(_orig.y());
        setValue(pos.y());

    }
    vp->setPt(pos);
    _poly.compose();
    emit currentPoint(pos);
    emit sig_updateView();
}

void DlgPointEdit::slot_editingFinished()
{
    EdgeSet & base = _poly.getBaseRW();
    VertexPtr vp   = base[row]->v1;
    QPointF pos    = vp->pt;
    bool ok;
    qreal val = text().toDouble(&ok);
    if (ok)
    {
        if (col == 0)
        {
            pos.setX(val);
        }
        else
        {
            pos.setY(val);
        }
        vp->setPt(pos);
        _poly.compose();
        emit currentPoint(pos);
        emit sig_updateView();
    }
    else
    {
        slot_undo();
    }
}

void DlgPointEdit::slot_updatePoint()
{
    VertexPtr vp = _poly.get()[row]->v1;
    QPointF pos  = vp->pt;
    if (col == 0)
    {
        setValue(pos.x());
    }
    else
    {
        setValue(pos.y());
    }
}
