#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QRadioButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QtConcurrent>
#include <QFuture>

#include "engine/compare_bmp_engine.h"
#include "engine/image_engine.h"
#include "engine/mosaic_bmp_engine.h"
#include "engine/tiling_bmp_engine.h"
#include "engine/version_stepper.h"
#include "engine/tiling_stepper.h"
#include "engine/mosaic_stepper.h"
#include "engine/worklist_stepper.h"
#include "engine/png_stepper.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "misc/fileservices.h"
#include "misc/qtapplog.h"
#include "misc/sys.h"
#include "misc/timers.h"
#include "mosaic/mosaic.h"
#include "mosaic/mosaic_manager.h"
#include "panels/page_image_tools.h"
#include "panels/panel_misc.h"
#include "panels/controlpanel.h"
#include "settings/configuration.h"
#include "tiledpatternmaker.h"
#include "tiledpatternmaker.h"
#include "viewers/view_controller.h"
#include "widgets/dlg_wlist_create.h"
#include "widgets/layout_sliderset.h"
#include "widgets/memory_combo.h"
#include "widgets/versioned_list_widget.h"
#include "widgets/worklist_widget.h"
#include "tiledpatternmaker.h"
#include "engine/stepping_engine.h"

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
using Qt::endl;
#else
#define endl Qt::endl
#endif

QMutex      page_image_tools::comparisonMutex;
QStringList page_image_tools::comparisonWorklist;

page_image_tools:: page_image_tools(ControlPanel * cpanel)  : panel_page(cpanel,PAGE_IMAGE_TOOLS,"Image Tools")
{
    generatorType     = ACT_GEN_MOSAIC_BMP;     // default
    created           = false;
    etimer            = new AQElapsedTimer(false);
    Sys::imageEngine  = &engine;

    int row = 0;
    grid = new QGridLayout();
    row = createCycleGenBox(row);
    row = createWorklistBox(row);
    row = createCompareImagesBox(row);
    row = createCompareVersionsBox(row);
          createViewImageBox(row);

    vbox->addLayout(grid);
    vbox->addStretch();

    connect(&engine,    &ImageEngine::sig_compareResult,   this, &page_image_tools::slot_compareResult);
    connect(&engine,    &ImageEngine::cycle_sig_workList,  this, &page_image_tools::slot_firstDirChanged,  Qt::QueuedConnection);
    connect(&engine,    &ImageEngine::sig_image0,          this, &page_image_tools::slot_setImageLeftCombo);
    connect(&engine,    &ImageEngine::sig_image1,          this, &page_image_tools::slot_setImageRightCombo);

    connect(&watcher,   &QFutureWatcher<bool>::finished,             this, &page_image_tools::slot_engineComplete);
    connect(&watcher,   &QFutureWatcher<bool>::progressValueChanged, this, &page_image_tools::slot_engineProgress);

    engine.verStepper->connect(mediaA, mediaB, versionsA, versionsB);
    engine.verStepper->loadVersionCombos();

    created = true;
}

page_image_tools::~page_image_tools()
{
    watcher.waitForFinished();
}

int  page_image_tools::createCycleGenBox(int row)
{
    directory                  = new QLineEdit("");
    QPushButton  * saveDirBtn  = new QPushButton("Dir");
    QRadioButton * rSavMosaics = new QRadioButton("Mosaics");
    QRadioButton * rSavTiles   = new QRadioButton("Tilings");
    genFilterCombo             = new QComboBox();
    QCheckBox    * skip        = new QCheckBox("Skip Existing");
    generateBMPsBtn            = new QPushButton("Generate BMPs");

    genFilterCombo->setFixedWidth(121);
    skip->setFixedWidth(97);
    generateBMPsBtn->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    directory->setMinimumWidth(151);
    setImageDirectory();

    skip->setChecked(config->skipExisting);

    genBtnGroup = new QButtonGroup;
    genBtnGroup->addButton(rSavMosaics,ACT_GEN_MOSAIC_BMP);
    genBtnGroup->addButton(rSavTiles,ACT_GEN_TILING_BMP);
    if (config->genCycleMosaic)
        rSavMosaics->setChecked(true);
    else
        rSavTiles->setChecked(true);

    connect(generateBMPsBtn,    &QPushButton::clicked,     this,  &page_image_tools::slot_genBMPs);
    connect(saveDirBtn,         &QPushButton::clicked,     this,  &page_image_tools::slot_opendir);
    connect(skip,               &QCheckBox::clicked,       this,  &page_image_tools::slot_skipExisting);
    connect(genBtnGroup,        &QButtonGroup::idClicked,  this,  &page_image_tools::slot_genTypeChanged);
    connect(genFilterCombo,   QOverload<int>::of(&QComboBox::currentIndexChanged), this, &page_image_tools::slot_gen_selectionChanged);

    loadFileFilterCombo();

    SpinSet * spCycleInterval = new SpinSet("Cycle Interval",0,0,9);
    QRadioButton * rStyles    = new QRadioButton("Mosaics");
    QRadioButton * rTiles     = new QRadioButton("Tilings");
    viewFilterCombo           = new QComboBox();
    QRadioButton * rPngs      = new QRadioButton("Original PNGs");
    viewImgesBtn              = new QPushButton("View Images");

    viewImgesBtn->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    spCycleInterval->setValue(config->cycleInterval);
    viewFilterCombo->setFixedWidth(121);
    rPngs->setFixedWidth(97);

    viewBtnGroup = new QButtonGroup;
    viewBtnGroup->addButton(rStyles,CYCLE_VIEW_MOSAICS);
    viewBtnGroup->addButton(rTiles,CYCLE_VIEW_TILINGS);
    viewBtnGroup->addButton(rPngs,CYCLE_VIEW_ORIGINAL_PNGS);

    viewBtnGroup->button(config->viewCycle2)->setChecked(true);

    loadViewFilterCombo();

    // title
    QLabel * l1 = new  QLabel("Generate BMPs :");
    l1->setStyleSheet("font-weight : bold");
    QHBoxLayout * hb0 = new QHBoxLayout();
    hb0->addWidget(l1);
    hb0->addStretch();
    grid->addLayout(hb0,row,0,1,3);

    // generation
    QHBoxLayout * hb1 = new QHBoxLayout;
    hb1->addWidget(directory);
    hb1->addStretch();
    hb1->addWidget(rSavMosaics );
    hb1->addWidget(rSavTiles);
    hb1->addWidget(genFilterCombo);
    hb1->addWidget(skip);

    row++;
    grid->addWidget(saveDirBtn,row,0);
    grid->addLayout(hb1,row,1);
    grid->addWidget(generateBMPsBtn,row,2);

    // separator
    row++;
    addHSeparator(row);

    // title
    QLabel * l2 = new  QLabel("View Images :");
    l2->setStyleSheet("font-weight : bold");
    QHBoxLayout * hb5 = new QHBoxLayout();
    hb5->addWidget(l2);
    hb5->addStretch();
    row++;
    grid->addLayout(hb5,row,0,1,3);

    QHBoxLayout * hb3 = new QHBoxLayout;
    hb3->addLayout(spCycleInterval);
    hb3->addStretch();
    hb3->addWidget(rStyles);
    hb3->addWidget(rTiles);
    hb3->addWidget(viewFilterCombo);
    hb3->addWidget(rPngs);

    row++;
    grid->addLayout(hb3,row,0,1,2);
    grid->addWidget(viewImgesBtn,row,2);

    connect(spCycleInterval, &SpinSet::valueChanged,    this,  &page_image_tools::slot_cycleIntervalChanged);
    connect(viewImgesBtn,    &QPushButton::clicked,     this,  &page_image_tools::slot_startStepping);
    connect(viewBtnGroup,    &QButtonGroup::idClicked,  this,  &page_image_tools::slot_viewTypeChanged);
    connect(genFilterCombo,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, &page_image_tools::slot_view_selectionChanged);
    return ++row;
}

