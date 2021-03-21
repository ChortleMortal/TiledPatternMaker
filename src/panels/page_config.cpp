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

#include "panels/page_config.h"
#include "panels/layout_sliderset.h"
#include "base/configuration.h"
#include "viewers/view.h"
#include "base/tiledpatternmaker.h"
#include "panels/panel.h"

page_config:: page_config(ControlPanel * cpanel)  : panel_page(cpanel,"Configuration")
{
    le_rootDesigns = new QLineEdit();
    le_rootImages  = new QLineEdit();
    le_xmlTool     = new QLineEdit();

    le_rootDesigns->setMinimumWidth(501);

    rootDesignsBtn = new QPushButton("Root Design Dir");
    rootImagesBtn  = new QPushButton("Root Image Dir");
    xmlToolBtn     = new QPushButton("XML Viewer/Editor");

    defaultDesigns = new QCheckBox("Default Designs root");
    defaultImages  = new QCheckBox("Default Images  root");

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(defaultDesigns);
    hbox->addWidget(defaultImages);
    hbox->addStretch();

    QGridLayout *configGrid = new QGridLayout();

    int row = 0;
    configGrid->addLayout(hbox,row,1);

    row++;
    configGrid->addWidget(rootDesignsBtn,row,0);
    configGrid->addWidget(le_rootDesigns,row,1);

    row++;
    configGrid->addWidget(rootImagesBtn,row,0);
    configGrid->addWidget(le_rootImages,row,1);

    row++;
    configGrid->addWidget(xmlToolBtn,row,0);
    configGrid->addWidget(le_xmlTool,row,1);

    QGroupBox * pathGroup = new QGroupBox("Media File Paths");
    pathGroup->setLayout(configGrid);

    QRadioButton * designerMode = new QRadioButton("Designer Mode (reccomended)");
    QRadioButton * insightMode  = new QRadioButton("Insight Mode");
    QButtonGroup * btnGroup     = new QButtonGroup;
    btnGroup->addButton(designerMode,0);
    btnGroup->addButton(insightMode,1);

    int button  = (config->insightMode) ? 1 : 0;
    btnGroup->button(button)->setChecked(true);

    hbox = new QHBoxLayout;
    hbox->addWidget(designerMode);
    hbox->addWidget(insightMode);
    hbox->addStretch();

    QGroupBox * userMode = new QGroupBox("User Mode");
    userMode->setLayout(hbox);

    QGroupBox    * vctrl  = createViewControl();

    // put it all together
    vbox->addWidget(userMode);
    vbox->addWidget(pathGroup);
    vbox->addWidget(vctrl);

    updatePaths();

    connect(rootImagesBtn,      &QPushButton::clicked,   this,  &page_config::selectRootImageDir);
    connect(rootDesignsBtn,     &QPushButton::clicked,   this,  &page_config::selectRootDesignDir);
    connect(xmlToolBtn,         &QPushButton::clicked,   this,  &page_config::selectXMLTool);

    connect(le_rootDesigns, &QLineEdit::textChanged,   this, &page_config::rootDesignChanged);
    connect(le_rootImages,  &QLineEdit::textChanged,   this, &page_config::rootImageChanged);

    connect(defaultDesigns, &QCheckBox::clicked, this, &page_config::designDefaultChanged);
    connect(defaultImages,  &QCheckBox::clicked, this, &page_config::imageDefaultChanged);

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(btnGroup,      SIGNAL(buttonClicked(int)), this, SLOT(slot_mode(int)));
#else
    connect(btnGroup,      &QButtonGroup::idClicked,  this,  &page_config::slot_mode);
#endif
}

QGroupBox * page_config::createViewControl()
{
    QGroupBox * ViewControlBox = new QGroupBox("View Control");

    QCheckBox   * chkSplit      = new QCheckBox("Split Screen");

    QCheckBox   * showCenterChk = new QCheckBox("Show Center");
    QPushButton * btnPrimary    = new QPushButton("Move to Primary Screen");

    QVBoxLayout * avbox = createGridSection();

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(showCenterChk);
    hbox->addWidget(chkSplit);
    hbox->addSpacing(5);
    hbox->addWidget(btnPrimary);
    hbox->addStretch();

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addLayout(avbox);

    ViewControlBox->setLayout(vbox);

    chkSplit->setChecked(config->splitScreen);
    showCenterChk->setChecked(config->showCenterDebug);

    connect(btnPrimary,     &QPushButton::clicked,    theApp,   &TiledPatternMaker::slot_bringToPrimaryScreen);
    connect(chkSplit,       &QPushButton::clicked,    theApp,   &TiledPatternMaker::slot_splitScreen);
    connect(showCenterChk,  &QCheckBox::stateChanged, this,     &page_config::slot_showCenterChanged);

    return ViewControlBox;
}

