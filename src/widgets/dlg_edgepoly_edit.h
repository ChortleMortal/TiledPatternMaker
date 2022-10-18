#ifndef DLG_EDGE_POLY_EDIT_H
#define DLG_EDGE_POLY_EDIT_H

#include "widgets/panel_misc.h"
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
    EdgePoly      original2;
    QTransform    T;
    AQTableWidget  table;

    class DoubleSpinSet * deltaX;
    class DoubleSpinSet * deltaY;
    QLabel              * label;
};

#endif // DLG_EDGE_POLY_EDIT_H
