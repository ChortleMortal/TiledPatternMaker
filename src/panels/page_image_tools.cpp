#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QRadioButton>
#include <QFileDialog>
#include <QMessageBox>

#include "makers/mosaic_maker/mosaic_maker.h"
#include "misc/cycler.h"
#include "misc/fileservices.h"
#include "misc/tpm_io.h"
#include "mosaic/mosaic.h"
#include "mosaic/mosaic_manager.h"
#include "panels/page_image_tools.h"
#include "panels/panel.h"
#include "settings/configuration.h"
#include "tile/tiling_manager.h"
#include "tiledpatternmaker.h"
#include "tiledpatternmaker.h"
#include "viewers/viewcontrol.h"
#include "widgets/dlg_wlist_create.h"
#include "widgets/layout_sliderset.h"
#include "widgets/memory_combo.h"
#include "widgets/versioned_list_widget.h"

page_image_tools:: page_image_tools(ControlPanel * cpanel)  : panel_page(cpanel,"Image Tools")
{
    created            = false;
    config->localCycle = false;

    QGroupBox * gbox;
    gbox = createCycleGenBox();
    vbox->addWidget(gbox);
    auto hbox = createWorklistBox();
    vbox->addLayout(hbox);
    gbox = createCompareImagesBox();
    vbox->addWidget(gbox);
    gbox = createCompareVersionsBox();
    vbox->addWidget(gbox);
    gbox = createViewImageBox();
    vbox->addWidget(gbox);
    gbox = createTransparencyBox();
    vbox->addWidget(gbox);

    setMaximumWidth(762);

    connect(theApp,&TiledPatternMaker::sig_compareResult,       this,   &page_image_tools::slot_compareResult);
    connect(this,  &page_image_tools::sig_view_image,           theApp, &TiledPatternMaker::slot_view_image);
    connect(this,  &page_image_tools::sig_compareBMPFiles,      theApp, &TiledPatternMaker::slot_compareBMPs,           Qt::QueuedConnection);
    connect(this,  &page_image_tools::sig_compareBMPFilesPath,  theApp, &TiledPatternMaker::slot_compareBMPsPath,       Qt::QueuedConnection);
    connect(this,  &page_image_tools::sig_compareBMPandLoaded,  theApp, &TiledPatternMaker::slot_compareBMMPandLoaded,  Qt::QueuedConnection);
    connect(this,  &page_image_tools::sig_loadMosaic,           mosaicMaker, &MosaicMaker::slot_loadMosaic);

    Cycler * cycler = Cycler::getInstance();
    connect(this,  &page_image_tools::sig_cyclerStart,          cycler,  &Cycler::slot_startCycle,                      Qt::QueuedConnection);
    connect(cycler, &Cycler::sig_workList,                      this,    &page_image_tools::slot_firstDirChanged,       Qt::QueuedConnection);

    ViewControl * vc = ViewControl::getInstance();
    connect(vc, &ViewControl::sig_cyclerQuit,                   this, [this] { config->localCycle = false; });

    created = true;
}

QGroupBox * page_image_tools::createCycleGenBox()
{
    //BMP generation
    QPushButton * generateBtn  = new QPushButton("Generate BMPS");
    generateBtn->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    QRadioButton * rSavMosaics = new QRadioButton("Mosaics");
    QRadioButton * rSavTiles   = new QRadioButton("Tilings");
    QCheckBox    * skip        = new QCheckBox("Skip Existing");
              fileFilterCombo  = new QComboBox();
                   directory   = new QLineEdit("");
    QPushButton  * saveDirBtn  = new QPushButton("Dir");

    directory->setMinimumWidth(151);
    setImageDirectory();

    skip->setChecked(config->skipExisting);

    cycleGenBtnGroup = new QButtonGroup;
    cycleGenBtnGroup->addButton(rSavMosaics,CYCLE_SAVE_MOSAIC_BMPS);
    cycleGenBtnGroup->addButton(rSavTiles,  CYCLE_SAVE_TILING_BMPS);
    cycleGenBtnGroup->button(config->genCycle)->setChecked(true);

    connect(generateBtn,        &QPushButton::clicked,     this,  &page_image_tools::slot_cycleGen);
    connect(saveDirBtn,         &QPushButton::clicked,     this,  &page_image_tools::slot_opendir);
    connect(skip,               &QCheckBox::clicked,       this,  &page_image_tools::slot_skipExisting);
    connect(cycleGenBtnGroup,   &QButtonGroup::idClicked,  this,  &page_image_tools::slot_genTypeChanged);
    connect(fileFilterCombo,    QOverload<int>::of(&QComboBox::currentIndexChanged), this, &page_image_tools::slot_gen_selectionChanged);

    loadFileFilterCombo();

    // Image Viewing
    SpinSet     * spCycleInterval   = new SpinSet("Cycle Interval",0,0,9);
    QPushButton * viewBtn           = new QPushButton("View Images");
    generateBtn->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    QRadioButton * rStyles    = new QRadioButton("Mosaics");
    QRadioButton * rTiles     = new QRadioButton("Tilings");
    QRadioButton * rPngs      = new QRadioButton("Original PNGs");

    cycleViewBtnGroup = new QButtonGroup;
    cycleViewBtnGroup->addButton(rStyles,CYCLE_MOSAICS);
    cycleViewBtnGroup->addButton(rTiles,CYCLE_TILINGS);
    cycleViewBtnGroup->addButton(rPngs,CYCLE_ORIGINAL_PNGS);

    cycleViewBtnGroup->button(config->viewCycle)->setChecked(true);

    // Layout
    QGridLayout * grid = new QGridLayout;

    QHBoxLayout * hb1 = new QHBoxLayout;
    hb1->addWidget(directory);
    hb1->addWidget(saveDirBtn);

    QHBoxLayout * hb2 = new QHBoxLayout;
    hb2->addWidget(rSavMosaics );
    hb2->addWidget(rSavTiles);
    hb2->addWidget(fileFilterCombo);
    hb2->addWidget(skip);
    hb2->addStretch();

    grid->addLayout(hb1,0,0);
    grid->addLayout(hb2,0,1);
    grid->addWidget(generateBtn,0,2);

    QHBoxLayout * hb3 = new QHBoxLayout;
    hb3->addLayout(spCycleInterval);

    QHBoxLayout * hb4 = new QHBoxLayout;
    hb4->addWidget(rStyles);
    hb4->addWidget(rTiles);
    hb4->addWidget(rPngs);
    hb4->addStretch();

    grid->addLayout(hb3,1,0);
    grid->addLayout(hb4,1,1);
    grid->addWidget(viewBtn,1,2);

    QGroupBox * cycleViewBox = new  QGroupBox("Generate BMPs and View Images");
    cycleViewBox->setLayout(grid);

    spCycleInterval->setValue(config->cycleInterval);

    connect(spCycleInterval,    &SpinSet::valueChanged,    this,  &page_image_tools::slot_cycleIntervalChanged);
    connect(viewBtn,            &QPushButton::clicked,     this,  &page_image_tools::slot_cycleView);
    connect(cycleViewBtnGroup,  &QButtonGroup::idClicked,  this,  [=](int id) { config->viewCycle = static_cast<eCycleMode>(id); } );

    return cycleViewBox;
}

