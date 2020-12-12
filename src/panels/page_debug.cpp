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

#include "panels/page_debug.h"
#include "panels/dlg_name.h"
#include "base/qtapplog.h"
#include "base/fileservices.h"
#include "base/mosaic_manager.h"
#include "base/tiledpatternmaker.h"
#include "viewers/view.h"
#include "panels/layout_sliderset.h"
#include "panels/versioned_list_widget.h"
#include "panels/panel.h"
#include "base/shared.h"
#include "tile/tiling.h"
#include "tile/tiling_manager.h"
#include "viewers/viewcontrol.h"

page_debug:: page_debug(ControlPanel * cpanel)  : panel_page(cpanel,"Debug Tools")
{
     QGroupBox   * cycle = createCycleSection();
     QGroupBox   * image = createImagesSection();
     QGroupBox   * debug = createDebugSection();

     vbox->addWidget(cycle);
     vbox->addWidget(image);
     vbox->addWidget(debug);

     connect(theApp,&TiledPatternMaker::sig_compareResult, this,  &page_debug::slot_compareResult);
     connect(this,  &page_debug::sig_view_image,           theApp, &TiledPatternMaker::slot_view_image);
     connect(this,  &page_debug::sig_compareImageFiles,    theApp, &TiledPatternMaker::slot_compareImagesReplace, Qt::QueuedConnection);

     Cycler * cycler = Cycler::getInstance();
     connect(this,  &page_debug::sig_cyclerStart,          cycler,  &Cycler::slot_startCycle, Qt::QueuedConnection);
}

QGroupBox * page_debug::createDebugSection()
{
    QGroupBox   * maps  = createVerifyMaps();

    QPushButton * pbVerifyTileNames     = new QPushButton("Verify Tile Names");
    QPushButton * pbReformatDesXMLBtn   = new QPushButton("Reformat All Design XML");
    QPushButton * pbReformatTileXMLBtn  = new QPushButton("Reformat All Tiling XML");
    QPushButton * pbReprocessDesXMLBtn  = new QPushButton("Reprocess All Design XML");
    QPushButton * pbReprocessTileXMLBtn = new QPushButton("Reprocess All Tiling XML");
    QPushButton * pbRender              = new QPushButton("Render");
    QPushButton * pbClearMakers         = new QPushButton("Clear Makers");
    QPushButton * pbClearView           = new QPushButton("Clear View");

    QGridLayout * grid = new QGridLayout();
    grid->setHorizontalSpacing(51);

    int row = 0;
    grid->addWidget(pbReformatDesXMLBtn,   row,0);
    grid->addWidget(pbReprocessDesXMLBtn,  row,1);
    grid->addWidget(pbReformatTileXMLBtn,  row,2);
    grid->addWidget(pbReprocessTileXMLBtn, row,3);

    row++;
    grid->addWidget(pbClearMakers,         row,0);
    grid->addWidget(pbClearView,           row,1);
    grid->addWidget(pbRender,              row,2);
    grid->addWidget(pbVerifyTileNames,     row,3);

    QVBoxLayout * vbox = new QVBoxLayout;;
    vbox->addLayout(grid);
    vbox->addWidget(maps);

    QGroupBox * debugGroup = new QGroupBox("Debug");
    debugGroup->setLayout(vbox);

    connect(pbVerifyTileNames,        &QPushButton::clicked,     this,   &page_debug::slot_verifyTilingNames);
    connect(pbReformatDesXMLBtn,      &QPushButton::clicked,     this,   &page_debug::slot_reformatDesignXML);
    connect(pbReformatTileXMLBtn,     &QPushButton::clicked,     this,   &page_debug::slot_reformatTilingXML);
    connect(pbReprocessDesXMLBtn,     &QPushButton::clicked,     this,   &page_debug::slot_reprocessDesignXML);
    connect(pbReprocessTileXMLBtn,    &QPushButton::clicked,     this,   &page_debug::slot_reprocessTilingXML);
    connect(pbRender,                 &QPushButton::clicked,     this,   &panel_page::sig_render);
    connect(pbClearView,              &QPushButton::clicked,     vcontrol,  &ViewControl::slot_clearView);
    connect(pbClearMakers,            &QPushButton::clicked,     vcontrol,  &ViewControl::slot_clearMakers);

    return debugGroup;
}

