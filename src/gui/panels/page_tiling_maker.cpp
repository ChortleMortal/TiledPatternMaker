#include <QComboBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QHeaderView>
#include <QGroupBox>
#include <QMessageBox>
#include <QTextEdit>
#include <QMenu>
#include <QButtonGroup>

#include "gui/model_editors/tiling_edit/tile_selection.h"
#include "gui/panels/page_tiling_maker.h"
#include "gui/top/controlpanel.h"
#include "gui/top/view_controller.h"
#include "gui/viewers/construction_lines_view.h"
#include "gui/viewers/image_view.h"
#include "gui/widgets/dlg_edgepoly_edit.h"
#include "gui/widgets/dlg_listnameselect.h"
#include "gui/widgets/dlg_listselect.h"
#include "gui/widgets/dlg_trim.h"
#include "gui/widgets/floatable_tab.h"
#include "gui/widgets/smx_widget.h"
#include "model/makers/motif_maker.h"
#include "model/makers/mosaic_maker.h"
#include "model/mosaics/mosaic.h"
#include "model/settings/configuration.h"
#include "model/styles/style.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/transform.h"
#include "sys/qt/utilities.h"
#include "sys/sys.h"
#include "sys/sys/fileservices.h"

typedef std::weak_ptr<Tiling>   WeakTilingPtr;

using std::make_shared;

Q_DECLARE_METATYPE(WeakPlacedTilePtr)

#define TILE_TABLE_LIMIT 30

page_tiling_maker:: page_tiling_maker(ControlPanel * cpanel)  : panel_page(cpanel,PAGE_TILING_MAKER,"Tiling Maker")
{
    initPageStatusString();

    controlTab = createCreateTab();
    repeatTab  = createRepeatTab();
    stateTab   = createAdjustTab();
    clinesTab  = createConstructionLinesTab();
    detailsTab = createDetailsTab();

    tilingMaker->resetKeybdMode(); // tirggers selection

    tabWidget = new QTabWidget;
    tabWidget->addTab(controlTab, "Create");
    tabWidget->addTab(repeatTab,  "Repeat");
    tabWidget->addTab(stateTab,   "Adjust");
    tabWidget->addTab(clinesTab,  "C-Lines");
    tabWidget->addTab(detailsTab, "Details");

    vbox->addWidget(tabWidget);
    vbox->addStretch();

    connect(tilingMaker,  &TilingMaker::sig_menuRefresh,    this, &page_tiling_maker::slot_refreshMenu);
    connect(tabWidget,    &QTabWidget::tabBarDoubleClicked, this, &page_tiling_maker::slot_detach);

    QSettings s = Sys::settings();

    // detach in descending tab-index order so removing one tab doesn't shift the index of another not-yet-processed tab
    QString name = QString("panel2/%1/float_details_tab").arg(pageName);
    bool tofloat = s.value(name,false).toBool();
    if (tofloat)
        slot_detach(4);

    name    = QString("panel2/%1/float_clines_tab").arg(pageName);
    tofloat = s.value(name,false).toBool();
    if (tofloat)
        slot_detach(3);

    name    = QString("panel2/%1/float_state_tab").arg(pageName);
    tofloat = s.value(name,false).toBool();
    if (tofloat)
        slot_detach(2);

    name    = QString("panel2/%1/float_repeat_tab").arg(pageName);
    tofloat = s.value(name,false).toBool();
    if (tofloat)
        slot_detach(1);

    name    = QString("panel2/%1/float_control_tab").arg(pageName);
    tofloat = s.value(name,false).toBool();
    if (tofloat)
        slot_detach(0);
}

page_tiling_maker::~page_tiling_maker()
{
    QSettings s = Sys::settings();

    QString name = QString("panel2/%1/float_control_tab").arg(pageName);
    s.setValue(name,controlTab->floating);
    if (controlTab->floating)
        controlTab->savePosition();
    controlTab->floating = false;   // so it closes correctly

    name = QString("panel2/%1/float_state_tab").arg(pageName);
    s.setValue(name,stateTab->floating);
    if (stateTab->floating)
        stateTab->savePosition();
    stateTab->floating = false;   // so it closes correctly

    name = QString("panel2/%1/float_repeat_tab").arg(pageName);
    s.setValue(name,repeatTab->floating);
    if (repeatTab->floating)
        repeatTab->savePosition();
    repeatTab->floating = false;   // so it closes correctly

    name = QString("panel2/%1/float_clines_tab").arg(pageName);
    s.setValue(name,clinesTab->floating);
    if (clinesTab->floating)
        clinesTab->savePosition();
    clinesTab->floating = false;   // so it closes correctly

    name = QString("panel2/%1/float_details_tab").arg(pageName);
    s.setValue(name,detailsTab->floating);
    if (detailsTab->floating)
        detailsTab->savePosition();
    detailsTab->floating = false;   // so it closes correctly
}

void page_tiling_maker::slot_detach(int index)
{
    QWidget * widget = tabWidget->widget(index);
    QString title    = tabWidget->tabText(index);

    tabWidget->removeTab(index);

    FloatableTab * floater = dynamic_cast<FloatableTab*>(widget);
    Q_ASSERT(floater);
    floater->detach(tabWidget,title);
}

FloatableTab * page_tiling_maker::createCreateTab()
{
    QGroupBox * tilingGroup = createTilingsGroup();
    QGroupBox * tileGroup   = createTileGroup();
    QGroupBox * actionGroup = createActionsGroup();
    QGroupBox * kbdGroup    = createKbdModesCreateTab();
    QGroupBox * modeGroup   = createModesGroup();
    QGroupBox * stackGroup  = createStackGroup();
    QVBoxLayout * bkgd      = createBackgroundInfo();

    QHBoxLayout * hbox      = new QHBoxLayout();
    hbox->addWidget(tilingGroup);
    hbox->addWidget(tileGroup);
    hbox->addWidget(actionGroup);

    QHBoxLayout * stackRow  = new QHBoxLayout();
    stackRow->addWidget(stackGroup);
    stackRow->addStretch();

    QVBoxLayout * layout1   = new QVBoxLayout();
    layout1->addWidget(kbdGroup);
    layout1->addLayout(hbox);
    layout1->addWidget(modeGroup);
    layout1->addLayout(stackRow);
    layout1->addLayout(bkgd);
    layout1->addStretch();

    FloatableTab *tab = new FloatableTab(this);
    tab->setLayout(layout1);

    return tab;
}

FloatableTab *page_tiling_maker::createAdjustTab()
{

    QGroupBox   * kbdGroup  = createKbdModesAdjustTab();
    QTableWidget * table    = createTilingTable();

    QCheckBox * chkShowExludesInTable = new QCheckBox("Show Excluded");
    chkShowExludesInTable->setChecked(config->tm_excludesInTable);

    QBoxLayout * hbox1 = new QHBoxLayout();
    hbox1->addWidget(chkShowExludesInTable);
    hbox1->addStretch();

    QGroupBox * distortGroup = createDistortionGroup();

    QVBoxLayout * layout2 = new QVBoxLayout();
    layout2->addWidget(kbdGroup);
    layout2->addWidget(table);
    layout2->addSpacing(11);
    layout2->addLayout(hbox1);
    layout2->addWidget(distortGroup);
    layout2->addStretch();

    FloatableTab *tab = new FloatableTab(this);
    tab->setLayout(layout2);

    connect(chkShowExludesInTable,  &QCheckBox::clicked,    this,   &page_tiling_maker::slot_showExludesInTable);

    return tab;
}

FloatableTab * page_tiling_maker::createDetailsTab()
{
    debugWidget = createDebugInfo();

    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget(debugWidget);
    layout->addStretch();

    FloatableTab * tab = new FloatableTab(this);
    tab->setLayout(layout);

    return tab;
}

FloatableTab * page_tiling_maker::createRepeatTab()
{
    QGroupBox * repsGroup = createRepetitionsGroup();

    drawTransRepeat = new AQPushButton("Set Translations (F3)");
    connect(drawTransRepeat, &QPushButton::clicked, this, &page_tiling_maker::slot_setTranslationsMode);

    QHBoxLayout * modeRow = new QHBoxLayout();
    modeRow->addWidget(drawTransRepeat);
    modeRow->addStretch();

    QVBoxLayout * layout3 = new QVBoxLayout();
    layout3->addLayout(modeRow);
    layout3->addWidget(repsGroup);
    layout3->addStretch();

    FloatableTab *tab = new FloatableTab(this);
    tab->setLayout(layout3);

    return tab;
}

QGroupBox * page_tiling_maker::createTilingsGroup()
{
    tilingList  = new QListWidget(this);
    tilingList->setFixedHeight(45);
    tilingList->setMaximumWidth(151);

    QPushButton * pbMerge = new QPushButton("Merge Tilings");
    pbMerge->setFixedHeight(23);

    AQVBoxLayout * avbox = new AQVBoxLayout;
    avbox->addWidget(tilingList);
    avbox->addWidget(pbMerge);

    tilingsGroup = new QGroupBox("Tilings");
    tilingsGroup->setContentsMargins(0,11,0,11);
    tilingsGroup->setLayout(avbox);

    connect(tilingList,    &QListWidget::currentItemChanged,  this, &page_tiling_maker::slot_currentItemChanged);
    connect(tilingList,    &QListWidget::itemChanged,         this, &page_tiling_maker::slot_itemChanged);
    connect(pbMerge,       &QPushButton::clicked,             this, &page_tiling_maker::slot_mergeTilings);

    return tilingsGroup;
}

QGroupBox * page_tiling_maker::createTileGroup()
{
    // itles
    QPushButton * addPolyBtn      = new QPushButton("Add Polygon");
    QPushButton * addGirihBtn     = new QPushButton("Add Girih");
    QPushButton * exportBtn       = new QPushButton("Save Tile");
    QPushButton * importBtn       = new QPushButton("Load Tile");

    QLabel * nlabel = new QLabel("Sides");
    sides = new AQSpinBox;
    sides->setFixedWidth(41);
    sides->setMinimum(1);
    sides->setMaximum(128);
    sides->setValue(config->tm_poly_side_count);
    AQHBoxLayout * hb1 = new AQHBoxLayout();
    hb1->addWidget(nlabel);
    hb1->addWidget(sides);

    QLabel * rlabel = new QLabel("Rot");
    tileRot = new FullPrecisionDoubleSpinBox;
    tileRot->setRange(-360.0,360.0);
    tileRot->setDisplayDecimals(4);
    tileRot->setValue(config->tm_poly_rotation);
    AQHBoxLayout * hb2 = new AQHBoxLayout();
    hb2->addWidget(rlabel);
    hb2->addSpacing(9);
    hb2->addWidget(tileRot);

    girihShapes = new QComboBox();
    girihShapes->addItem("Decagon", "gDecagon");
    girihShapes->addItem("Pentagon","gPentagon");
    girihShapes->addItem("Bowtie",  "gBowtie");
    girihShapes->addItem("Kite",    "gKite");
    girihShapes->addItem("Rhombus", "gRhombus");

    QGridLayout * grid = new QGridLayout();
    grid->setContentsMargins(0,0,0,0);

    grid->addWidget(addPolyBtn, 0,0);
    grid->addLayout(hb1,        1,0);
    grid->addLayout(hb2,        2,0);

    grid->addWidget(addGirihBtn,    0,1);
    grid->addWidget(girihShapes,    1,1);

    grid->addWidget(importBtn,      0,2);
    grid->addWidget(exportBtn,      1,2);

    QGroupBox * tileGroup = new QGroupBox("Tiles");
    //tileGroup->setContentsMargins(0,9,0,9);
    tileGroup->setLayout(grid);

    connect(addPolyBtn,     &QPushButton::clicked,          this,   [this]() {tilingMaker->addRegularPolygon(); });
    connect(tileRot,        &QDoubleSpinBox::valueChanged,  this,   [this](qreal value) { config->tm_poly_rotation = value;   });
    connect(sides,          &AQSpinBox::valueChanged,       this,   [this](int value)   { config->tm_poly_side_count = value; });

    connect(exportBtn,      &QPushButton::clicked,          this,   &page_tiling_maker::slot_exportPoly);
    connect(importBtn,      &QPushButton::clicked,          this,   &page_tiling_maker::slot_importGirihPoly);
    connect(addGirihBtn,    &QPushButton::clicked,          this,   &page_tiling_maker::slot_addGirihShape);

    return tileGroup;
}

