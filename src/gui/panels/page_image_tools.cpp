#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QRadioButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QtConcurrentMap>
#include <QFuture>

#include "gui/panels/page_image_tools.h"
#include "gui/panels/panel_misc.h"
#include "gui/top/controlpanel.h"
#include "gui/top/splash_screen.h"
#include "gui/top/system_view.h"
#include "gui/top/system_view_controller.h"
#include "gui/widgets/dlg_wlist_create.h"
#include "gui/widgets/image_widget.h"
#include "gui/widgets/layout_sliderset.h"
#include "gui/widgets/memory_combo.h"
#include "gui/widgets/worklist_widget.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/mosaics/mosaic.h"
#include "model/mosaics/mosaic_manager.h"
#include "model/settings/configuration.h"
#include "sys/engine/compare_bmp_engine.h"
#include "sys/engine/image_engine.h"
#include "sys/engine/mosaic_bmp_generator.h"
#include "sys/engine/mosaic_stepper.h"
#include "sys/engine/png_stepper.h"
#include "sys/engine/stepping_engine.h"
#include "sys/engine/tiling_bmp_generator.h"
#include "sys/engine/tiling_stepper.h"
#include "sys/engine/version_stepper.h"
#include "sys/engine/compare_bmp_stepper.h"
#include "sys/engine/view_bmp_stepper.h"
#include "sys/qt/qtapplog.h"
#include "sys/qt/timers.h"
#include "sys/sys.h"
#include "sys/sys/debugflags.h"
#include "sys/sys/fileservices.h"
#include "sys/tiledpatternmaker.h"
#include "sys/tiledpatternmaker.h"
#include "sys/tiledpatternmaker.h"

using Qt::endl;

QMutex      page_image_tools::comparisonMutex;
VersionList page_image_tools::comparisonList;