QHBoxLayout * page_image_tools::createWorklistBox()
{
    QPushButton * loadListBtn   = new QPushButton("Load");
    QPushButton * saveListBtn   = new QPushButton("Save");
    QPushButton * createListBtn = new QPushButton("Create");
    QPushButton * editListBtn   = new QPushButton("Edit");
    QPushButton * delBtn        = new QPushButton("Delete");
    QPushButton * replBtn       = new QPushButton("Replace Second Dir BMP");
                  loadFirstBtn  = new QPushButton("Load First");
                  loadSecondBtn = new QPushButton("Load Second");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(loadListBtn);
    hbox->addWidget(saveListBtn);
    hbox->addWidget(createListBtn);
    hbox->addWidget(editListBtn);
    hbox->addWidget(delBtn);
    wlistGroupBox = new  QGroupBox("Work List operations");
    wlistGroupBox->setLayout(hbox);

    QHBoxLayout * hbox2 = new QHBoxLayout;
    hbox2->addWidget(replBtn,0,Qt::AlignVCenter);
    hbox2->addWidget(loadFirstBtn);
    hbox2->addWidget(loadSecondBtn);
    auto wlistGroupBox2 = new  QGroupBox("View/BMP operations");
    wlistGroupBox2->setLayout(hbox2);

    QHBoxLayout * hbox3 = new QHBoxLayout;
    hbox3->addWidget(wlistGroupBox);
    hbox3->addWidget(wlistGroupBox2);

    connect(loadListBtn,  &QPushButton::clicked,     this,  &page_image_tools::loadWorkListFromFile);
    connect(saveListBtn,  &QPushButton::clicked,     this,  &page_image_tools::saveWorkListToFile);
    connect(editListBtn,  &QPushButton::clicked,     this,  &page_image_tools::editWorkList);
    connect(delBtn,       &QPushButton::clicked,     this,  [this] () { slot_deleteCurrentWLEntry(true);} );
    connect(replBtn,      &QPushButton::clicked,     this,  &page_image_tools::replaceBMP);
    connect(createListBtn,&QPushButton::clicked,     this,  &page_image_tools::slot_createList);
    connect(loadFirstBtn, &QPushButton::clicked,     this,  &page_image_tools::slot_loadFirst);
    connect(loadSecondBtn,&QPushButton::clicked,     this,  &page_image_tools::slot_loadSecond);
    connect(theApp,&TiledPatternMaker::sig_deleteCurrentInWorklist, this, &page_image_tools::slot_deleteCurrentWLEntry);

    return hbox3;
}