QGroupBox * page_tiling_maker::createActionsGroup()
{
    // actions
    QPushButton * removeExclBtn   = new QPushButton("Remove Excluded");
    QPushButton * pbClearTiling   = new QPushButton("Clear Tiling");
    QPushButton * reloadTilingBtn = new QPushButton("Re-load Tiling");
    QPushButton * dupTilingBtn    = new QPushButton("Duplicate Tiling");
                  pbRender        = new QPushButton("Render Motifs");

    QGridLayout * grid = new QGridLayout();
    grid->setContentsMargins(0,0,0,0);

    int row = 0;
    grid->addWidget(removeExclBtn,  row,0);
    grid->addWidget(pbClearTiling,  row,1);


    row++;
    grid->addWidget(reloadTilingBtn, row,0);
    grid->addWidget(dupTilingBtn,    row,1);

    row++;
    grid->addWidget(pbRender,       row,0);

    connect(pbClearTiling,     &QPushButton::clicked,  this, &page_tiling_maker::slot_clearTiling);
    connect(reloadTilingBtn,   &QPushButton::clicked,  this, &page_tiling_maker::slot_reloadTiling);
    connect(dupTilingBtn,      &QPushButton::clicked,  this, &page_tiling_maker::slot_duplicateTiling);
    connect(pbRender,          &QPushButton::clicked,  this, [] {Sys::render(RENDER_RESET_MOTIFS);} );
    connect(removeExclBtn,     &QPushButton::clicked,  this, [this]() { tilingMaker->removeExcludeds(); emit sig_updateView(); });

    QGroupBox * actionGroup = new QGroupBox("Actions");
    //actionGroup->setContentsMargins(0,9,0,9);
    actionGroup->setLayout(grid);

    return actionGroup;
}

QGroupBox * page_tiling_maker::createModesGroup()
{
    AQPushButton * nomode       = new AQPushButton("No Mode (ESC)");
    AQPushButton * drawTrans    = new AQPushButton("Set Translations (F3)");
    AQPushButton * newPoly      = new AQPushButton("Draw Tile (F4)");
    AQPushButton * copyPoly     = new AQPushButton("Copy Tile (F5)");
    AQPushButton * deletePoly   = new AQPushButton("Delete Tile (F6)");
    AQPushButton * includePoly  = new AQPushButton("Include/Exclude (F7)");
    AQPushButton * position     = new AQPushButton("Show Position (F8)");
    AQPushButton * measure      = new AQPushButton("Measure (F9)");
    AQPushButton * unify        = new AQPushButton("Unify Tiles (F10)");
    AQPushButton * editPoly     = new AQPushButton("Edit Tile (F11)");
    AQPushButton * mirrorX      = new AQPushButton("Mirror X");
    AQPushButton * mirrorY      = new AQPushButton("Mirror Y");
    AQPushButton * reflect      = new AQPushButton("Reflect Edge");
    AQPushButton * editEdge     = new AQPushButton("Edit Edge (F12)");
    AQPushButton * drawConst    = new AQPushButton("Construction Lines");
    AQPushButton * decompose    = new AQPushButton("Decompose Tiles");

    QCheckBox    * chkSnapTo     = new QCheckBox("Snap to Grid");
    QCheckBox    * chkSnapOnly   = new QCheckBox("Don't Rotate on Snap");
                   chkTileAnchorMode   = new QCheckBox("Tile Anchor Mode");
    QCheckBox    * chkVectorAnchorMode = new QCheckBox("Vector Anchor Mode");
    QCheckBox    * chkFillOnLoad = new QCheckBox("Fill on load");
    QCheckBox    * chkHideTiles  = new QCheckBox("Hide Tiles");
    QCheckBox    * chkHideVectors = new QCheckBox("Hide translation vectors");
                   chkPropagate  = new QCheckBox("Propagate");

    QGridLayout * modeBox = new QGridLayout();

    int row=0;
    modeBox->addWidget(nomode,row,0);
    modeBox->addWidget(drawTrans,row,1);
    modeBox->addWidget(newPoly,row,2);
    modeBox->addWidget(copyPoly,row,3);
    modeBox->addWidget(deletePoly,row,4);
    modeBox->addWidget(decompose,row,5);

    row++;
    modeBox->addWidget(includePoly,row,0);
    modeBox->addWidget(position,row,1);
    modeBox->addWidget(measure,row,2);
    modeBox->addWidget(editPoly,row,3);
    modeBox->addWidget(editEdge,row,4);
    modeBox->addWidget(unify,row,5);

    row++;
    modeBox->addWidget(drawConst,row,0);
    modeBox->addWidget(reflect,row,1);
    modeBox->addWidget(mirrorX,row,2);
    modeBox->addWidget(mirrorY,row,3);

    row++;
    QGridLayout * checkGrid = new QGridLayout();
    int crow=0;
    checkGrid->addWidget(chkFillOnLoad,       crow,0);
    checkGrid->addWidget(chkPropagate,        crow,1);

    crow++;
    checkGrid->addWidget(chkSnapTo,           crow,0);
    checkGrid->addWidget(chkSnapOnly,         crow,1);
    checkGrid->addWidget(chkTileAnchorMode,   crow,2);
    checkGrid->addWidget(chkVectorAnchorMode, crow,3);

    crow++;
    checkGrid->addWidget(chkHideTiles,        crow,0);
    checkGrid->addWidget(chkHideVectors,      crow,1);

    checkGrid->setColumnStretch(4,1);
    modeBox->addLayout(checkGrid,row,0,1,6);

    row++;
    QVBoxLayout * hb2 = createSecondControlRow();
    modeBox->addLayout(hb2,row,0,1,6);

    QGroupBox * modeGroup = new QGroupBox("Modes");
    modeGroup->setLayout(modeBox);

    // grouping
    mouseModeBtnGroup = new QButtonGroup;
    mouseModeBtnGroup->setExclusive(false);     // allows uncheck

    mouseModeBtnGroup->addButton(drawTrans,     (int)TilingMaker::MouseMode::TranslationVectors);
    mouseModeBtnGroup->addButton(newPoly,       (int)TilingMaker::MouseMode::DrawPolygon);
    mouseModeBtnGroup->addButton(copyPoly,      (int)TilingMaker::MouseMode::Copy);
    mouseModeBtnGroup->addButton(deletePoly,    (int)TilingMaker::MouseMode::Delete);
    mouseModeBtnGroup->addButton(includePoly,   (int)TilingMaker::MouseMode::Inclusion);
    mouseModeBtnGroup->addButton(nomode,        (int)TilingMaker::MouseMode::None);
    mouseModeBtnGroup->addButton(position,      (int)TilingMaker::MouseMode::Position);
    mouseModeBtnGroup->addButton(measure,       (int)TilingMaker::MouseMode::Measure);
    mouseModeBtnGroup->addButton(unify,         (int)TilingMaker::MouseMode::Unify);
    mouseModeBtnGroup->addButton(editPoly,      (int)TilingMaker::MouseMode::Edit);
    mouseModeBtnGroup->addButton(editEdge,      (int)TilingMaker::MouseMode::CurveEdge);
    mouseModeBtnGroup->addButton(mirrorX,       (int)TilingMaker::MouseMode::MirrorX);
    mouseModeBtnGroup->addButton(mirrorY,       (int)TilingMaker::MouseMode::MirrorY);
    mouseModeBtnGroup->addButton(reflect,       (int)TilingMaker::MouseMode::ReflectEdge);
    mouseModeBtnGroup->addButton(drawConst,     (int)TilingMaker::MouseMode::ConstructionLines);
    mouseModeBtnGroup->addButton(decompose,     (int)TilingMaker::MouseMode::Decompose);

    lastChecked = mouseModeBtnGroup->button((int)TilingMaker::MouseMode::None);
    lastChecked->setChecked(true);

    chkPropagate->setChecked(true);
    chkFillOnLoad->setChecked(config->tm_fillOnLoad);
    chkSnapTo->setChecked(config->tm_snapToGrid);
    chkSnapOnly->setChecked(config->tm_snapOnly);
    chkTileAnchorMode->setChecked(config->tm_tileAnchorMode);
    chkVectorAnchorMode->setChecked(config->tm_vectorAnchorMode);
    chkHideTiles->setChecked(Sys::tm_hideTiles);

    connect(chkSnapTo,      &QCheckBox::clicked,   this,   [this](bool checked) { config->tm_snapToGrid =  checked;});
    connect(chkSnapOnly,    &QCheckBox::clicked,   this,   [this](bool checked) { config->tm_snapOnly   =  checked;});
    connect(chkTileAnchorMode,   &QCheckBox::clicked,   this,   [this](bool checked) { config->tm_tileAnchorMode = checked; });
    connect(chkVectorAnchorMode, &QCheckBox::clicked,   this,   [this](bool checked) { config->tm_vectorAnchorMode = checked;
                                                                                if (checked)
                                                                                    tilingMaker->resolveTranslationAnchors();
                                                                                else
                                                                                    tilingMaker->clearTranslationAnchors(); });
    connect(chkFillOnLoad,  &QCheckBox::clicked,   this,   &page_tiling_maker::slot_fillOnLoad);
    connect(chkHideTiles,   &QCheckBox::clicked,   this,   [this](bool checked) { Sys::tm_hideTiles = checked; Sys::tilingMakerView->forceRedraw(); });
    connect(chkHideVectors, &QCheckBox::clicked,   this,   &page_tiling_maker::slot_hideVectors);
    connect(chkPropagate,   &QCheckBox::clicked,   tilingMaker, &TilingMaker::slot_propagate_changed);
    connect(mouseModeBtnGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, &page_tiling_maker::slot_setModes);

    return modeGroup;
}

QGroupBox * page_tiling_maker::createStackGroup()
{
    stackLabel = new QLabel("Stack:");

    uint maxW = 61;
    QPushButton * savStack  = new QPushButton("Save");
    savStack->setMaximumWidth(maxW);
    QPushButton * undoStack = new QPushButton("Undo");
    undoStack->setMaximumWidth(maxW);
    QPushButton * redoStack = new QPushButton("Redo");
    redoStack->setMaximumWidth(maxW);

    QHBoxLayout * stackL = new QHBoxLayout();
    stackL->addWidget(savStack);
    stackL->addSpacing(5);
    stackL->addWidget(undoStack);
    stackL->addSpacing(5);
    stackL->addWidget(redoStack);
    stackL->addSpacing(5);
    stackL->addWidget(stackLabel);
    stackL->addStretch();

    QGroupBox * stackGroup = new QGroupBox("Stack");
    stackGroup->setLayout(stackL);

    connect(savStack,  &QPushButton::clicked, tilingMaker, &TilingMaker::slot_stack_save);
    connect(undoStack, &QPushButton::clicked, tilingMaker, &TilingMaker::slot_stack_undo);
    connect(redoStack, &QPushButton::clicked, tilingMaker, &TilingMaker::slot_stack_redo);

    return stackGroup;
}

QGroupBox * page_tiling_maker::createConstructionLinesGroup()
{
    uint maxW = 61;
    QPushButton * clSave  = new QPushButton("Save");
    clSave->setMaximumWidth(maxW);
    QPushButton * clLoad  = new QPushButton("Load");
    clLoad->setMaximumWidth(maxW);
    QPushButton * clClear = new QPushButton("Clear");
    clClear->setMaximumWidth(maxW);
    chkShowCLines = new QCheckBox("Show Lines");
    clNotSaved    = new QLabel;
    clNotSaved->setStyleSheet("color: red");

    drawConstRepeat = new AQPushButton("Edit");
    drawConstRepeat->setMaximumWidth(maxW);

    smxwidgetCL = new SMXWidget(Sys::constructionLinesView.get(),false,true);

    QHBoxLayout * hb1 = new QHBoxLayout();
    hb1->addWidget(chkShowCLines);
    hb1->addStretch();
    hb1->addWidget(smxwidgetCL);

    QHBoxLayout * hb2 = new QHBoxLayout();
    hb2->addWidget(drawConstRepeat);
    hb2->addSpacing(5);
    hb2->addWidget(clLoad);
    hb2->addSpacing(5);
    hb2->addWidget(clSave);
    hb2->addSpacing(5);
    hb2->addWidget(clClear);
    hb2->addSpacing(15);
    hb2->addWidget(clNotSaved);
    hb2->addStretch();

    QVBoxLayout * clL = new QVBoxLayout();
    clL->addLayout(hb1);
    clL->addLayout(hb2);

    QGroupBox * clGroup = new QGroupBox("Construction Lines");
    clGroup->setLayout(clL);

    chkShowCLines->setChecked(viewControl->isEnabled(Layer::Type::ConstructionLines));

    connect(clSave,  &QPushButton::clicked, tilingMaker, &TilingMaker::saveConstructionLines);
    connect(clLoad,  &QPushButton::clicked, this, &page_tiling_maker::slot_loadConstructionLines);
    connect(clClear, &QPushButton::clicked, tilingMaker, &TilingMaker::clearConstructionLines);
    connect(chkShowCLines, &QCheckBox::clicked, this, [this](bool checked) { panel->userDelegateView(Layer::Type::ConstructionLines,checked); emit sig_reconstructView(); });
    connect(drawConstRepeat, &QPushButton::clicked, this, &page_tiling_maker::slot_setConstructionLinesMode);

    return clGroup;
}

