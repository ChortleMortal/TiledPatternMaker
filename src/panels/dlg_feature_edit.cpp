#include "panels/dlg_feature_edit.h"
#include "tile/Feature.h"
#include "panels/panel_page.h"
#include "panels/layout_sliderset.h"
#include "base/utilities.h"

DlgLineEdit::DlgLineEdit(EdgePoly & epoly, int row, int col) : QLineEdit(),
  _poly(epoly)
{
    canvas = Canvas::getInstance();

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &DlgLineEdit::customContextMenuRequested, this, &DlgLineEdit::slot_menu);
    connect(this, &DlgLineEdit::editingFinished,  this, &DlgLineEdit::slot_editingFinished);
    this->row = row;
    this->col = col;

    _orig = _poly[row]->getV1()->getPosition();
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
    emit currentPoint(_poly[row]->getV1()->getPosition());
}

void  DlgLineEdit::slot_down()
{
    VertexPtr vp = _poly[row]->getV1();
    QPointF pos  = vp->getPosition();
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
    vp->setPosition(pos);
    emit currentPoint(pos);
    canvas->update();
}

void  DlgLineEdit::slot_nearest()
{
    VertexPtr vp = _poly[row]->getV1();
    QPointF pos  = vp->getPosition();
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
    vp->setPosition(pos);
    emit currentPoint(pos);
    canvas->update();
}

void  DlgLineEdit::slot_up()
{
    VertexPtr vp = _poly[row]->getV1();
    QPointF pos  = vp->getPosition();
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
    vp->setPosition(pos);
    emit currentPoint(pos);
    canvas->update();
}

void DlgLineEdit::slot_undo()
{
    VertexPtr vp = _poly[row]->getV1();
    QPointF pos  = vp->getPosition();
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
    vp->setPosition(pos);
    emit currentPoint(pos);
    canvas->update();
}

void DlgLineEdit::slot_editingFinished()
{
    VertexPtr vp = _poly[row]->getV1();
    QPointF pos  = vp->getPosition();
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
        vp->setPosition(pos);
        emit currentPoint(pos);
        canvas->update();
    }
    else
    {
        slot_undo();
    }
}

void DlgLineEdit::slot_update()
{
    VertexPtr vp = _poly[row]->getV1();
    QPointF pos  = vp->getPosition();
    if (col == 0)
    {
        setText(QString::number(pos.x(),'g',16));
    }
    else
    {
        setText(QString::number(pos.y(),'g',16));
    }
}


/////////////////////////////////////////////////////////////////
///
/// DlgFeatureEdit::DlgFeatureEdit
///
/////////////////////////////////////////////////////////////////


DlgFeatureEdit::DlgFeatureEdit(EdgePoly & epoly, QTransform t, QWidget *parent) : QDialog(parent), epoly(epoly)
{
    T = t;
    original = epoly.getPoly();    // for undo

    setAttribute(Qt::WA_DeleteOnClose);

    label = new QLabel;

    // global adjust
    deltaX = new DoubleSpinSet("Delta-X",0,-10,10);
    deltaY = new DoubleSpinSet("Delta-Y",0,-10,10);
    QPushButton   * apply  = new QPushButton("Apply");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addLayout(deltaX);
    hbox->addLayout(deltaY);
    hbox->addWidget(apply);

    // table
    table.setRowCount(epoly.size());
    table.setColumnCount(6);
    QStringList qslH;
    qslH << "p1-x" << "p1-y" << "p2-x" << "p2-y" << "arc center" << "convex" ;
    table.setHorizontalHeaderLabels(qslH);
    table.verticalHeader()->setVisible(true);
    table.horizontalHeader()->setVisible(true);
    table.setColumnWidth(0,151);
    table.setColumnWidth(1,151);
    table.setColumnWidth(2,151);
    table.setColumnWidth(3,151);
    table.setContextMenuPolicy(Qt::CustomContextMenu);
    table.setSelectionMode(QAbstractItemView::SingleSelection);

    // buttons
    QPushButton * okbtn = new QPushButton("OK");
    QPushButton * canbtn = new QPushButton("Undo All");
    QHBoxLayout * hbox2 = new QHBoxLayout();
    hbox2->addWidget(okbtn);
    hbox2->addWidget(canbtn);

    QVBoxLayout * vlayout= new QVBoxLayout();
    vlayout->addWidget(label);
    vlayout->addLayout(hbox);
    vlayout->addWidget(&table);
    vlayout->addLayout(hbox2);
    setLayout(vlayout);

    connect(okbtn,  &QPushButton::clicked, this, &DlgFeatureEdit::accept);
    connect(canbtn, &QPushButton::clicked, this, &DlgFeatureEdit::slot_undo);
    connect(apply,  &QPushButton::clicked, this, &DlgFeatureEdit::slot_applyDeltas);

    display();
}

