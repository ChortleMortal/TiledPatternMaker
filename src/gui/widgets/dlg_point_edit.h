#pragma once
#ifndef DLG_POINT_EDIT_H
#define DLG_POINT_EDIT_H

#include <QDoubleSpinBox>
#include "sys/geometry/edge_poly.h"
#include "gui/widgets/layout_sliderset.h"

class EdgePoly;

class DlgPointEdit : public AQDoubleSpinBox
{
    Q_OBJECT

public:
    DlgPointEdit(EdgePoly & epoly, int row, int col, QWidget * parent);

    void focusInEvent(QFocusEvent *) override;

signals:
    void currentPoint(QPointF pt);
    void sig_updateView();

public slots:
    void slot_updatePoint();

protected slots:
    void slot_menu(QPointF spt);
    void slot_down();
    void slot_nearest();
    void slot_up();
    void slot_undo();
    void slot_editingFinished();

private:
    int row;
    int col;
    EdgePoly & _poly;
    QPointF    _orig;
};

#endif