FloatableTab * page_tiling_maker::createConstructionLinesTab()
{
    QGroupBox * clGroup = createConstructionLinesGroup();

    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget(clGroup);
    layout->addStretch();

    FloatableTab * tab = new FloatableTab(this);
    tab->setLayout(layout);

    return tab;
}

QGroupBox * page_tiling_maker::createDistortionGroup()
{
    distortCombo = new AQComboBox();
    distortCombo->addItem("No Distortion",     DISTORT_NONE);
    distortCombo->addItem("Canvas Distortion", DISTORT_CANVAS);
    distortCombo->addItem("Tile Distortion",   DISTORT_TILE);

    distortX = new DoubleSpinSet("X:",1.0,-9.99,9.99);
    distortX->setSingleStep(0.01);
    distortY = new DoubleSpinSet("Y:",1.0,-9.99,9.99);
    distortY->setSingleStep(0.01);

    connect(distortCombo, &QComboBox::currentIndexChanged, this, &page_tiling_maker::slot_setDistortion);
    connect(distortX,   &DoubleSpinSet::valueChanged,this, &page_tiling_maker::slot_setDistortion);
    connect(distortY,   &DoubleSpinSet::valueChanged,this, &page_tiling_maker::slot_setDistortion);

    QHBoxLayout * hb = new QHBoxLayout;
    hb->addWidget(distortCombo);
    hb->addLayout(distortX);
    hb->addSpacing(5);
    hb->addLayout(distortY);
    hb->addStretch();

    QGroupBox * gb = new QGroupBox("Canvas Distortion");
    gb->setLayout(hb);

    return gb;
}

QGroupBox * page_tiling_maker::createRepetitionsGroup()
{
    QHBoxLayout * fillRow = createFillDataRow();
    QVBoxLayout  * trow   = createTranslationsRow();

    QVBoxLayout * vb = new QVBoxLayout;
    vb->addLayout(fillRow);
    vb->addLayout(trow);

    QGroupBox * gb = new QGroupBox("Tiling Unit Repititions");
    gb->setLayout(vb);

    return gb;
}

QHBoxLayout * page_tiling_maker::createFillDataRow()
{
    const int rmin = -99;
    const int rmax =  99;

    chkSingle = new QCheckBox("Singleton");

    xRepMin = new AQSpinBox();
    xRepMax = new AQSpinBox();
    yRepMin = new AQSpinBox();
    yRepMax = new AQSpinBox();

    xRepMin->setRange(rmin,rmax);
    xRepMax->setRange(rmin,rmax);
    yRepMin->setRange(rmin,rmax);
    yRepMax->setRange(rmin,rmax);

    QHBoxLayout * repsLayout = new QHBoxLayout;
    repsLayout->addWidget(new QLabel("T1 Min"));
    repsLayout->addWidget(xRepMin);
    repsLayout->addWidget(new QLabel("T1 Max"));
    repsLayout->addWidget(xRepMax);
    repsLayout->addWidget(new QLabel("T2 Min"));
    repsLayout->addWidget(yRepMin);
    repsLayout->addWidget(new QLabel("T2 Max"));
    repsLayout->addWidget(yRepMax);
    repsLayout->addSpacing(5);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(chkSingle);
    hbox->addLayout(repsLayout);
    hbox->addStretch();
    if (config->insightMode)
    {
        QPushButton * swapBtn = new QPushButton("Swap T1/T2");
        hbox->addWidget(swapBtn);
        connect(swapBtn, &QPushButton::clicked,  this,   &page_tiling_maker::slot_swapTrans);
    }

    connect(chkSingle, &QCheckBox::clicked, this, &page_tiling_maker::singleton_changed);
    connect(xRepMin, SIGNAL(valueChanged(int)),  this, SLOT(slot_set_reps(int)));
    connect(xRepMax, SIGNAL(valueChanged(int)),  this, SLOT(slot_set_reps(int)));
    connect(yRepMin, SIGNAL(valueChanged(int)),  this, SLOT(slot_set_reps(int)));
    connect(yRepMax, SIGNAL(valueChanged(int)),  this, SLOT(slot_set_reps(int)));

    return hbox;
}

QVBoxLayout *page_tiling_maker::createTranslationsRow()
{
    t1x = new DoubleSpinSet("T1-X",0,-100.0,100.0);
    t1y = new DoubleSpinSet("T1-Y",0,-100.0,100.0);
    t2x = new DoubleSpinSet("T2-X",0,-100.0,100.0);
    t2y = new DoubleSpinSet("T2-Y",0,-100.0,100.0);
    t1x->setAlignment(Qt::AlignRight);
    t1y->setAlignment(Qt::AlignRight);
    t2x->setAlignment(Qt::AlignRight);
    t2y->setAlignment(Qt::AlignRight);

    t1x->setDecimals(16);
    t1y->setDecimals(16);
    t2x->setDecimals(16);
    t2y->setDecimals(16);

    t1x->setSingleStep(0.01);
    t1y->setSingleStep(0.01);
    t2x->setSingleStep(0.01);
    t2y->setSingleStep(0.01);

    T1len   = new DoubleSpinSet("Len ",0,0,99.0);
    T2len   = new DoubleSpinSet("Len ",0,0,99.0);
    T1angle = new DoubleSpinSet("Angl",0,-360,360.0);
    T2angle = new DoubleSpinSet("Angl",0,-360,360.0);

    T1len->setAlignment(Qt::AlignRight);
    T2len->setAlignment(Qt::AlignRight);
    T1angle->setAlignment(Qt::AlignRight);
    T2angle->setAlignment(Qt::AlignRight);

    T1len->setDecimals(16);
    T2len->setDecimals(16);
    T1angle->setDecimals(8);
    T2angle->setDecimals(8);

    T1len->setSingleStep(0.01);
    T2len->setSingleStep(0.01);
    T1angle->setSingleStep(0.01);
    T2angle->setSingleStep(0.01);

    QGridLayout * grid = new QGridLayout;
    int row = 0;
    grid->addLayout(t1x,row,0);
    grid->addLayout(t1y,row,1);
    grid->addLayout(t2x,row,2);
    grid->addLayout(t2y,row,3);

    row++;
    grid->addLayout(T1len,row,0);
    grid->addLayout(T1angle,row,1);
    grid->addLayout(T2len,row,2);
    grid->addLayout(T2angle,row,3);

    translationsWidget = new QWidget();
    translationsWidget->setLayout(grid);

    QVBoxLayout * vbox = new QVBoxLayout();
    vbox->addWidget(translationsWidget);

    connect(t1x,  &DoubleSpinSet::valueChanged, this, &page_tiling_maker::slot_t1t2Changed);
    connect(t1y,  &DoubleSpinSet::valueChanged, this, &page_tiling_maker::slot_t1t2Changed);
    connect(t2x,  &DoubleSpinSet::valueChanged, this, &page_tiling_maker::slot_t1t2Changed);
    connect(t2y,  &DoubleSpinSet::valueChanged, this, &page_tiling_maker::slot_t1t2Changed);

    connect(T1len,   &DoubleSpinSet::valueChanged, this, &page_tiling_maker::slot_t1t2LenChanged);
    connect(T2len,   &DoubleSpinSet::valueChanged, this, &page_tiling_maker::slot_t1t2LenChanged);
    connect(T1angle, &DoubleSpinSet::valueChanged, this, &page_tiling_maker::slot_t1t2AngleChanged);
    connect(T2angle, &DoubleSpinSet::valueChanged, this, &page_tiling_maker::slot_t1t2AngleChanged);

    return vbox;
}

QVBoxLayout * page_tiling_maker::createSecondControlRow()
{
    QCheckBox * chkShowOverlaps = new QCheckBox("Show Touch/Overlap");
    QCheckBox * chkShowExcluded = new QCheckBox("Show Excluded");
    QCheckBox * chkShowUnique   = new QCheckBox("Show Unique");

    chkShowOverlaps->setStyleSheet("QCheckBox { border: 1px solid #8f8f8f; border-radius: 6px; padding: 5px 8px;}"
                                  "QCheckBox::checked{background-color: LightGreen; color : black;}");
    chkShowExcluded->setStyleSheet("QCheckBox { border: 1px solid #8f8f8f; border-radius: 6px; padding: 5px 8px;}"
                                  "QCheckBox::checked{background-color: LightGreen; color : black;}");
    chkShowUnique->setStyleSheet(  "QCheckBox { border: 1px solid #8f8f8f; border-radius: 6px; padding: 5px 8px;}"
                                  "QCheckBox::checked{background-color: LightGreen; color : black;}");

    viewModeGroup = new QButtonGroup(this);
    viewModeGroup->setExclusive(false);
    viewModeGroup->addButton(chkShowOverlaps,(int)TilingMakerView::ViewMode::showAttachment);
    viewModeGroup->addButton(chkShowExcluded,(int)TilingMakerView::ViewMode::showExcluded);
    viewModeGroup->addButton(chkShowUnique,(int)TilingMakerView::ViewMode::showUnique);

    lastChecked2 = nullptr;
    connect(viewModeGroup, &QButtonGroup::idClicked, this, &page_tiling_maker::slot_viewModeChanged);

    fillBtn = new QCheckBox("Fill/Repeat");
    fillBtn->setStyleSheet(  "QCheckBox { border: 1px solid #8f8f8f; border-radius: 6px; padding: 5px 8px;}"
                             "QCheckBox::checked{background-color: LightGreen; color : black;}");
    connect(fillBtn, &QCheckBox::clicked, this,  [](bool clickstate) { Sys::tilingMakerView->setFill(clickstate); });

    SpinSet * tileOpacity   = new SpinSet("Opacity",config->tm_tileOpacity,0,100);
    connect(tileOpacity, &SpinSet::valueChanged, this, [this](int value){ config->tm_tileOpacity = value; Sys::tilingMakerView->forceRedraw(); });

    statusLabel   = new QLabel();
    overlapStatus = new QLabel();
    overlapStatus->setStyleSheet("color: green; font-weight: bold;");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(chkShowOverlaps);
    hbox->addWidget(chkShowExcluded);
    hbox->addWidget(chkShowUnique);
    hbox->addWidget(fillBtn);
    hbox->addStretch();
    hbox->addLayout(tileOpacity);

    QHBoxLayout * hbox2 = new QHBoxLayout;
    hbox2->addWidget(statusLabel);
    hbox2->addStretch();
    hbox2->addWidget(overlapStatus);

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addLayout(hbox2);

    return vbox;
}

AQTableWidget * page_tiling_maker::createTilingTable()
{
    // tile info table
    tileInfoTable = new AQTableWidget();
    tileInfoTable->setSortingEnabled(false);
    tileInfoTable->setRowCount(12);
    tileInfoTable->setSelectionMode(QAbstractItemView::SingleSelection);
    tileInfoTable->setSelectionBehavior(QAbstractItemView::SelectColumns);
    tileInfoTable->setContextMenuPolicy(Qt::CustomContextMenu);
    tileInfoTable->setMouseTracking(false);
    tileInfoTable->viewport()->setMouseTracking(false);
    tileInfoTable->setStyleSheet( "QTableWidget::item { selection-color: black; selection-background-color: LightGreen; }");

    QStringList qslv;
    qslv << "tile" << "show" << "type"  << "tile-rot" << "tile-scale" << "tile-sides"<< "placed-scale" << "placed-rot" << "placed-X" << "placed-Y" << "CW" << "tile-addr";
    tileInfoTable->setVerticalHeaderLabels(qslv);
    tileInfoTable->horizontalHeader()->setVisible(false);
    tileInfoTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    tileInfoTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QHeaderView * tileInfoHeader = tileInfoTable->verticalHeader();

    connect(tileInfoTable,   &QTableWidget::customContextMenuRequested, this, &page_tiling_maker::slot_infoTableMenu);
    connect(tileInfoTable,   &QTableWidget::cellClicked,                this, &page_tiling_maker::slot_infoTableCellSelected);
    connect(tileInfoHeader,  &QHeaderView::sectionClicked,              this, &page_tiling_maker::slot_infoTableHeaderClicked);

    return tileInfoTable;
}

