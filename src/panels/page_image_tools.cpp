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

#include "panels/page_image_tools.h"
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

page_image_tools:: page_image_tools(ControlPanel * cpanel)  : panel_page(cpanel,"Image Tools")
{
    QGroupBox * gbox;
    gbox = createCycleSection();
    vbox->addWidget(gbox);
    gbox = createWorklistSection();
    vbox->addWidget(gbox);
    gbox = createCompareSection();
    vbox->addWidget(gbox);

    connect(theApp,&TiledPatternMaker::sig_compareResult,       this,   &page_image_tools::slot_compareResult);
    connect(this,  &page_image_tools::sig_view_image,           theApp, &TiledPatternMaker::slot_view_image);
    connect(this,  &page_image_tools::sig_compareImageFiles,    theApp, &TiledPatternMaker::slot_compareImages, Qt::QueuedConnection);
    connect(this,  &page_image_tools::sig_loadMosaic,           theApp, &TiledPatternMaker::slot_loadMosaic);

    Cycler * cycler = Cycler::getInstance();
    connect(this,  &page_image_tools::sig_cyclerStart,          cycler,  &Cycler::slot_startCycle, Qt::QueuedConnection);
    connect(cycler, &Cycler::sig_workList,                      this,   &page_image_tools::slot_dir0Changed, Qt::QueuedConnection);
}

QGroupBox * page_image_tools::createCycleSection()
{
    SpinSet     * spCycleInterval       = new SpinSet("Cycle Interval",0,0,9);
    QPushButton * generateBtn           = new QPushButton("Generate");
    generateBtn->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    QRadioButton * rStyles    = new QRadioButton("Mosaics");
    QRadioButton * rTiles     = new QRadioButton("Tilings");
    QRadioButton * rPngs      = new QRadioButton("PNGS");
    QRadioButton * rSavStyles = new QRadioButton("Save Mosaic BMPs");
    QRadioButton * rSavTiles  = new QRadioButton("Save Tiling BMPs");
    QCheckBox    * skip       = new QCheckBox("Skip Existing");
    QCheckBox    * use_wlistG = new QCheckBox("Use Work List");
                   directory  = new QLineEdit("");
#ifdef Q_OS_WINDOWS
    QPushButton  * opendirBtn = new QPushButton("Open/See");
#endif
    QDateTime d = QDateTime::currentDateTime();
    QString date = d.toString("yyyy-MM-dd");
    directory->setText(date);
    directory->setMinimumWidth(132);

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
    hbox00->addWidget(directory);
#ifdef Q_OS_WINDOWS
    hbox00->addWidget(opendirBtn);
#endif
    hbox00->addWidget(rSavTiles);
    hbox00->addWidget(rSavStyles);
    hbox00->addWidget(use_wlistG);
    hbox00->addWidget(skip);
    hbox00->addStretch();

    AQHBoxLayout * hbox11 = new AQHBoxLayout;
    hbox11->addLayout(spCycleInterval);
    hbox11->addWidget(rPngs);
    hbox11->addWidget(rStyles);
    hbox11->addWidget(rTiles);
    hbox11->addStretch();

    QVBoxLayout * cycleLayout = new QVBoxLayout;
    cycleLayout->addLayout(hbox00);
    cycleLayout->addLayout(hbox11);

    QHBoxLayout * hbox22 = new QHBoxLayout;
    hbox22->addLayout(cycleLayout);
    hbox22->addWidget(generateBtn,0,Qt::AlignTop);

    QGroupBox * cycleGroupBox = new  QGroupBox("Generate Images");
    cycleGroupBox->setLayout(hbox22);

    spCycleInterval->setValue(config->cycleInterval);
    use_wlistG->setChecked(config->use_workListForGenerate);
    skip->setChecked(config->skipExisting);

    connect(spCycleInterval,    &SpinSet::valueChanged,    this,  &page_image_tools::slot_cycleIntervalChanged);
    connect(generateBtn,        &QPushButton::clicked,     this,  &page_image_tools::slot_cycle);
    connect(opendirBtn,         &QPushButton::clicked,     this,  &page_image_tools::slot_opendir);
    connect(use_wlistG,         &QCheckBox::clicked,       this,  &page_image_tools::slot_use_worklist_generate);
    connect(skip,               &QCheckBox::clicked,       this,  &page_image_tools::slot_skipExisting);

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(cycleGroup,         SIGNAL(buttonClicked(int)), this, SLOT(slot_cycleModeChanged(int)));
#else
    connect(cycleGroup,          &QButtonGroup::idClicked,  this,  &page_image_tools::slot_cycleModeChanged);
#endif
    return cycleGroupBox;
}