QGroupBox * page_debug::createCycleSection()
{
    SpinSet     * spCycleInterval       = new SpinSet("Cycle Interval",0,0,9);
    QPushButton * cycleBtn              = new QPushButton("Cycle");

    QRadioButton * rStyles    = new QRadioButton("Mosaics");
    QRadioButton * rTiles     = new QRadioButton("Tilings");
    QRadioButton * rPngs      = new QRadioButton("PNGS");
    QRadioButton * rSavStyles = new QRadioButton("Save Mosaic BMPs");
    QRadioButton * rSavTiles  = new QRadioButton("Save Tiling BMPs");

    QButtonGroup * cycleGroup = new QButtonGroup;
    cycleGroup->addButton(rStyles,CYCLE_STYLES);
    cycleGroup->addButton(rTiles,CYCLE_TILINGS);
    cycleGroup->addButton(rPngs,CYCLE_ORIGINAL_PNGS);
    cycleGroup->addButton(rSavStyles,CYCLE_SAVE_STYLE_BMPS);
    cycleGroup->addButton(rSavTiles,CYCLE_SAVE_TILING_BMPS);

    if (config->cycleMode >= CYCLE_MIN && config->cycleMode <= CYCLE_MAX)
    {
        cycleGroup->button(config->cycleMode)->setChecked(true);
    }

    QHBoxLayout * hbox00 = new QHBoxLayout;
    hbox00->addWidget(rSavStyles);
    hbox00->addWidget(rSavTiles);
    hbox00->addStretch();
    hbox00->addLayout(spCycleInterval);
    hbox00->addWidget(rPngs);
    hbox00->addWidget(rStyles);
    hbox00->addWidget(rTiles);
    hbox00->addStretch();
    hbox00->addWidget(cycleBtn);

    QVBoxLayout * cycleLayout = new QVBoxLayout;
    cycleLayout->addLayout(hbox00);

    QGroupBox * cycleGroupBox = new  QGroupBox("Cycling");
    cycleGroupBox->setLayout(cycleLayout);

    spCycleInterval->setValue(config->cycleInterval);

    connect(spCycleInterval,    &SpinSet::valueChanged,    this,  &page_debug::slot_cycleIntervalChanged);
    connect(cycleBtn,           &QPushButton::clicked,     this,  &page_debug::slot_cycle);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(cycleGroup,         SIGNAL(buttonClicked(int)), this, SLOT(slot_cycleModeChanged(int)));
#else
    connect(cycleGroup,          &QButtonGroup::idClicked,  this,  &page_debug::slot_cycleModeChanged);
#endif
    return cycleGroupBox;
}