QGroupBox * page_image_tools::createCompareImagesBox()
{
    firstFileCombo      = new QComboBox();
    firstFileCombo->setMinimumWidth(461);
    secondFileCombo     = new QComboBox();
    imageCompareResult = new QLineEdit();
    imageCompareResult->setReadOnly(true);

    QPushButton * compareDirFirstBtn  = new QPushButton("First Dir");
    QPushButton * compareDirSecondBtn = new QPushButton("Second Dir");
    firstDir                          = new MemoryCombo("leftDir");
    secondDir                         = new MemoryCombo("rightDir");

    QPushButton * viewImage0   = new QPushButton("View First");
    QPushButton * viewImage1   = new QPushButton("View Second");
    QPushButton * compareBtn   = new QPushButton("Compare");
    QPushButton * previousBtn  = new QPushButton("Previous");
    QPushButton * nextBtn      = new QPushButton("Next");
    QPushButton * reviewBtn    = new QPushButton("Cycle");

    reviewBtn->setStyleSheet( "QPushButton { background-color: yellow; color: red;}");
    compareBtn->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    QPushButton * swapBtn      = new QPushButton("Swap");
    QPushButton * continueBtn  = new QPushButton("Continue");

    QCheckBox   * cbStopIfDiff = new QCheckBox("Stop if Diff");
    QCheckBox   * differences  = new QCheckBox("Show Diffs");
    QCheckBox   * chkPopup     = new QCheckBox("Pop-up");
    QCheckBox   * transparent  = new QCheckBox("Transparent");
    compareView                = new QCheckBox("Compare with View");
    use_wlistForCompareChk     = new QCheckBox("Use WorkList");
    gen_wlistChk               = new QCheckBox("Gen WorkList");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(cbStopIfDiff);
    hbox->addWidget(differences);
    hbox->addWidget(chkPopup);
    hbox->addWidget(transparent);
    hbox->addWidget(use_wlistForCompareChk);
    hbox->addWidget(gen_wlistChk);

    QHBoxLayout * hbox2 = new QHBoxLayout;
    hbox2->addWidget(imageCompareResult);
    hbox2->addWidget(compareView);

    QGridLayout * imageGrid = new QGridLayout();

    int row = 0;
    imageGrid->addLayout(hbox,row,0,1,2);
    imageGrid->addWidget(reviewBtn,row,2);

    row++;
    imageGrid->addWidget(compareDirFirstBtn,row,0);
    imageGrid->addWidget(firstDir,row,1);
    imageGrid->addWidget(swapBtn,row,2);

    row++;
    imageGrid->addWidget(compareDirSecondBtn,row,0);
    imageGrid->addWidget(secondDir,row,1);
    imageGrid->addWidget(continueBtn,row,2);

    row++;
    imageGrid->addWidget(previousBtn,row,0);
    imageGrid->addWidget(firstFileCombo,row,1);
    imageGrid->addWidget(viewImage0,row,2);

    row++;
    imageGrid->addWidget(nextBtn,row,0);
    imageGrid->addWidget(secondFileCombo,row,1);
    imageGrid->addWidget(viewImage1,row,2);

    row++;
    imageGrid->addLayout(hbox2,row,1);
    imageGrid->addWidget(compareBtn,row,2);

    QGroupBox * imageGroup = new QGroupBox("Compare Images");
    imageGroup->setLayout(imageGrid);

    firstDir->initialise();
    loadCombo(firstFileCombo,firstDir->getCurrentText());
    secondDir->initialise();
    loadCombo(secondFileCombo,secondDir->getCurrentText());

    cbStopIfDiff->setChecked(config->stopIfDiff);
    transparent->setChecked(config->compare_transparent);
    chkPopup->setChecked(config->compare_popup);
    differences->setChecked(config->display_differences);
    use_wlistForCompareChk->setChecked(config->use_workListForCompare);
    gen_wlistChk->setChecked(config->generate_workList);

    connect(swapBtn,                &QPushButton::clicked,     this,  &page_image_tools::swapDirs);
    connect(continueBtn,            &QPushButton::clicked,     this,  &page_image_tools::continueCycle);
    connect(compareDirFirstBtn,     &QPushButton::clicked,     this,  &page_image_tools::selectDir0);
    connect(compareDirSecondBtn,    &QPushButton::clicked,     this,  &page_image_tools::selectDir1);

    connect(viewImage0,             &QPushButton::clicked,     this,   &page_image_tools::slot_viewImageLeft);
    connect(viewImage1,             &QPushButton::clicked,     this,   &page_image_tools::slot_viewImageRight);
    connect(compareBtn,             &QPushButton::clicked,     this,   &page_image_tools::slot_compareImages);
    connect(reviewBtn,              &QPushButton::clicked,     this,   &page_image_tools::slot_compareCycle);
    connect(previousBtn,            &QPushButton::clicked,     this,   &page_image_tools::slot_previous);
    connect(nextBtn,                &QPushButton::clicked,     this,   &page_image_tools::slot_next);

    connect(transparent,            &QCheckBox::clicked,       this,   &page_image_tools::slot_transparentClicked);
    connect(chkPopup,               &QCheckBox::clicked,       this,   &page_image_tools::slot_popupClicked);
    connect(cbStopIfDiff,           &QCheckBox::clicked,       this,   &page_image_tools::slot_stopIfDiffClicked);
    connect(differences,            &QCheckBox::clicked,       this,   &page_image_tools::slot_differencesClicked);
    connect(use_wlistForCompareChk, &QCheckBox::clicked,       this,   &page_image_tools::slot_use_worklist_compare);
    connect(gen_wlistChk,           &QCheckBox::clicked,       this,   &page_image_tools::slot_gen_worklist_compare);
    connect(compareView,            &QCheckBox::clicked,       this,   &page_image_tools::slot_compareView);

    connect(firstDir,   &MemoryCombo::editTextChanged,         this,   &page_image_tools::slot_firstDirChanged);
    connect(secondDir,  &MemoryCombo::editTextChanged,         this,   &page_image_tools::slot_secondDirChanged);
    connect(firstDir,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, &page_image_tools::slot_firstDirChanged);
    connect(secondDir, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &page_image_tools::slot_secondDirChanged);

    connect(firstFileCombo,  SIGNAL(currentIndexChanged(int)), this,   SLOT(slot_ibox0_changed(int)));
    connect(secondFileCombo, SIGNAL(currentIndexChanged(int)), this,   SLOT(slot_ibox1_changed(int)));

    setCombo(firstFileCombo,config->image0);
    setCombo(secondFileCombo,config->image1);

    return  imageGroup;
}

