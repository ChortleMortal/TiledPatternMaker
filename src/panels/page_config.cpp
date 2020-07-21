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

#include "page_config.h"
#include "panels/layout_sliderset.h"

page_config:: page_config(ControlPanel * cpanel)  : panel_page(cpanel,"Configuration")
{
    le_rootTile   = new QLineEdit();
    le_newTile    = new QLineEdit();
    le_rootDesign = new QLineEdit();
    le_newDesign  = new QLineEdit();
    le_newDesign->setMinimumWidth(501);
    le_rootImage  = new QLineEdit();
    le_examples  = new QLineEdit();
    le_xmlTool  = new QLineEdit();

    rootDesignBtn = new QPushButton("Root Design Dir");
    newDesignBtn  = new QPushButton("New Design Dir");
    rootTileBtn   = new QPushButton("Root Tile Dir");
    newTileBtn    = new QPushButton("New Tile Dir");
    examplesBtn   = new QPushButton("Examples Dir");
    rootImageBtn  = new QPushButton("Image Dir");
    xmlToolBtn  = new QPushButton("XML Viewer/Editor");
    QPushButton *reconfigurePathsBtn = new QPushButton("Reset Paths");

    QGridLayout *configGrid = new QGridLayout();

    int row = 0;
    configGrid->addWidget(rootDesignBtn,row,0);
    configGrid->addWidget(le_rootDesign,row,1);

    row++;
    configGrid->addWidget(newDesignBtn,row,0);
    configGrid->addWidget(le_newDesign,row,1);

    row++;
    configGrid->addWidget(rootTileBtn,row,0);
    configGrid->addWidget(le_rootTile,row,1);

    row++;
    configGrid->addWidget(newTileBtn,row,0);
    configGrid->addWidget(le_newTile,row,1);

    row++;
    configGrid->addWidget(examplesBtn,row,0);
    configGrid->addWidget(le_examples,row,1);

    row++;
    configGrid->addWidget(rootImageBtn,row,0);
    configGrid->addWidget(le_rootImage,row,1);

    row++;
    configGrid->addWidget(xmlToolBtn,row,0);
    configGrid->addWidget(le_xmlTool,row,1);

    row++;
    configGrid->addWidget(reconfigurePathsBtn,row,0);

    vbox->addLayout(configGrid);

    updatePaths();

    connect(rootImageBtn,   SIGNAL(clicked()),                  this,   SLOT(selectRootImageDir()));
    connect(rootTileBtn,    SIGNAL(clicked()),                  this,   SLOT(selectRootTileDir()));
    connect(newTileBtn,     SIGNAL(clicked()),                  this,   SLOT(selectNewTileDir()));
    connect(rootDesignBtn,  SIGNAL(clicked()),                  this,   SLOT(selectRootDesignDir()));
    connect(newDesignBtn,   SIGNAL(clicked()),                  this,   SLOT(selectNewDesignDir()));
    connect(examplesBtn,    SIGNAL(clicked()),                  this,   SLOT(selectExamplesDir()));
    connect(xmlToolBtn,     SIGNAL(clicked()),                  this,   SLOT(selectXMLTool()));
    connect(reconfigurePathsBtn, SIGNAL(clicked()),             this,   SLOT(slot_reconfigurePaths()));

    connect(le_rootDesign, &QLineEdit::textChanged,   this, &page_config::rootDesignChanged);
    connect(le_newDesign,  &QLineEdit::textChanged,   this, &page_config::newDesignChanged);
    connect(le_rootTile,   &QLineEdit::textChanged,   this, &page_config::rootTileChanged);
    connect(le_newTile,    &QLineEdit::textChanged,   this, &page_config::newTtileChanged);
    connect(le_rootImage,  &QLineEdit::textChanged,   this, &page_config::rootImageChanged);
    connect(le_examples,   &QLineEdit::textChanged,   this, &page_config::examplesChanged);

    QCheckBox    * hideBackImage       = new QCheckBox("Hide background image");
    QCheckBox    * showCenterChk       = new QCheckBox("Show Center");
    scaleSceneToView                   = new QCheckBox("Scale Scene to View");

    QHBoxLayout * hbox2 = new QHBoxLayout;
    hbox2->addWidget(hideBackImage);
    hbox2->addWidget(showCenterChk);
    hbox2->addWidget(scaleSceneToView);
    hbox2->addStretch();

    QLabel       * gridLabel           = new QLabel("Grid:");
    QRadioButton * gridScreen          = new QRadioButton("Screen");
    QRadioButton * gridModel           = new QRadioButton("Model");
    gridSpacing                        = new DoubleSpinSet("Spacing",1.0,0.0001,900);
    gridSpacing->setDecimals(8);
    QCheckBox    * gridCentered        = new QCheckBox("Centered");
    SpinSet      * gridWidth           = new SpinSet("Width",config->gridWidth,1,9);

    showCenterChk->setChecked(config->showCenter);
    gridCentered->setChecked(config->gridCenter);

    // map editor group
    gridModelGroup.addButton(gridScreen,GRID_SCREEN);
    gridModelGroup.addButton(gridModel,GRID_MODEL);
    gridModelGroup.button(config->gridModel)->setChecked(true);

    connect(&gridModelGroup,    SIGNAL(buttonClicked(int)),   this,  SLOT(slot_gridModel_pressed(int)));
    connect(gridSpacing,        &DoubleSpinSet::valueChanged, this, &page_config::slot_gridSpacingChanged);
    connect(gridWidth,          &SpinSet::valueChanged,       this, &page_config::slot_gridWidthChanged);
    connect(showCenterChk,      &QCheckBox::stateChanged,     this, &page_config::slot_showCenterChanged);
    connect(scaleSceneToView,      &QCheckBox::stateChanged,  this, &page_config::slot_scaleSceneToView);
    connect(hideBackImage,      &QCheckBox::stateChanged,     this, &page_config::slot_hideBackChanged);
    connect(gridCentered,       &QCheckBox::stateChanged,     this, &page_config::slot_centeredChanged);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(gridLabel);
    hbox->addWidget(gridScreen);
    hbox->addWidget(gridModel);
    hbox->addLayout(gridSpacing);
    hbox->addWidget(gridCentered);
    hbox->addLayout(gridWidth);
    hbox->addStretch();

    QCheckBox   * cbVerifyMaps          = new QCheckBox("Verify Maps");
    QCheckBox   * cbVerifyDump          = new QCheckBox("Dump Map");
    QCheckBox   * cbVerifyVerbose       = new QCheckBox("Verbose");

    vbox->addSpacing(9);
    vbox->addLayout(hbox2);
    vbox->addLayout(hbox);
    vbox->addWidget(cbVerifyMaps);

    hbox = new QHBoxLayout();
    hbox->addSpacing(15);
    hbox->addWidget(cbVerifyDump);
    vbox->addLayout(hbox);

    hbox = new QHBoxLayout();
    hbox->addSpacing(15);
    hbox->addWidget(cbVerifyVerbose);
    vbox->addLayout(hbox);

    cbVerifyMaps->setChecked(config->verifyMaps);
    cbVerifyDump->setChecked(config->verifyDump);
    cbVerifyVerbose->setChecked(config->verifyVerbose);

    connect(cbVerifyMaps,   &QCheckBox::clicked,    this,   &page_config::slot_verifyMapsClicked);
    connect(cbVerifyDump,   &QCheckBox::clicked,    this,   &page_config::slot_verifyDumpClicked);
    connect(cbVerifyVerbose,&QCheckBox::clicked,    this,   &page_config::slot_verifyVerboseClicked);

}

