#include <QPushButton>

#include "gui/widgets/dlg_magnitude.h"
#include "gui/widgets/layout_sliderset.h"
#include "sys/geometry/edge.h"
#include  "gui/model_editors/tiling_edit/tile_selection.h"

DlgMagnitude::DlgMagnitude(PlacedTileSelectorPtr sel, QWidget * parent) :  QDialog(parent)
{
    this->sel = sel;
    edge = sel->getModelEdge();

    qreal mag = edge->getArcMagnitude();

    magWidget = new DoubleSliderSet("Magnitude",mag,0.0,5.0,1000);

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
    ArcData & ad = edge->getArcData();
    ad.setArcMagnitude(val);

    emit sig_magnitudeChanged();
}


