/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "panels/page_grid.h"
#include "panels/layout_sliderset.h"
#include "settings/configuration.h"
#include "viewers/view.h"
#include "base/tiledpatternmaker.h"
#include "panels/panel.h"
#include "base/shared.h"
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

    QRadioButton * gridScreen          = new QRadioButton("Screen");
    QCheckBox    * gridScreenCentered  = new QCheckBox("Model Centered");
    SpinSet      * gridScreenSpacing   = new SpinSet("Spacing",100,10,990);
    SpinSet      * gridScreenWidth     = new SpinSet("Width",config->gridScreenWidth,1,9);

    QRadioButton * gridModel           = new QRadioButton("Model");
    QCheckBox    * gridModelCentered   = new QCheckBox("Layer Centered");
    DoubleSpinSet* gridModelSpacing    = new DoubleSpinSet("Spacing",1.0,0.0001,900);
    SpinSet      * gridModelWidth      = new SpinSet("Width",config->gridModelWidth,1,9);

    gridModel->setFixedWidth(71);
    gridScreen->setFixedWidth(71);

    gridModelSpacing->setDecimals(8);
    gridModelSpacing->setSingleStep(0.01);

    gridUnitGroup.addButton(gridScreen,GRID_UNITS_SCREEN);
    gridUnitGroup.addButton(gridModel,GRID_UNITS_MODEL);

    // initial values

    gridBox->setChecked(config->showGrid);
    gridUnitGroup.button(config->gridUnits)->setChecked(true);

    gridScreenCentered->setChecked(config->gridScreenCenter);
    gridScreenSpacing->setValue(config->gridScreenSpacing);
    gridScreenWidth->setValue(config->gridScreenWidth);
    gridModelCentered->setChecked(config->gridModelCenter);
    gridModelSpacing->setValue(config->gridModelSpacing);
    gridModelWidth->setValue(config->gridModelWidth);

    connect(&gridUnitGroup,     &QButtonGroup::idClicked,     this, &page_grid::slot_gridUnitsChanged);
    connect(gridBox,            &QGroupBox::clicked,          this, &page_grid::slot_showGridChanged);
    connect(gridScreenSpacing,  &SpinSet::valueChanged,       this, &page_grid::slot_gridScreenSpacingChanged);
    connect(gridModelSpacing,   &DoubleSpinSet::sig_valueChanged, this, &page_grid::slot_gridModelSpacingChanged);
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
    gridTypeGroup.addButton(btnOrthog,GRID_ORTHOGONAL);
    gridTypeGroup.addButton(btnIso,GRID_ISOMETRIC);
    gridTypeGroup.addButton(btnRhom,GRID_RHOMBIC);

    gridTypeGroup.button(config->gridType)->setChecked(true);
    angleSpin->setValue(config->gridAngle);

    connect(&gridTypeGroup, &QButtonGroup::idClicked,         this, &page_grid::slot_gridTypeSelected);
    connect(angleSpin,      &DoubleSpinSet::sig_valueChanged, this, &page_grid::slot_gridAngleChanged);

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

void  page_grid::refreshPage()
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