void  page_config::onEnter()
{
    scaleSceneToView->setChecked(config->scaleSceneToView);
    gridModelGroup.button(config->gridModel)->setChecked(true);
    switch (config->gridModel)
    {
    case GRID_MODEL:
        gridSpacing->setLabel("Model Units:");
        gridSpacing->setValue(config->gridStepModel);
        gridSpacing->setSingleStep(0.01);
        break;
    case GRID_SCREEN:
        gridSpacing->setLabel("Screen Units:");
        gridSpacing->setValue(config->gridStepScreen);
        gridSpacing->setSingleStep(1.0);
        break;
    }
}

void  page_config::refreshPage()
{

}

void page_config::selectRootTileDir()
{
    QString old = config->rootTileDir;
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Tiling Directory"),
                   old, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    le_rootTile->setText(dir);
    config->rootTileDir = dir;
}

void page_config::selectNewTileDir()
{
    QString old = config->newTileDir;
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Tiling Directory"),
                   old, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    le_newTile->setText(dir);
    config->newTileDir = dir;
}

void page_config::selectRootDesignDir()
{
    QString old = config->rootDesignDir;
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select XML Directory"),
                   old, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    le_rootDesign->setText(dir);
    config->rootDesignDir = dir;
}

void page_config::selectNewDesignDir()
{
    QString old = config->newDesignDir;
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select XML Directory"),
                   old, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    le_newDesign->setText(dir);
    config->newDesignDir = dir;
}