DlgFeatureEdit::~DlgFeatureEdit()
{
    emit sig_currentPoint(QPointF());
}

void DlgFeatureEdit::display()
{
    QPolygonF poly = epoly.getPoly();

    QString str1 = (Utils::isClockwise(poly)) ? "IS clockwise" : "NOT clockwise";
    //QString str2 = (Utils::isClockwiseKaplan(poly)) ? "IS clockwise" : "NOT clockwise";
    //label->setText(QString("%1  %2").arg(str1).arg(str2));
    label->setText(str1);

    table.clearContents();
    for (int row = 0; row < epoly.size(); row++)
    {
        EdgePtr edge = epoly[row];
        QPointF pt   = edge->getV1()->getPosition();
        QPointF pt2  = edge->getV2()->getPosition();
        int col      = 0;

        DlgLineEdit * le = new DlgLineEdit(epoly,row,col);
        le->setText(QString::number(pt.x(),'g',16));
        table.setCellWidget(row,col,le);
        connect(this, &DlgFeatureEdit::sig_update, le,   &DlgLineEdit::slot_update);
        connect(le,   &DlgLineEdit::currentPoint,  this, &DlgFeatureEdit::slot_currentPoint);

        col++;
        le = new DlgLineEdit(epoly,row,col);
        le->setText(QString::number(pt.y(),'g',16));
        table.setCellWidget(row,col,le);
        connect(this, &DlgFeatureEdit::sig_update, le, &DlgLineEdit::slot_update);
        connect(le,   &DlgLineEdit::currentPoint,  this, &DlgFeatureEdit::slot_currentPoint);

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

        if (edge->getType() == EDGE_CURVE)
        {
            QPointF arcC = edge->getArcCenter();
            bool  convex = edge->isConvex();

            col++;
            QString str = QString("%1 , %2").arg(QString::number(arcC.x(),'g',16)).arg(QString::number(arcC.y(),'g',16));
            QTableWidgetItem * item = new QTableWidgetItem(str);
            table.setItem(row,col,item);

            col++;
            str = (convex) ? "convex" : "concave";
            item = new QTableWidgetItem(str);
            table.setItem(row,col,item);
        }
    }

    table.resizeColumnToContents(4);
    table.resizeColumnToContents(5);
    panel_page::adjustTableSize(&table);
    updateGeometry();
}

void DlgFeatureEdit::slot_undo()
{
    epoly = original;

    Canvas * canvas = Canvas::getInstance();
    canvas->update();

    reject();
}

void DlgFeatureEdit::slot_applyDeltas()
{
    qreal dx = deltaX->value();
    qreal dy = deltaY->value();

    for (int i=0; i < epoly.size(); i++)
    {
        EdgePtr e = epoly[i];
        VertexPtr v1 = e->getV1();
        v1->setPosition(QPointF(v1->getPosition().x() + dx, v1->getPosition().y() + dy));
        // only do v1 since v2 will already be set
    }

    emit sig_update();
}

void DlgFeatureEdit::slot_currentPoint(QPointF pt)
{
    emit sig_currentPoint(T.map(pt));
}


