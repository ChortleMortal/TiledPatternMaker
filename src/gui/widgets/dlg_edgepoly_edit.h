#pragma once
#ifndef DLG_EDGE_POLY_EDIT_H
#define DLG_EDGE_POLY_EDIT_H

#include "gui/panels/panel_misc.h"
#include "model/makers/tiling_maker.h"
#include "sys/geometry/edge_poly.h"

typedef std::shared_ptr<class Tile>   TilePtr;
typedef std::shared_ptr<class Tiling> TilingPtr;

class DlgEdgePolyEdit : public QDialog
{
    Q_OBJECT

public:
    DlgEdgePolyEdit(TilingPtr tp, TilePtr tile, QTransform placement, QWidget * parent = nullptr);
    ~DlgEdgePolyEdit();

signals:
    void sig_updatePos();
    void sig_currentPoint(QPointF pt);
    void sig_updateView();

public slots:
    void display(eTileMenuRefresh refresh);

protected slots:
    void slot_ok();
    void slot_undo();
    void slot_moveUp();
    void slot_moveDown();
    void slot_currentPoint(QPointF pt);
    void slot_makeClockwise();

protected:
    void finish(bool good);

private:
    EdgePoly &    epoly;        // this is the epoly which is edited
    EdgePoly      original;

    TilingPtr     tiling;
    TilePtr       tile;

    QTransform    T;

    AQTableWidget table;
    QLabel      * cwLabel;
    QPushButton * makeCWBtn;

    bool          finished;
};

#endif // DLG_EDGE_POLY_EDIT_H