void page_config::selectRootImageDir()
{
    QString old = config->rootImageDir;
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Image Directory"),
                   old, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    le_rootImage->setText(dir);
    config->rootImageDir = dir;
}

void page_config::selectExamplesDir()
{
    QString old = config->examplesDir;
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Examples Directory"),
                                                    old, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    le_examples->setText(dir);
    config->examplesDir = dir;
}

void page_config::selectXMLTool()
{
    QString old = config->xmlTool;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select XML Editor/Viewer"), old);

    le_xmlTool->setText(fileName);
    config->xmlTool = fileName;
}

void page_config::rootDesignChanged(QString txt)
{
    config->rootDesignDir= txt;
}

void page_config::newDesignChanged(QString txt)
{
    config->newDesignDir = txt;
}

void page_config::rootTileChanged(QString txt)
{
    config->rootTileDir = txt;
}

void page_config::newTtileChanged(QString txt)
{
    config->newTileDir = txt;
}

void page_config::rootImageChanged(QString txt)
{
    config->rootImageDir = txt;
}

void page_config::examplesChanged(QString txt)
{
    config->examplesDir = txt;
}

void page_config::slot_reconfigurePaths()
{
    config->reconfigurePaths();
    updatePaths();
}

void page_config::updatePaths()
{
    // basic config
    le_rootDesign->setText(config->rootDesignDir);
    le_newDesign->setText(config->newDesignDir);
    le_rootTile->setText(config->rootTileDir);
    le_newTile->setText(config->newTileDir);
    le_rootImage->setText(config->rootImageDir);
    le_examples->setText(config->examplesDir);
    le_xmlTool->setText(config->xmlTool);
    update();
}

void  page_config::slot_verifyMapsClicked(bool enb)
{
    config->verifyMaps = enb;
}

void  page_config::slot_verifyDumpClicked(bool enb)
{
    config->verifyDump = enb;
}

void  page_config::slot_verifyVerboseClicked(bool enb)
{
    config->verifyVerbose = enb;
}

void page_config::slot_gridSpacingChanged(qreal value)
{
    switch(config->gridModel)
    {
    case GRID_MODEL:
        config->gridStepModel = value;
        break;
    case GRID_SCREEN:
        config->gridStepScreen = static_cast<int>(value);
        break;
    }
    view->update();
}

void page_config::slot_gridWidthChanged(int value)
{
    config->gridWidth = value;
    view->update();
}

void page_config::slot_gridModel_pressed(int id)
{
    config->gridModel = eGridModel(id);
    view->update();
    onEnter();
}

void page_config::slot_showCenterChanged(int id)
{
    config->showCenter = (id == Qt::Checked);
    view->update();
}

void page_config::slot_hideBackChanged(int id)
{
    config->hideBackgroundImage = (id == Qt::Checked);
    view->update();
}

void page_config::slot_centeredChanged(int id)
{
    config->gridCenter = (id == Qt::Checked);
    view->update();
}

void  page_config::slot_scaleSceneToView(int state)
{
    config->scaleSceneToView = (state == Qt::Checked);

}