QGroupBox * page_image_tools::createWorklistSection()
{
    QPushButton * loadListBtn= new QPushButton("Load Work List");
    QPushButton * saveListBtn= new QPushButton("Save Work List");
    QPushButton * editListBtn= new QPushButton("Edit Work List");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(loadListBtn);
    hbox->addWidget(saveListBtn);
    hbox->addWidget(editListBtn);
    hbox->addStretch();

    QGroupBox * box = new  QGroupBox("Work List operations");
    box->setLayout(hbox);

    connect(loadListBtn,  &QPushButton::clicked,     this,  &page_image_tools::loadWorkListFromFile);
    connect(saveListBtn,  &QPushButton::clicked,     this,  &page_image_tools::saveWorkListToFile);
    connect(editListBtn,  &QPushButton::clicked,     this,  &page_image_tools::editWorkList);

    return box;
}

QGroupBox * page_image_tools::createCompareSection()
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
    QPushButton * reviewBtn    = new QPushButton("Cycle");
    QPushButton * previousBtn  = new QPushButton("Previous");
    QPushButton * nextBtn      = new QPushButton("Next");
    QPushButton * loadBtn      = new QPushButton("Load");

    reviewBtn->setStyleSheet("QPushButton { background-color: yellow; color: red;}");
    compareBtn->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    QPushButton * swapBtn      = new QPushButton("Swap");
    QCheckBox   * cbStopIfDiff = new QCheckBox("Stop if Diff");
    QCheckBox   * transparent  = new QCheckBox("Transparent");
    QCheckBox   * differences  = new QCheckBox("Display Differences");
    QCheckBox   * use_wlistC   = new QCheckBox("Use Work List");
    QCheckBox   * gen_wlist    = new QCheckBox("Generate Work List");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(cbStopIfDiff);
    hbox->addWidget(transparent);
    hbox->addWidget(differences);
    hbox->addWidget(use_wlistC);
    hbox->addWidget(gen_wlist);
    hbox->addWidget(reviewBtn);

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
    imageGrid->addWidget(loadBtn,row,0);
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
    use_wlistC->setChecked(config->use_workListForCompare);
    gen_wlist->setChecked(config->generate_workList);

    connect(swapBtn,                &QPushButton::clicked,     this,  &page_image_tools::swapDirs);
    connect(compareDir0Btn,         &QPushButton::clicked,     this,  &page_image_tools::selectDir0);
    connect(compareDir1Btn,         &QPushButton::clicked,     this,  &page_image_tools::selectDir1);

    connect(viewImage0,             &QPushButton::clicked,     this,   &page_image_tools::slot_viewImage0);
    connect(viewImage1,             &QPushButton::clicked,     this,   &page_image_tools::slot_viewImage1);
    connect(compareBtn,             &QPushButton::clicked,     this,   &page_image_tools::slot_compareImages);
    connect(reviewBtn,              &QPushButton::clicked,     this,   &page_image_tools::slot_compareCycle);
    connect(previousBtn,            &QPushButton::clicked,     this,   &page_image_tools::slot_previous);
    connect(nextBtn,                &QPushButton::clicked,     this,   &page_image_tools::slot_next);
    connect(loadBtn,                &QPushButton::clicked,     this,   &page_image_tools::slot_load);

    connect(transparent,            &QCheckBox::clicked,       this,   &page_image_tools::slot_transparentClicked);
    connect(cbStopIfDiff,           &QCheckBox::clicked,       this,   &page_image_tools::slot_stopIfDiffClicked);
    connect(differences,            &QCheckBox::clicked,       this,   &page_image_tools::slot_differencesClicked);
    connect(use_wlistC,             &QCheckBox::clicked,       this,   &page_image_tools::slot_use_worklist_compare);
    connect(gen_wlist,              &QCheckBox::clicked,       this,   &page_image_tools::slot_gen_worklist);

    connect(dir0, &QLineEdit::editingFinished, this, &page_image_tools::slot_dir0Changed);
    connect(dir1, &QLineEdit::editingFinished, this, &page_image_tools::slot_dir1Changed);
    connect(ibox0, SIGNAL(currentIndexChanged(int)),  this, SLOT(slot_ibox0_changed(int)));
    connect(ibox1, SIGNAL(currentIndexChanged(int)),  this, SLOT(slot_ibox1_changed(int)));

    setCombo(ibox0,config->image0);
    setCombo(ibox1,config->image1);

    return  imageGroup;
}

