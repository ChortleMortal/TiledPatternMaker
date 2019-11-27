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

#include "page_debug.h"
#include "panels/dlg_name.h"
#include "base/qtapplog.h"
#include "base/canvas.h"
#include "base/tilingmanager.h"
#include "base/fileservices.h"
#include "base/tiledpatternmaker.h"
#include "panels/layout_sliderset.h"
#include "panels/panel.h"
#include "base/shared.h"
#include "tile/Tiling.h"

page_debug:: page_debug(ControlPanel * cpanel)  : panel_page(cpanel,"Debug Tools")
{
    QPushButton * btnVerTileNames       = new QPushButton("Verify Tile Names");
    QPushButton * reformatDesXMLBtn     = new QPushButton("Reformat All Design XML");
    QPushButton * reformatTileXMLBtn    = new QPushButton("Reformat All Tiling XML");
    QPushButton * reprocessDesXMLBtn    = new QPushButton("Reprocess All Design XML");
    QPushButton * reprocessTileXMLBtn   = new QPushButton("Reprocess All Tiling XML");

    QPushButton * compareDir0Btn        = new QPushButton("Select Compare Dir");
    QPushButton * compareDir1Btn        = new QPushButton("Select Compare Dir");
    QPushButton * swapBtn               = new QPushButton("Swap");

    comp0                               = new QLineEdit();
    comp1                               = new QLineEdit();

    QCheckBox   * cbAutoCycle           = new QCheckBox("AutoCycle");
    QCheckBox   * cbStopIfDiff          = new QCheckBox("Stop if Diff");

    SpinSet     * spCycleInterval       = new SpinSet("Cycle Interval",0,0,9);
    cycleCombo                          = new QComboBox();
    QPushButton * cycleBtn              = new QPushButton("Cycle");

    cbGridModel                         = new QCheckBox("Grid Model");
    gridWidth                           = new DoubleSpinSet("GridWidth",1.0,0.0001,900);
    gridWidth->setDecimals(8);

    QPushButton * pbClearCanvas         = new QPushButton("Clear Canvas");
    QPushButton * pbRenderLoaded        = new QPushButton("Render Loaded");
    QPushButton * pbRenderWS            = new QPushButton("Render Workspace");
    QPushButton * pbDrainAll            = new QPushButton("Drain The Swamp");
    QPushButton * pbClearWS         =    new QPushButton("Clear WS");

    QGridLayout * grid1 = new QGridLayout();
    grid1->setHorizontalSpacing(51);
    int row = 0;
    grid1->addWidget(btnVerTileNames,       row,0);
    grid1->addWidget(pbClearCanvas,         row,1);
    grid1->addWidget(pbDrainAll,            row,2);
    row++;
    grid1->addWidget(reformatDesXMLBtn,     row,0);
    grid1->addWidget(reprocessDesXMLBtn,    row,1);
    grid1->addWidget(cbGridModel,           row,2);
    row++;
    grid1->addWidget(reformatTileXMLBtn,    row,0);
    grid1->addWidget(reprocessTileXMLBtn,   row,1);
    grid1->addLayout(gridWidth,             row,2,Qt::AlignLeft);
    row++;
    grid1->addWidget(pbRenderLoaded,    row,0);
    grid1->addWidget(pbRenderWS,        row,1);
    grid1->addWidget(pbClearWS,         row,2);
    row++;

    QGroupBox * debugGroup = new QGroupBox("Debug");
    debugGroup->setLayout(grid1);

    QHBoxLayout * hbox00 = new QHBoxLayout;
    hbox00->addLayout(spCycleInterval);
    hbox00->addStretch();
    hbox00->addWidget(cycleCombo);
    hbox00->addStretch();
    hbox00->addWidget(cycleBtn);

    QHBoxLayout * hbox0 = new QHBoxLayout;
    hbox0->addWidget(compareDir0Btn);
    hbox0->addWidget(comp0);

    QHBoxLayout * hbox1 = new QHBoxLayout;
    hbox1->addWidget(compareDir1Btn);
    hbox1->addWidget(comp1);

    QHBoxLayout * hbox2 = new QHBoxLayout;
    hbox2->addWidget(cbAutoCycle);
    hbox2->addWidget(cbStopIfDiff);
    hbox2->addStretch(1);
    hbox2->addWidget(swapBtn);

    QVBoxLayout * cycleLayout = new QVBoxLayout;
    cycleLayout->addLayout(hbox00);
    cycleLayout->addLayout(hbox0);
    cycleLayout->addLayout(hbox1);
    cycleLayout->addLayout(hbox2);
    QGroupBox * cycleGroup = new  QGroupBox("Cycles");
    cycleGroup->setLayout(cycleLayout);

    cycleCombo->addItem(sCycleMode[CYCLE_NONE],             CYCLE_NONE);
    cycleCombo->addItem(sCycleMode[CYCLE_STYLES],           CYCLE_STYLES);
    cycleCombo->addItem(sCycleMode[CYCLE_TILINGS],          CYCLE_TILINGS);
    cycleCombo->addItem(sCycleMode[CYCLE_ORIGINAL_PNGS],    CYCLE_ORIGINAL_PNGS);
    cycleCombo->addItem(sCycleMode[CYCLE_SAVE_STYLE_BMPS],  CYCLE_SAVE_STYLE_BMPS);
    cycleCombo->addItem(sCycleMode[CYCLE_SAVE_TILING_BMPS], CYCLE_SAVE_TILING_BMPS);
    cycleCombo->addItem(sCycleMode[CYCLE_COMPARE_IMAGES],   CYCLE_COMPARE_IMAGES);

    imageName0      = new QLineEdit();
    imageName0->setMinimumWidth(461);
    imageName1      = new QLineEdit();
    imageCompareResult = new QLineEdit();
    selectImage0    = new QPushButton("Select");
    viewImage0      = new QPushButton("View");
    selectImage1    = new QPushButton("Select");
    viewImage1      = new QPushButton("View");
    compareImage    = new QPushButton("Compare Images");
    transparent     = new QCheckBox("Transparent");
    differences     = new QCheckBox("Differences");
    ping_pong       = new QCheckBox("Ping-pong");
    side_by_side    = new QCheckBox("Side-by-side");

    imageCompareResult->setReadOnly(true);

    QGridLayout * imageGrid = new QGridLayout();
    imageGrid->addWidget(selectImage0,3,0);
    imageGrid->addWidget(imageName0,3,1);
    imageGrid->addWidget(viewImage0,3,2);

    imageGrid->addWidget(selectImage1,4,0);
    imageGrid->addWidget(imageName1,4,1);
    imageGrid->addWidget(viewImage1,4,2);

    imageGrid->addWidget(compareImage,5,0);
    imageGrid->addWidget(imageCompareResult,5,1);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(transparent);
    hbox->addWidget(differences);
    hbox->addWidget(ping_pong);
    hbox->addWidget(side_by_side);
    imageGrid->addLayout(hbox,6,1);

    QGroupBox * imageGroup = new QGroupBox("Images");
    imageGroup->setLayout(imageGrid);


    cbAutoCycle->setChecked(config->autoCycle);
    cbStopIfDiff->setChecked(config->stopIfDiff);
    spCycleInterval->setValue(config->cycleInterval);
    int index = cycleCombo->findData(config->cycleMode);
    cycleCombo->setCurrentIndex(index);
    comp0->setText(config->compareDir0);
    comp1->setText(config->compareDir1);
    transparent->setChecked(config->compare_transparent);
    differences->setChecked(config->compare_differences);
    ping_pong->setChecked(config->compare_ping_pong);
    side_by_side->setChecked(config->compare_side_by_side);

    connect(btnVerTileNames,        &QPushButton::clicked,     this,   &page_debug::slot_verifyTilingNames);
    connect(reformatDesXMLBtn,      &QPushButton::clicked,     this,   &page_debug::slot_reformatDesignXML);
    connect(reformatTileXMLBtn,     &QPushButton::clicked,     this,   &page_debug::slot_reformatTilingXML);
    connect(reprocessDesXMLBtn,     &QPushButton::clicked,     this,   &page_debug::slot_reprocessDesignXML);
    connect(reprocessTileXMLBtn,    &QPushButton::clicked,     this,   &page_debug::slot_reprocessTilingXML);
    connect(pbClearCanvas,          &QPushButton::clicked,     workspace,  &Workspace::slot_clearCanvas);
    connect(pbRenderLoaded,         &QPushButton::clicked,     maker,      &TiledPatternMaker::slot_renderLoaded);
    connect(pbRenderWS,             &QPushButton::clicked,     maker,      &TiledPatternMaker::slot_renderWS);
    connect(pbDrainAll,             &QPushButton::clicked,     canvas,     &Canvas::drainTheSwamp);
    connect(pbClearWS,              &QPushButton::clicked,     workspace,  &Workspace::slot_clearWorkspace);


    connect(cbAutoCycle,            &QCheckBox::clicked,       this,   &page_debug::slot_autoCycleClicked);
    connect(cbGridModel,            &QCheckBox::clicked,       this,   &page_debug::slot_gridModelClicked);
    connect(cbStopIfDiff,           &QCheckBox::clicked,       this,   &page_debug::slot_stopIfDiffClicked);
    connect(transparent,            &QCheckBox::clicked,       this,   &page_debug::slot_transparentClicked);
    connect(differences,            &QCheckBox::clicked,       this,   &page_debug::slot_differencesClicked);
    connect(ping_pong,              &QCheckBox::clicked,       this,   &page_debug::slot_ping_pongClicked);
    connect(side_by_side,           &QCheckBox::clicked,       this,   &page_debug::slot_side_by_sideClicked);
    connect(spCycleInterval,        &SpinSet::valueChanged,    this,   &page_debug::slot_cycleIntervalChanged);
    connect(cycleCombo,             SIGNAL(currentIndexChanged(int)), this, SLOT(slot_cycleModeChanged(int)));
    connect(compareDir0Btn,         &QPushButton::clicked,     this,  &page_debug::selectDir0);
    connect(compareDir1Btn,         &QPushButton::clicked,     this,  &page_debug::selectDir1);
    connect(swapBtn,                &QPushButton::clicked,     this,  &page_debug::swapDirs);
    connect(gridWidth,              &DoubleSpinSet::valueChanged, this,&page_debug::slot_gridWidthChanged);
    connect(maker,          &TiledPatternMaker::sig_compareResult,this, &page_debug::slot_compareResult);


    connect(selectImage0,           SIGNAL(clicked()),         this,   SLOT(slot_selectImage0()));
    connect(viewImage0,             SIGNAL(clicked()),         this,   SLOT(slot_viewImage0()));
    connect(selectImage1,           SIGNAL(clicked()),         this,   SLOT(slot_selectImage1()));
    connect(viewImage1,             SIGNAL(clicked()),         this,   SLOT(slot_viewImage1()));
    connect(compareImage,           &QPushButton::clicked,     this,   &page_debug::slot_compareImages);

    connect(cycleBtn,               &QPushButton::clicked,      canvas, &Canvas::sig_cyclerStart);
    connect(cycleBtn,               &QPushButton::clicked,      this,  &page_debug::slot_startCycle);
    connect(this,           &page_debug::sig_view_image,        maker, &TiledPatternMaker::slot_view_image);
    connect(this,           &page_debug::sig_compareImageFiles, maker, &TiledPatternMaker::slot_compareImagesReplace, Qt::QueuedConnection);

    vbox->addWidget(debugGroup);
    vbox->addWidget(cycleGroup);
    vbox->addWidget(imageGroup);
    vbox->addStretch();
}

