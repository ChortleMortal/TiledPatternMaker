#include <QHeaderView>

#include "gui/widgets/dlg_edgepoly_edit.h"
#include "gui/widgets/layout_sliderset.h"
#include "gui/widgets/dlg_line_edit.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/vertex.h"
#include "model/makers/tiling_maker.h"
#include "sys/sys.h"
#include "gui/top/view_controller.h"
#include "model/tilings/tile.h"

/////////////////////////////////////////////////////////////////
///
/// DlgEdgePolyEdit
///
/////////////////////////////////////////////////////////////////


DlgEdgePolyEdit::DlgEdgePolyEdit(TilePtr tile, QTransform t, QWidget *parent) : QDialog(parent), epoly(tile->getEdgePolyRW())
{
    this->tile = tile;
    T = t;
    original2 = epoly;    // for undo

    setAttribute(Qt::WA_DeleteOnClose);

    label = new QLabel;

    // global adjust
    deltaX = new DoubleSpinSet("Delta-X",0,-10,10);
    deltaY = new DoubleSpinSet("Delta-Y",0,-10,10);
    QPushButton   * refresh   = new QPushButton("Refresh");
    QPushButton   * moveUp    = new QPushButton("Move Up");
    QPushButton   * moveDown  = new QPushButton("Move Down");
    QPushButton   * apply     = new QPushButton("Apply");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addLayout(deltaX);
    hbox->addLayout(deltaY);
    hbox->addWidget(refresh);
    hbox->addWidget(moveUp);
    hbox->addWidget(moveDown);
    hbox->addWidget(apply);

    // table
    table.setRowCount(epoly.size());
    table.setColumnCount(7);
    QStringList qslH;
    qslH << "p1-x" << "p1-y" << "p2-x" << "p2-y" << "angle" << "arc center" << "convex" ;
    table.setHorizontalHeaderLabels(qslH);
    table.verticalHeader()->setVisible(true);
    table.horizontalHeader()->setVisible(true);
    table.setColumnWidth(0,151);
    table.setColumnWidth(1,151);
    table.setColumnWidth(2,151);
    table.setColumnWidth(3,151);
    table.setColumnWidth(4,151);
    table.setContextMenuPolicy(Qt::CustomContextMenu);
    table.setSelectionMode(QAbstractItemView::SingleSelection);
    table.setSelectionBehavior(QAbstractItemView::SelectRows);

    // buttons
    QPushButton * okbtn  = new QPushButton("OK");
    QPushButton * canbtn = new QPushButton("Undo All");
    QHBoxLayout * hbox2  = new QHBoxLayout();
    hbox2->addWidget(okbtn);
    hbox2->addWidget(canbtn);

    QVBoxLayout * vlayout= new QVBoxLayout();
    vlayout->addWidget(label);
    vlayout->addLayout(hbox);
    vlayout->addWidget(&table);
    vlayout->addLayout(hbox2);
    setLayout(vlayout);

    connect(okbtn,      &QPushButton::clicked, this, &DlgEdgePolyEdit::slot_ok);
    connect(canbtn,     &QPushButton::clicked, this, &DlgEdgePolyEdit::slot_undo);
    connect(apply,      &QPushButton::clicked, this, &DlgEdgePolyEdit::slot_applyDeltas);
    connect(moveUp,     &QPushButton::clicked, this, &DlgEdgePolyEdit::slot_moveUp);
    connect(moveDown,   &QPushButton::clicked, this, &DlgEdgePolyEdit::slot_moveDown);
    connect(refresh,    &QPushButton::clicked, this, &DlgEdgePolyEdit::display);
    connect(this,       &DlgEdgePolyEdit::sig_update, Sys::view, &View::slot_update);

    display();
}

DlgEdgePolyEdit::~DlgEdgePolyEdit()
{
    emit sig_currentPoint(QPointF());
}

