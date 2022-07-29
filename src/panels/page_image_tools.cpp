#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QRadioButton>
#include <QFileDialog>
#include <QMessageBox>

#include "misc/cycler.h"
#include "misc/fileservices.h"
#include "misc/tpm_io.h"
#include "mosaic/mosaic_manager.h"
#include "panels/page_image_tools.h"
#include "panels/panel.h"
#include "settings/configuration.h"
#include "tile/tiling_manager.h"
#include "tiledpatternmaker.h"
#include "viewers/viewcontrol.h"
#include "widgets/layout_sliderset.h"
#include "widgets/memory_combo.h"
#include "widgets/versioned_list_widget.h"

page_image_tools:: page_image_tools(ControlPanel * cpanel)  : panel_page(cpanel,"Image Tools")
{
    localCycle = false;

    QGroupBox * gbox;
    gbox = createCycleGenSection();
    vbox->addWidget(gbox);
    gbox = createCycleViewSection();
    vbox->addWidget(gbox);
    gbox = createWorklistSection();
    vbox->addWidget(gbox);
    gbox = createCompareSection();
    vbox->addWidget(gbox);
    gbox = createViewSection();
    vbox->addWidget(gbox);
    gbox = createTransparencySection();
    vbox->addWidget(gbox);

    connect(theApp,&TiledPatternMaker::sig_compareResult,       this,   &page_image_tools::slot_compareResult);
    connect(this,  &page_image_tools::sig_view_image,           theApp, &TiledPatternMaker::slot_view_image);
    connect(this,  &page_image_tools::sig_compareImageFiles,    theApp, &TiledPatternMaker::slot_compareImages, Qt::QueuedConnection);
    connect(this,  &page_image_tools::sig_loadMosaic,           theApp, &TiledPatternMaker::slot_loadMosaic);

    Cycler * cycler = Cycler::getInstance();
    connect(this,  &page_image_tools::sig_cyclerStart,          cycler,  &Cycler::slot_startCycle, Qt::QueuedConnection);
    connect(cycler, &Cycler::sig_workList,                      this,   &page_image_tools::slot_dir0Changed, Qt::QueuedConnection);

    ViewControl * vc = ViewControl::getInstance();
    connect(vc, &ViewControl::sig_cyclerQuit,                   this, [this] { localCycle = false; });
}

QGroupBox * page_image_tools::createCycleGenSection()
{
    SpinSet     * spCycleInterval       = new SpinSet("Cycle Interval",0,0,9);
    QPushButton * generateBtn           = new QPushButton("Generate");
    generateBtn->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    QRadioButton * rSavStyles = new QRadioButton("Mosaic BMPs");
    QRadioButton * rSavTiles  = new QRadioButton("Tiling BMPs");
    QCheckBox    * skip       = new QCheckBox("Skip Existing");
              fileFilterCombo = new QComboBox();
                   directory  = new QLineEdit("");
#ifdef Q_OS_WINDOWS
    QPushButton  * opendirBtn = new QPushButton("Open/See");
#endif

    QDateTime d = QDateTime::currentDateTime();
    QString date = d.toString("yyyy-MM-dd");
    directory->setText(date);
    directory->setMinimumWidth(132);

    fileFilterCombo->addItem("All files",LOAD_ALL);
    fileFilterCombo->addItem("Worklist",LOAD_WORKLIST);
    fileFilterCombo->addItem("Load filtered",LOAD_FILTERED);

    int index = fileFilterCombo->findData(config->fileFilter);
    fileFilterCombo->setCurrentIndex(index);

    cycleGenBtnGroup = new QButtonGroup;
    cycleGenBtnGroup->addButton(rSavStyles,CYCLE_SAVE_STYLE_BMPS);
    cycleGenBtnGroup->addButton(rSavTiles,CYCLE_SAVE_TILING_BMPS);

    cycleGenBtnGroup->button(config->genCycle)->setChecked(true);

    QHBoxLayout * hboxGen = new QHBoxLayout;
    hboxGen->addWidget(directory);
#ifdef Q_OS_WINDOWS
    hboxGen->addWidget(opendirBtn);
#endif
    hboxGen->addWidget(rSavStyles);
    hboxGen->addWidget(rSavTiles);
    hboxGen->addWidget(fileFilterCombo);
    hboxGen->addWidget(skip);
    hboxGen->addStretch();
    hboxGen->addWidget(generateBtn,0,Qt::AlignTop);

    QGroupBox * cycleGenBox = new  QGroupBox("Generate Images");
    cycleGenBox->setLayout(hboxGen);

    spCycleInterval->setValue(config->cycleInterval);
    skip->setChecked(config->skipExisting);

    connect(spCycleInterval,    &SpinSet::valueChanged,    this,  &page_image_tools::slot_cycleIntervalChanged);
    connect(generateBtn,        &QPushButton::clicked,     this,  &page_image_tools::slot_cycleGen);
#ifdef Q_OS_WINDOWS
    connect(opendirBtn,         &QPushButton::clicked,     this,  &page_image_tools::slot_opendir);
#endif
    connect(skip,               &QCheckBox::clicked,       this,  &page_image_tools::slot_skipExisting);
    connect(cycleGenBtnGroup,   &QButtonGroup::idClicked,  this, [this](int id) { config->genCycle = static_cast<eCycleMode>(id); } );
    connect(fileFilterCombo,   QOverload<int>::of(&QComboBox::currentIndexChanged), [=]()
                                { config->fileFilter = static_cast<eLoadType>(fileFilterCombo->currentData().toInt()); } );

    return cycleGenBox;
}