void  page_image_tools::onEnter()
{
    imageCompareResult->setText("");
}

void page_image_tools::onExit()
{
    panel->hidePanelStatus();

    view->clearLayout();   // removes any cler pngs
    view->show();
}

void  page_image_tools::refreshPage()
{
}

void page_image_tools::slot_stopIfDiffClicked(bool enb)
{
    config->stopIfDiff = enb;
}

void page_image_tools::slot_cycleIntervalChanged(int value)
{
    config->cycleInterval = value;
}

void page_image_tools::slot_cycleModeChanged(int id)
{
    config->cycleMode = static_cast<eCycleMode>(id);
}

void page_image_tools::slot_dir0Changed()
{
    QString dir = dir0->text();
    config->compareDir0 = dir;
    loadCombo(ibox0,dir);
}

void page_image_tools::slot_dir1Changed()
{
    QString dir = dir1->text();
    config->compareDir1 = dir;
    loadCombo(ibox1,dir);
}

void page_image_tools::selectDir0()
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

void page_image_tools::selectDir1()
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

void page_image_tools::swapDirs()
{
    QString a = config->compareDir0;
    QString b = config->compareDir1;
    config->compareDir0 = b;
    config->compareDir1 = a;
    dir0->setText(b);
    dir1->setText(a);
}

void page_image_tools::slot_viewImage0()
{
    qDebug() << "slot_viewImage0";
    QString file = ibox0->currentText();
    config->image0 = file;
    QString path = dir0->text() + "/" + file + ".bmp";
    viewImage(path);
}

void page_image_tools::slot_viewImage1()
{
    qDebug() << "slot_viewImage1";
    QString file = ibox1->currentText();
    config->image1 = file;
    QString path = dir1->text() + "/" + file + ".bmp";
    viewImage(path);
}

void page_image_tools::viewImage(QString file)
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

    emit sig_view_image(file,file);     // use same file
}

void page_image_tools::slot_cycle()
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
    case CYCLE_COMPARE_ALL_IMAGES:
    case CYCLE_COMPARE_WORKLIST_IMAGES:
        emit sig_cyclerStart(config->cycleMode);
        break;

    case CYCLE_NONE:
        break;
    }
}

void page_image_tools::saveMosaicBitmaps()
{
    QStringList files;

    if (!config->use_workListForGenerate)
    {
        if (config->mosaicFilterCheck &&  !config->mosaicFilter.isEmpty())
        {
            files = FileServices::getFilteredDesignNames(config->mosaicFilter);
        }
        else
        {
            files = FileServices::getDesignNames();
        }
    }
    else
    {
        files = config->workList;
    }

    for (const auto & name : qAsConst(files))
    {
        // this forces immediate action
        if (config->skipExisting)
        {
            Q_ASSERT(!name.contains(".xml"));
            QString path = getPixmapPath();
            QString file = path + "/" + name + ".bmp";
            QFile afile(file);
            if (afile.exists())
            {
               continue;
            }
        }

        config->currentlyLoadedXML.clear();
        config->currentlyLoadingXML = name;

        MosaicManager mm;
        mm.loadMosaic(name);

        config->currentlyLoadingXML.clear();
        config->lastLoadedXML      = name;
        config->currentlyLoadedXML = name;

        vcontrol->slot_refreshView();
        view->repaint();
        savePixmap(name);
    }

    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    box.setText("Cycle complete");
    box.exec();
}

