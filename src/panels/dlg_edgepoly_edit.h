#ifndef DLG_EDGE_POLY_EDIT_H
#define DLG_EDGE_POLY_EDIT_H

#include <QtWidgets>
#include "base/shared.h"
#include "panels/layout_sliderset.h"
#include "panels/panel_misc.h"
#include "geometry/edgepoly.h"

class DlgEdgePolyEdit : public QDialog
{
    Q_OBJECT

public:
    DlgEdgePolyEdit(EdgePoly & epoly, QTransform t, QWidget * parent = nullptr);
    ~DlgEdgePolyEdit();

signals:
    void sig_update();
    void sig_currentPoint(QPointF pt);

public slots:
    void display();

protected slots:
    void slot_ok();
    void slot_undo();
    void slot_applyDeltas();
    void slot_moveUp();
    void slot_moveDown();
    void slot_currentPoint(QPointF pt);

private:
    EdgePoly &    epoly;
    QPolygonF     original;
    QTransform    T;
    AQTableWidget  table;

    DoubleSpinSet * deltaX;
    DoubleSpinSet * deltaY;
    QLabel        * label;
};

#endif // DLG_EDGE_POLY_EDIT_H