int page_image_tools::createWorklistBox(int row)
{
    QPushButton * loadListBtn   = new QPushButton("Load");
    QPushButton * saveListBtn   = new QPushButton("Save");
    QPushButton * createListBtn = new QPushButton("Create");
    QPushButton * editListBtn   = new QPushButton("Edit");
    QPushButton * delBtn        = new QPushButton("Delete");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(loadListBtn);
    hbox->addWidget(saveListBtn);
    hbox->addWidget(createListBtn);
    hbox->addWidget(editListBtn);
    hbox->addWidget(delBtn);

    wlistLabel = new QLabel("Work List operations");
    wlistLabel->setStyleSheet("font-weight : bold");

    QHBoxLayout * hbox0 = new QHBoxLayout;
    hbox0->addWidget(wlistLabel);
    hbox0->addStretch();

    addHSeparator(row);

    row++;
    grid->addLayout(hbox0,row,0,1,3);

    row++;
    grid->addLayout(hbox,row,1);

    connect(loadListBtn,  &QPushButton::clicked,     this,  &page_image_tools::slot_loadWorkListFromFile);
    connect(saveListBtn,  &QPushButton::clicked,     this,  &page_image_tools::slot_saveWorkListToFile);
    connect(editListBtn,  &QPushButton::clicked,     this,  &page_image_tools::slot_editWorkList);
    connect(delBtn,       &QPushButton::clicked,     this,  [this] () { slot_deleteCurrentWLEntry(true);} );
    connect(createListBtn,&QPushButton::clicked,     this,  &page_image_tools::slot_createList);
    connect(&engine,      &ImageEngine::sig_deleteCurrentInWorklist, this, &page_image_tools::slot_deleteCurrentWLEntry);

    return ++row;
}