QGroupBox * page_image_tools::createCompareVersionsBox()
{
    radImg  = new QRadioButton("Images");
    radXML  = new QRadioButton("XML");
    chkLock = new QCheckBox("Lock");

    QPushButton  * compareBtn = new QPushButton("Compare");
    QPushButton  * cycleBtn   = new QPushButton("Cycle");
    compareBtn->setStyleSheet("QPushButton { background-color: yellow; color: red;}");
    cycleBtn->setStyleSheet  ("QPushButton { background-color: yellow; color: red;}");

    mosaicA    = new QComboBox();
    versionsA  = new QComboBox();
    mosaicB    = new QComboBox();
    versionsB  = new QComboBox();

    QPushButton * viewImageBtn3  = new QPushButton("View");
    QPushButton * viewImageBtn4  = new QPushButton("View");
    QLabel      * dummyLabel     = new QLabel(" ");
    dummyLabel->setMinimumWidth(79);

    QWidget * container = new QWidget;
    container->setMinimumWidth(461);
    AQHBoxLayout * hbox = new AQHBoxLayout();
    hbox->addStretch();
    hbox->addWidget(radImg);
    hbox->addWidget(radXML);
    hbox->addWidget(chkLock);
    container->setLayout(hbox);

    QGridLayout * grid = new QGridLayout();
    int row = 0;

    hbox = new AQHBoxLayout();
    hbox->addWidget(mosaicA);
    hbox->addWidget(versionsA);
    grid->addWidget(viewImageBtn3,row,0);
    grid->addLayout(hbox,row,1);
    grid->addWidget(cycleBtn,row,2);

    row++;
    hbox = new AQHBoxLayout();
    hbox->addWidget(mosaicB);
    hbox->addWidget(versionsB);
    grid->addWidget(viewImageBtn4,row,0);
    grid->addLayout(hbox,row,1);

    row++;
    grid->addWidget(dummyLabel,row,0);
    grid->addWidget(container,row,1);
    grid->addWidget(compareBtn,row,2);

    QGroupBox * imageGroup = new QGroupBox("Compare Versions");
    imageGroup->setLayout(grid);

    chkLock->setChecked(config->vCompLock);
    if (config->vCompXML)
        radXML->setChecked(true);
    else
        radImg->setChecked(true);

    connect(compareBtn,     &QPushButton::clicked,   this,   &page_image_tools::slot_compareVersions);
    connect(cycleBtn,       &QPushButton::clicked,   this,   &page_image_tools::slot_cycleVersions);
    connect(viewImageBtn3,  &QPushButton::clicked,   this,   &page_image_tools::slot_viewImage3);
    connect(viewImageBtn4,  &QPushButton::clicked,   this,   &page_image_tools::slot_viewImage4);
    connect(mosaicA,        QOverload<int>::of(&QComboBox::currentIndexChanged), this, &page_image_tools::slot_mosaicAChanged);
    connect(mosaicB,        QOverload<int>::of(&QComboBox::currentIndexChanged), this, &page_image_tools::slot_mosaicBChanged);
    connect(chkLock,        &QCheckBox::clicked,     this,   [this](bool checked) { config->vCompLock = checked; });
    connect(radXML,         &QRadioButton::clicked,  this,   [this](bool checked) { config->vCompXML  = checked; });
    connect(radImg,         &QRadioButton::clicked,  this,   [this](bool checked) { config->vCompXML  = !checked; });
    connect(theApp,         &TiledPatternMaker::sig_takeNext,   this, &page_image_tools::slot_nextImage);
    connect(theApp,         &TiledPatternMaker::sig_cyclerQuit, this, &page_image_tools::slot_quitImageCycle);

    loadVersionCombos();

    return  imageGroup;
}

QGroupBox * page_image_tools::createViewImageBox()
{
    QPushButton * selectImgBtn1  = new QPushButton("Select");
    QPushButton * selectImgBtn2  = new QPushButton("Select");
    QPushButton * viewImageBtn1  = new QPushButton("View");
    QPushButton * viewImageBtn2  = new QPushButton("View");
    QCheckBox   * chkTransparent = new QCheckBox("Transparent");
    QCheckBox   * chkPopup       = new QCheckBox("Pop up");
    QPushButton * compareBtn   = new QPushButton("Compare");
    viewFileCombo1  = new MemoryCombo("viewFileCombo1");
    viewFileCombo2  = new MemoryCombo("viewFileCombo2");
    viewFileCombo1->setMinimumWidth(549);
    viewFileCombo2->setMinimumWidth(549);
    compareBtn->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addStretch();
    hbox->addWidget(chkPopup);
    hbox->addWidget(chkTransparent);

    QGridLayout * grid = new QGridLayout();
    int row = 0;

    grid->addWidget(selectImgBtn1,row,0);
    grid->addWidget(viewFileCombo1,row,1);
    grid->addWidget(viewImageBtn1,row,2);

    row++;
    grid->addWidget(selectImgBtn2,row,0);
    grid->addWidget(viewFileCombo2,row,1);
    grid->addWidget(viewImageBtn2,row,2);

    row++;
    grid->addLayout(hbox,row,0,1,2);
    grid->addWidget(compareBtn,row,2);

    QGroupBox * imageGroup = new QGroupBox("View Image File");
    imageGroup->setLayout(grid);

    viewFileCombo1->initialise();
    viewFileCombo2->initialise();
    chkTransparent->setChecked(config->view_transparent);
    chkPopup->setChecked(config->view_popup);

    connect(selectImgBtn1,  &QPushButton::clicked,   this,   &page_image_tools::slot_selectImage1);
    connect(selectImgBtn2,  &QPushButton::clicked,   this,   &page_image_tools::slot_selectImage2);
    connect(viewImageBtn1,  &QPushButton::clicked,   this,   &page_image_tools::slot_viewImage1);
    connect(viewImageBtn2,  &QPushButton::clicked,   this,   &page_image_tools::slot_viewImage2);
    connect(chkTransparent, &QCheckBox::clicked,     this,   &page_image_tools::slot_view_transparentClicked);
    connect(chkPopup,       &QCheckBox::clicked,     this,   &page_image_tools::slot_view_popupClicked);
    connect(compareBtn,     &QPushButton::clicked,   this,   &page_image_tools::slot_compareImages2);
    connect(viewFileCombo1, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &page_image_tools::slot_imageSelectionChanged1);
    connect(viewFileCombo2, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &page_image_tools::slot_imageSelectionChanged2);

    return  imageGroup;
}

QGroupBox * page_image_tools::createTransparencyBox()
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
    QString left  = firstFileCombo->currentText();
    QString right = secondFileCombo->currentText();
    loadCombo(firstFileCombo,firstDir->getCurrentText());
    loadCombo(secondFileCombo,secondDir->getCurrentText());
    slot_setImageLeftCombo(left);
    slot_setImageRightCombo(right);
    loadVersionCombos();
}

void page_image_tools::onExit()
{
    panel->popPanelStatus();

    view->clearLayout();   // removes any cler pngs
    view->show();
}

void  page_image_tools::onRefresh()
{
    Worklist & wlist = config->worklist;
    QString str = QString("Work Llist Operations : <%1> contains %2 entries").arg(wlist.name()).arg(wlist.get().size());
    wlistGroupBox->setTitle(str);
}