QGroupBox * page_image_tools::createCycleViewSection()
{
    SpinSet     * spCycleInterval       = new SpinSet("Cycle Interval",0,0,9);
    QPushButton * generateBtn           = new QPushButton("View");
    generateBtn->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    QRadioButton * rStyles    = new QRadioButton("Mosaics");
    QRadioButton * rTiles     = new QRadioButton("Tilings");
    QRadioButton * rPngs      = new QRadioButton("Original PNGs");

    cycleViewBtnGroup = new QButtonGroup;
    cycleViewBtnGroup->addButton(rStyles,CYCLE_STYLES);
    cycleViewBtnGroup->addButton(rTiles,CYCLE_TILINGS);
    cycleViewBtnGroup->addButton(rPngs,CYCLE_ORIGINAL_PNGS);

    cycleViewBtnGroup->button(config->viewCycle)->setChecked(true);

    QHBoxLayout * hboxView = new QHBoxLayout;
    hboxView->addLayout(spCycleInterval);
    hboxView->addWidget(rStyles);
    hboxView->addWidget(rTiles);
    hboxView->addWidget(rPngs);
    hboxView->addStretch();
    hboxView->addWidget(generateBtn,0,Qt::AlignTop);

    QGroupBox * cycleViewBox = new  QGroupBox("View Images");
    cycleViewBox->setLayout(hboxView);

    spCycleInterval->setValue(config->cycleInterval);

    connect(spCycleInterval,    &SpinSet::valueChanged,    this,  &page_image_tools::slot_cycleIntervalChanged);
    connect(generateBtn,        &QPushButton::clicked,     this,  &page_image_tools::slot_cycleView);
    connect(cycleViewBtnGroup,  &QButtonGroup::idClicked, [=](int id) { config->viewCycle = static_cast<eCycleMode>(id); } );

    return cycleViewBox;
}

