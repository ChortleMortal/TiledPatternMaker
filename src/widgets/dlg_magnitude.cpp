#include <QPushButton>

#include "widgets/dlg_magnitude.h"
#include "widgets/layout_sliderset.h"
#include "geometry/edge.h"
#include "makers/tiling_maker/tile_selection.h"

DlgMagnitude::DlgMagnitude(TilingSelectorPtr sel, QWidget * parent) :  QDialog(parent)
{
    this->sel = sel;
    edge = sel->getModelEdge();

    qreal mag = edge->getArcMagnitude();

    magWidget = new DoubleSliderSet("Magnitude",mag,0.0,5.0,1000);
    magWidget->setPrecision(4);

    QPushButton * doneBtn  = new QPushButton("Quit");

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addLayout(magWidget);
    vbox->addWidget(doneBtn);

    setLayout(vbox);

    setFixedWidth(750);

    connect(doneBtn,   &QPushButton::clicked,          this, &QDialog::accept);
    connect(magWidget, &DoubleSliderSet::valueChanged, this, &DlgMagnitude::slot_valueChanged);
}

void DlgMagnitude::slot_valueChanged(qreal val)
{
    edge->setArcMagnitude(val);
    emit sig_magnitudeChanged();
}