void page_image_tools::loadFileFilterCombo()
{
    fileFilterCombo->blockSignals(true);
    fileFilterCombo->clear();

    if (config->genCycle == CYCLE_SAVE_TILING_BMPS)
    {
        fileFilterCombo->addItem("All Tilings",     ALL_TILINGS);
        fileFilterCombo->addItem("Selected Tilings",SELECTED_TILINGS);
        fileFilterCombo->addItem("Worklist Tilings",WORKLIST);
    }
    else
    {
        Q_ASSERT(config->genCycle == CYCLE_SAVE_MOSAIC_BMPS);
        fileFilterCombo->addItem("All Mosaics",     ALL_MOSAICS);
        fileFilterCombo->addItem("Selected mosaics",SELECTED_MOSAICS);
        fileFilterCombo->addItem("Worklist Mosaics",WORKLIST);
    }

    fileFilterCombo->blockSignals(false);

    int index = fileFilterCombo->findData(config->fileFilter);
    if (index < 0)
        index = 0;
    fileFilterCombo->setCurrentIndex(index);
}

void page_image_tools::slot_stopIfDiffClicked(bool enb)
{
    config->stopIfDiff = enb;
}

void page_image_tools::slot_cycleIntervalChanged(int value)
{
    config->cycleInterval = value;
}

void page_image_tools::slot_firstDirChanged()
{
    firstDir->select(firstDir->currentIndex());
    loadCombo(firstFileCombo,firstDir->getCurrentText());
}

void page_image_tools::slot_secondDirChanged()
{
    secondDir->select(secondDir->currentIndex());
    loadCombo(secondFileCombo,secondDir->getCurrentText());
}

void page_image_tools::selectDir0()
{
    QString  dir = firstDir->getCurrentText();
    QString fdir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), dir,
                                                 QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (fdir.isEmpty())
            return;

    firstDir->setCurrentText(fdir);
    slot_firstDirChanged();
}

void page_image_tools::selectDir1()
{
    QString  dir = secondDir->getCurrentText();
    QString fdir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), dir,
                                                 QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (fdir.isEmpty())
            return;

    secondDir->setCurrentText(fdir);
    slot_secondDirChanged();
}

void page_image_tools::swapDirs()
{
    QString a = firstDir->getCurrentText();
    QString b = secondDir->getCurrentText();
    firstDir->setCurrentText(b);
    secondDir->setCurrentText(a);
    loadCombo(firstFileCombo,firstDir->getCurrentText());
    loadCombo(secondFileCombo,secondDir->getCurrentText());
}

void page_image_tools::slot_viewImageLeft()
{
    qDebug() << "slot_viewImageLeft";
    QString file = firstFileCombo->currentText();
    config->image0 = file;
    QString path = firstDir->getCurrentText() + "/" + file + ".bmp";
    viewImage(path,config->compare_transparent,config->compare_popup);
}

void page_image_tools::slot_viewImageRight()
{
    qDebug() << "slot_viewImageRight";
    QString file = secondFileCombo->currentText();
    config->image1 = file;
    QString path = secondDir->getCurrentText() + "/" + file + ".bmp";
    viewImage(path,config->compare_transparent,config->compare_popup);
}

bool page_image_tools::viewImage(QString file, bool transparent, bool popup)
{
    imageCompareResult->setText("");

    QPixmap pixmap(file);
    if (pixmap.isNull())
    {
        QMessageBox box(this);
        box.setText("Image not found or not valid");
        box.exec();
        return false;
    }

    emit sig_view_image(file,file,transparent,popup);     // use same file
    return true;
}

void page_image_tools::slot_cycleGen()
{
    if (config->localCycle)
    {
        config->localCycle = false;
        return;
    }

    config->localCycle = true;

    switch (config->genCycle)
    {
    case CYCLE_SAVE_MOSAIC_BMPS:
        panel->selectViewer(VIEW_MOSAIC);
        saveMosaicBitmaps();
        break;

    case CYCLE_SAVE_TILING_BMPS:
        panel->selectViewer(VIEW_TILING);
        saveTilingBitmaps();
        break;

    default:
        config->localCycle = false;
        break;
    }
}

void page_image_tools::slot_cycleView()
{
    switch (config->viewCycle)
    {
    case CYCLE_MOSAICS:
        panel->selectViewer(VIEW_MOSAIC);
        emit sig_cyclerStart(CYCLE_MOSAICS);
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
        if (!config->localCycle)
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

    config->localCycle = false;

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
        if (!config->localCycle)
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
        TilingPtr tp = tm.loadTiling(name,TILM_LOAD_SINGLE);
        if (tp)
        {
            view->slot_refreshView();
            view->repaint();
            savePixmap(name);
        }
        qApp->processEvents();
    }

    config->localCycle = false;

    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    box.setText("Cycle complete");
    box.exec();
}

void page_image_tools::slot_compareImages()
{

    imageCompareResult->setText("");
    panel->pushPanelStatus("Spacebar=next C=compare P=ping-pong S=side-by-side L=log Q=quit D=delete-from-worklist");
    if (!compareView->isChecked())
    {
        emit sig_compareBMPFiles(firstFileCombo->currentText(),secondFileCombo->currentText(),false);
    }
    else
    {
        emit sig_compareBMPandLoaded(firstFileCombo->currentText(),true);
    }
}

void page_image_tools::slot_compareImages2()
{
    panel->pushPanelStatus("Spacebar=next C=compare P=ping-pong S=side-by-side L=log Q=quit D=delete-from-worklist");

    emit sig_compareBMPFilesPath(viewFileCombo1->currentText(),viewFileCombo2->currentText());
}

void page_image_tools::slot_compareCycle()
{
    panel->pushPanelStatus("Spacebar=next C=compare P=ping-pong S=side-by-side L=log Q=quit D=delete-from-worklist");

    if (config->use_workListForCompare)
        emit sig_cyclerStart(CYCLE_COMPARE_WORKLIST_BMPS);
    else
        emit sig_cyclerStart(CYCLE_COMPARE_ALL_BMPS);
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
    slot_firstDirChanged();
    slot_secondDirChanged();
}