QWidget * page_tiling_maker::createDebugInfo()
{
    ///  Debug Status    ///
    debugLabel1  = new QLabel;
    debugLabel2  = new QLabel;

    tileInfo  = new QTextEdit;
    tileInfo->setFixedHeight(149);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(debugLabel1);
    hbox->addSpacing(3);
    hbox->addWidget(debugLabel2);
    hbox->addStretch();

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addSpacing(5);
    vbox->addWidget(tileInfo);
    vbox->addLayout(hbox);
   //vbox->addSpacing(7);

    debugWidget = new QWidget();
    debugWidget->setContentsMargins(0,0,0,0);
    debugWidget->setLayout(vbox);
    return debugWidget;
}

QVBoxLayout * page_tiling_maker::createBackgroundInfo()
{
    chkBkgd = new QCheckBox("Has Background Image");
    pbExam  = new QPushButton("Examine or Load Background Image");

    QHBoxLayout * hb1 = new QHBoxLayout();
    hb1->addWidget(chkBkgd);
    hb1->addWidget(pbExam);
    hb1->addStretch();

    QVBoxLayout * vb1 = new QVBoxLayout;
    vb1->addLayout(hb1);

    connect (pbExam, &QPushButton::pressed, this, [this] { panel->setCurrentPage("Backgrounds");} );
    return vb1;
}

QGroupBox * page_tiling_maker::createKbdModesAdjustTab()
{
    QRadioButton * rbView  = new QRadioButton("Adjust View");
    QRadioButton * rbSTile = new QRadioButton("Adjust Selected Tiling");
    QRadioButton * rbUTile = new QRadioButton("Adjust Unique Tile");
    QRadioButton * rbPTile = new QRadioButton("Adjust Placed Tile");
    smxwidget2             = new SMXWidget(Sys::tilingMakerView.get(),true,true);

    // grouping
    kbdBtnGroup2           = new QButtonGroup();
    kbdBtnGroup2->setExclusive(true);

    kbdBtnGroup2->addButton(rbView,     (int)TilingMaker::KeybdMode::XformAll);
    kbdBtnGroup2->addButton(rbSTile,    (int)TilingMaker::KeybdMode::XformTiling);
    kbdBtnGroup2->addButton(rbUTile,    (int)TilingMaker::KeybdMode::XFormUniqueTile);
    kbdBtnGroup2->addButton(rbPTile,    (int)TilingMaker::KeybdMode::XFormPlacedTile);

    connect(kbdBtnGroup2, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, [this] (QAbstractButton* btn) { slot_setKbdMode1(btn, kbdBtnGroup2); });
    connect(tilingMaker, &TilingMaker::sig_TMKbdMode, this, &page_tiling_maker::slot_kbdMode1);

    QHBoxLayout * hb = new QHBoxLayout;
    hb->addWidget(rbView);
    hb->addWidget(rbSTile);
    hb->addWidget(rbUTile);
    hb->addWidget(rbPTile);
    hb->addStretch();
    hb->addWidget(smxwidget2);

    QGroupBox * gb = new QGroupBox("Keryboard/Mouse Actions");
    gb->setLayout(hb);

    return gb;
}

QGroupBox * page_tiling_maker::createKbdModesCreateTab()
{
    QRadioButton * rbView  = new QRadioButton("Adjust View");
    QRadioButton * rbSTile = new QRadioButton("Adjust Selected Tiling");
    QRadioButton * rbUTile = new QRadioButton("Adjust Unique Tile");
    QRadioButton * rbPTile = new QRadioButton("Adjust Placed Tile");
    smxwidget1             = new SMXWidget(Sys::tilingMakerView.get(),false,true);

    // grouping
    kbdBtnGroup1           = new QButtonGroup();
    kbdBtnGroup1->setExclusive(true);

    kbdBtnGroup1->addButton(rbView,     (int)TilingMaker::KeybdMode::XformAll);
    kbdBtnGroup1->addButton(rbSTile,    (int)TilingMaker::KeybdMode::XformTiling);
    kbdBtnGroup1->addButton(rbUTile,    (int)TilingMaker::KeybdMode::XFormUniqueTile);
    kbdBtnGroup1->addButton(rbPTile,    (int)TilingMaker::KeybdMode::XFormPlacedTile);

    connect(kbdBtnGroup1, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, [this] (QAbstractButton* btn) { slot_setKbdMode1(btn, kbdBtnGroup1); });
    connect(tilingMaker, &TilingMaker::sig_TMKbdMode, this, &page_tiling_maker::slot_kbdMode1);

    QHBoxLayout * smxRow = new QHBoxLayout;
    smxRow->addStretch();
    smxRow->addWidget(smxwidget1);

    QHBoxLayout * radioRow = new QHBoxLayout;
    radioRow->addWidget(rbView);
    radioRow->addWidget(rbSTile);
    radioRow->addWidget(rbUTile);
    radioRow->addWidget(rbPTile);
    radioRow->addStretch();

    QVBoxLayout * vb = new QVBoxLayout;
    vb->addLayout(smxRow);
    vb->addLayout(radioRow);

    QGroupBox * gb = new QGroupBox("Keryboard/Mouse Actions");
    gb->setLayout(vb);

    return gb;
}

// kbdGroup selection
void page_tiling_maker::slot_setKbdMode1(QAbstractButton * btn, QButtonGroup * kbdGroup)
{
    int mode = kbdGroup->id(btn);
    TilingMaker::KeybdMode tmm = static_cast<TilingMaker::KeybdMode>(mode);

    tilingMaker->setKeybdMode(tmm);

    if (tmm != TilingMaker::KeybdMode::XformAll)
        emit Sys::systemViewController->sig_solo(Sys::tilingMakerView.get(),true);
    else
        emit Sys::systemViewController->sig_solo(Sys::tilingMakerView.get(),false);

    tallyKbdMode();
}

// from canvas setKbdMode
void page_tiling_maker::slot_kbdMode1(TilingMaker::KeybdMode mode)
{
    QAbstractButton * button = kbdBtnGroup1->button(mode);
    if (button)
    {
        button->setChecked(true);
    }

    button = kbdBtnGroup2->button(mode);
    {
        button->setChecked(true);
    }
}

// TilingView mode
void page_tiling_maker::slot_viewModeChanged(int id)
{
    QAbstractButton *clicked = viewModeGroup->button(id);

    if (clicked == lastChecked2)
    {
        // re-clicking the active one turns it off
        setViewState(TilingMakerView::ViewMode::showTiles);
        lastChecked2 = nullptr;
    }
    else
    {

        if(lastChecked2)
            lastChecked2->setChecked(false);

        TilingMakerView::ViewMode newMode = (TilingMakerView::ViewMode)id;
        setViewState(newMode);
        lastChecked2 = clicked;
    }
}

void page_tiling_maker::setViewState(TilingMakerView::ViewMode mode)
{
    Sys::tilingMakerView->setViewMode(mode);
    currentTiling.lock()->setTilingViewChanged();
    emit sig_updateView();
}

QString colorToRgbString(const QColor & c)
{
    return QString("rgb(%1,%2,%3)")
    .arg(c.red())
        .arg(c.green())
        .arg(c.blue());
}


void page_tiling_maker::initPageStatusString()
{
    const QString s = QString(
                          "<body>"
                          "<span style=\"color:%1\">tile</span>  |  "
                          "<span style=\"color:%2\">touching</span> |  "
                          "<span style=\"color:%3\">overlap</span>  |  "
                          "<span style=\"color:%4\">included</span>  |  "
                          "<span style=\"color:%5\">excluded</span>  |  "
                          "<span style=\"color:%6\">unique</span>  |  "
                          "<span style=\"color:%7\">selected</span>  |  "
                          "<span style=\"color:%8\">hover</span>"
                          "</body>")
                          .arg(colorToRgbString(TilingMakerView::tile_color))
                          .arg(colorToRgbString(TilingMakerView::touching_color))
                          .arg(colorToRgbString(TilingMakerView::overlapping_color))
                          .arg(colorToRgbString(TilingMakerView::included_color))
                          .arg(colorToRgbString(TilingMakerView::excluded_color))
                          .arg(colorToRgbString(TilingMakerView::unique_color))
                          .arg(colorToRgbString(TilingMakerView::selected_color))
                          .arg(colorToRgbString(TilingMakerView::under_mouse_color));
    pageStatusString = s;
}

/////////////////////////////////////
///
///  Common slots
///
/////////////////////////////////////

void page_tiling_maker::onEnter()
{
    setPageStatus();
    // refresh everything
    currentTiling = tilingMaker->getSelectedTiling();
    refresh(TMR_ALL);
}

void page_tiling_maker::onExit()
{
    clearPageStatus();
}

void  page_tiling_maker::onRefresh()
{
    if (currentTiling.lock() != tilingMaker->getSelectedTiling())
    {
        currentTiling = tilingMaker->getSelectedTiling();
        refresh(TMR_ALL);
    }
    refreshMenuStatus();

    chkShowCLines->setChecked(viewControl->isEnabled(Layer::Type::ConstructionLines));

    bool notSaved = tilingMaker->constructionLinesNotSaved(tilingMaker->getSelectedTiling());
    clNotSaved->setText(notSaved ? "Not saved" : "");
}

void page_tiling_maker::slot_refreshMenu(eTileMenuRefresh scope)
{
    refresh(scope);
}

void page_tiling_maker::refresh(eTileMenuRefresh scope)
{
    //qDebug() << "scope" << scope;
    switch (scope)
    {
    case TMR_ALL_CLEAR:
        __refreshOther(true);
        __refreshTilingSelector();
        __refreshTilingHeader();
        __buildTilingTable();
        __refreshTilingTable();
        break;

    case TMR_ALL:
        __refreshOther(false);
        __refreshTilingSelector();
        __refreshTilingHeader();
        __buildTilingTable();
        __refreshTilingTable();
        break;

    case TMR_MAKER_TILINGS:
        // the loaded tilings have changed
        __refreshTilingSelector();
        __refreshTilingHeader();
        __buildTilingTable();
        __refreshTilingTable();
        break;

    case TMR_TILING:
        // the selected tiling has changed
        __refreshTilingHeader();
        __buildTilingTable();
        __refreshTilingTable();
        break;

    case TMR_TILING_HEADER:
        // the selected tiling header has changed
        __refreshTilingHeader();
        break;

    case TMR_TILING_UNIT:
        // placed tilings have been added/removed
        __buildTilingTable();
        __refreshTilingTable();
        break;

    case TMR_PLACED_TILE:
        __refreshTilingTable();
        chkTileAnchorMode->setChecked(config->tm_tileAnchorMode);
        break;
    }
}

void page_tiling_maker::__refreshOther(bool clear)
{
    if (clear)
    {
        tilingMaker->setMouseMode(TilingMaker::MouseMode::None);
        tileInfo->clear();
    }

    if (viewControl->isEnabled(Layer::Type::TilingMaker))
    {
        Sys::tilingMakerView->forceRedraw();
    }

    //fillVectorsChk->setChecked()
}

void page_tiling_maker::__refreshTilingSelector()
{
    TilingPtr selected = tilingMaker->getSelectedTiling();
    selected->resetOverlaps();

    tilingList->blockSignals(true);

    tilingList->clear();
    const QVector<TilingPtr> & tilings = tilingMaker->getTilings();
    for (auto& tiling : std::as_const(tilings))
    {
        auto item = new QListWidgetItem(tiling->getVName().get());
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        auto checkstate = (tiling->isViewable()) ? Qt::Checked : Qt::Unchecked;
        item->setCheckState(checkstate);
        tilingList->addItem(item);
    }

    auto qlist = tilingList->findItems(selected->getVName().get(), Qt::MatchFixedString);
    if (!qlist.isEmpty())
    {
        auto item = qlist.first();
        tilingList->setCurrentItem(item);
        tilingsGroup->setTitle(QString("Tilings (%1)").arg(tilingList->count()));
    }
    tilingList->blockSignals(false);
    tilingList->adjustSize();
}