void  page_debug::onEnter()
{
    imageName0->setText(config->image0);
    imageName1->setText(config->image1);
    imageCompareResult->setText("");

    cbGridModel->setChecked(config->fgdGridModel);
    if (config->fgdGridModel)
    {
        gridWidth->setLabel("Model Units:");
        gridWidth->setValue(config->fgdGridStepModel);
    }
    else
    {
        gridWidth->setLabel("Screen Units:");
        gridWidth->setValue(config->fgdGridStepScreen);
    }    
    panel->setStatus("");
}

void page_debug::onExit()
{
    panel->setStatus("");
}

void  page_debug::refreshPage()
{
}

void  page_debug::slot_autoCycleClicked(bool enb)
{
    config->autoCycle = enb;
}

void page_debug::slot_stopIfDiffClicked(bool enb)
{
    config->stopIfDiff = enb;
}

void page_debug::slot_verifyTilingNames()
{
    TilingManager * tm = TilingManager::getInstance();
    bool rv = tm->verifyNameFiles();

    QMessageBox box;
    if (rv)
    {
        box.setIcon(QMessageBox::Information);
        box.setText("Tiling Names Verified: OK");
    }
    else
    {
        box.setIcon(QMessageBox::Warning);
        box.setText("ERROR in verifying tiling names. See log");
    }
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}