QGroupBox * page_debug::createImagesSection()
{
    ibox0      = new QComboBox();
    ibox0->setMinimumWidth(461);
    ibox1      = new QComboBox();
    imageCompareResult = new QLineEdit();
    imageCompareResult->setReadOnly(true);

    QPushButton * compareDir0Btn = new QPushButton("Compare Dir");
    QPushButton * compareDir1Btn = new QPushButton("Compare Dir");
    dir0                         = new QLineEdit();
    dir1                         = new QLineEdit();

    QPushButton * viewImage0   = new QPushButton("View");
    QPushButton * viewImage1   = new QPushButton("View");
    QPushButton * compareBtn   = new QPushButton("Compare");
    QPushButton * cycleBtn     = new QPushButton("Cycle");
    QPushButton * previousBtn  = new QPushButton("Previous");
    QPushButton * nextBtn      = new QPushButton("Next");

    QPushButton * swapBtn      = new QPushButton("Swap");
    QCheckBox   * cbStopIfDiff = new QCheckBox("Stop if Diff");
    QCheckBox   * transparent  = new QCheckBox("Transparent");
    QCheckBox   * differences  = new QCheckBox("Display Differences");
    QCheckBox   * ping_pong    = new QCheckBox("Ping-pong");
    QCheckBox   * side_by_side = new QCheckBox("Side-by-side");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(cbStopIfDiff);
    hbox->addWidget(transparent);
    hbox->addWidget(differences);
    hbox->addWidget(ping_pong);
    hbox->addWidget(side_by_side);
    hbox->addWidget(cycleBtn);

    QGridLayout * imageGrid = new QGridLayout();

    int row = 0;
    imageGrid->addLayout(hbox,row,0,1,3);

    row++;
    imageGrid->addWidget(compareDir0Btn,row,0);
    imageGrid->addWidget(dir0,row,1);
    imageGrid->addWidget(swapBtn,row,2);

    row++;
    imageGrid->addWidget(compareDir1Btn,row,0);
    imageGrid->addWidget(dir1,row,1);

    row++;
    imageGrid->addWidget(previousBtn,row,0);
    imageGrid->addWidget(ibox0,row,1);
    imageGrid->addWidget(viewImage0,row,2);

    row++;
    imageGrid->addWidget(nextBtn,row,0);
    imageGrid->addWidget(ibox1,row,1);
    imageGrid->addWidget(viewImage1,row,2);

    row++;
    imageGrid->addWidget(imageCompareResult,row,1);
    imageGrid->addWidget(compareBtn,row,2);

    QGroupBox * imageGroup = new QGroupBox("View/Compare Images");
    imageGroup->setLayout(imageGrid);

    QString dir = config->compareDir0;
    dir0->setText(dir);
    loadCombo(ibox0,dir);
    dir = config->compareDir1;
    dir1->setText(dir);
    loadCombo(ibox1,dir);

    cbStopIfDiff->setChecked(config->stopIfDiff);
    transparent->setChecked(config->compare_transparent);
    differences->setChecked(config->display_differences);
    ping_pong->setChecked(config->compare_ping_pong);
    side_by_side->setChecked(config->compare_side_by_side);

    connect(swapBtn,                &QPushButton::clicked,     this,  &page_debug::swapDirs);
    connect(compareDir0Btn,         &QPushButton::clicked,     this,  &page_debug::selectDir0);
    connect(compareDir1Btn,         &QPushButton::clicked,     this,  &page_debug::selectDir1);

    connect(viewImage0,             &QPushButton::clicked,     this,   &page_debug::slot_viewImage0);
    connect(viewImage1,             &QPushButton::clicked,     this,   &page_debug::slot_viewImage1);
    connect(compareBtn,             &QPushButton::clicked,     this,   &page_debug::slot_compareImages);
    connect(cycleBtn,               &QPushButton::clicked,     this,   &page_debug::slot_compareCycle);
    connect(previousBtn,            &QPushButton::clicked,     this,   &page_debug::slot_previous);
    connect(nextBtn,                &QPushButton::clicked,     this,   &page_debug::slot_next);

    connect(transparent,            &QCheckBox::clicked,       this,   &page_debug::slot_transparentClicked);
    connect(cbStopIfDiff,           &QCheckBox::clicked,       this,   &page_debug::slot_stopIfDiffClicked);
    connect(differences,            &QCheckBox::clicked,       this,   &page_debug::slot_differencesClicked);
    connect(ping_pong,              &QCheckBox::clicked,       this,   &page_debug::slot_ping_pongClicked);
    connect(side_by_side,           &QCheckBox::clicked,       this,   &page_debug::slot_side_by_sideClicked);

    connect(dir0, &QLineEdit::editingFinished, this, &page_debug::slot_dir0Changed);
    connect(dir1, &QLineEdit::editingFinished, this, &page_debug::slot_dir1Changed);
    connect(ibox0, SIGNAL(currentIndexChanged(int)),  this, SLOT(slot_ibox0_changed(int)));
    connect(ibox1, SIGNAL(currentIndexChanged(int)),  this, SLOT(slot_ibox1_changed(int)));

    setCombo(ibox0,config->image0);
    setCombo(ibox1,config->image1);

    return  imageGroup;
}