void page_tiling_maker::__refreshTilingHeader()
{
    if (!panel->isVisiblePage(this))
        return;

    blockPage(true);
    blockSignals(true);

    TilingPtr tiling = tilingMaker->getSelectedTiling();

    QPointF t1 = tiling->hdr().getTrans1();
    QPointF t2 = tiling->hdr().getTrans2();

    QLineF tl1(QPointF(0.0,0.0),t1);
    QLineF tl2(QPointF(0.0,0.0),t2);

    int xMin,xMax,yMin,yMax;
    bool singleton;
    const FillData & fd = tiling->hdr().getTilingCanvas()->getFillData();
    fd.get(singleton, xMin ,xMax,yMin,yMax);

    t1x->setValue(t1.x());
    t1y->setValue(t1.y());
    t2x->setValue(t2.x());
    t2y->setValue(t2.y());

    T1len->setValue(tl1.length());
    T2len->setValue(tl2.length());
    T1angle->setValue(tl1.angle());
    T2angle->setValue(tl2.angle());

    chkSingle->setChecked(singleton);
    if (!singleton)
    {
        xRepMin->setDisabled(false);
        xRepMax->setDisabled(false);
        yRepMin->setDisabled(false);
        yRepMax->setDisabled(false);

        xRepMin->setValue(xMin);
        xRepMax->setValue(xMax);
        yRepMin->setValue(yMin);
        yRepMax->setValue(yMax);
        //repsWidget->show();
    }
    else
    {
        //repsWidget->hide();
        xRepMin->setValue(0);
        xRepMax->setValue(0);
        yRepMin->setValue(0);
        yRepMax->setValue(0);

        xRepMin->setDisabled(true);
        xRepMax->setDisabled(true);
        yRepMin->setDisabled(true);
        yRepMax->setDisabled(true);
    }

    eDistortionMode distortMode; qreal distortValX,distortValY;
    tiling->hdr().getTilingCanvas()->getDistortion().get(distortMode,distortValX,distortValY);
    distortCombo->select(distortMode);
    distortX->setValue(distortValX);
    distortY->setValue(distortValY);

    blockSignals(false);
    blockPage(false);
}

void page_tiling_maker::__refreshTilingTable()
{
    if (!panel->isVisiblePage(this))
        return;

    blockPage(true);
    blockSignals(true);

    TilingPtr tiling = tilingMaker->getSelectedTiling();

    PlacedTileSet tileSet;
    if (config->tm_excludesInTable)
        tileSet  = tiling->unit().getWithExcluded();
    else
        tileSet  = tiling->unit().getIncluded();

    int col = 0;
    int count = 0;
    for (const auto & pfp : std::as_const(tileSet))
    {
        QString inclusion = QString("%1 (%2)").arg((pfp->isIncluded()) ? "Included" : "Excluded").arg(col);

        refreshTableEntry(pfp,col++,inclusion);

        if (++count == TILE_TABLE_LIMIT)
        {
            qWarning() << "page_tiling_maker: tile table count" << tileSet.count() << "exceeds" << TILE_TABLE_LIMIT;
        }
    }

    tileInfoTable->adjustTableSize(750);

    blockSignals(false);
    blockPage(false);
}

void page_tiling_maker::refreshMenuStatus()
{
    static TilingMaker::MouseMode oldMode = TilingMaker::MouseMode::None;

    TilingMaker::MouseMode currentMode = tilingMaker->getMouseMode();
    if (currentMode != oldMode)
    {
        switch (currentMode)
        {
        case TilingMaker::MouseMode::Inclusion:
            pageStatusString = "Click polygons to toggle their inclusion in the tiling.";
            break;
        case TilingMaker::MouseMode::DrawPolygon:
            pageStatusString = "Click vertices clockwise to draw a free-form polygon.";
            break;
        case TilingMaker::MouseMode::TranslationVectors:
            pageStatusString = "L-drag polygon center/vertex to its repeat point. Twice: once per direction.";
            break;
        case TilingMaker::MouseMode::ConstructionLines:
            pageStatusString = "L-drag: new C-line. R-click: select, drag ends, L=move R=extend, Del=delete";
            break;
        default:
            initPageStatusString();
            break;
        }
        setPageStatus();

        oldMode = currentMode;
    }

    stackLabel->setText(tilingMaker->getStack().getStackStatus());

    // debug info
    if (config->insightMode && (detailsTab->floating || tabWidget->currentWidget() == detailsTab))
    {
        QString dbgStatus = tilingMaker->getStatus();
        debugLabel1->setText(dbgStatus);

        QPointF a = Sys::tilingMakerView->getMousePos();
        QPointF b = Sys::tilingMakerView->screenToModel(a);
        QString astring;
        QTextStream ts(&astring);
        ts.setRealNumberPrecision(8);
        ts << "pos: (" << b.x() << ", " << b.y() << ") ";

        PlacedTileSelectorPtr selector = Sys::tilingMakerView->viewPlacedTileSelector();
        if (selector)
        {
            QPointF c = selector->getPlacedPoint();
            ts << selector->getTypeString()  << "(" << c.x() << ", " << c.y() << ")";
        }
        debugLabel2->setText(astring);

        astring.clear();
        if (selector)
        {
            auto type = selector->getType();
            switch(type)
            {
            case INTERIOR:
                astring = getTileInfo(selector->getPlacedTile());
                break;
            case EDGE:
                astring = selector->getPlacedEdge()->info();
                break;

            case ARC_POINT:
            case TILE_CENTER:
            case VERTEX:
            case MID_POINT:
            case SCREEN_POINT:
                break;
            }
        }
        tileInfo->setText(astring);

        smxwidget1->refresh();
        smxwidget2->refresh();
        smxwidgetCL->refresh();
    }

    // always visible status line
    TilingPtr tp = tilingMaker->getSelectedTiling();
    QString str;
    if (tp->hasIntrinsicOverlaps())
    {
        str = "Intrinsic overlaps";
    }
    else if (tp->hasTiledOverlaps())
    {
        str = "Tiled overlaps";
    }
    overlapStatus->setText(str);

    QString status = QString("Tiles: %1  Unique: %2 Included: %3 Excluded: %4").arg(tp->unit().numAll()).arg(tp->unit().numUnique()).arg(tp->unit().numIncluded()).arg(tp->unit().numExcluded());
    statusLabel->setText(status);

    tallyMouseMode();
    tallyKbdMode();

    fillBtn->setChecked(Sys::tm_fill);

    PlacedTilePtr selected = tilingMaker->selectedTile();
    int col = getColumn(selected);
    if (col == -1)
        tileInfoTable->clearSelection();
    else
        tileInfoTable->selectColumn(col); // tally info table

    if (!chkPropagate->isChecked())
        pbRender->setStyleSheet("QPushButton { background-color: yellow; color: red;}");
    else
        pbRender->setStyleSheet("");

    if (Sys::backgroundImage->isLoaded())
    {
        chkBkgd->setChecked(true);
        pbExam->setText("Examine Background Image");
    }
    else
    {
        chkBkgd->setChecked(false);
        pbExam->setText("Add Background Image");
    }

    sides->setValue(Sys::config->tm_poly_side_count);
    tileRot->setValue(Sys::config->tm_poly_rotation);
}

bool page_tiling_maker::canExit()
{
    QString txt;
    QString info;
    TilingPtr tp = tilingMaker->getSelectedTiling();
    if (tp->unit().numAll())
    {
        if (tp->unit().numIncluded() == 0)
        {
            txt  = "There are no polygons included in the tiling";
            info = "To include polygons, press Cancel and 'Include' the polygons you want in the tiling";
        }
        else
        {
            const FillData & fd = tp->hdr().getTilingCanvas()->getFillData();
            int minX, maxX, minY, maxY;
            bool singleton;
            fd.get(singleton,minX, maxX, minY, maxY);

            if (!singleton)
            {
                QPointF t1    = tp->hdr().getTrans1();
                QPointF t2    = tp->hdr().getTrans2();

                if (t1.isNull() || t2.isNull())
                {
                    txt  = "There are no Repeat Points for the tiling                                                   ";
                    info = "To set the Repeat Points, press Cancel\n then press F3 to enter 'Set Repeat Points' mode.\nOr press 'Make Singleton' to make a non-repeating tiling";
                }
            }
        }
    }

    if (!txt.isEmpty())
    {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(txt);
        msgBox.setInformativeText(info);
        QAbstractButton * singleton = msgBox.addButton(tr("Make Singleton"),QMessageBox::YesRole);
        QPushButton * save   = msgBox.addButton(QMessageBox::Ignore);
        QPushButton * cancel = msgBox.addButton(QMessageBox::Cancel);
        msgBox.setDefaultButton(cancel);
        msgBox.exec();
        if (msgBox.clickedButton() == cancel)
        {
            return false;
        }
        else if (msgBox.clickedButton() == singleton)
        {
            singleton_changed(true);
            return true;
        }
        else
        {
            Q_UNUSED(save);
            return true;
        }
    }
    return true;
}

////////////////////////////////////////////////
///
///  Protected Slots
///
////////////////////////////////////////////////

void page_tiling_maker::slot_clearTiling()
{
    // clears current tiling
    Sys::imageViewer->unloadLayerContent();

    TilingPtr tp = tilingMaker->getSelectedTiling();
    tilingMaker->removeTiling(tp);  // re-creates if becomes empty
    emit sig_reconstructView();

    refresh(TMR_MAKER_TILINGS);
}

void page_tiling_maker::slot_reloadTiling()
{
    TilingPtr oldTiling  = tilingMaker->getSelectedTiling();
    VersionedName vname  = oldTiling->getVName();
    VersionedFile vfile  = FileServices::getFile(vname,FILE_TILING);

    tilingMaker->loadTiling(vfile,TilingMaker::Event::Reload);

    refresh(TMR_MAKER_TILINGS);
}

void page_tiling_maker::slot_duplicateTiling()
{
    tilingMaker->duplicateSelectedTiling();
    refresh(TMR_MAKER_TILINGS);

    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    box.setText("Duplicate added");
    box.exec();
}

/////////////////////////////////////
///
///  Build Menu
///
/////////////////////////////////////


void page_tiling_maker::__buildTilingTable()
{
    blockPage(true);

    tileInfoTable->clearContents();
    tileInfoTable->setColumnCount(0);

    TilingPtr tp = tilingMaker->getSelectedTiling();
    Q_ASSERT(tp);

    bool showAll = (config->tm_excludesInTable && (tp->unit().numAll() < TILE_TABLE_LIMIT));   // throttle show all for large number pf tiles

    PlacedTileSet viewable;
    if (showAll)
        viewable = tp->unit().getWithExcluded();
    else
        viewable = tp->unit().getIncluded();

    int col = 0;
    for (const auto & pfp : std::as_const(viewable))
    {
        QString inclusion = QString("%1 (%2)").arg(pfp->isIncluded() ? "Included" : "Excluded").arg(col);
        buildTableEntry(pfp,col++,inclusion);
    }

    for (int i=0; i < tileInfoTable->columnCount(); i++)
    {
        tileInfoTable->resizeColumnToContents(i);
    }

    tileInfoTable->adjustTableSize(750);

    updateGeometry();

    blockPage(false);
}