int page_image_tools::createCompareImagesBox(int row)
{
    firstFileCombo              = new QComboBox();
    secondFileCombo             = new QComboBox();
    imageCompareResult          = new QLineEdit();
    firstDir                    = new MemoryCombo("leftDir");
    secondDir                   = new MemoryCombo("rightDir");

    QPushButton * firstDirBtn   = new QPushButton("First Dir");
    QPushButton * secondDirBtn  = new QPushButton("Second Dir");
    QPushButton * viewImage0    = new QPushButton("View First");
    QPushButton * viewImage1    = new QPushButton("View Second");
    QPushButton * previousBtn   = new QPushButton("Previous");
    QPushButton * nextBtn       = new QPushButton("Next");
    QPushButton * compareDirBMP = new QPushButton("Compare");
    QPushButton * genCompareBtn = new QPushButton("Generate WL");
    startBtn                    = new QPushButton("Start");
    QPushButton * swapBtn       = new QPushButton("Swap Dirs");

    firstFileCombo->setMinimumWidth(461);
    imageCompareResult->setReadOnly(true);

    genCompareBtn->setStyleSheet("QPushButton { background-color: yellow; color: red;}");
    startBtn->setStyleSheet(     "QPushButton { background-color: yellow; color: red;}");
    compareDirBMP->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    QCheckBox   * cbStopIfDiff = new QCheckBox("Stop if Diff");
    QCheckBox   * differences  = new QCheckBox("Show Diffs");
    QCheckBox   * chkPopup     = new QCheckBox("Pop-up");
    QCheckBox   * transparent  = new QCheckBox("Transparent");
    use_wlistForCompareChk     = new QCheckBox("Use WorkList");

    compareView                = new QCheckBox("Use loaded");

    QPushButton * replBtn      = new QPushButton("Replace Second BMP");
                  loadFirstBtn = new QPushButton("Load First");
                 loadSecondBtn = new QPushButton("Load Second");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(cbStopIfDiff);
    hbox->addWidget(differences);
    hbox->addWidget(chkPopup);
    hbox->addWidget(transparent);

    QHBoxLayout * hbox2 = new QHBoxLayout;
    hbox2->addWidget(imageCompareResult);
    hbox2->addWidget(compareView);

    QLabel * label = new QLabel("Comparison Options :");
    label->setStyleSheet("font-weight : bold");

    QHBoxLayout * hbox3 = new QHBoxLayout;
    hbox3->addWidget(label);
    hbox3->addStretch();

    addHSeparator(row);

    row++;
    grid->addLayout(hbox3,row,0,1,3);

    QPushButton * colorEdit      = new QPushButton("Set Color");
    colorLabel                   = new QLabel();
    colorLabel->setFixedWidth(51);
    QCheckBox  * chkUseFilter    = new QCheckBox("Filter color");

    row++;
    QHBoxLayout * hbox4 = new QHBoxLayout();
    hbox4->addWidget(chkUseFilter);
    hbox4->addWidget(colorLabel);
    hbox4->addWidget(colorEdit);
    hbox4->addLayout(hbox);
    hbox4->addStretch();
    grid->addLayout(hbox4,row,0,1,2);
    grid->addWidget(use_wlistForCompareChk,row,2);

    row++;
    addHSeparator(row);

    row++;
    label = new QLabel("Directory BMP comparisons :");
    label->setStyleSheet("font-weight : bold");
    hbox3 = new QHBoxLayout;
    hbox3->addWidget(label);
    hbox3->addStretch();
    hbox3->addWidget(loadFirstBtn);
    hbox3->addWidget(loadSecondBtn);
    hbox3->addWidget(replBtn);
    grid->addLayout(hbox3,row,0,1,2);
    grid->addWidget(genCompareBtn,row,2);

    row++;
    grid->addWidget(firstDirBtn,row,0);
    grid->addWidget(firstDir,row,1);
    grid->addWidget(startBtn,row,2);

    row++;
    grid->addWidget(secondDirBtn,row,0);
    grid->addWidget(secondDir,row,1);
    grid->addWidget(previousBtn,row,2);

    row++;
    grid->addWidget(viewImage0,row,0);
    grid->addWidget(firstFileCombo,row,1);
    grid->addWidget(nextBtn,row,2);

    row++;
    grid->addWidget(viewImage1,row,0);
    grid->addWidget(secondFileCombo,row,1);

    row++;
    grid->addWidget(swapBtn,row,0);
    grid->addLayout(hbox2,row,1);
    grid->addWidget(compareDirBMP,row,2);

    firstDir->initialise();
    loadCombo(firstFileCombo,firstDir->getCurrentText());
    secondDir->initialise();
    loadCombo(secondFileCombo,secondDir->getCurrentText());

    cbStopIfDiff->setChecked(config->stopIfDiff);
    transparent->setChecked(config->compare_transparent);
    chkPopup->setChecked(config->compare_popup);
    differences->setChecked(config->display_differences);
    use_wlistForCompareChk->setChecked(config->use_workListForCompare);
    chkUseFilter->setChecked(config->filterColor);

    connect(colorEdit,              &QPushButton::clicked,  this,   &page_image_tools::slot_colorEdit);
    connect(chkUseFilter,           &QCheckBox::clicked,    this,   &page_image_tools::slot_useFilter);

    connect(firstDirBtn,            &QPushButton::clicked,  this,   &page_image_tools::slot_selectDir0);
    connect(secondDirBtn,           &QPushButton::clicked,  this,   &page_image_tools::slot_selectDir1);
    connect(swapBtn,                &QPushButton::clicked,  this,   &page_image_tools::slor_swapDirs);

    connect(genCompareBtn,          &QPushButton::clicked,  this,   &page_image_tools::slot_compareGen);
    connect(startBtn,               &QPushButton::clicked,  this,   &page_image_tools::slot_startCompare);
    connect(nextBtn,                &QPushButton::clicked,  this,   &page_image_tools::slot_next);
    connect(previousBtn,            &QPushButton::clicked,  this,   &page_image_tools::slot_previous);
    connect(compareDirBMP,          &QPushButton::clicked,  this,   &page_image_tools::slot_compareDiffDirBMPs);

    connect(viewImage0,             &QPushButton::clicked,  this,   &page_image_tools::slot_viewImageLeft);
    connect(viewImage1,             &QPushButton::clicked,  this,   &page_image_tools::slot_viewImageRight);

    connect(transparent,            &QCheckBox::clicked,    this,   &page_image_tools::slot_transparentClicked);
    connect(chkPopup,               &QCheckBox::clicked,    this,   &page_image_tools::slot_popupClicked);
    connect(cbStopIfDiff,           &QCheckBox::clicked,    this,   &page_image_tools::slot_stopIfDiffClicked);
    connect(differences,            &QCheckBox::clicked,    this,   &page_image_tools::slot_differencesClicked);
    connect(use_wlistForCompareChk, &QCheckBox::clicked,    this,   &page_image_tools::slot_use_worklist_compare);
    connect(compareView,            &QCheckBox::clicked,    this,   &page_image_tools::slot_compareView);

    connect(replBtn,                &QPushButton::clicked,  this,   &page_image_tools::slot_replaceBMP);
    connect(loadFirstBtn,           &QPushButton::clicked,  this,   &page_image_tools::slot_loadFirst);
    connect(loadSecondBtn,          &QPushButton::clicked,  this,   &page_image_tools::slot_loadSecond);

    connect(firstDir,  &MemoryCombo::editTextChanged,       this,   &page_image_tools::slot_firstDirChanged);
    connect(secondDir, &MemoryCombo::editTextChanged,       this,   &page_image_tools::slot_secondDirChanged);

    connect(firstDir,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, &page_image_tools::slot_firstDirChanged);
    connect(secondDir, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &page_image_tools::slot_secondDirChanged);

    connect(firstFileCombo,  SIGNAL(currentIndexChanged(int)), this,   SLOT(slot_ibox0_changed(int)));
    connect(secondFileCombo, SIGNAL(currentIndexChanged(int)), this,   SLOT(slot_ibox1_changed(int)));

    setCombo(firstFileCombo,config->image0);
    setCombo(secondFileCombo,config->image1);

    return ++row;
}