QGroupBox * page_image_tools::createWorklistSection()
{
    QPushButton * loadListBtn = new QPushButton("Load Work List");
    QPushButton * saveListBtn = new QPushButton("Save Work List");
    QPushButton * editListBtn = new QPushButton("Edit Work List");
                  wlistStatus = new QLabel();
    QPushButton * delBtn        = new QPushButton("Delete Current");
    QPushButton * replBtn       = new QPushButton("Replace Current");
    QPushButton * loadBtn       = new QPushButton("Load Current");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(loadListBtn);
    hbox->addWidget(saveListBtn);
    hbox->addWidget(editListBtn);
    hbox->addStretch();
    hbox->addWidget(wlistStatus);
    hbox->addStretch();
    hbox->addWidget(delBtn);
    hbox->addWidget(replBtn);
    hbox->addWidget(loadBtn);

    QGroupBox * box = new  QGroupBox("Work List operations");
    box->setLayout(hbox);

    connect(loadListBtn,  &QPushButton::clicked,     this,  &page_image_tools::loadWorkListFromFile);
    connect(saveListBtn,  &QPushButton::clicked,     this,  &page_image_tools::saveWorkListToFile);
    connect(editListBtn,  &QPushButton::clicked,     this,  &page_image_tools::editWorkList);
    connect(delBtn,       &QPushButton::clicked,     this,  &page_image_tools::slot_deleteCurrent);
    connect(replBtn,      &QPushButton::clicked,     this,  &page_image_tools::replaceCurrent);
    connect(loadBtn,      &QPushButton::clicked,     this,  &page_image_tools::loadCurrent);
    connect(theApp,&TiledPatternMaker::sig_deleteCurrentInWorklist, this, &page_image_tools::slot_deleteCurrent);

    return box;
}