void page_debug::slot_reformatDesignXML()
{
    QMessageBox box;
    box.setIcon(QMessageBox::Question);
    box.setText("Reformat XML: this is very drastic. Are you sure?");
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::No);
    if (box.exec() == QMessageBox::No)
    {
        return;
    }

    qDebug() << "Reformatting designs...";

    int goodDes = 0;
    int badDes  = 0;
    QStringList files = FileServices::getDesignFiles();
    for (int i=0; i < files.size(); i++)
    {
        bool rv =  FileServices::reformatXML(files[i]);
        if (rv)
            goodDes++;
        else
            badDes++;
    }
    qDebug() << "Reformatted" << goodDes << "good designs, " << badDes << "bad designs";

    QMessageBox box2;
    box2.setIcon(QMessageBox::Information);
    box2.setText(QString("Reformat XML: %1 good designs, %2 bad designs").arg(goodDes).arg(badDes));
    box2.setStandardButtons(QMessageBox::Ok);
    box2.exec();
}

void page_debug::slot_reformatTilingXML()
{
    QMessageBox box;
    box.setIcon(QMessageBox::Question);
    box.setText("Reformat XML: this is very drastic. Are you sure?");
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::No);
    if (box.exec() == QMessageBox::No)
    {
        return;
    }
    int badTiles  = 0;
    int goodTiles = 0;

    qDebug() << "Reformatting tilings...";

    QStringList files = FileServices::getTilingFiles();
    for (int i=0; i < files.size(); i++)
    {
        bool rv =  FileServices::reformatXML(files[i]);
        if (rv)
            goodTiles++;
        else
            badTiles++;
    }
    qDebug() << "Reformatted" << goodTiles << "good tilings, " << badTiles << "bad tilings";

    QMessageBox box2;
    box2.setIcon(QMessageBox::Information);
    box2.setText(QString("Reformat XML: %1 good tilings, %2 bad tilings").arg(goodTiles).arg(badTiles));
    box2.setStandardButtons(QMessageBox::Ok);
    box2.exec();
}

