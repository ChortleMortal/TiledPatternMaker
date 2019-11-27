#ifndef DLG_FEATURE_EDIT_H
#define DLG_FEATURE_EDIT_H

#include <QtWidgets>
#include "base/shared.h"
#include "base/canvas.h"
#include "panels/layout_sliderset.h"
#include "geometry/edgepoly.h"

class DlgFeatureEdit : public QDialog
{
    Q_OBJECT

public:
    DlgFeatureEdit(EdgePoly & epoly, QTransform t, QWidget * parent = nullptr);
    ~DlgFeatureEdit();
signals:
    void sig_update();
    void sig_currentPoint(QPointF pt);

protected slots:
    void slot_undo();
    void slot_applyDeltas();
    void slot_currentPoint(QPointF pt);

protected:
    void display();

private:
    EdgePoly &    epoly;
    QPolygonF     original;
    QTransform    T;
    QTableWidget  table;

    DoubleSpinSet * deltaX;
    DoubleSpinSet * deltaY;
    QLabel        * label;

};

#endif // DLG_FEATURE_EDIT_H