QGroupBox * page_image_tools::createCompareSection()
{
    leftFile      = new QComboBox();
    leftFile->setMinimumWidth(461);
    rightFile     = new QComboBox();
    imageCompareResult = new QLineEdit();
    imageCompareResult->setReadOnly(true);

    QPushButton * compareDirLeftBtn  = new QPushButton("Compare Dir");
    QPushButton * compareDirRightBtn = new QPushButton("Compare Dir");
    leftDir                          = new QLineEdit();
    rightDir                         = new QLineEdit();

    QPushButton * viewImage0   = new QPushButton("View");
    QPushButton * viewImage1   = new QPushButton("View");
    QPushButton * compareBtn   = new QPushButton("Compare");
    QPushButton * reviewBtn    = new QPushButton("Cycle");
    QPushButton * previousBtn  = new QPushButton("Previous");
    QPushButton * nextBtn      = new QPushButton("Next");
    QPushButton * loadBtn      = new QPushButton("Load Mosaic");

    reviewBtn->setStyleSheet("QPushButton { background-color: yellow; color: red;}");
    compareBtn->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    QPushButton * swapBtn      = new QPushButton("Swap");
    QPushButton * continueBtn  = new QPushButton("Continue");
    QCheckBox   * cbStopIfDiff = new QCheckBox("Stop if Diff");
    QCheckBox   * differences  = new QCheckBox("Display Differences");
    use_wlistForCompareChk     = new QCheckBox("Use Work List");
    gen_wlistChk               = new QCheckBox("Generate Work List");
    QCheckBox   * transparent  = new QCheckBox("Transparent");
    QCheckBox   * chkPopup     = new QCheckBox("Pop up");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(cbStopIfDiff);
    hbox->addWidget(differences);
    hbox->addWidget(chkPopup);
    hbox->addWidget(transparent);
    hbox->addStretch();
    hbox->addWidget(use_wlistForCompareChk);
    hbox->addWidget(gen_wlistChk);
    hbox->addWidget(reviewBtn);

    QGridLayout * imageGrid = new QGridLayout();

    int row = 0;
    imageGrid->addLayout(hbox,row,0,1,3);

    row++;
    imageGrid->addWidget(compareDirLeftBtn,row,0);
    imageGrid->addWidget(leftDir,row,1);
    imageGrid->addWidget(swapBtn,row,2);

    row++;
    imageGrid->addWidget(compareDirRightBtn,row,0);
    imageGrid->addWidget(rightDir,row,1);
    imageGrid->addWidget(continueBtn,row,2);

    row++;
    imageGrid->addWidget(previousBtn,row,0);
    imageGrid->addWidget(leftFile,row,1);
    imageGrid->addWidget(viewImage0,row,2);

    row++;
    imageGrid->addWidget(nextBtn,row,0);
    imageGrid->addWidget(rightFile,row,1);
    imageGrid->addWidget(viewImage1,row,2);

    row++;
    imageGrid->addWidget(loadBtn,row,0);
    imageGrid->addWidget(imageCompareResult,row,1);
    imageGrid->addWidget(compareBtn,row,2);

    QGroupBox * imageGroup = new QGroupBox("Compare Images");
    imageGroup->setLayout(imageGrid);

    QString dir = config->compareDir0;
    leftDir->setText(dir);
    loadCombo(leftFile,dir);
    dir = config->compareDir1;
    rightDir->setText(dir);
    loadCombo(rightFile,dir);

    cbStopIfDiff->setChecked(config->stopIfDiff);
    transparent->setChecked(config->compare_transparent);
    chkPopup->setChecked(config->compare_popup);
    differences->setChecked(config->display_differences);
    use_wlistForCompareChk->setChecked(config->use_workListForCompare);
    gen_wlistChk->setChecked(config->generate_workList);

    connect(swapBtn,                &QPushButton::clicked,     this,  &page_image_tools::swapDirs);
    connect(continueBtn,            &QPushButton::clicked,     this,  &page_image_tools::continueCycle);
    connect(compareDirLeftBtn,      &QPushButton::clicked,     this,  &page_image_tools::selectDir0);
    connect(compareDirRightBtn,     &QPushButton::clicked,     this,  &page_image_tools::selectDir1);

    connect(viewImage0,             &QPushButton::clicked,     this,   &page_image_tools::slot_viewImageLeft);
    connect(viewImage1,             &QPushButton::clicked,     this,   &page_image_tools::slot_viewImageRight);
    connect(compareBtn,             &QPushButton::clicked,     this,   &page_image_tools::slot_compareImages);
    connect(reviewBtn,              &QPushButton::clicked,     this,   &page_image_tools::slot_compareCycle);
    connect(previousBtn,            &QPushButton::clicked,     this,   &page_image_tools::slot_previous);
    connect(nextBtn,                &QPushButton::clicked,     this,   &page_image_tools::slot_next);
    connect(loadBtn,                &QPushButton::clicked,     this,   &page_image_tools::slot_load);

    connect(transparent,            &QCheckBox::clicked,       this,   &page_image_tools::slot_transparentClicked);
    connect(chkPopup,               &QCheckBox::clicked,       this,   &page_image_tools::slot_popupClicked);
    connect(cbStopIfDiff,           &QCheckBox::clicked,       this,   &page_image_tools::slot_stopIfDiffClicked);
    connect(differences,            &QCheckBox::clicked,       this,   &page_image_tools::slot_differencesClicked);
    connect(use_wlistForCompareChk, &QCheckBox::clicked,       this,   &page_image_tools::slot_use_worklist_compare);
    connect(gen_wlistChk,           &QCheckBox::clicked,       this,   &page_image_tools::slot_gen_worklist_compare);

    connect(leftDir,   &QLineEdit::editingFinished,       this, &page_image_tools::slot_dir0Changed);
    connect(rightDir,  &QLineEdit::editingFinished,       this, &page_image_tools::slot_dir1Changed);
    connect(leftFile,  SIGNAL(currentIndexChanged(int)),  this, SLOT(slot_ibox0_changed(int)));
    connect(rightFile, SIGNAL(currentIndexChanged(int)),  this, SLOT(slot_ibox1_changed(int)));

    setCombo(leftFile,config->image0);
    setCombo(rightFile,config->image1);

    return  imageGroup;
}