int page_image_tools::createCompareVersionsBox(int row)
{
    versionFilterCombo = new QComboBox();

    radMosaic  = new QRadioButton("Mosaics");
    radTile    = new QRadioButton("Tiles");
    mediaGroup = new QButtonGroup();
    mediaGroup->addButton(radMosaic);
    mediaGroup->addButton(radTile);

    radImg    = new QRadioButton("Images");
    radXML    = new QRadioButton("XML");
    typeGroup = new QButtonGroup();
    typeGroup->addButton(radImg);
    typeGroup->addButton(radXML);

    chkLock = new QCheckBox("Lock");

    QPushButton  * compareVerBMP = new QPushButton("Compare");
                  versionStepBtn = new QPushButton("Cycle");
    compareVerBMP->setStyleSheet(" QPushButton { background-color: yellow; color: red;}");
    versionStepBtn->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    mediaA    = new QComboBox();
    versionsA = new QComboBox();
    mediaB    = new QComboBox();
    versionsB = new QComboBox();

    QPushButton * viewImageBtn3  = new QPushButton("View");
    QPushButton * viewImageBtn4  = new QPushButton("View");
    QLabel      * dummyLabel     = new QLabel(" ");
    dummyLabel->setMinimumWidth(79);

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addStretch();
    hbox->addWidget(radMosaic);
    hbox->addWidget(radTile);
    hbox->addSpacing(9);
    hbox->addWidget(versionFilterCombo);
    hbox->addSpacing(9);
    hbox->addWidget(radImg);
    hbox->addWidget(radXML);
    hbox->addSpacing(9);
    hbox->addWidget(chkLock);

    QHBoxLayout * hbox2 = new QHBoxLayout();
    hbox2->addWidget(mediaA);
    hbox2->addWidget(versionsA);

    QLabel * label = new QLabel("Load Version comparisons :");
    label->setStyleSheet("font-weight : bold");

    QHBoxLayout * hbox3 = new QHBoxLayout;
    hbox3->addWidget(label);
    hbox3->addStretch();

    addHSeparator(row);

    row++;
    grid->addLayout(hbox3,row,0,1,3);

    row++;
    grid->addWidget(viewImageBtn3,row,0);
    grid->addLayout(hbox2,row,1);
    grid->addWidget(versionStepBtn,row,2);

    row++;
    QHBoxLayout * hbox4 = new QHBoxLayout();
    hbox4->addWidget(mediaB);
    hbox4->addWidget(versionsB);
    grid->addWidget(viewImageBtn4,row,0);
    grid->addLayout(hbox4,row,1);

    row++;
    grid->addWidget(dummyLabel,row,0);
    grid->addLayout(hbox,row,1);
    grid->addWidget(compareVerBMP,row,2);

    chkLock->setChecked(config->vCompLock);
    if (config->vCompXML)
        radXML->setChecked(true);
    else
        radImg->setChecked(true);
    if (config->vCompTile)
        radTile->setChecked(true);
    else
        radMosaic->setChecked(true);

    loadVersionFilterCombo();

    connect(versionFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &page_image_tools::slot_ver_selectionChanged);
    connect(mediaA,             QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { engine.verStepper->mediaAChanged(); });
    connect(mediaB,             QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { engine.verStepper->mediaBChanged(); });

    connect(compareVerBMP,  &QPushButton::clicked,   this,  &page_image_tools::slot_compareDiffVerBMPs);
    connect(versionStepBtn, &QPushButton::clicked,   this,  &page_image_tools::slot_cycleVersions);
    connect(chkLock,        &QCheckBox::clicked,     this,  [this](bool checked) { config->vCompLock = checked; });
    connect(radXML,         &QRadioButton::clicked,  this,  [this](bool checked) { config->vCompXML  = checked; });
    connect(radImg,         &QRadioButton::clicked,  this,  [this](bool checked) { config->vCompXML  = !checked; });
    connect(radMosaic,      &QRadioButton::clicked,  this,  [this](bool checked) { config->vCompTile = !checked;
                                                                                   loadVersionFilterCombo();
                                                                                   engine.verStepper->loadVersionCombos(); });
    connect(radTile,        &QRadioButton::clicked,  this,  [this](bool checked) { config->vCompTile = checked;
                                                                                   loadVersionFilterCombo();
                                                                                   engine.verStepper->loadVersionCombos(); });

    connect(&engine,        &ImageEngine::sig_ready, this,  &page_image_tools::slot_nextImage, Qt::QueuedConnection);

    connect(viewImageBtn3,  &QPushButton::clicked,   this,  &page_image_tools::slot_viewImage3);
    connect(viewImageBtn4,  &QPushButton::clicked,   this,  &page_image_tools::slot_viewImage4);

    return  ++row;
}

int page_image_tools::createViewImageBox(int row)
{
    QPushButton * selectImgBtn1  = new QPushButton("Select");
    QPushButton * selectImgBtn2  = new QPushButton("Select");
    QPushButton * viewImageBtn1  = new QPushButton("View");
    QPushButton * viewImageBtn2  = new QPushButton("View");
    QPushButton * compareFileBMP = new QPushButton("Compare");
    compareFileBMP->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    viewFileCombo1               = new MemoryCombo("viewFileCombo1");
    viewFileCombo2               = new MemoryCombo("viewFileCombo2");
    viewFileCombo1->setMinimumWidth(540);
    viewFileCombo2->setMinimumWidth(540);

    QLabel * label = new QLabel("Image File comparisons :");
    label->setStyleSheet("font-weight : bold");
    QHBoxLayout * hbox3 = new QHBoxLayout;
    hbox3->addWidget(label);
    hbox3->addStretch();

    addHSeparator(row);

    row++;
    grid->addLayout(hbox3,row,0,1,2);
    grid->addWidget(compareFileBMP,row,2);

    row++;
    grid->addWidget(selectImgBtn1,row,0);
    grid->addWidget(viewFileCombo1,row,1);
    grid->addWidget(viewImageBtn1,row,2);

    row++;
    grid->addWidget(selectImgBtn2,row,0);
    grid->addWidget(viewFileCombo2,row,1);
    grid->addWidget(viewImageBtn2,row,2);

    viewFileCombo1->initialise();
    viewFileCombo2->initialise();

    QVariant variant = config->transparentColor;
    QString colcode  = variant.toString();
    colorLabel->setStyleSheet("QLabel { background-color : "+colcode+" ; border : 1px solid black; }");

    connect(selectImgBtn1,  &QPushButton::clicked,   this,   &page_image_tools::slot_selectImage1);
    connect(selectImgBtn2,  &QPushButton::clicked,   this,   &page_image_tools::slot_selectImage2);
    connect(viewImageBtn1,  &QPushButton::clicked,   this,   &page_image_tools::slot_viewImage1);
    connect(viewImageBtn2,  &QPushButton::clicked,   this,   &page_image_tools::slot_viewImage2);
    connect(compareFileBMP, &QPushButton::clicked,   this,   &page_image_tools::slot_compareFileBMPs);

    connect(viewFileCombo1, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &page_image_tools::slot_imageSelectionChanged1);
    connect(viewFileCombo2, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &page_image_tools::slot_imageSelectionChanged2);

    return ++row;
}

void page_image_tools::bump(int row, int stretch)
{
    row++;
    grid->setRowStretch(row,stretch);
}

