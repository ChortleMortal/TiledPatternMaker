#ifndef DLG_FEATURE_EDIT_H
#define DLG_FEATURE_EDIT_H

#include <QtWidgets>
#include "base/shared.h"
#include "base/canvas.h"
#include "panels/layout_sliderset.h"
#include "geometry/edgepoly.h"

class DlgLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    DlgLineEdit(EdgePoly & epoly, int row, int col);

    void focusInEvent(QFocusEvent *) override;

signals:
    void currentPoint(QPointF pt);

public slots:
    void slot_update();

protected slots:
    void slot_menu(QPointF spt);
    void slot_down();
    void slot_nearest();
    void slot_up();
    void slot_undo();
    void slot_editingFinished();

private:
    Canvas * canvas;

    int row;
    int col;
    EdgePoly & _poly;
    QPointF    _orig;
};

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