QGroupBox * page_image_tools::createViewSection()
{
    //QPushButton * selectImgBtn   = new QPushButton("Select Image File");
    QPushButton * selectImgBtn   = new QPushButton("Select");
    QPushButton * viewImageBtn   = new QPushButton("View");
    QCheckBox   * chkTransparent = new QCheckBox("Transparent");
    QCheckBox   * chkPopup       = new QCheckBox("Pop up");
                  viewFileCombo  = new MemoryCombo("ViewImageCombo");
                  viewFileCombo->setMinimumWidth(561);

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(chkPopup);
    hbox->addWidget(chkTransparent);
    hbox->addStretch();

    QHBoxLayout * hbox2 = new QHBoxLayout();
    hbox2->addWidget(selectImgBtn);
    hbox2->addWidget(viewFileCombo);
    hbox2->addWidget(viewImageBtn);

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addLayout(hbox2);

    QGroupBox * imageGroup = new QGroupBox("View Image File");
    imageGroup->setLayout(vbox);

    unsigned int index =0;
    for (auto file : config->viewImages)
    {
        if (!file.isEmpty())
            viewFileCombo->insertItem(index++,file);
    }
    viewFileCombo->setCurrentIndex(0);
    chkTransparent->setChecked(config->view_transparent);
    chkPopup->setChecked(config->view_popup);

    connect(selectImgBtn,             &QPushButton::clicked,   this,   &page_image_tools::slot_selectImage);
    connect(viewImageBtn,             &QPushButton::clicked,   this,   &page_image_tools::slot_viewImage);
    connect(chkTransparent,           &QCheckBox::clicked,     this,   &page_image_tools::slot_view_transparentClicked);
    connect(chkPopup,                 &QCheckBox::clicked,     this,   &page_image_tools::slot_view_popupClicked);
    connect(viewFileCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &page_image_tools::slot_imageSelectionChanged);

    return  imageGroup;
}

QGroupBox * page_image_tools::createTransparencySection()
{
    colorLabel   = new QLabel();
    colorLabel->setFixedWidth(121);

    QPushButton * colorEdit     = new QPushButton("Set Filter Color");
    QCheckBox   * chkUseFilter  = new QCheckBox("Use Color Filter");

    QHBoxLayout * hbox = new QHBoxLayout();

    hbox->addWidget(chkUseFilter);
    hbox->addWidget(colorLabel);
    hbox->addWidget(colorEdit);
    hbox->addStretch();

    QGroupBox * imageGroup = new QGroupBox("Transparency Filter");
    imageGroup->setLayout(hbox);

    QVariant variant = config->transparentColor;
    QString colcode  = variant.toString();
    colorLabel->setStyleSheet("QLabel { background-color : "+colcode+" ; border : 1px solid black; }");

    chkUseFilter->setChecked(config->filter_transparent);

    connect(colorEdit,              &QPushButton::clicked,   this,   &page_image_tools::slot_colorEdit);
    connect(chkUseFilter,           &QCheckBox::clicked,     this,   &page_image_tools::slot_useFilter);

    return  imageGroup;
}

void  page_image_tools::onEnter()
{
    imageCompareResult->setText("");
}

void page_image_tools::onExit()
{
    panel->popPanelStatus();

    view->clearLayout();   // removes any cler pngs
    view->show();
}

void  page_image_tools::onRefresh()
{
    QString str = QString("Worklist contains %1 entries").arg(config->workList.size());
    wlistStatus->setText(str);
}

void page_image_tools::slot_stopIfDiffClicked(bool enb)
{
    config->stopIfDiff = enb;
}

void page_image_tools::slot_cycleIntervalChanged(int value)
{
    config->cycleInterval = value;
}

void page_image_tools::slot_dir0Changed()
{
    QString dir = leftDir->text();
    config->compareDir0 = dir;
    loadCombo(leftFile,dir);
}

void page_image_tools::slot_dir1Changed()
{
    QString dir = rightDir->text();
    config->compareDir1 = dir;
    loadCombo(rightFile,dir);
}

void page_image_tools::selectDir0()
{
    QString  dir = leftDir->text();
    QString fdir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), dir,
                                                 QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (fdir.isEmpty())
            return;

    config->compareDir0 = fdir;
    leftDir->setText(fdir);
    slot_dir0Changed();
}

void page_image_tools::selectDir1()
{
    QString  dir = rightDir->text();
    QString fdir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), dir,
                                                 QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (fdir.isEmpty())
            return;

    config->compareDir1 = fdir;
    rightDir->setText(fdir);
    slot_dir1Changed();
}

void page_image_tools::swapDirs()
{
    QString a = config->compareDir0;
    QString b = config->compareDir1;
    config->compareDir0 = b;
    config->compareDir1 = a;
    leftDir->setText(b);
    rightDir->setText(a);
}


void page_image_tools::slot_viewImage()
{
    qDebug() << "slot_viewImage";
    QString file = viewFileCombo->currentText();
    viewImage(file,config->view_transparent,config->view_popup);
}