void page_tiling_maker::buildTableEntry(PlacedTilePtr pf, int col, QString inclusion)
{
    TilePtr tile  = pf->getTile();
    QTransform T  = pf->getPlacement();

    tileInfoTable->setColumnCount(col+1);

    QString type;
    if (tile->isRegular())
        type = "Regular";
    else
        type = "Irregular";
    QTableWidgetItem * twi = new QTableWidgetItem(type);
    twi->setData(Qt::UserRole,QVariant::fromValue(WeakPlacedTilePtr(pf)));
    tileInfoTable->setItem(TI_TYPE_PFP,col,twi);

    QCheckBox * cb = new QCheckBox("Show");
    cb->setStyleSheet("padding-left: 10px;");
    tileInfoTable->setCellWidget(TI_SHOW,col,cb);
    connect(cb, &QCheckBox::clicked, this, [this,col] { slot_showTileChanged(col); });

    BQSpinBox * sp = new BQSpinBox(this,pf);
    sp->setAlignment(Qt::AlignCenter);
    sp->setValue(tile->numPoints());
    sp->setReadOnly(!tile->isRegular());
    tileInfoTable->setCellWidget(TI_TILE_SIDES,col,sp);
    connect(sp,static_cast<void (BQSpinBox::*)(int)>(&BQSpinBox::valueChanged), this,
            [this,col] { slot_sidesChanged(col); });

    BQDoubleSpinBox * dsp = new BQDoubleSpinBox(this,pf);
    dsp->setRange(-360.0,360.0);
    dsp->setDisplayDecimals(16);
    dsp->setAlignment(Qt::AlignRight);
    dsp->setValue(tile->getRotation());
    tileInfoTable->setCellWidget(TI_TILE_ROT,col,dsp);
    connect(dsp, static_cast<void (BQDoubleSpinBox::*)(double)>(&BQDoubleSpinBox::valueChanged), this,
            [this,col] { slot_tileRotChanged(col); });

    dsp = new BQDoubleSpinBox(this,pf);
    dsp->setRange(0,128);
    dsp->setDisplayDecimals(16);
    dsp->setSingleStep(0.01);
    dsp->setAlignment(Qt::AlignRight);
    dsp->setValue(tile->getScale());
    tileInfoTable->setCellWidget(TI_TILE_SCALE,col,dsp);
    connect(dsp, static_cast<void (BQDoubleSpinBox::*)(double)>(&BQDoubleSpinBox::valueChanged), this,
            [this,col] { slot_tileScaleChanged(col); });

    dsp = new BQDoubleSpinBox(this,pf);
    dsp->setRange(0,128);
    dsp->setDisplayDecimals(16);
    dsp->setSingleStep(0.01);
    dsp->setAlignment(Qt::AlignRight);
    dsp->setValue(Transform::scalex(T));
    tileInfoTable->setCellWidget(TI_PLACEMENT_SCALE,col,dsp);
    connect(dsp, static_cast<void (BQDoubleSpinBox::*)(double)>(&BQDoubleSpinBox::valueChanged), this,
            [this,col] { slot_placedScaleChanged(col); });

    dsp = new BQDoubleSpinBox(this,pf);
    dsp->setRange(-360.0,360.0);
    dsp->setDisplayDecimals(16);
    dsp->setAlignment(Qt::AlignRight);
    dsp->setValue(qRadiansToDegrees(Transform::rotation(T)));
    tileInfoTable->setCellWidget(TI_PLACEMENT_ROT,col,dsp);
    connect(dsp, static_cast<void (BQDoubleSpinBox::*)(double)>(&BQDoubleSpinBox::valueChanged), this,
            [this,col] { slot_placedRotateChanged(col); });

    dsp = new BQDoubleSpinBox(this,pf);
    dsp->setRange(-100.0,100.0);
    dsp->setDisplayDecimals(16);
    dsp->setSingleStep(0.1);
    dsp->setAlignment(Qt::AlignRight);
    dsp->setValue(Transform::transx(T));
    tileInfoTable->setCellWidget(TI_PLACEMENT_X,col,dsp);
    connect(dsp,static_cast<void (BQDoubleSpinBox::*)(double)>(&BQDoubleSpinBox::valueChanged), this,
            [this,col] { slot_placedTranslateChanged(col); });

    dsp = new BQDoubleSpinBox(this,pf);
    dsp->setRange(-100.0,100.0);
    dsp->setDisplayDecimals(16);
    dsp->setSingleStep(0.1);
    dsp->setAlignment(Qt::AlignRight);
    dsp->setValue(Transform::transy(T));
    tileInfoTable->setCellWidget(TI_PLACEMENT_Y,col,dsp);
    connect(dsp, static_cast<void (BQDoubleSpinBox::*)(double)>(&BQDoubleSpinBox::valueChanged), this,
            [this,col] { slot_placedTranslateChanged(col); });

    QString cw = tile->isClockwise() ? "Clockwise" : "Anticlockwise";
    twi = new QTableWidgetItem(cw);
    tileInfoTable->setItem(TI_INFO_CW,col,twi);

    twi = new QTableWidgetItem(addr(tile.get()));
    tileInfoTable->setItem(TI_TILE_ADDR,col,twi);

    twi = new QTableWidgetItem(inclusion);
    tileInfoTable->setItem(TI_LOCATION,col,twi);
}

void page_tiling_maker::refreshTableEntry(PlacedTilePtr pf, int col, QString inclusion)
{
    TilePtr tile  = pf->getTile();
    QTransform T  = pf->getPlacement();

    QWidget * w = tileInfoTable->cellWidget(TI_TILE_SIDES,col);
    if (!w)
    {
        qDebug() << "table col not found";
        return;
    }
    BQSpinBox * sp = dynamic_cast<BQSpinBox*>(w);
    Q_ASSERT(sp);
    sp->setValue(tile->numPoints());

    QString type =  (tile->isRegular()) ?  "Regular" : "Irregular";
    QTableWidgetItem * twi = tileInfoTable->item(TI_TYPE_PFP,col);
    twi->setData(Qt::UserRole,QVariant::fromValue(WeakPlacedTilePtr(pf)));
    twi->setText(type);

    w = tileInfoTable->cellWidget(TI_SHOW,col);
    QCheckBox * cb = dynamic_cast<QCheckBox*>(w);
    cb->blockSignals(true);
    cb->setChecked(pf->show());
    cb->blockSignals(false);

    w = tileInfoTable->cellWidget(TI_TILE_ROT,col);
    BQDoubleSpinBox * dsp = dynamic_cast<BQDoubleSpinBox*>(w);
    Q_ASSERT(dsp);
    dsp->setValue(tile->getRotation());

    w = tileInfoTable->cellWidget(TI_TILE_SCALE,col);
    dsp = dynamic_cast<BQDoubleSpinBox*>(w);
    Q_ASSERT(dsp);
    dsp->setValue(tile->getScale());

    w = tileInfoTable->cellWidget(TI_PLACEMENT_SCALE,col);
    dsp = dynamic_cast<BQDoubleSpinBox*>(w);
    Q_ASSERT(dsp);
    dsp->setValue(Transform::scalex(T));

    w = tileInfoTable->cellWidget(TI_PLACEMENT_ROT,col);
    dsp = dynamic_cast<BQDoubleSpinBox*>(w);
    Q_ASSERT(dsp);
    dsp->setValue(qRadiansToDegrees(Transform::rotation(T)));

    w = tileInfoTable->cellWidget(TI_PLACEMENT_X,col);
    dsp = dynamic_cast<BQDoubleSpinBox*>(w);
    Q_ASSERT(dsp);
    dsp->setValue(Transform::transx(T));

    w = tileInfoTable->cellWidget(TI_PLACEMENT_Y,col);
    dsp = dynamic_cast<BQDoubleSpinBox*>(w);
    Q_ASSERT(dsp);
    dsp->setValue(Transform::transy(T));

    QString cw = tile->isClockwise() ? "Clockwise" : "Anticlockwise";
    twi = tileInfoTable->item(TI_INFO_CW,col);
    twi->setText(cw);

    twi = tileInfoTable->item(TI_TILE_ADDR,col);
    twi->setText(addr(tile.get()));

    twi = tileInfoTable->item(TI_LOCATION,col);
    twi->setText(inclusion);
}

PlacedTilePtr page_tiling_maker::getTileColumn(int col)
{
    PlacedTilePtr pf;
    if (col == -1) return pf;

    QTableWidgetItem * twi = tileInfoTable->item(TI_TYPE_PFP,col);
    if (twi)
    {
        QVariant var = twi->data(Qt::UserRole);
        if (var.canConvert<WeakPlacedTilePtr>())
        {
            WeakPlacedTilePtr wkpf = var.value<WeakPlacedTilePtr>();
            pf = wkpf.lock();
        }
    }
    return pf;
}


int page_tiling_maker::getColumn(PlacedTilePtr pfp)
{
    for (int i=0; i < tileInfoTable->columnCount(); i++)
    {
        QTableWidgetItem * twi = tileInfoTable->item(TI_TYPE_PFP,i);
        Q_ASSERT(twi);
        QVariant var = twi->data(Qt::UserRole);
        if (var.canConvert<WeakPlacedTilePtr>())
        {
            WeakPlacedTilePtr wkpf = var.value<WeakPlacedTilePtr>();
            if (pfp == wkpf.lock())
            {
                return i;
            }
        }
    }
    return -1;
}

/////////////////////////////////////
///
///  Private Slots
///
/////////////////////////////////////

void page_tiling_maker::slot_sidesChanged(int col)
{
    if (pageBlocked()) return;

    QWidget * cw  = tileInfoTable->cellWidget(TI_TILE_SIDES,col);
    BQSpinBox * sp = dynamic_cast<BQSpinBox*>(cw);
    Q_ASSERT(sp);
    int sides = sp->value();

    PlacedTilePtr pf = getTileColumn(col);
    tilingMaker->selectTile(pf);

    TilePtr tile     = pf->getTile();
    tile->setN(sides);
    tilingMaker->getSelectedTiling()->setTilingViewChanged();

    if (tilingMaker->getPropagate() && pf->isIncluded())
    {
        MotifEvent motifEvent;
        motifEvent.event  = MotifMaker::Event::UniqueTileNatureChanged;
        motifEvent.tiling = tilingMaker->getSelectedTiling();
        motifEvent.tile   = tile;
        motifMaker->sm_takeUp(motifEvent);
    }

    //refresh(TMR_PLACED_TILE);

    emit sig_reconstructView();
}

void page_tiling_maker::slot_tileRotChanged(int col)
{
    if (pageBlocked()) return;

    QWidget * cw  = tileInfoTable->cellWidget(TI_TILE_ROT,col);
    BQDoubleSpinBox * sp = dynamic_cast<BQDoubleSpinBox*>(cw);
    Q_ASSERT(sp);
    qreal rotation = sp->value();

    PlacedTilePtr pf = getTileColumn(col);
    tilingMaker->selectTile(pf);

    TilePtr tile  = pf->getTile();
    tile->setRotate(rotation);
    tilingMaker->getSelectedTiling()->setTilingViewChanged();

    if (tilingMaker->getPropagate() && pf->isIncluded())
    {
        MotifEvent motifEvent;
        motifEvent.event  = MotifMaker::Event::UniqueTileTransformChanged;
        motifEvent.tiling = tilingMaker->getSelectedTiling();
        motifEvent.tile   = tile;
        motifMaker->sm_takeUp(motifEvent);
    }

    if (config->tm_tileAnchorMode)
        tilingMaker->propagateAnchors(tilingMaker->selectedTile());

    //refresh(TMR_PLACED_TILE);

    emit sig_reconstructView();
}

void page_tiling_maker::slot_tileScaleChanged(int col)
{
    if (pageBlocked()) return;

    QWidget * cw  = tileInfoTable->cellWidget(TI_TILE_SCALE,col);
    BQDoubleSpinBox * sp = dynamic_cast<BQDoubleSpinBox*>(cw);
    Q_ASSERT(sp);
    qreal scale = sp->value();

    PlacedTilePtr pf = getTileColumn(col);
    tilingMaker->selectTile(pf);

    TilePtr tile  = pf->getTile();
    tile->setScale(scale);
    tilingMaker->getSelectedTiling()->setTilingViewChanged();

    if (tilingMaker->getPropagate() && pf->isIncluded())
    {
        MotifEvent motifEvent;
        motifEvent.event  = MotifMaker::Event::UniqueTileTransformChanged;
        motifEvent.tiling = tilingMaker->getSelectedTiling();
        motifEvent.tile   = tile;
        motifMaker->sm_takeUp(motifEvent);
    }

    if (config->tm_tileAnchorMode)
        tilingMaker->propagateAnchors(tilingMaker->selectedTile());

    //refresh(TMR_PLACED_TILE);

    emit sig_reconstructView();
}

void page_tiling_maker::slot_placedTranslateChanged(int col)
{
    qWarning() << "slot_placedTranslateChanged";
    if (pageBlocked()) return;

    PlacedTilePtr placedTile = getTileColumn(col);
    tilingMaker->selectTile(placedTile);

    auto widget = tileInfoTable->cellWidget(TI_PLACEMENT_X,col);
    auto dsp    = dynamic_cast<BQDoubleSpinBox*>(widget);
    Q_ASSERT(dsp);
    qreal tx = dsp->value();

    widget = tileInfoTable->cellWidget(TI_PLACEMENT_Y,col);
    dsp    = dynamic_cast<BQDoubleSpinBox*>(widget);
    Q_ASSERT(dsp);
    qreal ty = dsp->value();

    tilingMaker->placedTileSetTranslate(tx,ty);

    if (tilingMaker->getPropagate() && placedTile->isIncluded())
    {        
        MotifEvent motifEvent;
        motifEvent.event = MotifMaker::Event::TilingChanged;
        motifEvent.tiling = tilingMaker->getSelectedTiling();
        motifMaker->sm_takeUp(motifEvent);
    }

    //refresh(TMR_PLACED_TILE);

    emit sig_reconstructView();
}

void page_tiling_maker::slot_placedScaleChanged(int col)
{
    qWarning() << "slot_placedScaleChanged col=" << col;
    if (pageBlocked()) return;

    PlacedTilePtr placedTile = getTileColumn(col);
    tilingMaker->selectTile(placedTile);

    QWidget  * cw  = tileInfoTable->cellWidget(TI_PLACEMENT_SCALE,col);
    BQDoubleSpinBox * dsp = dynamic_cast<BQDoubleSpinBox*>(cw);
    Q_ASSERT(dsp);
    qreal scale = dsp->value();
    if (scale <= 0.0 || scale > 128.0)
    {
        return;     // fixes problem with scales of 0 when entering data
    }

    tilingMaker->placedTileSetScale(scale);

    if (tilingMaker->getPropagate() && placedTile->isIncluded())
    {
        MotifEvent motifEvent;
        motifEvent.event = MotifMaker::Event::TilingChanged;
        motifEvent.tiling = tilingMaker->getSelectedTiling();
        motifMaker->sm_takeUp(motifEvent);
    }

    //refresh(TMR_PLACED_TILE);

    emit sig_updateView();
}

