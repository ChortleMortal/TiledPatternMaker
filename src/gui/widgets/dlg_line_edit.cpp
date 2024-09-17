#include <QMenu>

#include "gui/widgets/dlg_line_edit.h"
#include "gui/top/view.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/vertex.h"
#include "sys/sys.h"

#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <QDebug>
#include <QtMath>
#endif

DlgLineEdit::DlgLineEdit(EdgePoly & epoly, int row, int col) : QLineEdit(), _poly(epoly)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &DlgLineEdit::customContextMenuRequested, this, &DlgLineEdit::slot_menu);
    connect(this, &DlgLineEdit::editingFinished,            this, &DlgLineEdit::slot_editingFinished);
    connect(this, &DlgLineEdit::sig_updateView,             Sys::view, &View::slot_update);

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
    vp->setPt(pos);
    emit currentPoint(pos);
    emit sig_updateView();
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
    vp->setPt(pos);
    emit currentPoint(pos);
    emit sig_updateView();
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
    vp->setPt(pos);
    emit currentPoint(pos);
    emit sig_updateView();
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
    vp->setPt(pos);
    emit currentPoint(pos);
    emit sig_updateView();
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
        vp->setPt(pos);
        emit currentPoint(pos);
        emit sig_updateView();
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