page_image_tools:: page_image_tools(ControlPanel * cpanel)  : panel_page(cpanel,PAGE_IMAGE_TOOLS,"Image Tools")
{
    generatorType     = ACT_GEN_MOSAIC_BMP;     // default
    created           = false;
    logDebug          = false;
    etimer            = new AQElapsedTimer(false);
    Sys::imageEngine  = &engine;

    grid = new QGridLayout();

    int row = 0;
    addHSeparator(row);
    row++;
    row = createImageSelectionBox(row);
    row = createCycleGenBox(row);
    row = createViewImagesBox(row);
    row = createWorklistBox(row);
    row = createCompareOptionsBox(row);
    row = createCompareImagesBox(row);
    row = createCompareVersionsBox(row);
    row = createViewImageBox(row);
    addHSeparator(row);

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

int  page_image_tools::createImageSelectionBox(int row)
{
    QRadioButton * rSavMosaics = new QRadioButton("Mosaics");
    QRadioButton * rSavTiles   = new QRadioButton("Tilings");

    imageFilterCombo             = new QComboBox();
    imageFilterCombo->setFixedWidth(121);

    imageBtnGroup = new QButtonGroup;
    imageBtnGroup->addButton(rSavMosaics,IMAGE_MOSAICS);
    imageBtnGroup->addButton(rSavTiles,  IMAGE_TILINGS);

    if (config->imageType == IMAGE_MOSAICS)
        rSavMosaics->setChecked(true);
    else
        rSavTiles->setChecked(true);

    loadFileFilterCombo();

    QLabel * l1 = new  QLabel("Image Select :");
    l1->setStyleSheet("font-weight : bold");

    grid->addWidget(l1,row,0);

    QHBoxLayout * hb1 = new QHBoxLayout;
    hb1->addStretch();
    hb1->addWidget(rSavMosaics );
    hb1->addWidget(rSavTiles);
    hb1->addWidget(imageFilterCombo);
    hb1->addStretch();

    grid->addLayout(hb1,row,1);
    row ++;

    addHSeparator(row);
    row++;

    connect(imageBtnGroup,      &QButtonGroup::idClicked,        this, &page_image_tools::slot_imageTypeChanged);
    connect(imageFilterCombo,   &QComboBox::currentIndexChanged, this, &page_image_tools::slot_imgFilterChanged);

    return row;
}

int  page_image_tools::createCycleGenBox(int row)
{
    // title row
    QLabel * l1 = new  QLabel("Generate BMPs :");
    l1->setStyleSheet("font-weight : bold");

    QCheckBox   * chkMultiThread  = new QCheckBox("Multi-thread");
    chkMultiThread->setChecked(config->multithreadedGeneration);

    QHBoxLayout * hb0 = new QHBoxLayout();
    hb0->addWidget(l1);
    hb0->addStretch();

    grid->addLayout(hb0,row,0,1,2);
    grid->addWidget(chkMultiThread,row,2);
    row++;

    // second row
    QPushButton  * saveDirBtn  = new QPushButton("Dir");
    saveDirBtn->setMaximumWidth(51);

    directoryCombo             = new DirMemoryCombo("DirCombo");
    directoryCombo->initialise();
    QString text = Sys::getSysBMPDirectory();
    directoryCombo->setCurrentText(text);

    QFontMetrics fm(directoryCombo->font());
    auto rect = fm.boundingRect(text);
    int width = rect.width() + 7;
    if (width < 161)
    {
        width = 161;
    }
    directoryCombo->setFixedWidth(width);    // this width seems to set th whole page width

    QCheckBox   * includeBkgds    = new QCheckBox("Include Background Images");
    includeBkgds->setChecked(config->includeBkgdGeneration);

    QCheckBox    * skip        = new QCheckBox("Skip Existing");
    skip->setFixedWidth(97);
    skip->setChecked(config->skipExisting);

    generateBMPsBtn            = new QPushButton("Generate BMPs");
    generateBMPsBtn->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    // generation
    QHBoxLayout * hb1 = new QHBoxLayout;
    hb1->addWidget(saveDirBtn,row);
    hb1->addWidget(directoryCombo);
    hb1->addStretch();
    hb1->addWidget(includeBkgds);
    hb1->addWidget(skip);

    grid->addLayout(hb1,row,0,1,2);
    grid->addWidget(generateBMPsBtn,row,2);
    row++;

    // separator
    addHSeparator(row);
    row++;

    connect(includeBkgds,    &QCheckBox::clicked,       this,  [this](bool checked) { config->includeBkgdGeneration = checked; });
    connect(chkMultiThread,  &QCheckBox::clicked,       this,  [this](bool checked) { config->multithreadedGeneration = checked; });
    connect(generateBMPsBtn, &QPushButton::clicked,     this,  &page_image_tools::slot_genBMPs);
    connect(saveDirBtn,      &QPushButton::clicked,     this,  &page_image_tools::slot_opendir);
    connect(skip,            &QCheckBox::clicked,       this,  &page_image_tools::slot_skipExisting);

    return row;
}

int  page_image_tools::createViewImagesBox(int row)
{
    QLabel * l2 = new  QLabel("View Images :");
    l2->setStyleSheet("font-weight : bold");

    SpinSet * spCycleInterval = new SpinSet("Cycle Interval",0,0,9,true);
    spCycleInterval->setValue(config->cycleInterval);

    chkPngs = new QCheckBox("Original PNGs");

    viewImgesBtn = new QPushButton("View Images");
    viewImgesBtn->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    QHBoxLayout * hb = new QHBoxLayout();
    hb->addStretch();
    hb->addLayout(spCycleInterval);
    hb->addWidget(chkPngs);

    grid->addWidget(l2,row,0);
    grid->addLayout(hb,row,1);
    grid->addWidget(viewImgesBtn,row,2);
    row++;

    addHSeparator(row);
    row++;

    connect(spCycleInterval, &SpinSet::valueChanged,    this,  &page_image_tools::slot_cycleIntervalChanged);
    connect(viewImgesBtn,    &QPushButton::clicked,     this,  &page_image_tools::slot_startStepping);

    return row;
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

    wlistLabel = new QLabel("Work List operations ");
    wlistLabel->setStyleSheet("font-weight : bold");

    QHBoxLayout * hbox0 = new QHBoxLayout;
    hbox0->addWidget(wlistLabel);
    hbox0->addStretch();

    grid->addLayout(hbox0,row,0,1,3);
    row++;

    grid->addLayout(hbox,row,1);
    row++;

    addHSeparator(row);
    row++;

    connect(loadListBtn,  &QPushButton::clicked,     this,  &page_image_tools::slot_loadWorkListFromFile);
    connect(saveListBtn,  &QPushButton::clicked,     this,  &page_image_tools::slot_saveWorkListToFile);
    connect(editListBtn,  &QPushButton::clicked,     this,  &page_image_tools::slot_editWorkList);
    connect(delBtn,       &QPushButton::clicked,     this,  [this] () { slot_deleteCurrentWLEntry(true);} );
    connect(createListBtn,&QPushButton::clicked,     this,  &page_image_tools::slot_createWorkList);
    connect(&engine,      &ImageEngine::sig_deleteCurrentInWorklist, this, &page_image_tools::slot_deleteCurrentWLEntry);

    return row;
}

int page_image_tools::createCompareOptionsBox(int row)
{
    QLabel * label = new QLabel("Comparison Options :");
    label->setStyleSheet("font-weight : bold");

    QHBoxLayout * hbox3 = new QHBoxLayout;
    hbox3->addWidget(label);
    hbox3->addStretch();

    grid->addLayout(hbox3,row,0,1,3);
    row++;

    QRadioButton  * rInPlace     = new QRadioButton("In-place");
    QRadioButton  * rPopup       = new QRadioButton("Pop-up");
    popupGroup = new QButtonGroup();
    popupGroup->addButton(rInPlace);
    popupGroup->addButton(rPopup);

    DoubleSpinSet * popupScale   = new DoubleSpinSet("Pop-up Scale",1.0,0.1,9.0);
    popupScale->setSingleStep(0.1);
    popupScale->setDecimals(1);

    QCheckBox   * transparent    = new QCheckBox("Transparent");
    QPushButton * colorEdit      = new QPushButton("Set Filter Color");
    colorLabel                   = new QLabel();
    colorLabel->setFixedWidth(51);
    QCheckBox  * chkUseFilter    = new QCheckBox("Use Filter Color");

    QHBoxLayout * hbox4 = new QHBoxLayout();
    hbox4->addWidget(rInPlace);

    hbox4->addStretch();
    hbox4->addWidget(rPopup);
    hbox4->addLayout(popupScale);
    hbox4->addSpacing(5);
    hbox4->addWidget(transparent);
    hbox4->addWidget(chkUseFilter);
    hbox4->addWidget(colorLabel);
    hbox4->addSpacing(3);
    hbox4->addWidget(colorEdit);
    hbox4->addStretch();

    grid->addLayout(hbox4,row,0,1,3);
    row++;

    addHSeparator(row);
    row++;

    transparent->setChecked(config->compare_transparent);
    if (config->compare_popup)
        rPopup->setChecked(true);
    else
        rInPlace->setChecked(true);
    chkUseFilter->setChecked(config->compare_filterColor);

    connect(colorEdit,              &QPushButton::clicked,  this,   &page_image_tools::slot_colorEdit);
    connect(chkUseFilter,           &QCheckBox::clicked,    this,   &page_image_tools::slot_useFilter);
    connect(transparent,            &QCheckBox::clicked,    this,   &page_image_tools::slot_transparentClicked);
    connect(rPopup,                 &QRadioButton::clicked, this,   &page_image_tools::slot_popupClicked);
    connect(rInPlace,               &QRadioButton::clicked, this,   &page_image_tools::slot_inPlaceClicked);
    connect(popupScale,       &DoubleSpinSet::valueChanged, this,   [this](qreal val) { engine.setPopupScale(val); } );

    return row;
}

int page_image_tools::createCompareImagesBox(int row)
{
    firstBMPCompareCombo        = new QComboBox();
    secondBMPCompareCombo       = new QComboBox();
    imageCompareResult          = new QLineEdit();
    firstDir                    = new MemoryCombo("leftDir");
    secondDir                   = new MemoryCombo("rightDir");

    QPushButton * firstDirBtn   = new QPushButton("1st Dir");
    QPushButton * secondDirBtn  = new QPushButton("2nd Dir");
    QPushButton * viewImage0    = new QPushButton("View 1st");
    QPushButton * viewImage1    = new QPushButton("View 2nd");
    QPushButton * previousBtn   = new QPushButton("Previous");
    QPushButton * nextBtn       = new QPushButton("Next");
    QPushButton * compareDirBMP = new QPushButton("Compare");
    QPushButton * genCompareBtn = new QPushButton("Generate WL");
    startBtn                    = new QPushButton("Start");
    QPushButton * swapBtn       = new QPushButton("Swap Dirs");

    bmpView    = new QRadioButton("View");
    bmpCompare = new QRadioButton("Compare");
    compareBtnGroup = new QButtonGroup();
    compareBtnGroup->addButton(bmpCompare);
    compareBtnGroup->addButton(bmpView);
    bmpCompare->setChecked(true);       // always start in compare mode

    firstBMPCompareCombo->setMinimumWidth(461);
    imageCompareResult->setReadOnly(true);

    genCompareBtn->setStyleSheet("QPushButton { background-color: yellow; color: red;}");
    compareDirBMP->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    useCompareWL               = new QCheckBox("Use WorkList");
    compareLoaded2nd           = new QCheckBox("2nd use loaded");
    QPushButton * replBtn      = new QPushButton("Replace Second BMP");
                 loadSecondBtn = new QPushButton("Load Second");
                 viewSecondBtn = new QPushButton("View Second");

    QHBoxLayout * hbox2 = new QHBoxLayout;
    hbox2->addWidget(imageCompareResult);
    hbox2->addWidget(compareLoaded2nd);

    QLabel * label = new QLabel("Compare BMP Dirs :");
    label->setStyleSheet("font-weight : bold");

    QHBoxLayout * hbox3 = new QHBoxLayout;
    hbox3->addWidget(label);
    hbox3->addStretch();
    hbox3->addWidget(bmpCompare);
    hbox3->addWidget(bmpView);
    hbox3->addStretch();
    hbox3->addWidget(viewSecondBtn);
    hbox3->addWidget(loadSecondBtn);
    hbox3->addWidget(replBtn);

    grid->addLayout(hbox3,row,0,1,2);
    grid->addWidget(useCompareWL,row,2);
    row++;

    grid->addWidget(firstDirBtn,row,0);
    grid->addWidget(firstDir,row,1);
    grid->addWidget(genCompareBtn,row,2);
    row++;

    grid->addWidget(secondDirBtn,row,0);
    grid->addWidget(secondDir,row,1);
    grid->addWidget(startBtn,row,2);
    row++;

    grid->addWidget(viewImage0,row,0);
    grid->addWidget(firstBMPCompareCombo,row,1);
    grid->addWidget(nextBtn,row,2);
    row++;

    grid->addWidget(viewImage1,row,0);
    grid->addWidget(secondBMPCompareCombo,row,1);
    grid->addWidget(previousBtn,row,2);
    row++;

    grid->addWidget(swapBtn,row,0);
    grid->addLayout(hbox2,row,1);
    grid->addWidget(compareDirBMP,row,2);
    row++;

    addHSeparator(row);
    row++;

    firstDir->initialise();
    loadCombo(firstBMPCompareCombo,firstDir->getCurrentText());
    secondDir->initialise();
    loadCombo(secondBMPCompareCombo,secondDir->getCurrentText());

    useCompareWL->setChecked(config->use_workListForCompare);

    connect(firstDirBtn,            &QPushButton::clicked,  this,   &page_image_tools::slot_selectDir0);
    connect(secondDirBtn,           &QPushButton::clicked,  this,   &page_image_tools::slot_selectDir1);
    connect(swapBtn,                &QPushButton::clicked,  this,   &page_image_tools::slor_swapDirs);

    connect(genCompareBtn,          &QPushButton::clicked,  this,   &page_image_tools::slot_generateComparisonWL);
    connect(startBtn,               &QPushButton::clicked,  this,   &page_image_tools::slot_startCompareCycle);
    connect(nextBtn,                &QPushButton::clicked,  this,   &page_image_tools::slot_next);
    connect(previousBtn,            &QPushButton::clicked,  this,   &page_image_tools::slot_previous);
    connect(compareDirBMP,          &QPushButton::clicked,  this,   &page_image_tools::slot_compareSelectedBMPs);

    connect(viewImage0,             &QPushButton::clicked,  this,   &page_image_tools::slot_viewImageLeft);
    connect(viewImage1,             &QPushButton::clicked,  this,   &page_image_tools::slot_viewImageRight);

    connect(useCompareWL,           &QCheckBox::toggled,    this,   &page_image_tools::slot_toggle_useWL);
    connect(compareLoaded2nd,       &QCheckBox::toggled,    this,   &page_image_tools::slot_toggle_loaded2nd);

    connect(replBtn,                &QPushButton::clicked,  this,   &page_image_tools::slot_replaceBMP);
    connect(loadSecondBtn,          &QPushButton::clicked,  this,   &page_image_tools::slot_loadSecond);
    connect(viewSecondBtn,          &QPushButton::clicked,  this,   &page_image_tools::slot_viewSecond);

    connect(firstDir,  &MemoryCombo::editTextChanged,       this,   &page_image_tools::slot_firstDirChanged);
    connect(secondDir, &MemoryCombo::editTextChanged,       this,   &page_image_tools::slot_secondDirChanged);

    connect(firstDir,  &QComboBox::currentIndexChanged,     this,   &page_image_tools::slot_firstDirChanged);
    connect(secondDir, &QComboBox::currentIndexChanged,     this,   &page_image_tools::slot_secondDirChanged);

    connect(firstBMPCompareCombo,  SIGNAL(currentIndexChanged(int)), this,   SLOT(slot_BMPCompare0_changed(int)));
    connect(secondBMPCompareCombo, SIGNAL(currentIndexChanged(int)), this,   SLOT(slot_BMPCompare1_changed(int)));

    setCombo(firstBMPCompareCombo,config->BMPCompare0);
    setCombo(secondBMPCompareCombo,config->BMPCompare1);

    return row;
}

int page_image_tools::createCompareVersionsBox(int row)
{
    QLabel * label = new QLabel("Compare Versions :");
    label->setStyleSheet("font-weight : bold");

    radImg    = new QRadioButton("Images");
    radXML    = new QRadioButton("XML");
    typeGroup = new QButtonGroup();
    typeGroup->addButton(radImg);
    typeGroup->addButton(radXML);

    chkLock = new QCheckBox("Lock");

    QPushButton  * compareVerBMP = new QPushButton("Compare");
    compareVerBMP->setStyleSheet(" QPushButton { background-color: yellow; color: red;}");

    mediaA    = new QComboBox();
    versionsA = new QComboBox();
    mediaB    = new QComboBox();
    versionsB = new QComboBox();

    QPushButton * viewImageBtn3  = new QPushButton("View");
    QPushButton * viewImageBtn4  = new QPushButton("View");
    QLabel      * dummyLabel     = new QLabel(" ");
    dummyLabel->setMinimumWidth(79);

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(label);
    hbox->addStretch();
    hbox->addWidget(radImg);
    hbox->addWidget(radXML);
    hbox->addStretch();

    QHBoxLayout * hbox2 = new QHBoxLayout();
    hbox2->addWidget(mediaA);
    hbox2->addWidget(versionsA);

    QHBoxLayout * hbox4 = new QHBoxLayout();
    hbox4->addWidget(mediaB);
    hbox4->addWidget(versionsB);

    grid->addLayout(hbox,row,0,1,2);
    grid->addWidget(chkLock,row,2);
    row++;

    grid->addWidget(viewImageBtn3,row,0);
    grid->addLayout(hbox2,row,1);
    row++;

    grid->addWidget(viewImageBtn4,row,0);
    grid->addLayout(hbox4,row,1);
    grid->addWidget(compareVerBMP,row,2);
    row++;

    addHSeparator(row);
    row++;

    chkLock->setChecked(config->vCompLock);
    if (config->vCompXML)
        radXML->setChecked(true);
    else
        radImg->setChecked(true);

    connect(mediaA,  &QComboBox::currentIndexChanged, this, [this]() { engine.verStepper->mediaAChanged(); });
    connect(mediaB,  &QComboBox::currentIndexChanged, this, [this]() { engine.verStepper->mediaBChanged(); });

    connect(compareVerBMP,  &QPushButton::clicked,   this,  &page_image_tools::slot_compareDiffVerBMPs);
    connect(chkLock,        &QCheckBox::clicked,     this,  [this](bool checked) { config->vCompLock = checked; });
    connect(radXML,         &QRadioButton::clicked,  this,  [this](bool checked) { config->vCompXML  = checked; });
    connect(radImg,         &QRadioButton::clicked,  this,  [this](bool checked) { config->vCompXML  = !checked; });

    connect(&engine,        &ImageEngine::sig_ready, this,  &page_image_tools::slot_nextImage, Qt::QueuedConnection);

    connect(viewImageBtn3,  &QPushButton::clicked,   this,  &page_image_tools::slot_viewImage3);
    connect(viewImageBtn4,  &QPushButton::clicked,   this,  &page_image_tools::slot_viewImage4);

    return row;
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

    QLabel * label = new QLabel("Compare Image Files :");
    label->setStyleSheet("font-weight : bold");
    QHBoxLayout * hbox3 = new QHBoxLayout;
    hbox3->addWidget(label);
    hbox3->addStretch();

    grid->addLayout(hbox3,row,0,1,2);
    grid->addWidget(compareFileBMP,row,2);
    row++;

    grid->addWidget(viewImageBtn1,row,0);
    grid->addWidget(viewFileCombo1,row,1);
    grid->addWidget(selectImgBtn1,row,2);
    row++;

    grid->addWidget(viewImageBtn2,row,0);
    grid->addWidget(viewFileCombo2,row,1);
    grid->addWidget(selectImgBtn2,row,2);
    row++;

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

    connect(viewFileCombo1, &QComboBox::currentIndexChanged, this, &page_image_tools::slot_imageSelectionChanged1);
    connect(viewFileCombo2, &QComboBox::currentIndexChanged, this, &page_image_tools::slot_imageSelectionChanged2);

    return row;
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
    engine.verStepper->loadVersionCombos();

    // I'm not sure what the original intention of this code was
    // but now it aligns the right to the left
    QString left  = firstBMPCompareCombo->currentText();
    loadCombo(firstBMPCompareCombo,firstDir->getCurrentText());
    loadCombo(secondBMPCompareCombo,secondDir->getCurrentText());
    slot_setImageLeftCombo(left);
    slot_setImageRightCombo(left);
}

void page_image_tools::onExit()
{
    clearPageStatus();
    viewControl->clearLayout();   // removes any cycler pngs
    Sys::viewController->showView();
}

void  page_image_tools::onRefresh()
{
    Worklist & wlist = config->worklist;
    QString str = QString("Work List Operations : <%1> contains %2 entries").arg(wlist.getName()).arg(wlist.count());
    wlistLabel->setText(str);

    if (engine.mosaicStepper->isStarted() || engine.tilingStepper->isStarted())
        viewImgesBtn->setStyleSheet("QPushButton { background-color: red; color: yellow;}");
    else
        viewImgesBtn->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    if (bmpCompare->isChecked())
    {
        if (engine.compareBMPStepper->isStarted())
            startBtn->setStyleSheet("QPushButton { background-color: red; color: yellow;}");
        else
            startBtn->setStyleSheet("QPushButton { background-color: yellow; color: red;}");
    }
    else
    {
        if (engine.viewBMPStepper->isStarted())
            startBtn->setStyleSheet("QPushButton { background-color: red; color: yellow;}");
        else
            startBtn->setStyleSheet("QPushButton { background-color: yellow; color: red;}");
    }
}

///////////////////////////////////////////////////////////////////////////
///
///  Worklist slots
///
///////////////////////////////////////////////////////////////////////////

void page_image_tools::worklistWrapup()
{
    int sz = config->worklist.count();

    if (sz)
    {
        slot_editWorkList();
    }

    if (useCompareWL->isChecked())
    {
        slot_toggle_useWL(true);
    }

    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    box.setText(QString("%1 filenames loaded into Work List").arg(sz));
    box.exec();
}

void page_image_tools::slot_toggle_useWL(bool checked)
{
    useCompareWL->blockSignals(true);
    useCompareWL->setChecked(checked);
    useCompareWL->blockSignals(false);

    config->use_workListForCompare = checked;

    slot_firstDirChanged();
    slot_secondDirChanged();
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

    // The basic list is a list names e.g. Tes1.v3
    // More sophisticated processing extracts from a string containing a double quoted name
    // e.g.  "This is the name of the file "Test1.v3.xml" OK"

    QTextStream textStream(&file);

    VersionList newVersionList;
    while (!textStream.atEnd())
    {
        QString line = textStream.readLine();
        qDebug().noquote() << line;
        line = line.trimmed();

        int pos = line.indexOf(".xml");
        if (pos != -1)
        {
            line.truncate(pos);
        }

        if (line.contains("\""))
        {
            QStringList qsl = line.split('"');
            line = qsl[1];
        }
        else
        {
#ifdef Q_OS_WINDOWS
            QStringList qsl = line.split('/');
            int n = qsl.size();
            line = qsl[n-1];
#endif
        }
        VersionedName vn(line);
        newVersionList.add(vn);
    }
    file.close();

    newVersionList.removeDuplicates();
    newVersionList.sort();

    qDebug() << "file list: " << newVersionList.getNames();
    if (newVersionList.isEmpty())
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

    // list is prepared, add or replace
    bool replace = true;
    if (config->worklist.count() > 0)
    {
        QMessageBox box2(this);
        box2.setIcon(QMessageBox::Question);
        box2.setText(QString("Replace Worklist or Add to existing worklist"));
        auto b1 = box2.addButton("Add", QMessageBox::YesRole);
        auto b2 = box2.addButton("Replace", QMessageBox::NoRole);
        box2.setDefaultButton(b2);
        box2.exec();
        if (box2.clickedButton() == b1)
            replace = false;
    }

    if (replace)
    {
        // replace
        config->worklist.set(fname,newVersionList);
    }
    else
    {
        // add
        VersionList oldList = config->worklist.get();
        oldList += newVersionList;

        oldList.removeDuplicates();
        oldList.sort();

        config->worklist.set(fname,oldList);
    }

    worklistWrapup();
}

void page_image_tools::slot_saveWorkListToFile()
{
    Worklist & wlist = config->worklist;

    config->worklist.removeDuplicates();

    const VersionList & vlist = wlist.get();
    if (vlist.isEmpty())
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

    for (const auto & vn :  vlist)
    {
        ts << vn.get().toLocal8Bit().constData() << endl;
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
    plw->addItems(config->worklist.get().getNames());

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addWidget(plw);

    QDialog * ewl = new QDialog(this);
    ewl->setAttribute(Qt::WA_DeleteOnClose);
    ewl->setLayout(vbox);

    ewl->exec();
}

void page_image_tools::slot_createWorkList()
{
#define QT_NO_USE_NODISCARD_FILE_OPEN   // for Qt 6.10

    DlgWorklistCreate dlg(this);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted)
        return;

    VersionList vlist;
    VersionList wlist;
    QString     listName;

    if (dlg.selMosaic->isChecked())
    {
        listName = "mosaic_list";
        QStringList targetStringList;
        if (dlg.chkLoadFilter->isChecked())
        {
            vlist = FileServices::getMosaicNames(SELECTED_MOSAICS);
        }
        else
        {
            vlist = FileServices::getMosaicNames(ALL_MOSAICS);
        }

        if (!dlg.inverseMosaicWorklist->isChecked())
        {
            if (dlg.radText->isChecked())
            {
                targetStringList << dlg.text->text();
            }
            else if (dlg.radMotif->isChecked())
            {
                targetStringList << dlg.selectedMotifNames();
            }
            else if (dlg.radStyle->isChecked())
            {
                targetStringList << dlg.styleNames->currentText();
            }

            if (targetStringList.isEmpty())
                return;

            for (VersionedName & name : vlist)
            {
                VersionedFile mosaic = FileServices::getFile(name,FILE_MOSAIC);

                QFile file(mosaic.getPathedName());
                if (file.open(QIODevice::ReadOnly))
                {
                    QTextStream in (&file);
                    const QString content = in.readAll();
                    for (auto & target : std::as_const(targetStringList))
                    {
                        if (content.contains(target))
                        {
                            wlist.add(name);
                        }
                    }
                }
            }
        }
        else
        {
            listName = "inverse_mosaic_list";
        }
    }
    else
    {
        listName = "tiling_list";
        QString target;
        if (dlg.chkLoadFilter->isChecked())
        {
            vlist = FileServices::getTilingNames(SELECTED_TILINGS);
        }
        else
        {
            vlist = FileServices::getTilingNames(ALL_TILINGS);
        }

        if (!dlg.inverseMosaicWorklist->isChecked())
        {
            if (dlg.radText->isChecked())
            {
                target = dlg.text->text();
            }

            if (target.isEmpty())
                return;

            for (VersionedName & name : vlist)
            {
                VersionedFile tiling = FileServices::getFile(name,FILE_TILING);

                QFile file(tiling.getPathedName());
                if (file.open(QIODevice::ReadOnly))
                {
                    QTextStream in (&file);
                    const QString content = in.readAll();
                    if (content.contains(target))
                    {
                        wlist.add(name);
                    }
                }
            }
        }
        else
        {
            listName = "inverse_tiling_list";
        }
    }

    if (dlg.inverseMosaicWorklist->isChecked())
    {
        // remove currebt worklist from vlist (ignores wlist)
        for (VersionedName & vname : config->worklist.get())
        {
            vlist.remove(vname);
        }
        config->worklist.clear();
        config->worklist.set(listName,vlist);
    }
    else if (dlg.useWorklist->isChecked())
    {
        // eliminate worklist items not in current selection
        VersionList oldvl = config->worklist.get();
        VersionList newvl;
        for (auto & vn : oldvl)
        {
            if (wlist.contains(vn))
            {
                newvl.add(vn);
            }
        }
        config->worklist.clear();
        config->worklist.set(listName,newvl);
    }
    else
    {
        config->worklist.clear();
        config->worklist.set(listName,wlist);
    }

    worklistWrapup();
}

///////////////////////////////////////////////////////////////////////////
///
///  Slots
///
///////////////////////////////////////////////////////////////////////////

void page_image_tools::slot_imageTypeChanged(int id)
{
    // switches between mosaics and tilings
    //qDebug() << "page_image_tools::slot_genTypeChanged" << id;
    config->imageType = (eImageType)id;
    loadFileFilterCombo();

    engine.verStepper->loadVersionCombos();
}

void page_image_tools::slot_imgFilterChanged()
{
    // triggered when filter combo is loaded or its selection changes
    config->imageFileFilter = static_cast<eLoadType>(imageFilterCombo->currentData().toInt());
    //qDebug() << "generate BMP file filter" << config->imageFileFilter;

    engine.verStepper->loadVersionCombos();
}

void page_image_tools::slot_cycleIntervalChanged(int value)
{
    config->cycleInterval = value;
}

void page_image_tools::slot_firstDirChanged()
{
    firstDir->select(firstDir->currentIndex());
    loadCombo(firstBMPCompareCombo,firstDir->getCurrentText());
}

void page_image_tools::slot_secondDirChanged()
{
    secondDir->select(secondDir->currentIndex());
    loadCombo(secondBMPCompareCombo,secondDir->getCurrentText());
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
    loadCombo(firstBMPCompareCombo,firstDir->getCurrentText());
    loadCombo(secondBMPCompareCombo,secondDir->getCurrentText());
}

void page_image_tools::slot_viewImageLeft()
{
    qDebug() << "slot_viewImageLeft";
    VersionedName name(firstBMPCompareCombo->currentText());
    if (name.isEmpty())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("No filename");
        box.exec();
        return;
    }
    config->BMPCompare0 = name.get();
    VersionedFile file(firstDir->getCurrentText() + "/" + name.get() + ".bmp");
    viewImage(file);
}