void page_image_tools::slot_viewImageLeft()
{
    qDebug() << "slot_viewImageLeft";
    QString file = leftFile->currentText();
    config->image0 = file;
    QString path = leftDir->text() + "/" + file + ".bmp";
    viewImage(path,config->compare_transparent,config->compare_popup);
}

void page_image_tools::slot_viewImageRight()
{
    qDebug() << "slot_viewImageRight";
    QString file = rightFile->currentText();
    config->image1 = file;
    QString path = rightDir->text() + "/" + file + ".bmp";
    viewImage(path,config->compare_transparent,config->compare_popup);
}

void page_image_tools::viewImage(QString file, bool transparent, bool popup)
{
    imageCompareResult->setText("");

    QPixmap pixmap(file);
    if (pixmap.isNull())
    {
        QMessageBox box(this);
        box.setText("Image not found or not valid");
        box.exec();
        return;
    }

    emit sig_view_image(file,file,transparent,popup);     // use same file
}

void page_image_tools::slot_cycleGen()
{
    if (localCycle)
    {
        localCycle = false;
        return;
    }

    localCycle = true;

    switch (config->genCycle)
    {
    case CYCLE_SAVE_STYLE_BMPS:
        panel->selectViewer(VIEW_MOSAIC);
        saveMosaicBitmaps();
        break;

    case CYCLE_SAVE_TILING_BMPS:
        panel->selectViewer(VIEW_TILING);
        saveTilingBitmaps();
        break;

    default:
        localCycle = false;
        break;
    }
}

void page_image_tools::slot_cycleView()
{
    switch (config->viewCycle)
    {
    case CYCLE_STYLES:
        panel->selectViewer(VIEW_MOSAIC);
        emit sig_cyclerStart(CYCLE_STYLES);
        break;

    case CYCLE_TILINGS:
        panel->selectViewer(VIEW_TILING);
        emit sig_cyclerStart(CYCLE_TILINGS);
        break;

    case CYCLE_ORIGINAL_PNGS:
        emit sig_cyclerStart(CYCLE_ORIGINAL_PNGS);
        break;

    default:
        break;
    }
}

void page_image_tools::continueCycle()
{
    emit theApp->sig_closeAllImageViewers();
    emit theApp->sig_ready();
}

void page_image_tools::saveMosaicBitmaps()
{
    auto id = config->fileFilter;
    auto files = FileServices::getMosaicNames(id);

    for (const auto & name : qAsConst(files))
    {
        if (!localCycle)
        {
            QMessageBox box(this);
            box.setIcon(QMessageBox::Warning);
            box.setText("Cycle terminated");
            box.exec();
            return;
        }

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

        if (loadMosaic(name))
        {
            savePixmap(name);
        }
        qApp->processEvents();
    }

    localCycle = false;

    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    box.setText("Cycle complete");
    box.exec();
}

bool page_image_tools::loadMosaic(QString name)
{
    config->currentlyLoadedXML.clear();

    MosaicManager mm;
    if (mm.loadMosaic(name))
    {
        config->lastLoadedXML      = name;
        config->currentlyLoadedXML = name;

        view->slot_refreshView();
        view->repaint();
        return true;
    }
    return false;
}

void page_image_tools::saveTilingBitmaps()
{
    auto id = config->fileFilter;
    auto files = FileServices::getTilingNames(id);

    for (const auto & name : qAsConst(files))
    {
        if (!localCycle)
        {
            QMessageBox box(this);
            box.setIcon(QMessageBox::Warning);
            box.setText("Cycle terminated");
            box.exec();
            return;
        }

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

        TilingManager tm;
        TilingPtr tp = tm.loadTiling(name,SM_LOAD_SINGLE);
        if (tp)
        {
            view->slot_refreshView();
            view->repaint();
            savePixmap(name);
        }
        qApp->processEvents();
    }

    localCycle = false;

    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    box.setText("Cycle complete");
    box.exec();
}

void page_image_tools::slot_compareImages()
{
    panel->pushPanelStatus("Spacebar=next C=compare P=ping-pong S=side-by-side L=log Q=quit D=delete-from-worklist");

    imageCompareResult->setText("");
    emit sig_compareImageFiles(leftFile->currentText(),rightFile->currentText(),false);
}