QGroupBox * page_debug::createVerifyMaps()
{
    QGroupBox * gbVerifyMaps = new QGroupBox("Verify Maps");
    gbVerifyMaps->setCheckable(true);

    QCheckBox * cbVerifyDump    = new QCheckBox("Dump Map");
    QCheckBox * cbVerifyVerbose = new QCheckBox("Verbose");

    QHBoxLayout * hverBox = new QHBoxLayout();
    hverBox->addSpacing(15);
    hverBox->addWidget(cbVerifyDump);
    hverBox->addWidget(cbVerifyVerbose);
    hverBox->addStretch();

    gbVerifyMaps->setLayout(hverBox);

    gbVerifyMaps->setChecked(config->verifyMaps);
    cbVerifyDump->setChecked(config->verifyDump);
    cbVerifyVerbose->setChecked(config->verifyVerbose);

    connect(gbVerifyMaps,   &QGroupBox::clicked,    this,   &page_debug::slot_verifyMapsClicked);
    connect(cbVerifyDump,   &QCheckBox::clicked,    this,   &page_debug::slot_verifyDumpClicked);
    connect(cbVerifyVerbose,&QCheckBox::clicked,    this,   &page_debug::slot_verifyVerboseClicked);

    return gbVerifyMaps;
}

void  page_debug::onEnter()
{
    imageCompareResult->setText("");
}

void page_debug::onExit()
{
    panel->hidePanelStatus();

    view->clearLayout();   // removes any cler pngs
    view->show();
}

void  page_debug::refreshPage()
{
}

void page_debug::slot_stopIfDiffClicked(bool enb)
{
    config->stopIfDiff = enb;
}

void page_debug::slot_verifyTilingNames()
{
    TilingManager tm;
    bool rv = tm.verifyNameFiles();

    QMessageBox box(this);
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
    QMessageBox box(this);
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

    QMessageBox box2(this);
    box2.setIcon(QMessageBox::Information);
    box2.setText(QString("Reformat XML: %1 good designs, %2 bad designs").arg(goodDes).arg(badDes));
    box2.setStandardButtons(QMessageBox::Ok);
    box2.exec();
}

void page_debug::slot_reformatTilingXML()
{
    QMessageBox box(this);
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

    QMessageBox box2(this);
    box2.setIcon(QMessageBox::Information);
    box2.setText(QString("Reformat XML: %1 good tilings, %2 bad tilings").arg(goodTiles).arg(badTiles));
    box2.setStandardButtons(QMessageBox::Ok);
    box2.exec();
}

void page_debug::slot_reprocessDesignXML()
{
    QMessageBox box(this);
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
        MosaicManager mm;
        bool rv = mm.loadMosaic(name);
        if (rv)
        {
            QString outfile;
            rv = mm.saveMosaic(name,outfile,true);
        }
        if (rv)
            goodDes++;
        else
            badDes++;
    }

    qDebug() << "Reprocessed" << goodDes << "good designs, " << badDes << "bad designs";

    QMessageBox box2(this);
    box2.setIcon(QMessageBox::Information);
    box2.setText(QString("Reprocess Design XML: %1 good designs, %2 bad designs").arg(goodDes).arg(badDes));
    box2.setStandardButtons(QMessageBox::Ok);
    box2.exec();
}

void page_debug::slot_reprocessTilingXML()
{
    QMessageBox box(this);
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

    QStringList files = FileServices::getTilingNames();
    for (int i=0; i < files.size(); i++)
    {
        bool rv = false;

        QString name = files[i];

        TilingManager tm;
        TilingPtr tp = tm.loadTiling(name,SM_LOAD_SINGLE);
        if (tp)
        {
            Q_ASSERT(tp->getName() == name);
            rv = tm.saveTiling(name,tp);
        }
        if (rv)
            goodTiles++;
        else
            badTiles++;
    }
    qDebug() << "Reformatted" << goodTiles << "good tilings, " << badTiles << "bad tilings";

    QMessageBox box2(this);
    box2.setIcon(QMessageBox::Information);
    box2.setText(QString("Reformat XML: %1 good tilings, %2 bad tilings").arg(goodTiles).arg(badTiles));
    box2.setStandardButtons(QMessageBox::Ok);
    box2.exec();
}

void page_debug::slot_cycleIntervalChanged(int value)
{
    config->cycleInterval = value;
}

void page_debug::slot_cycleModeChanged(int id)
{
    config->cycleMode = static_cast<eCycleMode>(id);
}

