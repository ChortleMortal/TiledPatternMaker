#include <QGroupBox>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QRadioButton>
#include <QCheckBox>

#include "panels/page_grid.h"
#include "widgets/layout_sliderset.h"
#include "settings/configuration.h"
#include "viewers/view.h"
#include "panels/panel.h"
#include "viewers/grid.h"

page_grid:: page_grid(ControlPanel * cpanel)  : panel_page(cpanel,"Grid")
{
    QGroupBox * gbox  = createGridSection();
    vbox->addWidget(gbox);
}

QGroupBox *  page_grid::createGridSection()
{
    gridBox = new QGroupBox("Show Grid");
    gridBox->setCheckable(true);

    QHBoxLayout  * gridTypeLayout      = createGridTypeLayout();

    QRadioButton * gridScreen          = new QRadioButton("Screen Units");
    QCheckBox    * gridScreenCentered  = new QCheckBox("Model Centered");
    SpinSet      * gridScreenSpacing   = new SpinSet("Spacing",100,10,990);
    SpinSet      * gridScreenWidth     = new SpinSet("Width",config->gridScreenWidth,1,9);

    QRadioButton * gridModel           = new QRadioButton("Model Units");
    QCheckBox    * gridModelCentered   = new QCheckBox("Layer Centered");
    DoubleSpinSet* gridModelSpacing    = new DoubleSpinSet("Spacing",1.0,0.0001,900);
    SpinSet      * gridModelWidth      = new SpinSet("Width",config->gridModelWidth,1,9);

    gridScreen->setFixedWidth(91);
    gridScreenCentered->setFixedWidth(111);
    gridModel->setFixedWidth(91);
    gridModelCentered->setFixedWidth(111);

    gridModelSpacing->setDecimals(8);
    gridModelSpacing->setSingleStep(0.01);

    gridUnitGroup = new QButtonGroup;
    gridUnitGroup->addButton(gridScreen,GRID_UNITS_SCREEN);
    gridUnitGroup->addButton(gridModel,GRID_UNITS_MODEL);

    // initial values

    gridBox->setChecked(config->showGrid);
    gridUnitGroup->button(config->gridUnits)->setChecked(true);

    gridScreenCentered->setChecked(config->gridScreenCenter);
    gridScreenSpacing->setValue(config->gridScreenSpacing);
    gridScreenWidth->setValue(config->gridScreenWidth);
    gridModelCentered->setChecked(config->gridModelCenter);
    gridModelSpacing->setValue(config->gridModelSpacing);
    gridModelWidth->setValue(config->gridModelWidth);

    connect(gridUnitGroup ,     &QButtonGroup::idClicked,     this, &page_grid::slot_gridUnitsChanged);
    connect(gridBox,            &QGroupBox::clicked,          this, &page_grid::slot_showGridChanged);
    connect(gridScreenSpacing,  &SpinSet::valueChanged,       this, &page_grid::slot_gridScreenSpacingChanged);
    connect(gridModelSpacing,   &DoubleSpinSet::valueChanged, this, &page_grid::slot_gridModelSpacingChanged);
    connect(gridScreenWidth,    &SpinSet::valueChanged,       this, &page_grid::slot_gridScreenWidthChanged);
    connect(gridModelWidth,     &SpinSet::valueChanged,       this, &page_grid::slot_gridModelWidthChanged);
    connect(gridScreenCentered, &QCheckBox::stateChanged,     this, &page_grid::slot_gridScreenCenteredChanged);
    connect(gridModelCentered,  &QCheckBox::stateChanged,     this, &page_grid::slot_gridModelCenteredChanged);

    QVBoxLayout * vboxG = new QVBoxLayout();

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(gridScreen);
    hbox->addWidget(gridScreenCentered);
    hbox->addLayout(gridScreenWidth);
    hbox->addSpacing(7);
    hbox->addLayout(gridScreenSpacing);
    hbox->addStretch();
    vboxG->addLayout(hbox);

    hbox = new QHBoxLayout();
    hbox->addWidget(gridModel);
    hbox->addWidget(gridModelCentered);
    hbox->addLayout(gridModelWidth);
    hbox->addSpacing(7);
    hbox->addLayout(gridModelSpacing);
    hbox->addStretch();
    vboxG->addLayout(hbox);

    vboxG->addLayout(gridTypeLayout);

    gridBox->setLayout(vboxG);

    return gridBox;
}

QHBoxLayout * page_grid::createGridTypeLayout()
{
    QLabel       * glabel    = new QLabel("Grid Type:");
    QRadioButton * btnOrthog = new QRadioButton("Orthogonal");
    QRadioButton * btnIso    = new QRadioButton("Isometric");
    QRadioButton * btnRhom   = new QRadioButton("Rhombic");
    DoubleSpinSet* angleSpin = new DoubleSpinSet("Rhombus angle",30,0,180);

    gridTypeGroup = new QButtonGroup;
    gridTypeGroup->addButton(btnOrthog,GRID_ORTHOGONAL);
    gridTypeGroup->addButton(btnIso,GRID_ISOMETRIC);
    gridTypeGroup->addButton(btnRhom,GRID_RHOMBIC);

    gridTypeGroup->button(config->gridType)->setChecked(true);
    angleSpin->setValue(config->gridAngle);

    connect(gridTypeGroup,  &QButtonGroup::idClicked,     this, &page_grid::slot_gridTypeSelected);
    connect(angleSpin,      &DoubleSpinSet::valueChanged, this, &page_grid::slot_gridAngleChanged);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(glabel);
    hbox->addWidget(btnOrthog);
    hbox->addWidget(btnIso);
    hbox->addWidget(btnRhom);
    hbox->addLayout(angleSpin);

    return hbox;
}

void  page_grid::onEnter()
{
}

void  page_grid::onRefresh()
{
    gridBox->setChecked(config->showGrid);
}

void page_grid::slot_showGridChanged(bool checked)
{
    config->showGrid = checked;
    if (checked)
    {
        Grid::getSharedInstance()->create();
    }
    emit sig_refreshView();
}

void page_grid::slot_gridModelSpacingChanged(qreal value)
{
    config->gridModelSpacing = value;
    emit sig_refreshView();

}

void page_grid::slot_gridScreenSpacingChanged(int value)
{
    config->gridScreenSpacing = value;
    emit sig_refreshView();

}

void page_grid::slot_gridScreenWidthChanged(int value)
{
    config->gridScreenWidth = value;
    emit sig_refreshView();
}

void page_grid::slot_gridModelWidthChanged(int value)
{
    config->gridModelWidth = value;
    emit sig_refreshView();
}

void page_grid::slot_gridUnitsChanged(int id)
{
    config->gridUnits = eGridUnits(id);
    emit sig_refreshView();
}

void page_grid::slot_gridScreenCenteredChanged(int id)
{
    config->gridScreenCenter = (id == Qt::Checked);
    emit sig_refreshView();
}

void page_grid::slot_gridModelCenteredChanged(int id)
{
    config->gridModelCenter = (id == Qt::Checked);
    emit sig_refreshView();
}

void page_grid::slot_gridTypeSelected(int type)
{
    config->gridType = eGridType(type);
    emit sig_refreshView();
}

void page_grid::slot_gridAngleChanged(qreal angle)
{
    config->gridAngle = angle;
    emit sig_refreshView();
}