void page_tiling_maker::slot_placedRotateChanged(int col)
{
    qDebug() << "slot_placedRotateChanged";
    if (pageBlocked()) return;

    PlacedTilePtr placedTile = getTileColumn(col);
    tilingMaker->selectTile(placedTile);

    auto rot_widget = tileInfoTable->cellWidget(TI_PLACEMENT_ROT,col);
    auto rot_dsp = dynamic_cast<BQDoubleSpinBox*>(rot_widget);
    Q_ASSERT(rot_dsp);
    qreal cell_rotation = rot_dsp->value();

    tilingMaker->placedTileSetRotate(cell_rotation);

    if (tilingMaker->getPropagate() && placedTile->isIncluded())
    {
        MotifEvent motifEvent;
        motifEvent.event = MotifMaker::Event::TilingChanged;
        motifEvent.tiling = tilingMaker->getSelectedTiling();
        motifMaker->sm_takeUp(motifEvent);
    }

    //refresh(TMR_PLACED_TILE);

    emit sig_updateView();
}

void page_tiling_maker::slot_showTileChanged(int col)
{
    QWidget   * cw = tileInfoTable->cellWidget(TI_SHOW,col);
    QCheckBox * cb = dynamic_cast<QCheckBox*>(cw);
    Q_ASSERT(cb);
    bool checked = cb->isChecked();
    qDebug() << "col=" << col << "checked:" << checked;

    PlacedTilePtr tile = getTileColumn(col);
    tilingMaker->selectTile(tile);

    tile->setShow(checked);

    //refresh(TMR_PLACED_TILE);

    Sys::tilingMakerView->forceRedraw();
}

void page_tiling_maker::slot_set_reps(int val)
{
    Q_UNUSED(val);

    if (pageBlocked()) return;

    TilingPtr tiling = tilingMaker->getSelectedTiling();
    if (!tiling) return;

    FillData fdata;
    fdata.set(chkSingle->isChecked(),xRepMin->value(), xRepMax->value(), yRepMin->value(), yRepMax->value());

    Canvas * cs = tiling->hdr().getTilingCanvas();
    cs->setFillData(fdata);

    tilingMaker->updateReps();
    tiling->setTilingViewChanged();

    refresh(TMR_TILING_HEADER);

    Sys::tilingMakerView->forceRedraw();
}

void page_tiling_maker::singleton_changed(bool checked)
{
    TilingPtr tiling = tilingMaker->getSelectedTiling();
    if (!tiling) return;

    FillData fd;
    if (!checked)
        fd.set(false,-3, 3, -3, 3);
    else
        fd.set(true,0,0,0,0);

    Canvas * cs = tiling->hdr().getTilingCanvas();
    cs->setFillData(fd);

    // a singleton tiling has no repeat by definition - a real translation vector doesn't belong
    if (checked && (!tiling->hdr().getTrans1().isNull() || !tiling->hdr().getTrans2().isNull()))
        tilingMaker->clearTranslationVectors();

    tilingMaker->updateReps();
    tiling->setTilingViewChanged();

    Sys::tilingMakerView->forceRedraw();

    refresh(TMR_TILING_HEADER);
}

void page_tiling_maker::slot_setDistortion()
{
    TilingPtr tiling = tilingMaker->getSelectedTiling();
    if (!tiling) return;

    Distortion d;
    d.set(static_cast<eDistortionMode>(distortCombo->currentData().toInt()),distortX->value(),distortY->value());

    // always the tiling's own canvas directly - same as the Canvas Settings page's
    // Tiling Canvas Settings box
    Canvas * cs = tiling->hdr().getTilingCanvas();
    cs->setDistortion(d);

    // when the mosaic canvas is the one actually being rendered, it's the only
    // canvas any layer draws from, so mirror the edit there too or nothing visibly
    // happens
    if (Sys::getActiveCanvasOwner() == ACTIVE_MOSAIC)
    {
        Sys::mosaicMaker->getMosaic()->getMosaicCanvas()->setDistortion(d);
    }

    // touching/overlap state is cached on the draw set and only recomputed on a tiling
    // edit - it's classified on distorted geometry, so a distortion change must invalidate
    // that cache too or the highlight colors go stale
    tiling->setTilingViewChanged();

    Sys::tilingMakerView->forceLayerRecalc(true);

    // distortion now bakes into the prototype map (Prototype::_buildPrototypeMap), so it
    // must reset prototypes/styles too, not just repaint with the old cached geometry
    Sys::render(RENDER_RESET_PROTOTYPES);
}

void page_tiling_maker::slot_loadConstructionLines()
{
    tilingMaker->loadConstructionLines();
    chkShowCLines->setChecked(true);
    panel->userDelegateView(Layer::Type::ConstructionLines,true);
    emit sig_reconstructView();
}

void page_tiling_maker::slot_hideVectors(bool checked)
{
    Sys::tilingMakerView->hideVectors(checked);
}

void page_tiling_maker::slot_t1t2Changed(double val)
{
    Q_UNUSED(val)

    if (pageBlocked()) return;

    qreal x1 = t1x->value();
    qreal y1 = t1y->value();
    qreal x2 = t2x->value();
    qreal y2 = t2y->value();

    TilingPtr tiling  = tilingMaker->getSelectedTiling();
    tiling->hdr().setTranslationVectors(QPointF(x1,y1), QPointF(x2,y2), tiling->hdr().getTranslateOrigin());
    tilingMaker->updateVectors();
    tiling->setTilingViewChanged();

    refresh(TMR_TILING_HEADER);
}

void page_tiling_maker::slot_t1t2LenChanged(double val)
{
    Q_UNUSED(val)

    if (pageBlocked()) return;

    TilingPtr tiling = tilingMaker->getSelectedTiling();

    QPointF t1 = tiling->hdr().getTrans1();
    QPointF t2 = tiling->hdr().getTrans2();

    QLineF tl1(QPointF(0.0,0.0),t1);
    QLineF tl2(QPointF(0.0,0.0),t2);

    qreal len1 = T1len->value();
    qreal len2 = T2len->value();

    tl1.setLength(len1);
    tl2.setLength(len2);

    qreal x1 = tl1.p2().x();
    qreal y1 = tl1.p2().y();
    qreal x2 = tl2.p2().x();
    qreal y2 = tl2.p2().y();

    tiling->hdr().setTranslationVectors(QPointF(x1,y1),QPointF(x2,y2),tiling->hdr().getTranslateOrigin());
    tilingMaker->updateVectors();
    tiling->setTilingViewChanged();

    refresh(TMR_TILING_HEADER);
}

void page_tiling_maker::slot_t1t2AngleChanged(double val)
{
    Q_UNUSED(val)

    if (pageBlocked()) return;

    TilingPtr tiling = tilingMaker->getSelectedTiling();

    QPointF t1 = tiling->hdr().getTrans1();
    QPointF t2 = tiling->hdr().getTrans2();

    QLineF tl1(QPointF(0.0,0.0),t1);
    QLineF tl2(QPointF(0.0,0.0),t2);

    qreal angle1 = T1angle->value();
    qreal angle2 = T2angle->value();

    tl1.setAngle(angle1);
    tl2.setAngle(angle2);

    qreal x1 = tl1.p2().x();
    qreal y1 = tl1.p2().y();
    qreal x2 = tl2.p2().x();
    qreal y2 = tl2.p2().y();

    tiling->hdr().setTranslationVectors(QPointF(x1,y1),QPointF(x2,y2),tiling->hdr().getTranslateOrigin());
    tilingMaker->updateVectors();
    tiling->setTilingViewChanged();

    refresh(TMR_TILING_HEADER);
}

void page_tiling_maker::slot_swapTrans()
{
    qreal x1 = t1x->value();
    qreal y1 = t1y->value();
    qreal x2 = t2x->value();
    qreal y2 = t2y->value();

    t1x->setValue(x2);
    t1y->setValue(y2);
    t2x->setValue(x1);
    t2y->setValue(y1);

    slot_t1t2Changed(0);
}

// tilingList
void page_tiling_maker::slot_currentItemChanged(QListWidgetItem * current, QListWidgetItem * prev)
{
    Q_UNUSED(prev);

    QString name = current->text();
    qDebug() << "page_tiling_maker::slot_currentItemChanged" << name;

    TilingPtr tp = tilingMaker->findTilingByName(name);

    tilingMaker->selectTiling(tp);

    // this has already fully handled the selection change - record it so the next poll
    // tick's onRefresh() doesn't see it as a fresh change and redundantly redo this same
    // rebuild (more heavily - TMR_ALL - than what we just did here)
    currentTiling = tp;

    refresh(TMR_TILING);
}

void page_tiling_maker::slot_itemChanged(QListWidgetItem * item)
{
    QString name = item->text();
    qDebug() << "page_tiling_maker::slot_itemChanged" << name;

    auto checkstate = item->checkState();
    TilingPtr tp    = tilingMaker->findTilingByName(name);

    if (tilingMaker->getTilings().size() < 2)
    {
        if (checkstate != Qt::Checked)
        {
            checkstate = Qt::Checked;
            item->setCheckState(checkstate);
            if (!tp->isViewable())
            {
                tp->setTilingViewChanged();
            }
        }
    }
    else
    {
        if (checkstate == Qt::Checked)
        {
            tp->setTilingViewChanged();
        }
        else
        {
            // don't allow the last visible tiling to be unchecked
            int numChecked = 0;
            for (int i = 0; i < tilingList->count(); i++)
            {
                if (tilingList->item(i)->checkState() == Qt::Checked)
                    numChecked++;
            }
            if (numChecked == 0)
            {
                checkstate = Qt::Checked;
                item->setCheckState(checkstate);
                if (!tp->isViewable())
                {
                    tp->setTilingViewChanged();
                }
            }
        }
    }

    tp->setViewable(checkstate);

    refresh(TMR_TILING);

    emit sig_updateView();
}

void page_tiling_maker::slot_setModes(QAbstractButton * btn)
{
    int mode = mouseModeBtnGroup->id(btn);
    TilingMaker::MouseMode mm = static_cast<TilingMaker::MouseMode>(mode);
    qDebug() << sTilingMakerMouseMode[(int)mm] << btn->isChecked();

    if (mm == TilingMaker::MouseMode::ConstructionLines)
        enterConstructionLinesMode(btn->isChecked());
    else if (btn->isChecked())
        tilingMaker->setMouseMode(mm);
    else
        tilingMaker->setMouseMode(TilingMaker::MouseMode::None);

    tallyMouseMode();
}

void page_tiling_maker::slot_setConstructionLinesMode(bool checked)
{
    enterConstructionLinesMode(checked);
    tallyMouseMode();
}

void page_tiling_maker::enterConstructionLinesMode(bool checked)
{
    if (checked)
    {
        tilingMaker->setMouseMode(TilingMaker::MouseMode::ConstructionLines);

        chkShowCLines->setChecked(true);
        panel->userDelegateView(Layer::Type::ConstructionLines,true);

        if (Sys::tilingMakerView->getConstructionLines().isEmpty())
            tilingMaker->loadConstructionLines();

        emit sig_reconstructView();
    }
    else
    {
        tilingMaker->setMouseMode(TilingMaker::MouseMode::None);
    }
}

void page_tiling_maker::slot_setTranslationsMode(bool checked)
{
    if (checked)
        tilingMaker->setMouseMode(TilingMaker::MouseMode::TranslationVectors);
    else
        tilingMaker->setMouseMode(TilingMaker::MouseMode::None);

    tallyMouseMode();
}

void page_tiling_maker::tallyMouseMode()
{
    // dont need to block signals because connect is based on 'clicked'
    TilingMaker::MouseMode mmode = tilingMaker->getMouseMode();
    QAbstractButton * current   = mouseModeBtnGroup->button((int)mmode);

    if (current != lastChecked)
    {
        lastChecked->setChecked(false);
        current->setChecked(true);
        lastChecked = current;
    }

    // the Repeat/C-Lines tabs' duplicate buttons aren't in mouseModeBtnGroup, so tally them separately
    drawTransRepeat->setChecked(mmode == TilingMaker::MouseMode::TranslationVectors);
    drawConstRepeat->setChecked(mmode == TilingMaker::MouseMode::ConstructionLines);
}

void page_tiling_maker::tallyKbdMode()
{
    TilingMaker::KeybdMode mode = tilingMaker->getKeybdMode();

    QAbstractButton * button = kbdBtnGroup1->button(mode);
    button->setChecked(true);

    button = kbdBtnGroup2->button(mode);
    button->setChecked(true);
}