void page_image_tools::slot_viewImageRight()
{
    qDebug() << "slot_viewImageRight";

    if (compareLoaded2nd->isChecked())
    {
        panel->delegateView(VIEW_BMP_IMAGE,false);
        panel->delegateView(VIEW_MOSAIC,true);
        emit sig_reconstructView();
    }
    else
    {
        VersionedName name(secondBMPCompareCombo->currentText());
        if (name.isEmpty())
        {
            QMessageBox box(this);
            box.setIcon(QMessageBox::Warning);
            box.setText("No filename");
            box.exec();
            return;
        }

        config->BMPCompare1 = name.get();
        VersionedFile file(secondDir->getCurrentText() + "/" + name.get() + ".bmp");
        viewImage(file);
    }
}

void page_image_tools::slot_transparentClicked(bool checked)
{
    config->compare_transparent = checked;
}

void page_image_tools::slot_popupClicked(bool checked)
{
    if (checked)
        config->compare_popup = true;
}

void page_image_tools::slot_inPlaceClicked(bool checked)
{
    if (checked)
    {
        engine.closeAllImageViewers();
        config->compare_popup = false;
    }
}

void page_image_tools::slot_toggle_loaded2nd(bool checked)
{
    if (checked)
    {
        loadSecondBtn->setStyleSheet("QPushButton { background-color: yellow; color: red;}");
        viewSecondBtn->setStyleSheet("QPushButton { background-color: yellow; color: red;}");
    }
    else
    {
        loadSecondBtn->setStyleSheet("");
        viewSecondBtn->setStyleSheet("");
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

void page_image_tools::loadCombo(QComboBox * combo, QString dir)
{
    QStringList names;
    if (!config->use_workListForCompare)
    {
        names = FileServices::getDirBMPFiles(dir).getNames();
    }
    else
    {
        names = config->worklist.get().getNames();
    }

    combo->blockSignals(true);
    combo->clear();
    combo->addItems(names);
    combo->blockSignals(false);
}

void page_image_tools::setCombo(QComboBox * combo, QString name)
{
    combo->blockSignals(true);
    int index = combo->findText(name);
    if (index == -1) index = 0;
    combo->setCurrentIndex(index);
    combo->blockSignals(false);
}

void page_image_tools::slot_setImageLeftCombo(QString name)
{
    int index = firstBMPCompareCombo->findText(name);
    firstBMPCompareCombo->blockSignals(true);
    firstBMPCompareCombo->setCurrentIndex(index);
    firstBMPCompareCombo->blockSignals(false);
}

void page_image_tools::slot_setImageRightCombo(QString name)
{
    int index = secondBMPCompareCombo->findText(name);
    secondBMPCompareCombo->blockSignals(true);
    secondBMPCompareCombo->setCurrentIndex(index);
    secondBMPCompareCombo->blockSignals(false);
}

void page_image_tools::slot_BMPCompare0_changed(int index)
{
    Q_UNUSED(index);
    QString current = firstBMPCompareCombo->currentText();
    config->BMPCompare0 = current;

    if (!current.isEmpty())
    {
        // special case for ibox1 - not symmetric
        slot_setImageRightCombo(current);  // makes it the same

        VersionedName vn(current);
        engine.compareBMPStepper->resync(current);
    }
}

void page_image_tools::slot_BMPCompare1_changed(int index)
{
    Q_UNUSED(index);
    QString current = secondBMPCompareCombo->currentText();
    config->BMPCompare1 = current;
}

void page_image_tools::slot_loadSecond()
{
    engine.closeAllImageViewers();
    QString mos = secondBMPCompareCombo->currentText();
    VersionedName vn(mos);

    if (config->imageType == IMAGE_MOSAICS)
    {
        VersionedFile vf = FileServices::getFile(vn,FILE_MOSAIC);
        mosaicMaker->loadMosaic(vf);
    }
    else
    {
        VersionedFile vf = FileServices::getFile(vn,FILE_TILING);
        tilingMaker->loadTiling(vf,TILM_LOAD_SINGLE);
    }
}

void page_image_tools::slot_viewSecond()
{
    engine.closeAllImageViewers();
    panel->delegateView(VIEW_MOSAIC,true);
}

void page_image_tools::slot_opendir()
{
    QString old = Sys::getBMPPath(generatorType);
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Image Directory"),
                                                    old, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty())
    {
        qDebug() << "Cancel Pressed";
        //directoryCombo->setCurrentText(Sys::getWorkingBMPDirectory());
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

        directoryCombo->setCurrentText(dir);    // this sets Sys:: too
    }

#ifdef Q_OS_WINDOWS
    QString path = Sys::getBMPPath(generatorType);
    qDebug() <<  "Path:" << path;

    QString path2 = QDir::toNativeSeparators(path);
    qDebug() <<  "Path2:" << path2;

    QStringList args;
    args << path2;

    QProcess::startDetached("explorer",args);
#endif
}

void page_image_tools::slot_replaceBMP()
{
    // replace bitmp in second dir

    QString name = secondBMPCompareCombo->currentText();
    VersionedName vn(name);
    VersionedFile vf = FileServices::getFile(vn,FILE_MOSAIC);
    bool rv = false;
    MosaicPtr mosaic = mosaicMaker->loadMosaic(vf);
    if (mosaic)
    {
        QPixmap pixmap = Sys::viewController->grabView();
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
        VersionedName nameA(firstBMPCompareCombo->currentText());
        VersionedName nameB(secondBMPCompareCombo->currentText());

        engine.closeAllImageViewers();
        engine.compareBMPs(nameA,nameB);
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

    QString name = secondBMPCompareCombo->currentText();
    VersionList newList;
    const VersionList & list = config->worklist.get();
    for (auto & vn : list)
    {
        if (vn.get() != name)
        {
            newList.add(vn);
        }
        else
            qDebug() << name << ": deleted";
    }

    auto existingName = config->worklist.getName();
    config->worklist.set(existingName,newList);
    emit engine.sig_next(); // immediate
    engine.compareBMPStepper->setWorklist(newList);
}

void page_image_tools::slot_useFilter(bool checked)
{
    config->compare_filterColor = checked;
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
    VersionedFile file(viewFileCombo1->currentText());
    bool rv = viewImage(file);
    if (!rv)
    {
        viewFileCombo1->eraseCurrent();
    }
}

void page_image_tools::slot_viewImage2()
{
    qDebug() << "slot_viewImage2";
    VersionedFile file(viewFileCombo2->currentText());
    bool rv = viewImage(file);
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

void page_image_tools::slot_viewImage3()
{
    qDebug() << "slot_viewImage3";
    QString name = versionsA->currentText();
    VersionedName vn(name);
    VersionedFile vf = FileServices::getFile(vn,FILE_MOSAIC);
    auto mosaic = mosaicMaker->loadMosaic(vf);

    if (mosaic)
    {
        mosaic->dumpMotifs();
        mosaic->dumpStyles();
    }
}

void page_image_tools::slot_viewImage4()
{
    qDebug() << "slot_viewImag4";
    QString name = versionsB->currentText();
    VersionedName vn(name);
    VersionedFile vf = FileServices::getFile(vn,FILE_MOSAIC);
    auto mosaic = mosaicMaker->loadMosaic(vf);

    if (mosaic)
    {
        mosaic->dumpMotifs();
        mosaic->dumpStyles();
    }
}

//////////////////////////////////////////////////////////////////////////////////////
//
//  Comparisons
//
//////////////////////////////////////////////////////////////////////////////////////

void page_image_tools::slot_compareSelectedBMPs()
{
    if (bmpView->isChecked())
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Warning);
        box.setText("Notihng to compare in View mode");
        box.exec();
        return;
    }

    engine.setCompareMode(true);

    imageCompareResult->setText("");
    pageStatusString = "Spacebar=next C=compare P=ping-pong L=log Q=quit D=delete-from-worklist";
    setPageStatus();
    if (!compareLoaded2nd->isChecked())
    {
        VersionedName nameA(firstBMPCompareCombo->currentText());
        VersionedName nameB(secondBMPCompareCombo->currentText());
        engine.compareBMPs(nameA,nameB);
    }
    else
    {
        if (!viewControl->isEnabled(VIEW_MOSAIC))
        {
            QMessageBox box(this);
            box.setIcon(QMessageBox::Warning);
            box.setText("Load Mosaic into view as second before comparing");
            box.exec();
            return;
        }
        VersionedName nameA(firstBMPCompareCombo->currentText());
        engine.compareBMPwithLoaded(nameA);
    }
}

void  page_image_tools::slot_compareDiffVerBMPs()
{
    if (config->imageType == IMAGE_TILINGS)
        panel->delegateView(VIEW_TILING,true);
    else
        panel->delegateView(VIEW_MOSAIC,true);

    engine.verStepper->compareVersions();
}

void page_image_tools::slot_compareFileBMPs()
{
    pageStatusString = "Spacebar=next C=compare P=ping-pong L=log Q=quit D=delete-from-worklist";
    setPageStatus();

    VersionedFile fileA(viewFileCombo1->currentText());
    VersionedFile fileB(viewFileCombo2->currentText());
    engine.compareBMPs(fileA,fileB);
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

    if (config->imageType == IMAGE_TILINGS)
        panel->delegateView(VIEW_TILING,true);
    else
        panel->delegateView(VIEW_MOSAIC,true);

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
    if (chkPngs->isChecked())
    {
        engine.pngStepper->begin();
    }
    else if (config->imageType == IMAGE_MOSAICS)
    {
        pageStatusString = "Press P to Pause/unPause - press Q to Quit";
        setPageStatus();
        panel->delegateView(VIEW_MOSAIC,true);
        engine.mosaicStepper->setImmediat((config->cycleInterval == 0));
        engine.mosaicStepper->begin();
    }
    else
    {
        Q_ASSERT(config->imageType == IMAGE_TILINGS);
        pageStatusString = "Press P to Pause/unPause - press Q to Quit";
        setPageStatus();
        panel->delegateView(VIEW_TILING,true);
        engine.tilingStepper->begin();
    }
}

void page_image_tools::slot_startCompareCycle()
{
    // could be compare or view

    if (bmpCompare->isChecked())
    {
        pageStatusString = "Spacebar=next alt-Spacebar=previous C=compare P=ping-pong Q=quit D=delete-from-worklist";
        setPageStatus();
    }
    else
    {
        Q_ASSERT(bmpView->isChecked());
        pageStatusString = "Spacebar=next alt-Spacebar=previous Q=quit D=delete-from-worklist";
        setPageStatus();
    }

    engine.closeAllImageViewers(true);

    VersionList vlist;
    if (config->use_workListForCompare)
    {
        vlist = config->worklist.get();
    }
    else
    {
        for(int i = 0; i < firstBMPCompareCombo->count(); i++)
        {
            VersionedName vn;
            vn.set(firstBMPCompareCombo->itemText(i));
            vlist.add(vn);
        }
    }

    bool rv = false;
    if (bmpCompare->isChecked())
    {
        engine.compareBMPStepper->setWorklist(vlist);

        rv = engine.compareBMPStepper->begin();
    }
    else
    {
        Q_ASSERT(bmpView->isChecked());

        engine.viewBMPStepper->setWorklist(vlist);

        rv = engine.viewBMPStepper->begin();
    }
    if (rv)
    {
        if (config->compare_popup)
        {
            auto img = Sys::imageEngine->currentPopup();
            if (img)
            {
                img->activateWindow();
                int X = img->width() / 2;
                int Y = img->height() / 2;
                QPoint globalCenter = img->mapToGlobal(QPoint(X,Y));
                QCursor::setPos(globalCenter.x(), globalCenter.y());
            }
        }
        else
        {
            Sys::viewController->activateWindow();
            int X = Sys::viewController->viewWidth() / 2;
            int Y = Sys::viewController->viewHeight() / 2;
            QPoint globalCenter = Sys::viewController->mapToGlobal(QPoint(X,Y));
            QCursor::setPos(globalCenter.x(), globalCenter.y());
        }
    }
}

void page_image_tools::slot_previous()
{
    engine.closeAllImageViewers(false);
    if (bmpCompare->isChecked())
    {
        engine.compareBMPStepper->prev();
    }
    else
    {
        Q_ASSERT(bmpView->isChecked());
        engine.viewBMPStepper->prev();
    }
}

void page_image_tools::slot_next()
{
    engine.closeAllImageViewers(false);
    if (bmpCompare->isChecked())
    {
        engine.compareBMPStepper->next();
    }
    else
    {
        Q_ASSERT(bmpView->isChecked());
        engine.viewBMPStepper->next();
    }
}

//////////////////////////////////////////////////////////////////////////////////////
//
//  Multi-threading
//
//////////////////////////////////////////////////////////////////////////////////////

bool takeAction(sAction action)
{
    if (Sys::imgGeneratorInUse)
    {
        if (action.type ==  ACT_GEN_MOSAIC_BMP)
        {
            MosaicBMPGenerator engine;
            bool rv = engine.saveBitmap(action.name,action.path);
            return rv;
        }
        else if (action.type == ACT_GEN_TILING_BMP)
        {
            TilingBMPGenerator engine;
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
        qInfo() << "Cancelled" << action.name.get();
        return false;
    }
}

void page_image_tools::processActionList(QList<sAction> &actions)
{
    totalEngineImages = actions.size();

    auto log = qtAppLog::getInstance();
    logDebug = log->getLogDebug();

    if (config->multithreadedGeneration)
    {
        qInfo() << "Concurrent processes - starting";
        log->logDebug(false);
        log->logToPanel(false);     // thread safety: dont write to gui
        watcher.setFuture(QtConcurrent::mapped(actions,takeAction));
    }
    else
    {
        Q_ASSERT(Sys::imgGeneratorInUse);
        for (const auto & action : std::as_const(actions))
        {
            QString astring = "Processing : " + action.name.get();
            Sys::splash->display(astring,true);
            takeAction(action);
            Sys::splash->remove(true);
        }

        Sys::imgGeneratorInUse = false;
        log->logDebug(logDebug);

        if (generatorType == ACT_GEN_MOSAIC_BMP)
            panel->delegateView(VIEW_MOSAIC,true);
        else if (generatorType == ACT_GEN_TILING_BMP)
            panel->delegateView(VIEW_TILING,true);

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
    auto log = qtAppLog::getInstance();
    log->logDebug(logDebug);
    log->logToPanel(config->logToPanel);

    qInfo() << "Image Engine completed";

    Sys::imgGeneratorInUse = false;
    Sys::localCycle        = false;

    QString str = QString("Concurrent processes took %1 seconds").arg(etimer->getElapsed().trimmed());
    QString str2 =        "Cycle complete";

    Sys::splash->remove();

    switch(generatorType)
    {
    case ACT_GEN_MOSAIC_BMP:
        panel->delegateView(VIEW_MOSAIC,true);
        break;

    case ACT_GEN_TILING_BMP:
        panel->delegateView(VIEW_TILING,true);
        break;

    case ACT_GEN_COMPARE_WLIST:
        Sys::viewController->appSuspendPaint(false);
        auto size = comparisonList.count();
        if (size)
        {
            config->worklist.set("generated",comparisonList);
            slot_toggle_useWL(true);
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
    Sys::splash->replace(txt,true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
//
// BMPs
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

void page_image_tools::saveMosaicBitmaps()
{
    qInfo() << "saveMosaicBitmaps";

    etimer->start();

    eLoadType id = config->imageFileFilter;

    VersionList files;
    if (id == SINGLE_MOSAIC)
    {
        QString file = firstBMPCompareCombo->currentText();
        VersionedName vn(file);
        files.add(vn);
    }
    else
    {
        files = FileServices::getMosaicNames(id);
    }

    QString pixmapPath  = Sys::getBMPPath(generatorType);

    QList<sAction> actions;
    for (const VersionedName & name : std::as_const(files))
    {
        if (config->skipExisting)
        {
            QString file = pixmapPath + "/" + name.get() + ".bmp";
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

    auto id            = config->imageFileFilter;
    VersionList files  = FileServices::getTilingNames(id);
    QString pixmapPath = Sys::getBMPPath(generatorType);

    QList<sAction> actions;
    for (const VersionedName & name : std::as_const(files))
    {
        if (config->skipExisting)
        {
            QString file = pixmapPath + "/" + name.get() + ".bmp";
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

    VersionList imgList;
    if (config->use_workListForCompare)
    {
        imgList = config->worklist.get();

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
        QStringList names = FileServices::getDirBMPFiles(MemoryCombo::getTextFor("leftDir")).getNames();
        imgList.setNames(names);
    }

    comparisonList.clear();

    QList<sAction> actions;
    for (const auto & name : std::as_const(imgList))
    {
        QString pathLeft  = MemoryCombo::getTextFor("leftDir")  + "/" + name.get()  + ".bmp";
        QString pathRight = MemoryCombo::getTextFor("rightDir") + "/" + name.get() + ".bmp";

        sAction action;
        action.type  = ACT_GEN_COMPARE_WLIST;
        action.name  = name;
        action.path  = pathLeft;
        action.path2 = pathRight;
        actions.push_back(action);
    }

    processActionList(actions);

    if (!config->multithreadedGeneration && comparisonList.count())
    {
        config->worklist.set("generated",comparisonList);
        slot_toggle_useWL(true);
    }
}

void page_image_tools::slot_genBMPs()
{
    qDebug() << Sys::getWorkingBMPDirectory();

    Sys::flags->enable(false); // turn off debug

    if (config->imageType == IMAGE_MOSAICS)
        generatorType = ACT_GEN_MOSAIC_BMP;
    else
        generatorType = ACT_GEN_TILING_BMP;

    setupActions();

    secondDir->setCurrentText(Sys::getBMPPath(generatorType));
}

void page_image_tools::slot_generateComparisonWL()
{
    if (bmpView->isChecked())
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Warning);
        box.setText("Cannot generate worklist in View mode");
        box.exec();
        return;
    }

    generatorType = ACT_GEN_COMPARE_WLIST;

    setupActions();
}

void page_image_tools::setupActions()
{
    if (Sys::imgGeneratorInUse)
    {
        // this is a cancel
        watcher.cancel();
        qInfo() << "Generate Mosaic bitmaps  Cancelled: waiting to finish";
        Sys::splash->remove();
        Sys::splash->display("CANCELLED - waiting to finish");

        watcher.waitForFinished();

        Sys::localCycle        = false;
        Sys::imgGeneratorInUse = false;
        qInfo() << "Generate Mosaic bitmaps : Finished";
        Sys::splash->remove();
        Sys::splash->display("CANCELLED - Finished");

        return;
    }

    panel->delegateView(VIEW_DEBUG,false);

    if (generatorType == ACT_GEN_MOSAIC_BMP)
    {
        Q_ASSERT(!Sys::imgGeneratorInUse);
        Sys::imgGeneratorInUse = true;
        Sys::localCycle        = true;
        Sys::viewController->disableAllViews();
        emit sig_reconstructView();
        saveMosaicBitmaps();
    }
    else if (generatorType == ACT_GEN_TILING_BMP)
    {
        Q_ASSERT(!Sys::imgGeneratorInUse);
        Sys::imgGeneratorInUse = true;
        Sys::localCycle        = true;
        Sys::viewController->disableAllViews();
        emit sig_reconstructView();
        saveTilingBitmaps();
    }
    else if (generatorType == ACT_GEN_COMPARE_WLIST)
    {
        Q_ASSERT(!Sys::imgGeneratorInUse);
        Sys::imgGeneratorInUse = true;
        Sys::localCycle        = true;
        Sys::viewController->appSuspendPaint(true);
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
    imageFilterCombo->blockSignals(true);
    imageFilterCombo->clear();
    eLoadType defaultType;
    if (config->imageType == IMAGE_MOSAICS)
    {
        imageFilterCombo->addItem("All Mosaics",          ALL_MOSAICS);
        imageFilterCombo->addItem("Loader mosaics",       SELECTED_MOSAICS);
        imageFilterCombo->addItem("Worklist Mosaics",     WORKLIST);
        imageFilterCombo->addItem("All except worklist",  ALL_MOS_EXCEPT_WL);
        imageFilterCombo->addItem("View 1st name only",   SINGLE_MOSAIC);
        defaultType = ALL_MOSAICS;
    }
    else
    {
        imageFilterCombo->addItem("All Tilings",      ALL_TILINGS);
        imageFilterCombo->addItem("Loader Tilings",   SELECTED_TILINGS);
        imageFilterCombo->addItem("Worklist Tilings", WORKLIST);
        defaultType = ALL_TILINGS;
    }
    imageFilterCombo->blockSignals(false);

    int index = imageFilterCombo->findData(config->imageFileFilter);
    if (index < 0)
    {
        config->imageFileFilter = defaultType;
        index = imageFilterCombo->findData(defaultType);
        Q_ASSERT(index >=0);
    }
    imageFilterCombo->setCurrentIndex(index);
}

bool page_image_tools::viewImage(VersionedFile &file)
{
    imageCompareResult->setText("");

    QPixmap pixmap(file.getPathedName());
    if (pixmap.isNull())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Image not found or not valid");
        box.exec();
        return false;
    }

    engine.view_image(file);
    return true;
}

void page_image_tools::addToComparisonWorklist(VersionedName name)
{
    QMutexLocker lock(&comparisonMutex);
    comparisonList.add(name);
}