void page_image_tools::slot_gen_worklist_compare(bool checked)
{
    config->generate_workList = checked;
    if (checked)
    {
        config->use_workListForCompare = false;
        use_wlistForCompareChk->setChecked(false); // do both
    }
    slot_firstDirChanged();
    slot_secondDirChanged();
}

void page_image_tools::slot_compareView(bool checked)
{
    if (checked)
    {
        loadFirstBtn->setStyleSheet( "QPushButton { background-color: yellow; color: red;}");
        loadSecondBtn->setStyleSheet("QPushButton { background-color: yellow; color: red;}");
    }
    else
    {
        loadFirstBtn->setStyleSheet("");
        loadSecondBtn->setStyleSheet("");
    }
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
        names2 = config->worklist.get();
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

void page_image_tools::slot_setImageLeftCombo(QString name)
{
    int index = firstFileCombo->findText(name);
    firstFileCombo->setCurrentIndex(index);
}

void page_image_tools::slot_setImageRightCombo(QString name)
{
    int index = secondFileCombo->findText(name);
    secondFileCombo->setCurrentIndex(index);
}

void page_image_tools::slot_ibox0_changed(int index)
{
    Q_UNUSED(index);
    config->image0 = firstFileCombo->currentText();

    // special case for ibox1 - not symmetric
    slot_setImageRightCombo(config->image0);  // makes it the same
}

void page_image_tools::slot_ibox1_changed(int index)
{
    Q_UNUSED(index);
    config->image1 = secondFileCombo->currentText();
}

void page_image_tools::slot_previous()
{
    int index = firstFileCombo->currentIndex();
    if (index == 0) return;
    index--;
    firstFileCombo->setCurrentIndex(index);
    imageCompareResult->setText("");
    emit theApp->sig_closeAllImageViewers();
    emit sig_compareBMPFiles(firstFileCombo->currentText(),secondFileCombo->currentText(),false);
}

void page_image_tools::slot_next()
{
    int index = firstFileCombo->currentIndex();
    if (index >= firstFileCombo->count()-1) return;
    index++;
    firstFileCombo->setCurrentIndex(index);
    imageCompareResult->setText("");
    emit theApp->sig_closeAllImageViewers();
    emit sig_compareBMPFiles(firstFileCombo->currentText(),secondFileCombo->currentText(),false);
}

void page_image_tools::slot_loadFirst()
{
    QString mos = firstFileCombo->currentText();
    emit sig_loadMosaic(mos,true);
}

void page_image_tools::slot_loadSecond()
{
    QString mos = secondFileCombo->currentText();
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
    QString old = getPixmapPath();
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Image Directory"),
                                                    old, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty())
    {
        qDebug() << "Cancel Pressed";
        setImageDirectory();
        return;
    }

    if (dir != old)
    {
        qDebug() << "Selected image dir" << dir;
        auto i  = dir.lastIndexOf("/");
        auto front = dir.size() - i;
#if (QT_VERSION >= QT_VERSION_CHECK(6,5,0))
        dir = dir.last(front-1);
#else
        dir = dir.right(front-1);
#endif
        qDebug() << "Image dir" << dir;

        directory->setText(dir);
    }

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
    QString str = config->worklist.name();
    if (!str.isEmpty())
    {
        dir += str;
        dir += ".txt";
    }

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

    stringList.removeDuplicates();

    qDebug() << "file list: " << stringList;
    if (stringList.empty())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText(QString("No filenames found in %1").arg(fileName));
        box.exec();
        return;
    }

    QFileInfo fi(fileName);
    QString fname = fi.fileName();
    fname.remove(".txt");

    config->worklist.setName(fname);
    config->worklist.set(stringList);

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
    Worklist & wlist = config->worklist;

    config->worklist.removeDuplicates();

    const QStringList & list = wlist.get();
    if (list.isEmpty())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Empty Work List - nothing to save");
        box.exec();
        return;
    }

    QString dir = config->worklistsDir + wlist.name() + ".txt";
    QString fileName = QFileDialog::getSaveFileName(this,"Select text file",dir, "All  files (*)");
    if (fileName.isEmpty())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Worklist not saved");
        box.exec();
        return;
    }

    QFileInfo fi(fileName);
    QString saveName = fi.fileName();
    saveName.remove(".txt");

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
    for (constIterator = list.constBegin(); constIterator != list.constEnd(); ++constIterator)
    {
       ts << (*constIterator).toLocal8Bit().constData() << endl;
    }
    file.close();

    config->worklist.setName(saveName);

    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    box.setText("Work List saved - OK");
    box.exec();
}

void page_image_tools::editWorkList()
{
    WorklistWidget * plw = new WorklistWidget(this);
    plw->addItems(config->worklist.get());

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addWidget(plw);

    QDialog * ewl = new QDialog(this);
    ewl->setAttribute(Qt::WA_DeleteOnClose);
    ewl->setLayout(vbox);

    ewl->exec();
}

void page_image_tools::replaceBMP()
{
    // replace bitmp in second dir

    QString name = secondFileCombo->currentText();
    bool rv = loadMosaic(name);
    if (rv)
    {
        QPixmap pixmap = view->grab();
        QString path   = secondDir->getCurrentText();
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
    emit sig_compareBMPFiles(firstFileCombo->currentText(),secondFileCombo->currentText(),false);
}

void page_image_tools::slot_deleteCurrentWLEntry(bool confirm)
{
    // delete current worklist entry
    if (!config->use_workListForCompare)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Worklist not in use");
        box.exec();
        return;
    }

    if (confirm)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Question);
        box.setText("Delete current worklist entry?");
        box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        auto rv = box.exec();
        if (rv == QMessageBox::No)
            return;
    }

    QString name = secondFileCombo->currentText();
    QStringList newList;
    const QStringList & list = config->worklist.get();
    for (int i = 0; i < list.size(); ++i)
    {
        if (list.at(i) != name)
        {
            newList << list.at(i);
        }
        else
            qDebug() << name << ": deleted";
    }

    config->worklist.set(newList);
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