void page_image_tools::addHSeparator(int row)
{
    QFrame * line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    if (!Sys::isDarkTheme) line->setStyleSheet("QFrame {background-color: LightGray;}");

    grid->addWidget(line,row,0,1,3);
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
    engine.verStepper->loadVersionCombos();
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
    QString str = QString("Work List Operations : <%1> contains %2 entries").arg(wlist.getName()).arg(wlist.get().size());
    wlistLabel->setText(str);

    if (engine.mosaicStepper->isStarted() || engine.tilingStepper->isStarted())
        viewImgesBtn->setStyleSheet("QPushButton { background-color: red; color: yellow;}");
    else
        viewImgesBtn->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    if (engine.wlStepper->isStarted())
        startBtn->setStyleSheet("QPushButton { background-color: red; color: yellow;}");
    else
        startBtn->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    if (engine.verStepper->isStarted())
        versionStepBtn->setStyleSheet("QPushButton { background-color: red; color: yellow;}");
    else
        versionStepBtn->setStyleSheet("QPushButton { background-color: yellow; color: red;}");
}


///////////////////////////////////////////////////////////////////////////
///
///  Slots
///
///////////////////////////////////////////////////////////////////////////

void page_image_tools::slot_genTypeChanged(int id)
{
    qDebug() << "page_image_tools::slot_genTypeChanged" << id;
    config->genCycleMosaic = (id == ACT_GEN_MOSAIC_BMP);
    loadFileFilterCombo();
}

void page_image_tools::slot_gen_selectionChanged()
{
    config->genFileFilter = static_cast<eLoadType>(genFilterCombo->currentData().toInt());
    qDebug() << "generate BMP file filter" << config->genFileFilter;
}

void page_image_tools::slot_viewTypeChanged(int id)
{
    config->viewCycle2 = static_cast<eCycleMode>(id);
    loadViewFilterCombo();
}

void page_image_tools::slot_view_selectionChanged()
{
    config->viewFileFilter = static_cast<eLoadType>(viewFilterCombo->currentData().toInt());
    qDebug() << "view BMP file filter" << config->viewFileFilter;
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

void page_image_tools::slot_selectDir0()
{
    QString  dir = firstDir->getCurrentText();
    QString fdir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), dir,
                                                 QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (fdir.isEmpty())
            return;

    firstDir->setCurrentText(fdir);
    slot_firstDirChanged();
}

void page_image_tools::slot_selectDir1()
{
    QString  dir = secondDir->getCurrentText();
    QString fdir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), dir,
                                                 QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (fdir.isEmpty())
            return;

    secondDir->setCurrentText(fdir);
    slot_secondDirChanged();
}

void page_image_tools::slor_swapDirs()
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
    if (file.isEmpty())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("No filename");
        box.exec();
        return;
    }
    config->image0 = file;
    QString path = firstDir->getCurrentText() + "/" + file + ".bmp";
    viewImage(path,false,config->compare_popup);
}

void page_image_tools::slot_viewImageRight()
{
    qDebug() << "slot_viewImageRight";
    QString file = secondFileCombo->currentText();
    if (file.isEmpty())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("No filename");
        box.exec();
        return;
    }
    config->image1 = file;
    QString path = secondDir->getCurrentText() + "/" + file + ".bmp";
    viewImage(path,false,config->compare_popup);
}

void page_image_tools::slot_transparentClicked(bool checked)
{
    config->compare_transparent = checked;
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
    use_wlistForCompareChk->blockSignals(true);
    use_wlistForCompareChk->setChecked(checked);
    use_wlistForCompareChk->blockSignals(false);

    config->use_workListForCompare = checked;

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
    box->addItems(names2);
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

    engine.wlStepper->resync(config->image0);
}

void page_image_tools::slot_ibox1_changed(int index)
{
    Q_UNUSED(index);
    config->image1 = secondFileCombo->currentText();
}

void page_image_tools::slot_loadFirst()
{
    QString mos = firstFileCombo->currentText();
    mosaicMaker->loadMosaic(mos);
}

