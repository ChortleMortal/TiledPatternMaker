#include <QHeaderView>

#include "gui/top/system_view_controller.h"
#include "gui/widgets/dlg_edgepoly_edit.h"
#include "gui/widgets/dlg_point_edit.h"
#include "model/makers/tiling_maker.h"
#include "model/makers/prototype_maker.h"
#include "model/tilings/tile.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/vertex.h"
#include "sys/sys.h"

/////////////////////////////////////////////////////////////////
///
/// DlgEdgePolyEdit
///
/////////////////////////////////////////////////////////////////

DlgEdgePolyEdit::DlgEdgePolyEdit(TilingPtr tp, TilePtr tile, QTransform placement, QWidget *parent) : QDialog(parent), epoly(tile->getEdgePolyRW())
{
    tiling     = tp;
    this->tile = tile;
    T          = placement;
    original   = epoly;    // for undo
    finished   = false;

    // editing tile always makes it irregular unless undo is pressed
    if (tile->isRegular())
    {
        Sys::tilingMaker->flipTileRegularity(tile);
    }

    setAttribute(Qt::WA_DeleteOnClose);

    cwLabel   = new QLabel;
    makeCWBtn = new QPushButton("Make Clockwise");

    // global adjust
    QPushButton   * refresh   = new QPushButton("Refresh");
    QPushButton   * moveUp    = new QPushButton("Move Up");
    QPushButton   * moveDown  = new QPushButton("Move Down");

    // table
    table.setRowCount(epoly.size());
    table.setColumnCount(8);
    QStringList qslH;
    qslH << "p1-x" << "p1-y" << "p2-x" << "p2-y" << "angle" << "arc center" << "convex" << "magnitude" ;
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
    QPushButton * canbtn = new QPushButton("Cancel");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(cwLabel);
    hbox->addWidget(makeCWBtn);

    hbox->addStretch();
    hbox->addWidget(refresh);
    hbox->addWidget(moveUp);
    hbox->addWidget(moveDown);
    hbox->addStretch();
    hbox->addWidget(canbtn);
    hbox->addWidget(okbtn);

    QVBoxLayout * vlayout= new QVBoxLayout();
    vlayout->addWidget(&table);
    vlayout->addLayout(hbox);
    setLayout(vlayout);

    connect(okbtn,      &QPushButton::clicked, this, &DlgEdgePolyEdit::slot_ok);
    connect(canbtn,     &QPushButton::clicked, this, &DlgEdgePolyEdit::slot_undo);
    connect(moveUp,     &QPushButton::clicked, this, &DlgEdgePolyEdit::slot_moveUp);
    connect(moveDown,   &QPushButton::clicked, this, &DlgEdgePolyEdit::slot_moveDown);
    connect(refresh,    &QPushButton::clicked, this, [this] { display(TMR_PLACED_TILE);} );
    connect(makeCWBtn,  &QPushButton::clicked, this, &DlgEdgePolyEdit::slot_makeClockwise);
    connect(this,       &DlgEdgePolyEdit::sig_updateView,   Sys::viewController, &SystemViewController::slot_updateView);
    connect(this,       &DlgEdgePolyEdit::sig_currentPoint, Sys::tilingMakerView.get(), &TilingMakerView::slot_setTileEditPoint);
    connect(Sys::tilingMaker, &TilingMaker::sig_close_editor,this,&DlgEdgePolyEdit::slot_ok);
    connect(Sys::tilingMaker, &TilingMaker::sig_menuRefresh, this,&DlgEdgePolyEdit::display);

    display(TMR_PLACED_TILE);
}

DlgEdgePolyEdit::~DlgEdgePolyEdit()
{
    if (!finished)
    {
        finish(false);
    }
}

void DlgEdgePolyEdit::finish(bool good)
{
    if (good)
    {
        // the edited epoly (in the tile) has already been edited
        tile->decompose();
    }
    else
    {
        epoly = original;
    }
    if (Sys::tilingMaker->getPropagate())
    {
        ProtoEvent pevent;
        pevent.event  = PROM_TILE_UNIQUE_NATURE_CHANGED;
        pevent.tiling = tiling;
        pevent.tile   = tile;
        Sys::prototypeMaker->sm_takeUp(pevent);
    }
    Sys::tilingMaker->setEdgePolyEditor(nullptr);   // must be in this order
    emit sig_updateView();
    emit Sys::tilingMaker->sig_menuRefresh(TMR_PLACED_TILE);
    finished = true;
}