void page_image_tools::saveTilingBitmaps()
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

void page_image_tools::slot_compareImages()
{
    panel->showPanelStatus("Spacebar=next P=ping-pong S=side-by-side L=log Q=quit");

    imageCompareResult->setText("");
    emit sig_compareImageFiles(ibox0->currentText(),ibox1->currentText(),false);
}

void page_image_tools::slot_compareCycle()
{
    panel->showPanelStatus("Spacebar=next P=ping-pong S=side-by-side L=log Q=quit");

    if (config->use_workListForCompare)
        emit sig_cyclerStart(CYCLE_COMPARE_WORKLIST_IMAGES);
    else
        emit sig_cyclerStart(CYCLE_COMPARE_ALL_IMAGES);
}

void page_image_tools::slot_transparentClicked(bool checked)
{
    config->compare_transparent = checked;
}

void page_image_tools::slot_differencesClicked(bool checked)
{
    config->display_differences = checked;
}

void page_image_tools::slot_use_worklist_compare(bool checked)
{
    config->use_workListForCompare = checked;
    slot_dir0Changed();
    slot_dir1Changed();
}

void page_image_tools::slot_use_worklist_generate(bool checked)
{
    config->use_workListForGenerate = checked;
    slot_dir0Changed();
    slot_dir1Changed();
}

void page_image_tools::slot_workList()
{
    Q_ASSERT(config->generate_workList);
    slot_dir0Changed();
    slot_dir1Changed();
}

void page_image_tools::slot_gen_worklist(bool checked)
{
    config->generate_workList = checked;
}

void page_image_tools::slot_compareResult(QString result)
{
    imageCompareResult->setText(result);
}

void  page_image_tools::slot_skipExisting(bool checked)
{
    config->skipExisting = checked;
}

void page_image_tools::loadCombo(QComboBox * box,QString dir)
{
    QStringList names2;
    if (!config->use_workListForCompare)
    {
        QMap<QString,QString> map;
        map = FileServices::getDirBMPFiles(dir);
        box->clear();

        QStringList names = map.keys();

        VersionList vlist;
        vlist.create(names);
        names2 = vlist.recompose();
    }
    else
    {
        names2 = config->workList;
    }

    box->clear();
    for (const auto & name : qAsConst(names2))
    {
        box->addItem(name);
    }
}

void page_image_tools::setCombo(QComboBox * box, QString name)
{
    int index = box->findText(name);
    if (index == -1) index = 0;
    box->setCurrentIndex(index);
}

void page_image_tools::slot_setImage0(QString name)
{
    int index = ibox0->findText(name);
    ibox0->setCurrentIndex(index);
}

void page_image_tools::slot_setImage1(QString name)
{
    int index = ibox1->findText(name);
    ibox1->setCurrentIndex(index);
}

void page_image_tools::slot_ibox0_changed(int index)
{
    Q_UNUSED(index);
    config->image0 = ibox0->currentText();

    // special case for ibox1 - not symmetric
    slot_setImage1(config->image0);  // makes it the same
}

void page_image_tools::slot_ibox1_changed(int index)
{
    Q_UNUSED(index);
    config->image1 = ibox1->currentText();
}

void page_image_tools::slot_previous()
{
    int index = ibox0->currentIndex();
    if (index == 0) return;
    index--;
    ibox0->setCurrentIndex(index);
    imageCompareResult->setText("");
    emit theApp->sig_closeAllImageViewers();
    emit sig_compareImageFiles(ibox0->currentText(),ibox1->currentText(),false);
}