void page_debug::slot_reprocessDesignXML()
{
    QMessageBox box;
    box.setIcon(QMessageBox::Question);
    box.setText("Reprocessing Design XML: this is very drastic. Are you sure?");
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::No);
    if (box.exec() == QMessageBox::No)
    {
        return;
    }

    qDebug() << "Reprocessing designs...";

    int goodDes = 0;
    int badDes  = 0;
    QStringList files = FileServices::getDesignNames();
    for (int i=0; i < files.size(); i++)
    {
        QString name = files[i];
        bool rv = workspace->loadDesignXML(name);
        if (rv)
        {
            QString outfile;
            rv = workspace->saveDesignXML(name,outfile,true);
        }
        if (rv)
            goodDes++;
        else
            badDes++;
    }

    qDebug() << "Reprocessed" << goodDes << "good designs, " << badDes << "bad designs";

    QMessageBox box2;
    box2.setIcon(QMessageBox::Information);
    box2.setText(QString("Reprocess Design XML: %1 good designs, %2 bad designs").arg(goodDes).arg(badDes));
    box2.setStandardButtons(QMessageBox::Ok);
    box2.exec();
}

void page_debug::slot_reprocessTilingXML()
{
    QMessageBox box;
    box.setIcon(QMessageBox::Question);
    box.setText("Reprocssing Tiling XML: this is very drastic. Are you sure?");
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::No);
    if (box.exec() == QMessageBox::No)
    {
        return;
    }
    int badTiles  = 0;
    int goodTiles = 0;

    qDebug() << "Reprocessing tilings...";

    TilingManager * tm = TilingManager::getInstance();

    QStringList files = FileServices::getTilingNames();
    for (int i=0; i < files.size(); i++)
    {
        bool rv = false;

        QString name = files[i];

        TilingPtr tp = tm->loadTiling(name);
        if (tp)
        {
            Q_ASSERT(tp->getName() == name);
            rv = tp->writeTilingXML();
        }
        if (rv)
            goodTiles++;
        else
            badTiles++;
    }
    qDebug() << "Reformatted" << goodTiles << "good tilings, " << badTiles << "bad tilings";

    QMessageBox box2;
    box2.setIcon(QMessageBox::Information);
    box2.setText(QString("Reformat XML: %1 good tilings, %2 bad tilings").arg(goodTiles).arg(badTiles));
    box2.setStandardButtons(QMessageBox::Ok);
    box2.exec();
}

void page_debug::slot_cycleIntervalChanged(int value)
{
    config->cycleInterval = value;
}