void page_debug::slot_dir0Changed()
{
    QString dir = dir0->text();
    config->compareDir0 = dir;
    loadCombo(ibox0,dir);
}

void page_debug::slot_dir1Changed()
{
    QString dir = dir1->text();
    config->compareDir1 = dir;
    loadCombo(ibox1,dir);
}

void page_debug::selectDir0()
{
    QString  dir = dir0->text();
    QString fdir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), dir,
                                                 QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (fdir.isEmpty())
            return;

    config->compareDir0 = fdir;
    dir0->setText(fdir);
    slot_dir0Changed();
}

void page_debug::selectDir1()
{
    QString  dir = dir1->text();
    QString fdir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), dir,
                                                 QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (fdir.isEmpty())
            return;

    config->compareDir1 = fdir;
    dir1->setText(fdir);
    slot_dir1Changed();
}

void page_debug::swapDirs()
{
    QString a = config->compareDir0;
    QString b = config->compareDir1;
    config->compareDir0 = b;
    config->compareDir1 = a;
    dir0->setText(b);
    dir1->setText(a);
}

void page_debug::slot_viewImage0()
{
    qDebug() << "slot_viewImage0";
    QString file = ibox0->currentText();
    config->image0 = file;
    QString path = dir0->text() + "/" + file + ".bmp";
    viewImage(path);
}

void page_debug::slot_viewImage1()
{
    qDebug() << "slot_viewImage1";
    QString file = ibox1->currentText();
    config->image1 = file;
    QString path = dir1->text() + "/" + file + ".bmp";
    viewImage(path);
}

void page_debug::viewImage(QString file)
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

void page_debug::slot_cycle()
{
    switch (config->cycleMode)
    {
    case CYCLE_SAVE_STYLE_BMPS:
        panel->selectViewer(VIEW_MOSAIC);
        saveMosaicBitmaps();
        break;

    case CYCLE_SAVE_TILING_BMPS:
        panel->selectViewer(VIEW_TILING);
        saveTilingBitmaps();
        break;

    case CYCLE_STYLES:
        panel->selectViewer(VIEW_MOSAIC);
        emit sig_cyclerStart(config->cycleMode);
        break;

    case CYCLE_TILINGS:
        panel->selectViewer(VIEW_TILING);
        emit sig_cyclerStart(config->cycleMode);
        break;

    case CYCLE_ORIGINAL_PNGS:
    case CYCLE_COMPARE_IMAGES:
        emit sig_cyclerStart(config->cycleMode);
        break;

    case CYCLE_NONE:
        break;
    }
}

void page_debug::saveMosaicBitmaps()
{
    QStringList files;
    if (config->mosaicFilterCheck &&  !config->mosaicFilter.isEmpty())
    {
        files = FileServices::getFilteredDesignNames(config->mosaicFilter);
    }
    else
    {
        files = FileServices::getDesignNames();
    }

    for (const auto & name : qAsConst(files))
    {
        // this forces immediate action
        MosaicManager mm;
        mm.loadMosaic(name);
        vcontrol->slot_refreshView();
        view->repaint();
        savePixmap(name);
    }

    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    box.setText("Cycle complete");
    box.exec();
}

void page_debug::saveTilingBitmaps()
{
    QStringList files;
    if (config->tileFilterCheck && !config->tileFilter.isEmpty())
    {
        files = FileServices::getFilteredTilingNames(config->tileFilter);
    }
    else
    {
        files = FileServices::getTilingNames();
    }

    for (const auto & name : qAsConst(files))
    {
        // this forces immediate action
        TilingManager tm;
        TilingPtr tp = tm.loadTiling(name,SM_LOAD_SINGLE);
        if (tp)
        {
            vcontrol->slot_refreshView();
            view->repaint();
            savePixmap(name);
        }
    }

    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    box.setText("Cycle complete");
    box.exec();
}

void page_debug::slot_compareImages()
{
    imageCompareResult->setText("");
    emit sig_compareImageFiles(ibox0->currentText(),ibox1->currentText(),false);
}

void page_debug::slot_compareCycle()
{
    panel->showPanelStatus("L=log  V=view Q=quit Spacebar=next");
    emit sig_cyclerStart(CYCLE_COMPARE_IMAGES);
}

