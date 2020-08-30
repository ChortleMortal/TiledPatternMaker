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

    QRadioButton * designerMode = new QRadioButton("Designer Mode");
    QRadioButton * nerdMode     = new QRadioButton("Nerd Mode");
    QButtonGroup * btnGroup     = new QButtonGroup;
    btnGroup->addButton(designerMode,0);
    btnGroup->addButton(nerdMode,1);

    int button  = (config->nerdMode) ? 1 : 0;
    btnGroup->button(button)->setChecked(true);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(designerMode);
    hbox->addWidget(nerdMode);
    hbox->addStretch();
    vbox->addLayout(hbox);

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

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(btnGroup,      SIGNAL(buttonClicked(int)), this, SLOT(slot_mode(int)));
#else
    connect(btnGroup,      &QButtonGroup::idClicked,  this,  &page_config::slot_mode);
#endif
}

void  page_config::onEnter()
{
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

void page_config::slot_mode(int id)
{
    config->nerdMode = (id == 1);

    // restart the application
    qApp->quit();
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
}
