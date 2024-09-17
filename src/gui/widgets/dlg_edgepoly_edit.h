#pragma once
#ifndef DLG_EDGE_POLY_EDIT_H
#define DLG_EDGE_POLY_EDIT_H

#include "gui/panels/panel_misc.h"
#include "sys/geometry/edgepoly.h"

typedef std::shared_ptr<class Tile>  TilePtr;

class DlgEdgePolyEdit : public QDialog
{
    Q_OBJECT

public:
    DlgEdgePolyEdit(TilePtr tile, QTransform t, QWidget * parent = nullptr);
    ~DlgEdgePolyEdit();

signals:
    void sig_update();
    void sig_currentPoint(QPointF pt);
    void sig_updateView();

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
    TilePtr       tile;
    EdgePoly &    epoly;
    EdgePoly      original2;
    QTransform    T;
    AQTableWidget  table;

    class DoubleSpinSet * deltaX;
    class DoubleSpinSet * deltaY;
    QLabel              * label;
};

#endif // DLG_EDGE_POLY_EDIT_H