void page_image_tools::slot_viewImage1()
{
    qDebug() << "slot_viewImage1";
    QString file = viewFileCombo1->currentText();
    bool rv = viewImage(file,config->view_transparent,config->view_popup);
    if (!rv)
    {
        viewFileCombo1->eraseCurrent();
    }
}

void page_image_tools::slot_viewImage2()
{
    qDebug() << "slot_viewImage2";
    QString file = viewFileCombo2->currentText();
    bool rv = viewImage(file,config->view_transparent,config->view_popup);
    if (!rv)
    {
        viewFileCombo2->eraseCurrent();
    }
}

void page_image_tools::slot_selectImage1()
{
    QString dir;
    auto filename = viewFileCombo1->getCurrentText();
    if (filename.isEmpty())
    {
        dir = config->rootMediaDir;
    }
    else
    {
        QFileInfo info(filename);
        dir = info.absolutePath();
    }

    QString fileName = QFileDialog::getOpenFileName(this,"Select image file",dir, "Image Files (*.png *.jpg *.bmp *.heic)");
    if (fileName.isEmpty())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("FILE NOT FOUND");
        box.exec();
        return;
    }
    viewFileCombo1->setCurrentText(fileName);
}

void page_image_tools::slot_selectImage2()
{
    QString dir;
    auto filename = viewFileCombo2->getCurrentText();
    if (filename.isEmpty())
    {
        dir = config->rootMediaDir;
    }
    else
    {
        QFileInfo info(filename);
        dir = info.absolutePath();
    }

    QString fileName = QFileDialog::getOpenFileName(this,"Select image file",dir, "Image Files (*.png *.jpg *.bmp *.heic)");
    if (fileName.isEmpty())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("FILE NOT FOUND");
        box.exec();
        return;
    }
    viewFileCombo2->setCurrentText(fileName);
}

void page_image_tools::slot_imageSelectionChanged1()
{
    viewFileCombo1->select(viewFileCombo1->currentIndex());
}

void page_image_tools::slot_imageSelectionChanged2()
{
    viewFileCombo2->select(viewFileCombo2->currentIndex());
}

void page_image_tools::slot_createList()
{
    DlgWorklistCreate dlg(this);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted)
        return;

    QStringList list;
    QStringList wlist;
    if (dlg.chkLoadFilter->isChecked())
    {
        list = FileServices::getMosaicNames(SELECTED_MOSAICS);
    }
    else
    {
        list = FileServices::getMosaicNames(ALL_MOSAICS);
    }

    QString target;
    if (dlg.chkMotif)
    {
        target = dlg.motifNames->currentText();
    }
    else if (dlg.chkStyle)
    {
        target = dlg.styleNames->currentText();
    }

    if (target.isEmpty())
        return;

    for (auto & name : list)
    {
        QString designFile = FileServices::getMosaicXMLFile(name);

        QFile XMLFile(designFile);
        XMLFile.open(QIODevice::ReadOnly);
        QTextStream in (&XMLFile);
        const QString content = in.readAll();
        if (content.contains(target))
        {
            wlist << name;
        }
    }

    config->worklist.clear();
    config->worklist.set(wlist);
}

void page_image_tools::slot_genTypeChanged(int id)
{
    config->genCycle = static_cast<eCycleMode>(id) ;
    qDebug() << "load cycle mode" << sCycleMode[config->genCycle];
    loadFileFilterCombo();
}

void page_image_tools::slot_gen_selectionChanged()
{
    config->fileFilter = static_cast<eLoadType>(fileFilterCombo->currentData().toInt());
    qDebug() << "load file filter" << config->fileFilter;
    if (created)
    {
        loadVersionCombos();
    }
}

void page_image_tools::setImageDirectory()
{
    QDateTime d = QDateTime::currentDateTime();
    QString date = d.toString("yyyy-MM-dd");
    if (!panel->gitBranch.isEmpty())
        directory->setText(date + "-" + panel->gitBranch);
    else
        directory->setText(date);
}

void page_image_tools::loadVersionCombos()
{
    eLoadType loadType = (eLoadType)fileFilterCombo->currentData().toInt();

    mosaicA->blockSignals(true);
    mosaicB->blockSignals(true);

    mosaicA->clear();
    mosaicB->clear();

    auto mosaicNames = FileServices::getMosaicRootNames(loadType);

    mosaicA->addItems(mosaicNames);
    mosaicB->addItems(mosaicNames);

    mosaicB->blockSignals(false);
    mosaicA->blockSignals(false);

    int index = mosaicA->findText(config->lastCompareName);
    if (index < 0) index = 0;

    mosaicA->setCurrentIndex(index);
    mosaicB->setCurrentIndex(index);

    slot_mosaicAChanged();
    slot_mosaicBChanged();
}

void page_image_tools::slot_mosaicAChanged()
{
    versionsA->clear();

    auto name = mosaicA->currentText();

    config->lastCompareName = name;

    auto qsl = FileServices::getFileVersions(name,config->rootMosaicDir);
    if (qsl.isEmpty())
        return;

    versionsA->addItems(qsl);

    if (chkLock->isChecked())
    {
        mosaicB->blockSignals(true);
        mosaicB->setCurrentIndex(mosaicA->currentIndex());
        mosaicB->blockSignals(false);

        auto name = mosaicB->currentText();
        auto qsl = FileServices::getFileVersions(name,config->rootMosaicDir);

        versionsB->clear();
        versionsB->addItems(qsl);
    }
}

void page_image_tools::slot_mosaicBChanged()
{
    versionsB->clear();

    auto name = mosaicB->currentText();
    auto qsl = FileServices::getFileVersions(name,config->rootMosaicDir);
    if (qsl.isEmpty())
        return;

    versionsB->addItems(qsl);

    if (chkLock->isChecked())
    {
        mosaicA->blockSignals(true);
        mosaicA->setCurrentIndex(mosaicB->currentIndex());
        mosaicA->blockSignals(false);

        auto name = mosaicA->currentText();
        auto qsl = FileServices::getFileVersions(name,config->rootMosaicDir);

        versionsA->clear();
        versionsA->addItems(qsl);
    }
}