void page_image_tools::slot_loadSecond()
{
    QString mos = secondFileCombo->currentText();
    mosaicMaker->loadMosaic(mos);
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

void page_image_tools::slot_loadWorkListFromFile()
{
    QString dir = Sys::worklistsDir;
    QString str = config->worklist.getName();
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

    config->worklist.set(fname,stringList);

    int sz = stringList.size();

    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    box.setText(QString("%1 filenames loaded into Work List - OK").arg(sz));
    box.exec();

    use_wlistForCompareChk->setChecked(true);
    slot_use_worklist_compare(true);    // do both
}

void page_image_tools::slot_saveWorkListToFile()
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

    QString dir = Sys::worklistsDir + wlist.getName() + ".txt";
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

void page_image_tools::slot_editWorkList()
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

void page_image_tools::slot_replaceBMP()
{
    // replace bitmp in second dir

    QString name = secondFileCombo->currentText();
    bool rv = false;
    MosaicPtr mosaic = mosaicMaker->loadMosaic(name);
    if (mosaic)
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
    if (rv)
    {
        emit engine.sig_closeAllImageViewers();
        engine.compareBMPsByName(firstFileCombo->currentText(),secondFileCombo->currentText(),false);
    }
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

    auto existingName = config->worklist.getName();
    config->worklist.set(existingName,newList);
    emit engine.sig_next(); // immediate
    VersionList vlist;
    vlist.create(newList);
    engine.wlStepper->setWorklist(vlist);
}

void page_image_tools::slot_useFilter(bool checked)
{
    config->filterColor = checked;
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
    bool rv = viewImage(file,config->compare_transparent,config->compare_popup);
    if (!rv)
    {
        viewFileCombo1->eraseCurrent();
    }
}

void page_image_tools::slot_viewImage2()
{
    qDebug() << "slot_viewImage2";
    QString file = viewFileCombo2->currentText();
    bool rv = viewImage(file,config->compare_transparent,config->compare_popup);
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

    config->worklist.clear();

    QStringList list;
    QStringList wlist;
    QString     listName;

    if (dlg.selMosaic->isChecked())
    {
        listName = "mosaic_list";
        QStringList targetList;
        if (dlg.chkLoadFilter->isChecked())
        {
            list = FileServices::getMosaicNames(SELECTED_MOSAICS);
        }
        else
        {
            list = FileServices::getMosaicNames(ALL_MOSAICS);
        }

        if (dlg.radText->isChecked())
        {
            targetList << dlg.text->text();
        }
        else if (dlg.radMotif->isChecked())
        {
            targetList << dlg.selectedMotifNames();
        }
        else if (dlg.radStyle->isChecked())
        {
            targetList << dlg.styleNames->currentText();
        }

        if (targetList.isEmpty())
            return;

        for (auto & name : std::as_const(list))
        {
            QString designFile = FileServices::getMosaicXMLFile(name);

            QFile XMLFile(designFile);
            XMLFile.open(QIODevice::ReadOnly);
            QTextStream in (&XMLFile);
            const QString content = in.readAll();
            bool found = false;
            for (auto & target : std::as_const(targetList))
            {
                if (content.contains(target))
                {
                   found = true;
                }
            }
            if (found)
            {
                wlist << name;
            }
        }
    }
    else
    {
        listName = "tiling_list";
        QString target;
        if (dlg.chkLoadFilter->isChecked())
        {
            list = FileServices::getTilingNames(SELECTED_TILINGS);
        }
        else
        {
            list = FileServices::getTilingNames(ALL_TILINGS);
        }

        if (dlg.radText->isChecked())
        {
            target = dlg.text->text();
        }

        if (target.isEmpty())
            return;

        for (auto & name : std::as_const(list))
        {
            QString designFile = FileServices::getTilingXMLFile(name);

            QFile XMLFile(designFile);
            XMLFile.open(QIODevice::ReadOnly);
            QTextStream in (&XMLFile);
            const QString content = in.readAll();
            if (content.contains(target))
            {
               wlist << name;
            }
        }
    }

    config->worklist.set(listName,wlist);
}

void page_image_tools::slot_ver_selectionChanged()
{
    config->versionFilter = static_cast<eLoadType>(versionFilterCombo->currentData().toInt());
    qDebug() << "load version filter" << config->genFileFilter;
    if (created)
    {
        engine.verStepper->loadVersionCombos();
    }
}

void page_image_tools::slot_viewImage3()
{
    qDebug() << "slot_viewImage3";
    QString name = versionsA->currentText();
    auto mosaic = mosaicMaker->loadMosaic(name);

    if (mosaic)
    {
        mosaic->reportMotifs();
        mosaic->reportStyles();
    }
}

void page_image_tools::slot_viewImage4()
{
    qDebug() << "slot_viewImag4";
    QString name = versionsB->currentText();
    auto mosaic = mosaicMaker->loadMosaic(name);

    if (mosaic)
    {
        mosaic->reportMotifs();
        mosaic->reportStyles();
    }
}

//////////////////////////////////////////////////////////////////////////////////////
//
//  Comparisons
//
//////////////////////////////////////////////////////////////////////////////////////

void page_image_tools::slot_compareDiffDirBMPs()
{
    imageCompareResult->setText("");
    panel->pushPanelStatus("Spacebar=next C=compare P=ping-pong S=side-by-side L=log Q=quit D=delete-from-worklist");
    if (!compareView->isChecked())
    {
        engine.compareBMPsByName(firstFileCombo->currentText(),secondFileCombo->currentText(),false);
    }
    else
    {
        engine.compareBMPwithLoaded(firstFileCombo->currentText(),false);
    }
}

void  page_image_tools::slot_compareDiffVerBMPs()
{
    if (config->vCompTile)
        panel->selectViewer(VIEW_TILING);
    else
        panel->selectViewer(VIEW_MOSAIC);

    engine.verStepper->compareVersions();
}

void page_image_tools::slot_compareFileBMPs()
{
    panel->pushPanelStatus("Spacebar=next C=compare P=ping-pong S=side-by-side L=log Q=quit D=delete-from-worklist");

    engine.compareBMPsByFilename(viewFileCombo1->currentText(),viewFileCombo2->currentText());
}


//////////////////////////////////////////////////////////////////////////////////////
//
//  Stepping
//
//////////////////////////////////////////////////////////////////////////////////////

void  page_image_tools::slot_cycleVersions()
{
    // if cycling versios musts force a lock
    chkLock->setChecked(true);

    if (config->vCompTile)
        panel->selectViewer(VIEW_TILING);
    else
        panel->selectViewer(VIEW_MOSAIC);

    engine.verStepper->loadVersionCombos();
    engine.verStepper->begin();
}

void page_image_tools::slot_nextImage()
{
    // called by sig_ready()
    engine.verStepper->next();
}

void page_image_tools::slot_startStepping()
{
    switch (config->viewCycle2)
    {
    case CYCLE_VIEW_MOSAICS:
        panel->pushPanelStatus("Press P to Pause/unPause - press Q to Quit" );
        panel->selectViewer(VIEW_MOSAIC);
        engine.mosaicStepper->begin();
        break;

    case CYCLE_VIEW_TILINGS:
        panel->pushPanelStatus("Press P to Pause/unPause - press Q to Quit" );
        panel->selectViewer(VIEW_TILING);
        engine.tilingStepper->begin();
        break;

    case CYCLE_VIEW_ORIGINAL_PNGS:
        engine.pngStepper->begin();
        break;

    default:
        break;
    }
}

void page_image_tools::slot_startCompare()
{
    panel->pushPanelStatus("Spacebar=next C=compare P=ping-pong S=side-by-side L=log Q=quit D=delete-from-worklist");

    emit engine.sig_closeAllImageViewers();

    VersionList vlist;
    if (config->use_workListForCompare)
    {
        vlist.create(config->worklist.get());
    }
    else
    {
        QStringList names;
        for(int i = 0; i < firstFileCombo->count(); i++)
        {
            names << firstFileCombo->itemText(i);
        }
        vlist.create(names);
    }
    engine.wlStepper->setWorklist(vlist);
    engine.wlStepper->begin();
}

void page_image_tools::slot_previous()
{
    emit engine.sig_closeAllImageViewers();
    engine.wlStepper->prev();
}

void page_image_tools::slot_next()
{
    emit engine.sig_closeAllImageViewers();
    engine.wlStepper->next();
}


//////////////////////////////////////////////////////////////////////////////////////
//
//  Multi-threading
//
//////////////////////////////////////////////////////////////////////////////////////

bool takeAction(sAction action)
{
    if (Sys::usingImgGenerator)
    {
        if (action.type ==  ACT_GEN_MOSAIC_BMP)
        {
            MosaicBMPEngine engine;
            bool rv = engine.saveBitmap(action.name,action.path);
            return rv;
        }
        else if (action.type == ACT_GEN_TILING_BMP)
        {
            TilingBMPEngine engine;
            bool rv = engine.saveBitmap(action.name,action.path);
            return rv;
        }
        else
        {
            Q_ASSERT(action.type == ACT_GEN_COMPARE_WLIST);
            CompareBMPEngine engine;
            bool rv = engine.compareBMPs(action.name,action.path,action.path2);
            return rv;
        }
    }
    else
    {
        qInfo() << "Cancelled" << action.name;
        return false;
    }
}

void page_image_tools::processActionList(QList<sAction> &actions)
{
    totalEngineImages = actions.size();

    if (config->multithreadedGeneration)
    {
        qInfo() << "Concurrent processes - starting";
        qtAppLog::getInstance()->logToPanel(false);     // thread safety: dont write to gui

        watcher.setFuture(QtConcurrent::mapped(actions,takeAction));
    }
    else
    {
        Q_ASSERT(Sys::usingImgGenerator);
        for (const auto & action : std::as_const(actions))
        {
            QString astring = "Processing : " + action.name;
            theApp->splash(astring);
            takeAction(action);
            theApp->removeSplash();
        }

        Sys::usingImgGenerator = false;

        QString str = QString("Pocesses took %1 seconds").arg(etimer->getElapsed().trimmed());
        QString str2 =        "Cycle complete";
        qInfo().noquote() << str;
        QMessageBox box(this);
        box.setIcon(QMessageBox::Information);
        box.setText(str2);
        box.setInformativeText(str);
        box.exec();
    }
}

void page_image_tools::slot_engineComplete()
{
    qtAppLog::getInstance()->logToPanel(config->logToPanel);

    qInfo() << "Image Engine completed";

    Sys::usingImgGenerator = false;
    Sys::localCycle        = false;

    QString str = QString("Concurrent processes took %1 seconds").arg(etimer->getElapsed().trimmed());
    QString str2 =        "Cycle complete";

    theApp->removeSplash();

    switch(generatorType)
    {
    case ACT_GEN_MOSAIC_BMP:
        panel->selectViewer(VIEW_MOSAIC);
        break;

    case ACT_GEN_TILING_BMP:
        panel->selectViewer(VIEW_TILING);
        break;

    case ACT_GEN_COMPARE_WLIST:
        Sys::appDontPaint(false);
        auto size = getComparisonWorklist().size();
        if (size)
        {
            config->worklist.set("generated",getComparisonWorklist());
            slot_use_worklist_compare(true);
        }
        str2 += QString(" - %1 differences put into worklist").arg(size);
        break;
    }

    qInfo().noquote() << str;
    qInfo().noquote() << str2;

    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    box.setText(str2);
    box.setInformativeText(str);
    box.exec();
}

void page_image_tools::slot_engineProgress(int val)
{
    float perc = (qreal(val) / qreal(totalEngineImages)) * 100.0;
    QString txt = QString("%1% files processed (%2 out of %3)").arg(floor(perc)).arg(val).arg(totalEngineImages);
    theApp->removeSplash();
    theApp->splash(txt);
}


void page_image_tools::saveMosaicBitmaps()
{
    qInfo() << "saveMosaicBitmaps";

    etimer->start();

    auto id             = config->genFileFilter;
    auto files          = FileServices::getMosaicNames(id);
    QString pixmapPath  = getPixmapPath();

    QList<sAction> actions;
    for (const QString & name : std::as_const(files))
    {
        if (config->skipExisting)
        {
            Q_ASSERT(!name.contains(".xml"));
            QString file = pixmapPath + "/" + name + ".bmp";
            QFile afile(file);
            if (afile.exists())
            {
                continue;
            }
        }

        sAction action;
        action.type = ACT_GEN_MOSAIC_BMP;
        action.name = name;
        action.path = pixmapPath;
        actions.push_back(action);
    }

    processActionList(actions);
}

void page_image_tools::saveTilingBitmaps()
{
    qInfo() << "saveTilingBitmaps";

    etimer->start();

    auto id            = config->genFileFilter;
    auto files         = FileServices::getTilingNames(id);
    QString pixmapPath = getPixmapPath();

    QList<sAction> actions;
    for (const auto & name : std::as_const(files))
    {
        if (config->skipExisting)
        {
            Q_ASSERT(!name.contains(".xml"));
            QString file = pixmapPath + "/" + name + ".bmp";
            QFile afile(file);
            if (afile.exists())
            {
                continue;
            }
        }

        sAction action;
        action.type = ACT_GEN_TILING_BMP;
        action.name = name;
        action.path = pixmapPath;
        actions.push_back(action);
    }

    processActionList(actions);
}

void page_image_tools::createComparedWorklist()
{
    qInfo() << "createComparedWorklist";

    etimer->start();

    QStringList imgList;
    if (config->use_workListForCompare)
    {
        VersionList vlist;
        vlist.create(config->worklist.get());
        imgList    = vlist.recompose();

        if (imgList.isEmpty())
        {
            QMessageBox box(this);
            box.setIcon(QMessageBox::Information);
            box.setText("Worklist is EMPTY, Nothing to compare");
            box.exec();
            return;
        }
    }
    else
    {
        QMap<QString,QString>  mapa = FileServices::getDirBMPFiles(MemoryCombo::getTextFor("leftDir"));
        QStringList names = mapa.keys();

        VersionList vlist;
        vlist.create(names);
        imgList    = vlist.recompose();
    }

    clearComparisonWorklist();

    QList<sAction> actions;
    for (const auto & name : std::as_const(imgList))
    {
        QString pathLeft  = MemoryCombo::getTextFor("leftDir")  + "/" + name  + ".bmp";
        QString pathRight = MemoryCombo::getTextFor("rightDir") + "/" + name + ".bmp";

        sAction action;
        action.type  = ACT_GEN_COMPARE_WLIST;
        action.name  = name;
        action.path  = pathLeft;
        action.path2 = pathRight;
        actions.push_back(action);
    }

    processActionList(actions);

    if (!config->multithreadedGeneration && getComparisonWorklist().size())
    {
        config->worklist.set("generated",getComparisonWorklist());
        slot_use_worklist_compare(true);
    }
}

void page_image_tools::slot_genBMPs()
{
    if (config->genCycleMosaic)
        generatorType = ACT_GEN_MOSAIC_BMP;
    else
        generatorType = ACT_GEN_TILING_BMP;

    setupActions();
}

void page_image_tools::slot_compareGen()
{
    generatorType = ACT_GEN_COMPARE_WLIST;

    setupActions();
}

void page_image_tools::setupActions()
{
    if (Sys::usingImgGenerator)
    {
        // this is a cancel
        watcher.cancel();
        qInfo() << "Generate Mosaic bitmaps  Cancelled: waiting to finish";
        theApp->removeSplash();
        theApp->splash("CANCELLED - waiting to finish");

        watcher.waitForFinished();

        Sys::localCycle        = false;
        Sys::usingImgGenerator = false;
        qInfo() << "Generate Mosaic bitmaps : Finished";
        theApp->removeSplash();
        theApp->splash("CANCELLED - Finished");

        return;
    }

    if (generatorType == ACT_GEN_MOSAIC_BMP)
    {
        Q_ASSERT(!Sys::usingImgGenerator);
        Sys::usingImgGenerator = true;
        Sys::localCycle        = true;
        Sys::viewController->disableAllViews();
        saveMosaicBitmaps();
    }
    else if (generatorType == ACT_GEN_TILING_BMP)
    {
        Q_ASSERT(!Sys::usingImgGenerator);
        Sys::usingImgGenerator = true;
        Sys::localCycle        = true;
        Sys::viewController->disableAllViews();
        saveTilingBitmaps();
    }
    else if (generatorType == ACT_GEN_COMPARE_WLIST)
    {
        Q_ASSERT(!Sys::usingImgGenerator);
        Sys::usingImgGenerator = true;
        Sys::localCycle        = true;
        Sys::appDontPaint(true);
        createComparedWorklist();
    }
}


//////////////////////////////////////////////////////////////////////////
///
///  Utils
///
//////////////////////////////////////////////////////////////////////////

void page_image_tools::loadFileFilterCombo()
{
    genFilterCombo->blockSignals(true);
    genFilterCombo->clear();
    eLoadType defaultType;
    if (config->genCycleMosaic)
    {
        genFilterCombo->addItem("All Mosaics",     ALL_MOSAICS);
        genFilterCombo->addItem("Selected mosaics",SELECTED_MOSAICS);
        genFilterCombo->addItem("Worklist Mosaics",WORKLIST);
        defaultType = ALL_MOSAICS;
    }
    else
    {
        genFilterCombo->addItem("All Tilings",     ALL_TILINGS);
        genFilterCombo->addItem("Selected Tilings",SELECTED_TILINGS);
        genFilterCombo->addItem("Worklist Tilings",WORKLIST);
        defaultType = ALL_TILINGS;
    }
    genFilterCombo->blockSignals(false);

    int index = genFilterCombo->findData(config->genFileFilter);
    if (index < 0)
    {
        config->genFileFilter = defaultType;
        index = genFilterCombo->findData(defaultType);
        Q_ASSERT(index >=0);
    }
    genFilterCombo->setCurrentIndex(index);
}

void page_image_tools::loadViewFilterCombo()
{
    viewFilterCombo->blockSignals(true);
    viewFilterCombo->clear();
    eLoadType defaultType;
    if (config->viewCycle2 == CYCLE_VIEW_MOSAICS)
    {
        viewFilterCombo->addItem("All Mosaics",     ALL_MOSAICS);
        viewFilterCombo->addItem("Selected mosaics",SELECTED_MOSAICS);
        viewFilterCombo->addItem("Worklist Mosaics",WORKLIST);
        defaultType = ALL_MOSAICS;
    }
    else
    {
        viewFilterCombo->addItem("All Tilings",     ALL_TILINGS);
        viewFilterCombo->addItem("Selected Tilings",SELECTED_TILINGS);
        viewFilterCombo->addItem("Worklist Tilings",WORKLIST);
        defaultType = ALL_TILINGS;
    }
    viewFilterCombo->blockSignals(false);

    int index = viewFilterCombo->findData(config->viewFileFilter);
    if (index < 0)
    {
        config->viewFileFilter = defaultType;
        index = viewFilterCombo->findData(defaultType);
        Q_ASSERT(index >=0);
    }
    viewFilterCombo->setCurrentIndex(index);
}

void page_image_tools::loadVersionFilterCombo()
{
    versionFilterCombo->blockSignals(true);
    versionFilterCombo->clear();

    eLoadType defaultType;
    if (config->vCompTile)
    {
        versionFilterCombo->addItem("All Tilings",     ALL_TILINGS);
        versionFilterCombo->addItem("Selected Tilings",SELECTED_TILINGS);
        versionFilterCombo->addItem("Worklist Tilings",WORKLIST);
        defaultType = ALL_TILINGS;
    }
    else
    {
        versionFilterCombo->addItem("All Mosaics",     ALL_MOSAICS);
        versionFilterCombo->addItem("Selected mosaics",SELECTED_MOSAICS);
        versionFilterCombo->addItem("Worklist Mosaics",WORKLIST);
        defaultType = ALL_MOSAICS;
    }

    versionFilterCombo->blockSignals(false);

    int index = versionFilterCombo->findData(config->versionFilter);
    if (index < 0)
    {
        config->versionFilter = defaultType;
        index = versionFilterCombo->findData(defaultType);
        Q_ASSERT(index >=0);
    }
    versionFilterCombo->setCurrentIndex(index);
}

bool page_image_tools::viewImage(QString filename, bool transparent, bool popup)
{
    imageCompareResult->setText("");

    QPixmap pixmap(filename);
    if (pixmap.isNull())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Image not found or not valid");
        box.exec();
        return false;
    }

    engine.view_image(filename,transparent,popup);
    return true;
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
    if (generatorType == ACT_GEN_TILING_BMP)
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

void page_image_tools::clearComparisonWorklist()
{
    comparisonWorklist.clear();
}

void page_image_tools::addToComparisonWorklist(QString name)
{
    QMutexLocker lock(&comparisonMutex);
    comparisonWorklist << name;
}

QStringList & page_image_tools::getComparisonWorklist()
{
    return comparisonWorklist;
}