QVBoxLayout *  page_config::createGridSection()
{
    gridBox = new QGroupBox("Show Grid");
    gridBox->setCheckable(true);

    QHBoxLayout  * gridTypeLayout      = createGridTypeLayout();

    QRadioButton * gridScreen          = new QRadioButton("Screen");
    QCheckBox    * gridScreenCentered  = new QCheckBox("Centered");
    SpinSet      * gridScreenSpacing   = new SpinSet("Spacing",100,10,990);
    SpinSet      * gridScreenWidth     = new SpinSet("Width",config->gridScreenWidth,1,9);

    QRadioButton * gridModel           = new QRadioButton("Model");
    QCheckBox    * gridModelCentered   = new QCheckBox("Centered");
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

    connect(&gridUnitGroup,     &QButtonGroup::idClicked,     this, &page_config::slot_gridUnitsChanged);
    connect(gridBox,            &QGroupBox::clicked,          this, &page_config::slot_showGridChanged);
    connect(gridScreenSpacing,  &SpinSet::valueChanged,       this, &page_config::slot_gridScreenSpacingChanged);
    connect(gridModelSpacing,   &DoubleSpinSet::sig_valueChanged, this, &page_config::slot_gridModelSpacingChanged);
    connect(gridScreenWidth,    &SpinSet::valueChanged,       this, &page_config::slot_gridScreenWidthChanged);
    connect(gridModelWidth,     &SpinSet::valueChanged,       this, &page_config::slot_gridModelWidthChanged);
    connect(gridScreenCentered, &QCheckBox::stateChanged,     this, &page_config::slot_gridScreenCenteredChanged);
    connect(gridModelCentered,  &QCheckBox::stateChanged,     this, &page_config::slot_gridModelCenteredChanged);

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

    // put it together
    QVBoxLayout  * vbox = new QVBoxLayout();
    vbox->addWidget(gridBox);

    return vbox;
}

QHBoxLayout * page_config::createGridTypeLayout()
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

    connect(&gridTypeGroup, &QButtonGroup::idClicked,         this, &page_config::slot_gridTypeSelected);
    connect(angleSpin,      &DoubleSpinSet::sig_valueChanged, this, &page_config::slot_gridAngleChanged);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(glabel);
    hbox->addWidget(btnOrthog);
    hbox->addWidget(btnIso);
    hbox->addWidget(btnRhom);
    hbox->addLayout(angleSpin);

    return hbox;
}

void  page_config::onEnter()
{
}

void  page_config::refreshPage()
{
    gridBox->setChecked(config->showGrid);
}

void page_config::selectRootDesignDir()
{
    QString old = config->rootMediaDir;
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Root Media Directory"),
                   old, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty())
    {
        qDebug() << "Cancel Pressed";
        config->defaultMediaRoot = true;
        return;
    }

    le_rootDesigns->setText(dir);
    config->rootMediaDir = dir;
    config->defaultMediaRoot = false;
    slot_reconfigurePaths();
}

void page_config::selectRootImageDir()
{
    QString old = config->rootImageDir;
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Image Directory"),
                   old, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);


    if (dir.isEmpty())
    {
        qDebug() << "Cancel Pressed";
        config->defaultImageRoot = true;
        return;
    }
    if (!dir.endsWith("/"))
    {
        dir += "/";
    }

    le_rootImages->setText(dir);
    config->rootImageDir = dir;
    config->defaultImageRoot = false;
    updatePaths();
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
    if (!config->defaultMediaRoot)
    {
        slot_reconfigurePaths();    // reboot
    }
}

void page_config::rootImageChanged(QString txt)
{
    config->rootImageDir = txt;
    if (!config->defaultImageRoot)
    {
        config->configurePaths();   // does not reboot
    }
    updatePaths();
}

void page_config::designDefaultChanged(bool checked)
{
    config->defaultMediaRoot = checked;
    slot_reconfigurePaths();        // reboots
}

void page_config::imageDefaultChanged(bool checked)
{
    config->defaultImageRoot = checked;
    config->configurePaths();       // does not reboot
    updatePaths();
}

void page_config::slot_reconfigurePaths()
{
    config->configurePaths();
    config->save();

    qApp->quit();
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
}

void page_config::updatePaths()
{
    // basic config
    defaultDesigns->setChecked(config->defaultMediaRoot);
    defaultImages->setChecked(config->defaultImageRoot);

    le_rootDesigns->setText(config->rootMediaDir);
    le_rootImages->setText(config->rootImageDir);
    le_xmlTool->setText(config->xmlTool);

    update();
}

void page_config::slot_mode(int id)
{
    config->insightMode = (id == 1);

    // restart the application
    qApp->quit();
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
}

void page_config::slot_showGridChanged(bool checked)
{
    config->showGrid = checked;
    emit sig_refreshView();
}

void page_config::slot_gridModelSpacingChanged(qreal value)
{
    config->gridModelSpacing = value;
    view->update();
}

void page_config::slot_gridScreenSpacingChanged(int value)
{
    config->gridScreenSpacing = value;
    view->update();
}

void page_config::slot_gridScreenWidthChanged(int value)
{
    config->gridScreenWidth = value;
    view->update();
}

void page_config::slot_gridModelWidthChanged(int value)
{
    config->gridModelWidth = value;
    view->update();
}

void page_config::slot_gridUnitsChanged(int id)
{
    config->gridUnits = eGridUnits(id);
    view->update();
    onEnter();
}

void page_config::slot_showCenterChanged(int id)
{
    config->showCenterDebug = (id == Qt::Checked);
    view->update();
}

void page_config::slot_gridScreenCenteredChanged(int id)
{
    config->gridScreenCenter = (id == Qt::Checked);
    view->update();
}

void page_config::slot_gridModelCenteredChanged(int id)
{
    config->gridModelCenter = (id == Qt::Checked);
    view->update();
}

void page_config::slot_gridTypeSelected(int type)
{
    config->gridType = eGridType(type);
    view->update();
    onEnter();
}

void page_config::slot_gridAngleChanged(qreal angle)
{
    config->gridAngle = angle;
    view->update();
    onEnter();
}