void page_debug::slot_cycleModeChanged(int row)
{
    Q_UNUSED(row)
    int mode  = cycleCombo->currentData().toInt();
    config->cycleMode = static_cast<eCycleMode>(mode);
}

void page_debug::selectDir0()
{
    QStringList qsl = config->compareDir0.split("/");
    int n = qsl.size();
    QString dir;
    for (int i=0; i < n-1; i++)
    {
        dir += qsl[i];
        dir += "/";
    }

    QString fdir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), dir,
                                                 QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (fdir.isEmpty())
            return;
    config->compareDir0 = fdir;
    comp0->setText(fdir);
}

void page_debug::selectDir1()
{
    QStringList qsl = config->compareDir1.split("/");
    int n = qsl.size();
    QString dir;
    for (int i=0; i < n-1; i++)
    {
        dir += qsl[i];
        dir += "/";
    }

    QString fdir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), dir,
                                                 QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (fdir.isEmpty())
            return;
    config->compareDir1 = fdir;
    comp1->setText(fdir);
}

void page_debug::swapDirs()
{
    QString a = config->compareDir0;
    QString b = config->compareDir1;
    config->compareDir0 = b;
    config->compareDir1 = a;
    comp0->setText(b);
    comp1->setText(a);
}

void page_debug::slot_selectImage0()
{
    imageCompareResult->setText("");

    QString old = imageName0->text();
    QString file = QFileDialog::getOpenFileName(this,"Select image file",old, tr("Image Files (*.png *.jpg *.bmp)"));

    if (!file.isNull())
    {
        QFileInfo info(file);
        config->image0 = info.absoluteFilePath();
        imageName0->setText(config->image0);
        //slot_viewImage0();
    }
}

void page_debug::slot_viewImage0()
{
    qDebug() << "slot_viewImage0";
    QString file = imageName0->text();
    config->image0 = file;
    ViewImage(file);
}

void page_debug::slot_selectImage1()
{
    imageCompareResult->setText("");

    QString old = config->image1;
    QString file = QFileDialog::getOpenFileName(this,"Select image file",old, tr("Image Files (*.png *.jpg *.bmp)"));

    if (!file.isNull())
    {
        QFileInfo info(file);
        config->image1 = info.absoluteFilePath();
        imageName1->setText(config->image1);
        //slot_viewImage1();
    }
}

void page_debug::slot_viewImage1()
{
    qDebug() << "slot_viewImage1";
    QString file = imageName1->text();
    config->image1 = file;
    ViewImage(file);
}

void page_debug::ViewImage(QString file)
{
    imageCompareResult->setText("");

    QPixmap pixmap(file);
    if (pixmap.isNull())
    {
        QMessageBox box(this);
        box.setText("Image not found");
        box.exec();
        return;
    }


    emit sig_view_image(file);
}

void page_debug::slot_compareImages()
{
    imageCompareResult->setText("");
    emit sig_compareImageFiles(imageName0->text(),imageName1->text());
}

void page_debug::slot_transparentClicked(bool checked)
{
    config->compare_transparent = checked;
}

void page_debug::slot_differencesClicked(bool checked)
{
    config->compare_differences = checked;
}

void page_debug::slot_ping_pongClicked(bool checked)
{
    config->compare_ping_pong = checked;
}

void page_debug::slot_side_by_sideClicked(bool checked)
{
    config->compare_side_by_side = checked;
}

void page_debug::slot_gridModelClicked(bool enb)
{
    config->fgdGridModel = enb;
    if (enb)
    {
        gridWidth->setLabel("Model Units:");
        gridWidth->setValue(config->fgdGridStepModel);
    }
    else
    {
        gridWidth->setLabel("Screen Units:");
        gridWidth->setValue(config->fgdGridStepScreen);
    }
    canvas->update();
}

void page_debug::slot_gridWidthChanged(qreal value)
{
    if (config->fgdGridModel)
    {
        config->fgdGridStepModel = value;
    }
    else
    {
        config->fgdGridStepScreen = static_cast<int>(value);
    }
    canvas->update();
}

void page_debug::slot_compareResult(QString result)
{
    imageCompareResult->setText(result);
}

void page_debug::slot_startCycle()
{
    if (config->cycleMode == CYCLE_COMPARE_IMAGES)
    {
        panel->setStatus("L=log  V=view Q=quit Spacebar=next");
    }
}

void page_debug::slot_setImage0(QString name)
{
    imageName0->setText(name);
}

void page_debug::slot_setImage1(QString name)
{
    imageName1->setText(name);
}