void page_image_tools::slot_compareCycle()
{
    panel->pushPanelStatus("Spacebar=next C=compare P=ping-pong S=side-by-side L=log Q=quit D=delete-from-worklist");

    if (config->use_workListForCompare)
        emit sig_cyclerStart(CYCLE_COMPARE_WORKLIST_IMAGES);
    else
        emit sig_cyclerStart(CYCLE_COMPARE_ALL_IMAGES);
}

void page_image_tools::slot_transparentClicked(bool checked)
{
    config->compare_transparent = checked;
}

void page_image_tools::slot_view_transparentClicked(bool checked)
{
    config->view_transparent = checked;
}

void page_image_tools::slot_view_popupClicked(bool checked)
{
    config->view_popup = checked;
}

void page_image_tools::slot_popupClicked(bool checked)
{
    config->compare_popup = checked;
}

void page_image_tools::slot_differencesClicked(bool checked)
{
    config->display_differences = checked;
}

void page_image_tools::slot_use_worklist_compare(bool checked)
{
    config->use_workListForCompare = checked;
    if (checked)
    {
        config->generate_workList = false;
        gen_wlistChk->setChecked(false);    // do both

    }
    slot_dir0Changed();
    slot_dir1Changed();
}

void page_image_tools::slot_gen_worklist_compare(bool checked)
{
    config->generate_workList = checked;
    if (checked)
    {
        config->use_workListForCompare = false;
        use_wlistForCompareChk->setChecked(false); // do both
    }
    slot_dir0Changed();
    slot_dir1Changed();
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
    int index = leftFile->findText(name);
    leftFile->setCurrentIndex(index);
}

void page_image_tools::slot_setImage1(QString name)
{
    int index = rightFile->findText(name);
    rightFile->setCurrentIndex(index);
}

void page_image_tools::slot_ibox0_changed(int index)
{
    Q_UNUSED(index);
    config->image0 = leftFile->currentText();

    // special case for ibox1 - not symmetric
    slot_setImage1(config->image0);  // makes it the same
}

void page_image_tools::slot_ibox1_changed(int index)
{
    Q_UNUSED(index);
    config->image1 = rightFile->currentText();
}

void page_image_tools::slot_previous()
{
    int index = leftFile->currentIndex();
    if (index == 0) return;
    index--;
    leftFile->setCurrentIndex(index);
    imageCompareResult->setText("");
    emit theApp->sig_closeAllImageViewers();
    emit sig_compareImageFiles(leftFile->currentText(),rightFile->currentText(),false);
}

void page_image_tools::slot_next()
{
    int index = leftFile->currentIndex();
    if (index >= leftFile->count()-1) return;
    index++;
    leftFile->setCurrentIndex(index);
    imageCompareResult->setText("");
    emit theApp->sig_closeAllImageViewers();
    emit sig_compareImageFiles(leftFile->currentText(),rightFile->currentText(),false);
}

void page_image_tools::slot_load()
{
    QString mos = leftFile->currentText();
    emit sig_loadMosaic(mos,true);
}

void page_image_tools::loadCurrent()
{
    QString mos = rightFile->currentText();
    emit sig_loadMosaic(mos,false);
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
    QString path   = getPixmapPath();
    QString file   = path + "/" + name + ".bmp";
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

    QString path2 = QDir::toNativeSeparators(path);
    qDebug() <<  "Path2:" << path2;

    QStringList args;
    args << path2;

    QProcess::startDetached("explorer",args);
#endif
}

void page_image_tools::loadWorkListFromFile()
{
    QString dir = config->worklistsDir;
    QString fileName = QFileDialog::getOpenFileName(this,"Select text file",dir, "All  files (*)");
    if (fileName.isEmpty())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Not loaded");
        box.exec();
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox box(this);
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
        name = name.trimmed();
        name.remove(".xml");
        stringList << name;
    }
    file.close();

    qDebug() << "file list: " << stringList;
    if (stringList.empty())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText(QString("No filenames found in %1").arg(fileName));
        box.exec();
    }

    config->workList = stringList;

    int sz = stringList.size();

    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    box.setText(QString("%1 filenames loaded into Work List - OK").arg(sz));
    box.exec();

    use_wlistForCompareChk->setChecked(true);
    slot_use_worklist_compare(true);    // do both
}