void page_image_tools::slot_next()
{
    int index = ibox0->currentIndex();
    if (index >= ibox0->count()-1) return;
    index++;
    ibox0->setCurrentIndex(index);
    imageCompareResult->setText("");
    emit theApp->sig_closeAllImageViewers();
    emit sig_compareImageFiles(ibox0->currentText(),ibox1->currentText(),false);
}

void page_image_tools::slot_load()
{
    QString mos = ibox0->currentText();
    emit sig_loadMosaic(mos);
}

QString page_image_tools::getPixmapPath()
{
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

    QString date = directory->text();

    QString path = config->rootImageDir;
    if (config->getViewerType() == VIEW_TILING)
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
    return path;
}

void page_image_tools::savePixmap(QString name)
{
    Q_ASSERT(!name.contains(".xml"));

    QPixmap pixmap = view->grab();

    QString path = getPixmapPath();

    QString file = path + "/" + name + ".bmp";
    qDebug() << "saving" << file;

    bool rv = pixmap.save(file);
    if (!rv)
        qDebug() << file << "save ERROR";
}

void page_image_tools::slot_opendir()
{
#ifdef Q_OS_WINDOWS
    QString path = getPixmapPath();
    qDebug() <<  "Path:" << path;

    QStringList args;
    args << "/select," << QDir::toNativeSeparators(path);

    QProcess::startDetached("explorer",args);
#endif
}

void page_image_tools::loadWorkListFromFile()
{
    QString dir = config->rootMediaDir;
    QString fileName = QFileDialog::getOpenFileName(nullptr,"Select text file",dir, "All  files (*)");
    if (fileName.isEmpty())
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Warning);
        box.setText("FILE NOT FOUND");
        box.exec();
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Warning);
        box.setText(QString("Text file <%1> failed to open").arg(fileName));
        box.exec();

        return;

    }
    QTextStream textStream(&file);

    QStringList stringList;
    while (!textStream.atEnd())
    {
        QString name = textStream.readLine();
        name.remove(".xml");
        stringList << name;
    }
    file.close();

    qDebug() << "file list: " << stringList;
    if (stringList.empty())
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Warning);
        box.setText(QString("No filenames found in %1").arg(fileName));
        box.exec();
    }

    config->workList = stringList;

    int sz = stringList.size();

    QMessageBox box;
    box.setIcon(QMessageBox::Information);
    box.setText(QString("%1 filenames loaded into Work List - OK").arg(sz));
    box.exec();

    slot_use_worklist_compare(true);
}


void page_image_tools::saveWorkListToFile()
{
    if (config->workList.isEmpty())
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Warning);
        box.setText("Empty Work List - nothing to save");
        box.exec();
        return;
    }

    QString dir = config->rootMediaDir;
    QString fileName = QFileDialog::getSaveFileName(nullptr,"Select text file",dir, "All  files (*)");
    if (fileName.isEmpty())
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Warning);
        box.setText("FILE NOT FOUND");
        box.exec();
        return;
    }

    if (!fileName.contains(".txt"))
        fileName  += ".txt";

    qDebug() << "saving" << fileName;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Warning);
        box.setText(QString("Text file <%1> failed to open").arg(fileName));
        box.exec();
        return;
    }

    QTextStream ts(&file);

    QStringList::const_iterator constIterator;
    for (constIterator = config->workList.constBegin(); constIterator != config->workList.constEnd(); ++constIterator)
    {
       ts << (*constIterator).toLocal8Bit().constData() << Qt::endl;
    }
    file.close();

    QMessageBox box;
    box.setIcon(QMessageBox::Information);
    box.setText("Work List saved - OK");
    box.exec();
}

void page_image_tools::editWorkList()
{
    ListListWidget * plw = new ListListWidget(this);
    plw->addItems(config->workList);
    plw->establishSize();

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addWidget(plw);

    QDialog * ewl = new QDialog(this);
    ewl->setAttribute(Qt::WA_DeleteOnClose);
    ewl->setLayout(vbox);

    ewl->exec();

}