void DlgEdgePolyEdit::display()
{
    QString str1 = (epoly.isClockwise()) ? "Clockwise: YES" : "Clockwise: NO";
    label->setText(str1);

    table.clearContents();
    for (int row = 0; row < epoly.size(); row++)
    {
        EdgePtr edge = epoly[row];
        QPointF pt   = edge->v1->pt;
        QPointF pt2  = edge->v2->pt;
        int col      = 0;

        DlgLineEdit * le = new DlgLineEdit(epoly,row,col);
        le->setText(QString::number(pt.x(),'g',16));
        table.setCellWidget(row,col,le);
        connect(this, &DlgEdgePolyEdit::sig_update, le,  &DlgLineEdit::slot_update);
        connect(le,   &DlgLineEdit::currentPoint,  this, &DlgEdgePolyEdit::slot_currentPoint);

        col++;
        le = new DlgLineEdit(epoly,row,col);
        le->setText(QString::number(pt.y(),'g',16));
        table.setCellWidget(row,col,le);
        connect(this, &DlgEdgePolyEdit::sig_update, le, &DlgLineEdit::slot_update);
        connect(le,   &DlgLineEdit::currentPoint,  this, &DlgEdgePolyEdit::slot_currentPoint);

        col++;
        QLineEdit * le2 = new QLineEdit();
        le2->setText(QString::number(pt2.x(),'g',16));
        le2->setReadOnly(true);
        table.setCellWidget(row,col,le2);

        col++;
        le2 = new QLineEdit;
        le2->setText(QString::number(pt2.y(),'g',16));
        le2->setReadOnly(true);
        table.setCellWidget(row,col,le2);

        col++;
        QString str = QString::number(epoly.getAngle(row),'g',16);
        QTableWidgetItem * item  = new QTableWidgetItem(str);
        table.setItem(row,col,item);

        if (edge->getType() == EDGETYPE_CURVE || edge->getType() == EDGETYPE_CHORD)
        {
            QPointF arcC = edge->getArcCenter();
            bool  convex = edge->isConvex();

            col++;
            QString str = QString("%1 , %2").arg(QString::number(arcC.x(),'g',16)).arg(QString::number(arcC.y(),'g',16));
            item = new QTableWidgetItem(str);
            table.setItem(row,col,item);

            col++;
            str = (convex) ? "convex" : "concave";
            item = new QTableWidgetItem(str);
            table.setItem(row,col,item);
        }
    }

    table.resizeColumnToContents(4);
    table.resizeColumnToContents(5);
    table.adjustTableSize();
    updateGeometry();
}

void DlgEdgePolyEdit::slot_ok()
{
    tile->decompose();
    Sys::tilingMaker->pushTileToPrototypeMaker(PROM_TILE_EDGES_CHANGED,tile);
    accept();
}

void DlgEdgePolyEdit::slot_undo()
{
    epoly = original2;

    emit sig_update();

    reject();
}

void DlgEdgePolyEdit::slot_applyDeltas()
{
    qreal dx = deltaX->value();
    qreal dy = deltaY->value();

    for (int i=0; i < epoly.size(); i++)
    {
        EdgePtr e = epoly[i];
        VertexPtr v1 = e->v1;
        v1->setPt(QPointF(v1->pt.x() + dx, v1->pt.y() + dy));
        // only do v1 since v2 will already be set
    }

    emit sig_update();
}

void DlgEdgePolyEdit::slot_currentPoint(QPointF pt)
{
    emit sig_currentPoint(T.map(pt));
}


void DlgEdgePolyEdit::slot_moveUp()
{
    int row = table.currentRow();
    if (row == -1) return;  // not selected
    if (row == 0) return;   // at top

    EdgePtr prev = epoly[row-1];
    EdgePtr cur  = epoly[row];

    epoly[row-1] = cur;
    epoly[row]   = prev;

    display();

    emit sig_update();
}

void DlgEdgePolyEdit::slot_moveDown()
{
    int row = table.currentRow();
    if (row == -1) return;  // not selected
    if (row == epoly.size()-1) return;   // at bottom

    EdgePtr next = epoly[row+1];
    EdgePtr cur  = epoly[row];

    epoly[row+1] = cur;
    epoly[row]   = next;

    display();

    emit sig_update();
}