void page_tiling_maker::slot_showExludesInTable(bool checked)
{
    config->tm_excludesInTable = checked;

    refresh(TMR_TILING_UNIT);
}


void page_tiling_maker::slot_fillOnLoad(bool checked)
{
    config->tm_fillOnLoad = checked;
}

// called by clicking on tile info table column
void page_tiling_maker::slot_infoTableMenu(QPointF spt)
{
    int col = tileInfoTable->currentColumn();
    PlacedTilePtr pfp = getTileColumn(col);

    QString astr = QString("Side=%1").arg(pfp->getTile()->edgeLen(),0,'g',16);
    qDebug().noquote() << astr;

    QString bstr =  (pfp->isIncluded())  ? "Exclude" : "Include";

    QMenu myMenu(tileInfoTable);

    myMenu.addAction("De-Select", tilingMaker, &TilingMaker::slot_deselectTile);
    myMenu.addAction("Edit Tile", this, &page_tiling_maker::slot_table_menu_edit_tile);
    myMenu.addAction(bstr, this, &page_tiling_maker::slot_menu_includePlaced);
    if (pfp->getTile()->isRegular())
    {
        myMenu.addAction("Make Irregular", this, &page_tiling_maker::slot_convert_tile_regularity);
    }
    else
    {
        myMenu.addAction("Make Regular", this, &page_tiling_maker::slot_convert_tile_regularity);
    }
    myMenu.addAction("Delete Tile", this, &page_tiling_maker::slot_delete_clicked);
    myMenu.addAction("Uniquify Tile", this, &page_tiling_maker::slot_uniquify_clicked);

    myMenu.exec(tileInfoTable->viewport()->mapToGlobal(spt.toPoint()));
}

void page_tiling_maker::slot_infoTableHeaderClicked(int index)
{
    qDebug() << "index=" << index;
    if (index == TI_PLACEMENT_X || index == TI_PLACEMENT_Y)
    {
        DlgTrim * trim = new DlgTrim(this);
        connect (trim, &DlgTrim::sig_apply, this, &page_tiling_maker::slot_trim);
        trim->setModal(false);
        trim->show();
    }
}

void page_tiling_maker::slot_infoTableCellSelected(int row, int col)
{
    Q_UNUSED(row);
    PlacedTilePtr pfp = getTileColumn(col);
    if (row == TI_SHOW)
        tilingMaker->selectTile(pfp,false);
    else
        tilingMaker->selectTile(pfp,true);  // toggles
    Sys::tilingMakerView->forceRedraw();
}

void page_tiling_maker::slot_table_menu_edit_tile()
{
    // called by click on table column menu
    qDebug() << "page_tiling_maker::slot_menu_edit_tile";
    int           col = tileInfoTable->currentColumn();
    PlacedTilePtr pfp = getTileColumn(col);

    tilingMaker->setMouseMode(TilingMaker::MouseMode::Edit);
    tilingMaker->editTile(pfp);
}

void page_tiling_maker::slot_menu_includePlaced()
{
    int col = tileInfoTable->currentColumn();
    PlacedTilePtr pfp = getTileColumn(col);
    
    PlacedTileSelectorPtr tsp = make_shared<InteriorTilleSelector>(pfp);
    tilingMaker->toggleInclusion(tsp);
    refresh(TMR_TILING_UNIT);
}

void page_tiling_maker::slot_delete_clicked()
{
    int col = tileInfoTable->currentColumn();
    PlacedTilePtr pf = getTileColumn(col);
    tilingMaker->deletePlacedTile(pf);
    refresh(TMR_TILING_UNIT);
}

void page_tiling_maker::slot_convert_tile_regularity()
{
    int col          = tileInfoTable->currentColumn();
    PlacedTilePtr pf = getTileColumn(col);
    TilePtr tile     = pf->getTile();

    tilingMaker->flipTileRegularity(tile);
    refresh(TMR_PLACED_TILE);
}

void page_tiling_maker::slot_uniquify_clicked()
{
    int col = tileInfoTable->currentColumn();
    PlacedTilePtr pf = getTileColumn(col);
    tilingMaker->uniquifyTile(pf);
    refresh(TMR_TILING_UNIT);
}

QString page_tiling_maker::getTileInfo(PlacedTilePtr pfp)
{
    TilePtr          fp  = pfp->getTile();
    QPolygonF      poly  = fp->getPoints();
    QTransform        t  = pfp->getPlacement();
    QString astring;
    QTextStream str(&astring);
    str << "Tile info:" << Utils::addr(pfp.get()) << Transform::info(t) << "\n";
    for (int i=0; i < poly.size(); i++)
    {
        QPointF pt = poly.at(i);
        str << "[" << pt.x() << "," << pt.y() << "]";
    }
    return astring;
}

void page_tiling_maker::slot_exportPoly()
{
    int col = tileInfoTable->currentColumn();
    if (col < 0)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Please select a column in the table");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        tabWidget->setCurrentIndex(1);
        return;
    }

    PlacedTilePtr placedTile = getTileColumn(col);
    if (!placedTile) return;

    // make name for export
    VersionFileList xfl = FileServices::getFiles(FILE_GIRIH);

    DlgListNameSelect dlg(xfl,0);
    dlg.setWindowTitle("Poly Name");
    int retval = dlg.exec();
    if (retval == QDialog::Rejected)
    {
        qDebug() << "Canceled";
        return;
    }
    Q_ASSERT(retval == QDialog::Accepted);

    VersionedName vname(dlg.newEdit->text());

    // export
    bool rv = placedTile->saveAsGirihShape(vname);

    // report
    QMessageBox box(this);
    if (rv)
    {
        box.setIcon(QMessageBox::Information);
        box.setText(QString("Saved: %1 - OK").arg(vname.get()));
    }
    else
    {
        box.setIcon(QMessageBox::Critical);
        box.setText(QString("Error saving: %1 - FAILED").arg(vname.get()));
    }
    box.exec();
}

void page_tiling_maker::slot_importGirihPoly()
{
    VersionFileList xf = FileServices::getFiles(FILE_GIRIH);

    GirihListSelect dlg(xf,this);
    int retval = dlg.exec();
    if (retval == QDialog::Rejected)
    {
        qDebug() << "Canceled";
        return;
    }

    Q_ASSERT(retval == QDialog::Accepted);

    QStringList qsl = dlg.getSelected();
    if (qsl.isEmpty())
    {
        qInfo() << "No tile selected";
        return;
    }

    for (auto it = qsl.begin(); it != qsl.end(); it++)
    {
        VersionedName vname;
        vname.set(*it);
        PlacedTilePtr pfp = make_shared<PlacedTile>();
        bool rv =  pfp->loadFromGirihShape(vname);
        if (rv)
        {
            tilingMaker->addPlacedTile(pfp);
        }
    }

    refresh(TMR_TILING_UNIT);

    if (viewControl->isEnabled(Layer::Type::TilingMaker))
    {
        Sys::tilingMakerView->forceRedraw();
    }
}

void page_tiling_maker::slot_addGirihShape()
{
    VersionedName vname;
    vname.set(girihShapes->currentData().toString());
    PlacedTilePtr pfp = make_shared<PlacedTile>();
    bool rv =  pfp->loadFromGirihShape(vname);
    if (rv)
    {
        QTransform t;
        tilingMaker->addPlacedTile(pfp);
        refresh(TMR_TILING_UNIT);
        if (viewControl->isEnabled(Layer::Type::TilingMaker))
        {
            Sys::tilingMakerView->forceRedraw();
        }
    }
}

void page_tiling_maker::slot_trim(qreal valX, qreal valY)
{
    TilingPtr tp = tilingMaker->getSelectedTiling();
    PlacedTileSet ptiles;
    if ( config->tm_excludesInTable)
        ptiles = tp->unit().getWithExcluded();
    else
        ptiles = tp->unit().getIncluded();

    for (const auto & pfp : std::as_const(ptiles))
    {
        QTransform t1 = pfp->getPlacement();
        QTransform t2 = QTransform::fromTranslate(valX,valY);
        pfp->setPlacement(t1*t2);
    }

    Sys::tilingMakerView->forceRedraw();
    refresh(TMR_PLACED_TILE);
}

void page_tiling_maker::slot_mergeTilings()
{
    const QVector<TilingPtr> & tilings = tilingMaker->getTilings();
    if (tilings.size() < 2)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Cannot merge : There must be more than one tiling to merge");
        box.exec();
        return;
    }

    tilingMaker->mergeTilings();
    Sys::tilingMakerView->forceRedraw();

    refresh(TMR_MAKER_TILINGS);

    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    box.setText("Tilings Merged");
    box.exec();
}

///////////////////////////////////////////////////////////////////
///
///
///
///////////////////////////////////////////////////////////////////

BQSpinBox::BQSpinBox(page_tiling_maker *parent, PlacedTilePtr ptp) : QSpinBox()
{
    this->parent = parent;
    tile = ptp;

    blocked = false;
    setAlignment(Qt::AlignRight);
    setKeyboardTracking(false);

    lineEdit()->installEventFilter(this);

    setFocusPolicy(Qt::ClickFocus);
}

void BQSpinBox::setValue(int val)
{
    if  (blocked) return;

    blockSignals(true);
    QSpinBox::setValue(val);
    blockSignals(false);
}

void  BQSpinBox::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "mouse press";
    QSpinBox::mousePressEvent(event);
    Sys::tilingMaker->selectTile(tile.lock());
    emit parent->sig_updateView();
}

bool BQSpinBox::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == lineEdit() && event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::LeftButton)
        {
            qDebug() << "mouse press";
            Sys::tilingMaker->selectTile(tile.lock());
            emit parent->sig_updateView();
        }
    }

    return QWidget::eventFilter(watched, event);
}

void BQSpinBox::wheelEvent(QWheelEvent *event)
{
    if (!hasFocus())
    {
        event->ignore();
        return;
    }

    QSpinBox::wheelEvent(event);
}

void BQSpinBox::focusInEvent(QFocusEvent *event)
{
    if (!isReadOnly())
    {
        blocked = true;

        if (Sys::isDarkTheme)
            setStyleSheet("background-color: #22b895");
        else
            setStyleSheet("background-color:yellow");
    }

    QSpinBox::focusInEvent(event);
}

void BQSpinBox::focusOutEvent(QFocusEvent *event)
{
    if (!isReadOnly())
    {
        blocked = false;
        setStyleSheet("");
    }

    QSpinBox::focusOutEvent(event);
}

///////////////////////////////////////////////////////////////////
///
///
///
///////////////////////////////////////////////////////////////////

BQDoubleSpinBox::BQDoubleSpinBox(page_tiling_maker *parent, PlacedTilePtr ptp) : FullPrecisionDoubleSpinBox()
{
    this->parent = parent;
    tile = ptp;

    blocked = false;
    setAlignment(Qt::AlignRight);
    setKeyboardTracking(false);

    lineEdit()->installEventFilter(this);

    setFocusPolicy(Qt::ClickFocus);
}

void BQDoubleSpinBox::setValue(double val)
{
    if  (blocked) return;

    blockSignals(true);
    FullPrecisionDoubleSpinBox::setValue(val);
    blockSignals(false);
}

void BQDoubleSpinBox::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "mouse press";
    FullPrecisionDoubleSpinBox::mousePressEvent(event);
    Sys::tilingMaker->selectTile(tile.lock());
    emit parent->sig_updateView();
}

bool BQDoubleSpinBox::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == lineEdit() && event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::LeftButton)
        {
            qDebug() << "mouse press";
            Sys::tilingMaker->selectTile(tile.lock());
            emit parent->sig_updateView();
        }
    }

    return QWidget::eventFilter(watched, event);
}

void BQDoubleSpinBox::wheelEvent(QWheelEvent *event)
{
    if (!hasFocus())
    {
        event->ignore();
        return;
    }

    FullPrecisionDoubleSpinBox::wheelEvent(event);
}

void BQDoubleSpinBox::focusInEvent(QFocusEvent *event)
{
    if (!isReadOnly())
    {
        blocked = true;

        if (Sys::isDarkTheme)
            setStyleSheet("background-color: #22b895");
        else
            setStyleSheet("background-color:yellow");
    }

    FullPrecisionDoubleSpinBox::focusInEvent(event);
}

void BQDoubleSpinBox::focusOutEvent(QFocusEvent *event)
{
    if (!isReadOnly())
    {
        blocked = false;
        setStyleSheet("");
    }

    FullPrecisionDoubleSpinBox::focusOutEvent(event);
}