void page_image_tools::slot_viewImage3()
{
    qDebug() << "slot_viewImage3";
    QString name = versionsA->currentText();
    mosaicMaker->slot_loadMosaic(name,false);

    auto mosaic = mosaicMaker->getMosaic();
    mosaic->reportMotifs();
    mosaic->reportStyles();
}

void page_image_tools::slot_viewImage4()
{
    qDebug() << "slot_viewImag4";
    QString name = versionsB->currentText();
    mosaicMaker->slot_loadMosaic(name,false);

    auto mosaic = mosaicMaker->getMosaic();
    mosaic->reportMotifs();
    mosaic->reportStyles();
}

void  page_image_tools::slot_compareVersions()
{
    auto mosA = versionsA->currentText();
    auto mosB = versionsB->currentText();
    if (radXML->isChecked())
    {
        auto mosAA = FileServices::getMosaicXMLFile(mosA);
        auto mosBB = FileServices::getMosaicXMLFile(mosB);
        QStringList qsl;
        qsl << mosAA << mosBB;
        QProcess::startDetached(config->diffTool,qsl);
    }
    else
    {
        Q_ASSERT(radImg->isChecked());

        panel->selectViewer(VIEW_MOSAIC);

        mosaicMaker->slot_loadMosaic(mosA,false);
        auto pixA = view->grab();
        auto mosaicA = mosaicMaker->getMosaic();

        mosaicMaker->slot_loadMosaic(mosB,false);
        auto pixB = view->grab();
        auto mosaicB = mosaicMaker->getMosaic();

        mosaicA->reportMotifs();
        mosaicA->reportStyles();
        mosaicB->reportMotifs();
        mosaicB->reportStyles();

        imgA = pixA.toImage();
        imgB = pixB.toImage();
        emit theApp->sig_closeAllImageViewers();
        theApp->compareImages(imgA,imgB,mosA,mosB,false);
    }
}

void  page_image_tools::compareNextVersions()
{
    Q_ASSERT(comparingVersions);
    Q_ASSERT(radImg->isChecked());

    // this assumes imgB can bes used as the new img
    imgA = imgB;
    auto mosaicA = mosaicMaker->getMosaic();
    auto mosA    = versionsA->currentText();

    auto mosB = versionsB->currentText();
    mosaicMaker->slot_loadMosaic(mosB,false);
    auto pixB = view->grab();
    auto mosaicB = mosaicMaker->getMosaic();

    mosaicA->reportMotifs();
    mosaicA->reportStyles();
    mosaicB->reportMotifs();
    mosaicB->reportStyles();

    imgB = pixB.toImage();

    emit theApp->sig_closeAllImageViewers();
    theApp->compareImages(imgA,imgB,mosA,mosB,false);
}

void  page_image_tools::slot_cycleVersions()
{
    loadVersionCombos();

    panel->selectViewer(VIEW_MOSAIC);

    comparingVersions = true;

    // get root names
    eLoadType loadType = (eLoadType)fileFilterCombo->currentData().toInt();
    mosNames = FileServices::getMosaicRootNames(loadType);
    imgList_it = mosNames.begin();

    // get first name
    mosName = *imgList_it;
    int index = mosaicA->findText(mosName);
    mosaicA->setCurrentIndex(index);
    mosaicB->setCurrentIndex(index);

    // get versions
    versions = FileServices::getFileVersions(mosName,config->rootMosaicDir);
    imgListVerA_it = versions.begin();
    imgListVerB_it = imgListVerA_it;
    imgListVerB_it++;
    if (imgListVerB_it == versions.end())
    {
        slot_nextImage();
        return;
    }

    // compare first two versions
    auto vera = *imgListVerA_it;
    index = versionsA->findText(vera);
    versionsA->setCurrentIndex(index);
    auto verb = *imgListVerB_it;
    index = versionsB->findText(verb);
    versionsB->setCurrentIndex(index);

    slot_compareVersions();
}

void page_image_tools::slot_nextImage()
{
    if (!comparingVersions)
        return;

    if (++imgListVerB_it == versions.end())
    {
    next_mosaic:

        // go to next mosaic

        if (++imgList_it == mosNames.end())
        {
            // we are done
            comparingVersions = false;

            QMessageBox box(this);
            box.setText("Version comparison complete");
            box.setIcon(QMessageBox::Information);
            box.setStandardButtons(QMessageBox::Ok);
            box.exec();

            return;
        }

        mosName = *imgList_it;
        int index = mosaicA->findText(mosName);
        mosaicA->setCurrentIndex(index);
        mosaicB->setCurrentIndex(index);

        versions = FileServices::getFileVersions(mosName,config->rootMosaicDir);
        imgListVerA_it = versions.begin();
        imgListVerB_it = imgListVerA_it;
        imgListVerB_it++;
        if (imgListVerB_it == versions.end())
        {
            // there is no other version to compare to
            goto next_mosaic;
        }
        auto vera = *imgListVerA_it;
        index = versionsA->findText(vera);
        versionsA->setCurrentIndex(index);
        auto verb = *imgListVerB_it;
        index = versionsB->findText(verb);
        versionsB->setCurrentIndex(index);

        slot_compareVersions();
    }
    else
    {
        // go to next version
        imgListVerA_it++;

        auto vera = *imgListVerA_it;
        int index = versionsA->findText(vera);
        versionsA->setCurrentIndex(index);
        auto verb = *imgListVerB_it;
        index = versionsB->findText(verb);
        versionsB->setCurrentIndex(index);

        compareNextVersions();
    }
}

void page_image_tools::slot_quitImageCycle()
{
    if (!comparingVersions)
        return;
    comparingVersions = false;
}