void page_image_tools::saveWorkListToFile()
{
    if (config->workList.isEmpty())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Empty Work List - nothing to save");
        box.exec();
        return;
    }

    QString dir = config->worklistsDir;
    QString fileName = QFileDialog::getSaveFileName(this,"Select text file",dir, "All  files (*)");
    if (fileName.isEmpty())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Worklist not saved");
        box.exec();
        return;
    }

    if (!fileName.contains(".txt"))
        fileName  += ".txt";

    qDebug() << "saving" << fileName;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText(QString("Text file <%1> failed to open").arg(fileName));
        box.exec();
        return;
    }

    QTextStream ts(&file);

    QStringList::const_iterator constIterator;
    for (constIterator = config->workList.constBegin(); constIterator != config->workList.constEnd(); ++constIterator)
    {
       ts << (*constIterator).toLocal8Bit().constData() << endl;
    }
    file.close();

    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    box.setText("Work List saved - OK");
    box.exec();
}

void page_image_tools::editWorkList()
{
    WorklistWidget * plw = new WorklistWidget(this);
    plw->addItems(config->workList);

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addWidget(plw);

    QDialog * ewl = new QDialog(this);
    ewl->setAttribute(Qt::WA_DeleteOnClose);
    ewl->setLayout(vbox);

    ewl->exec();
}

void page_image_tools::replaceCurrent()
{
    qDebug() << "slot_replaceCurrent";
    if (!config->use_workListForCompare)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Worklist not in use");
        box.exec();
        return;
    }

    QString name = rightFile->currentText();
    bool rv = loadMosaic(name);
    if (rv)
    {
        QPixmap pixmap = view->grab();
        QString path   = rightDir->text();
        QString file   = path + "/" + name + ".bmp";
        qDebug() << "saving" << file;

        rv = pixmap.save(file);
    }
    if (rv)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Information );
        box.setText("Bitmap replacement : OK");
        box.exec();
    }
    else
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Bitmap replacement : FAILED");
        box.exec();
    }

    // recompare newly saved
    emit theApp->sig_closeAllImageViewers();
    emit sig_compareImageFiles(leftFile->currentText(),rightFile->currentText(),false);
}

void page_image_tools::slot_deleteCurrent()
{
    qDebug() << "slot_deleteCurrent";
    if (!config->use_workListForCompare)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Worklist not in use");
        box.exec();
        return;
    }

    QString name = rightFile->currentText();
    QStringList newList;
    for (int i = 0; i < config->workList.size(); ++i)
    {
        if (config->workList.at(i) != name)
        {
            newList << config->workList.at(i);
        }
        else
            qDebug() << name << ": deleted";
    }
    config->workList = newList;
    emit theApp->sig_ready();
}

void page_image_tools::slot_useFilter(bool checked)
{
    config->filter_transparent = checked;
}

void page_image_tools::slot_colorEdit()
{
    AQColorDialog dlg(config->transparentColor,this);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted) return;

    QColor newColor = dlg.selectedColor();
    if (newColor.isValid())
    {
        config->transparentColor = newColor;

        QVariant variant = config->transparentColor;
        QString colcode  = variant.toString();
        colorLabel->setStyleSheet("QLabel { background-color : "+colcode+" ;}");
    }
}

void page_image_tools::slot_selectImage()
{
    QString dir = config->rootMediaDir;
    QString fileName = QFileDialog::getOpenFileName(this,"Select image file",dir, "Image Files (*.png *.jpg *.bmp *.heic)");
    if (fileName.isEmpty())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("FILE NOT FOUND");
        box.exec();
        return;
    }
    viewFileCombo->setCurrentText(fileName);
}

void page_image_tools::slot_imageSelectionChanged()
{
    viewFileCombo->select(viewFileCombo->currentIndex());
}