void DlgEdgePolyEdit::display(eTileMenuRefresh refresh)
{
    Q_UNUSED(refresh);

    if (epoly.isClockwise())
    {
        cwLabel->setText("Clockwise: YES");
        makeCWBtn->setEnabled(false);
        makeCWBtn->setStyleSheet("");
    }
    else
    {
        cwLabel->setText("Clockwise: NO");
        makeCWBtn->setEnabled(true);
        makeCWBtn->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    }

    table.clearContents();
    const EdgeSet & eset = epoly.get();
    for (uint row = 0; row < epoly.size(); row++)
    {
        EdgePtr edge = eset[row];
        QPointF pos  = edge->v1->pt;
        QPointF pos2 = edge->v2->pt;
        int col      = 0;

        DlgPointEdit * le = new DlgPointEdit(epoly,row,col,this);
        le->setValue(pos.x());
        table.setCellWidget(row,col,le);
        connect(this, &DlgEdgePolyEdit::sig_updatePos, le,   &DlgPointEdit::slot_updatePoint);
        connect(le,   &DlgPointEdit::currentPoint,     this, &DlgEdgePolyEdit::slot_currentPoint);

        col++;
        le = new DlgPointEdit(epoly,row,col,this);
        le->setValue(pos.y());
        table.setCellWidget(row,col,le);
        connect(this, &DlgEdgePolyEdit::sig_updatePos, le,   &DlgPointEdit::slot_updatePoint);
        connect(le,   &DlgPointEdit::currentPoint,     this, &DlgEdgePolyEdit::slot_currentPoint);

        col++;
        QLineEdit * le2 = new QLineEdit();
        le2->setText(QString::number(pos2.x(),'g',16));
        le2->setReadOnly(true);
        table.setCellWidget(row,col,le2);

        col++;
        le2 = new QLineEdit;
        le2->setText(QString::number(pos2.y(),'g',16));
        le2->setReadOnly(true);
        table.setCellWidget(row,col,le2);

        col++;
        QString str = QString::number(epoly.getAngle(row),'g',16);
        QTableWidgetItem * item  = new QTableWidgetItem(str);
        table.setItem(row,col,item);

        if (edge->getType() == EDGETYPE_CURVE)
        {
            QPointF arcC = edge->getArcCenter();

            col++;
            QString str = QString("%1 , %2").arg(QString::number(arcC.x(),'g',12)).arg(QString::number(arcC.y(),'g',12));
            item = new QTableWidgetItem(str);
            table.setItem(row,col,item);

            col++;
            str = (edge->getCurveType() == CURVE_CONVEX) ? "YES" : "NO";
            item = new QTableWidgetItem(str);
            table.setItem(row,col,item);

            col++;
            str = QString::number(edge->getArcData().magnitude(),'g',8);
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
    finish(true);
    close();
}

void DlgEdgePolyEdit::slot_undo()
{
    finish(false);
    close();
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

    EdgeSet & base = epoly.getBaseRW();
    EdgePtr prev   = base[row-1];
    EdgePtr cur    = base[row];

    base[row-1]    = cur;
    base[row]      = prev;
    epoly.compose();

    display(TMR_PLACED_TILE);

    emit sig_updatePos();
}

void DlgEdgePolyEdit::slot_moveDown()
{
    int row = table.currentRow();
    if (row == -1) return;  // not selected
    if (row == epoly.size()-1) return;   // at bottom

    EdgeSet & base = epoly.getBaseRW();
    EdgePtr next   = base[row+1];
    EdgePtr cur    = base[row];

    base[row+1]    = cur;
    base[row]      = next;
    epoly.compose();

    display(TMR_PLACED_TILE);

    emit sig_updatePos();
}

void DlgEdgePolyEdit::slot_makeClockwise()
{
    epoly.reverseWindingOrder();
    display(TMR_PLACED_TILE);
    emit sig_updatePos();
}
