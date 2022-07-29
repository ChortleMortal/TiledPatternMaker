#include <QMenu>

#include "widgets/dlg_line_edit.h"
#include "viewers/viewcontrol.h"
#include "geometry/edge.h"
#include "geometry/vertex.h"

DlgLineEdit::DlgLineEdit(EdgePoly & epoly, int row, int col) : QLineEdit(),
  _poly(epoly)
{
    view = ViewControl::getInstance();

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &DlgLineEdit::customContextMenuRequested, this, &DlgLineEdit::slot_menu);
    connect(this, &DlgLineEdit::editingFinished,            this, &DlgLineEdit::slot_editingFinished);
    this->row = row;
    this->col = col;

    _orig = _poly[row]->v1->pt;
}

void DlgLineEdit::slot_menu(QPointF spt)
{
   qDebug() << "menu spt=" << spt;
   QMenu menu(this);
   menu.addAction("Round down",this, SLOT(slot_down()));
   menu.addAction("Nearest",this, SLOT(slot_nearest()));
   menu.addAction("Round Up",this, SLOT(slot_up()));
   menu.addAction("Undo",this, SLOT(slot_undo()));
   menu.exec(mapToGlobal(spt.toPoint()));
}

void DlgLineEdit::focusInEvent(QFocusEvent * event)
{
    Q_UNUSED(event)
    emit currentPoint(_poly[row]->v1->pt);
}

void  DlgLineEdit::slot_down()
{
    VertexPtr vp = _poly[row]->v1;
    QPointF pos  = vp->pt;
    if (col == 0)
    {
        qreal val = pos.x();
        int down  = qFloor(val);
        pos.setX(static_cast<qreal>(down));
        setText(QString::number(pos.x(),'g',16));
    }
    else
    {
        qreal val = pos.y();
        int down   = qFloor(val);
        pos.setY(static_cast<qreal>(down));
        setText(QString::number(pos.y(),'g',16));
    }
    vp->pt = pos;
    emit currentPoint(pos);
    view->update();
}

void  DlgLineEdit::slot_nearest()
{
    VertexPtr vp = _poly[row]->v1;
    QPointF pos  = vp->pt;
    if (col == 0)
    {
        qreal val     = pos.x();
        qreal nearest = qFabs(val);
        pos.setX(nearest);
        setText(QString::number(pos.x(),'g',16));
    }
    else
    {
        qreal val     = pos.y();
        qreal nearest = qFabs(val);
        pos.setY(nearest);
        setText(QString::number(pos.y(),'g',16));
    }
    vp->pt = pos;
    emit currentPoint(pos);
    view->update();
}

void  DlgLineEdit::slot_up()
{
    VertexPtr vp = _poly[row]->v1;
    QPointF pos  = vp->pt;
    if (col == 0)
    {
        qreal val = pos.x();
        int up    = qCeil(val);
        pos.setX(static_cast<qreal>(up));
        setText(QString::number(pos.x(),'g',16));
    }
    else
    {
        qreal val = pos.y();
        int up    = qCeil(val);
        pos.setY(static_cast<qreal>(up));
        setText(QString::number(pos.y(),'g',16));
    }
    vp->pt = pos;
    emit currentPoint(pos);
    view->update();
}

void DlgLineEdit::slot_undo()
{
    VertexPtr vp = _poly[row]->v1;
    QPointF pos  = vp->pt;
    if (col == 0)
    {
        pos.setX(_orig.x());
        setText(QString::number(pos.x(),'g',16));
    }
    else
    {
        pos.setY(_orig.y());
        setText(QString::number(pos.y(),'g',16));

    }
    vp->pt = pos;
    emit currentPoint(pos);
    view->update();
}

void DlgLineEdit::slot_editingFinished()
{
    VertexPtr vp = _poly[row]->v1;
    QPointF pos  = vp->pt;
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
        vp->pt = pos;
        emit currentPoint(pos);
        view->update();
    }
    else
    {
        slot_undo();
    }
}

void DlgLineEdit::slot_update()
{
    VertexPtr vp = _poly[row]->v1;
    QPointF pos  = vp->pt;
    if (col == 0)
    {
        setText(QString::number(pos.x(),'g',16));
    }
    else
    {
        setText(QString::number(pos.y(),'g',16));
    }
}