void page_debug::slot_transparentClicked(bool checked)
{
    config->compare_transparent = checked;
}

void page_debug::slot_differencesClicked(bool checked)
{
    config->display_differences = checked;
}

void page_debug::slot_ping_pongClicked(bool checked)
{
    config->compare_ping_pong = checked;
}

void page_debug::slot_side_by_sideClicked(bool checked)
{
    config->compare_side_by_side = checked;
}

void page_debug::slot_compareResult(QString result)
{
    imageCompareResult->setText(result);
}

void page_debug::loadCombo(QComboBox * box,QString dir)
{
    QMap<QString,QString> map;
    map = FileServices::getDirBMPFiles(dir);
    box->clear();

    QStringList names = map.keys();

    VersionList vlist;
    vlist.create(names);
    QStringList names2 = vlist.recompose();

    for (const auto & name : names2)
    {
        box->addItem(name);
    }
}

void page_debug::setCombo(QComboBox * box, QString name)
{
    int index = box->findText(name);
    if (index == -1) index = 0;
    box->setCurrentIndex(index);
}

void page_debug::slot_setImage0(QString name)
{
    int index = ibox0->findText(name);
    ibox0->setCurrentIndex(index);
}

void page_debug::slot_setImage1(QString name)
{
    int index = ibox1->findText(name);
    ibox1->setCurrentIndex(index);
}

void page_debug::slot_ibox0_changed(int index)
{
    Q_UNUSED(index);
    config->image0 = ibox0->currentText();

    // special case for ibox1 - not symmetric
    slot_setImage1(config->image0);  // makes it the same
}

void page_debug::slot_ibox1_changed(int index)
{
    Q_UNUSED(index);
    config->image1 = ibox1->currentText();
}

void page_debug::slot_previous()
{
    int index = ibox0->currentIndex();
    if (index == 0) return;
    index--;
    ibox0->setCurrentIndex(index);
#if 0
    index = ibox1->currentIndex();
    if (index == 0) return;
    index--;
    ibox1->setCurrentIndex(index);
#endif
    imageCompareResult->setText("");
    emit sig_compareImageFiles(ibox0->currentText(),ibox1->currentText(),false);
}

void page_debug::slot_next()
{
    int index = ibox0->currentIndex();
    if (index >= ibox0->count()-1) return;
    index++;
    ibox0->setCurrentIndex(index);

#if 0
    index = ibox1->currentIndex();
    if (index >= ibox1->count()-1) return;
    index++;
    ibox1->setCurrentIndex(index);
#endif
    imageCompareResult->setText("");
    emit sig_compareImageFiles(ibox0->currentText(),ibox1->currentText(),false);
}

void  page_debug::slot_verifyMapsClicked(bool enb)
{
    config->verifyMaps = enb;
}

void  page_debug::slot_verifyDumpClicked(bool enb)
{
    config->verifyDump = enb;
}

void  page_debug::slot_verifyVerboseClicked(bool enb)
{
    config->verifyVerbose = enb;
}

void page_debug::savePixmap(QString name)
{
    Q_ASSERT(!name.contains(".xml"));

    QPixmap pixmap = view->grab();

    QString subdir;
    switch (config->repeatMode)
    {
    case REPEAT_SINGLE:
        subdir = "single/";
        break;
    case REPEAT_PACK:
        subdir = "pack/";
        break;
    case REPEAT_DEFINED:
        subdir = "defined/";
        break;
    }

    QDateTime d = QDateTime::currentDateTime();
    QString date = d.toString("yyyy-MM-dd");

    QString path = config->rootImageDir;
    if (config->viewerType == VIEW_TILING)
        path += "tilings/" + subdir + date;
    else
        path += subdir + date;

    QDir adir(path);
    if (!adir.exists())
    {
        if (!adir.mkpath(path))
        {
            qFatal("could not make path");
        }
    }
    QString file = path + "/" + name + ".bmp";
    qDebug() << "saving" << file;

    //QFileInfo fileInfo(file);
    //QString path = fileInfo.filePath();
    //s.setValue("picPath",path);

    bool rv = pixmap.save(file);
    if (!rv)
        qDebug() << file << "save ERROR";
}


