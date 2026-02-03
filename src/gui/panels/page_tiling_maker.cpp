#include <QComboBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QHeaderView>
#include <QGroupBox>
#include <QMessageBox>
#include <QTextEdit>
#include <QMenu>

#include "gui/model_editors/tiling_edit/tile_selection.h"
#include "gui/panels/page_tiling_maker.h"
#include "gui/top/controlpanel.h"
#include "gui/top/system_view_controller.h"
#include "gui/viewers/image_view.h"
#include "gui/widgets/dlg_edgepoly_edit.h"
#include "gui/widgets/dlg_listnameselect.h"
#include "gui/widgets/dlg_listselect.h"
#include "gui/widgets/dlg_trim.h"
#include "gui/widgets/floatable_tab.h"
#include "gui/widgets/smx_widget.h"
#include "model/makers/prototype_maker.h"
#include "model/settings/configuration.h"
#include "model/styles/style.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "model/tilings/tiling_manager.h"
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

    if (!config->insightMode)
    {
        setFixedWidth(701);
    }

    controlTab = createControlTab();
    stateTab   = createStateTab();

    tilingMaker->resetTMKbdMode(); // tirggers selection

    tabWidget = new QTabWidget;
    tabWidget->addTab(controlTab, "Create");
    tabWidget->addTab(stateTab,   "Adjust");

    vbox->addWidget(tabWidget);
    vbox->addStretch();

    connect(tilingMaker,  &TilingMaker::sig_menuRefresh,    this, &page_tiling_maker::slot_refreshMenu);
    connect(tilingMaker,  &TilingMaker::sig_tileSelected,   this, [this](PlacedTilePtr ptp) { selectTileColumn(ptp); });
    connect(tabWidget,    &QTabWidget::tabBarDoubleClicked, this, &page_tiling_maker::slot_detach);

    translationsWidget->setVisible(!config->tm_hideTranslations);
    debugWidget->setVisible(config->tm_showDebug);

    QSettings s;

    QString name = QString("panel2/%1/float_control_tab").arg(pageName);
    bool tofloat = s.value(name,false).toBool();
    if (tofloat)
        slot_detach(0);

    name    = QString("panel2/%1/float_state_tab").arg(pageName);
    tofloat = s.value(name,false).toBool();
    if (tofloat)
        slot_detach(1);
}

page_tiling_maker::~page_tiling_maker()
{
    QSettings s;

    QString name = QString("panel2/%1/float_control_tab").arg(pageName);
    s.setValue(name,controlTab->floating);
    controlTab->floating = false;   // so it closes correctly

    name = QString("panel2/%1/float_state_tab").arg(pageName);
    s.setValue(name,stateTab->floating);
    stateTab->floating = false;   // so it closes correctly
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

FloatableTab * page_tiling_maker::createControlTab()
{
    QGroupBox * tilingGroup = createTilingsGroup();
    QGroupBox * actionGroup = createActionsGroup();
    kbdBtnGroup1            = new QButtonGroup();
    smxwidget1              = new SMXWidget(Sys::tilingMakerView.get(),true,true);
    QGroupBox   * kbdGroup  = createKbdModes(kbdBtnGroup1,smxwidget1);
    QGroupBox   * modeGroup = createModesGroup();
    QGroupBox   * repsGroup = createRepetitionsGroup();
    QVBoxLayout * bkgd      = createBackgroundInfo();

    QHBoxLayout * hbox      = new QHBoxLayout();
    hbox->addWidget(tilingGroup);
    hbox->addWidget(actionGroup);

    QVBoxLayout * layout1   = new QVBoxLayout();
    layout1->addWidget(kbdGroup);
    layout1->addLayout(hbox);
    layout1->addWidget(modeGroup);
    layout1->addWidget(repsGroup);
    layout1->addLayout(bkgd);
    layout1->addStretch();

    FloatableTab *tab = new FloatableTab();
    tab->setLayout(layout1);

    return tab;
}

FloatableTab *page_tiling_maker::createStateTab()
{
    kbdBtnGroup2            = new QButtonGroup();
    smxwidget2              = new SMXWidget(Sys::tilingMakerView.get(),true,true);
    QGroupBox   * kbdGroup  = createKbdModes(kbdBtnGroup2,smxwidget2);  // this creates a second selector
    QTableWidget * table    = createTilingTable();
    debugWidget             = createDebugInfo();

    chkShowDebug = new QCheckBox("Show Debug");
    chkShowDebug->setChecked(config->tm_showDebug);

    QCheckBox * chkViewExludes = new QCheckBox("View Excluded");
    chkViewExludes->setChecked(config->tm_showExcludes);

    QBoxLayout * hbox1 = new QHBoxLayout();
    hbox1->addWidget(chkShowDebug);
    hbox1->addWidget(chkViewExludes);
    hbox1->addStretch();

    QVBoxLayout * layout2 = new QVBoxLayout();
    layout2->addWidget(kbdGroup);
    layout2->addWidget(table);
    layout2->addSpacing(11);
    layout2->addLayout(hbox1);
    layout2->addWidget(debugWidget);
    layout2->addStretch();

    FloatableTab *tab = new FloatableTab();
    tab->setLayout(layout2);

    connect(chkShowDebug,    &QCheckBox::clicked,    this,   &page_tiling_maker::slot_showDebug);
    connect(chkViewExludes,  &QCheckBox::clicked,    this,   &page_tiling_maker::slot_showExludes);

    return tab;
}

QGroupBox * page_tiling_maker::createTilingsGroup()
{
    tilingList  = new QListWidget(this);
    tilingList->setFixedHeight(45);
    tilingList->setMaximumWidth(151);

    QPushButton * pbMerge = new QPushButton("Merge Tilings");
    pbMerge->setFixedHeight(23);

    QVBoxLayout * avbox = new QVBoxLayout;
    avbox->addWidget(tilingList);
    avbox->addWidget(pbMerge);

    tilingsGroup = new QGroupBox("Loaded Tilings");
    tilingsGroup->setLayout(avbox);

    connect(tilingList,    &QListWidget::currentItemChanged,  this, &page_tiling_maker::slot_currentItemChanged);
    connect(tilingList,    &QListWidget::itemChanged,         this, &page_tiling_maker::slot_itemChanged);
    connect(pbMerge,       &QPushButton::clicked,             this, &page_tiling_maker::slot_mergeTilings);

    return tilingsGroup;
}

QGroupBox * page_tiling_maker::createActionsGroup()
{
    // actions
    QGridLayout * grid = new QGridLayout();

    QPushButton * addPolyBtn      = new QPushButton("Add Regular Polygon (A)");
    QPushButton * addGirihBtn     = new QPushButton("Add Girih Shape");
    fillVectorsChk                = new QCheckBox(  "Fill/Repeat (F)");
    QPushButton * importBtn       = new QPushButton("Import Tile");

    int row = 0;
    grid->addWidget(addPolyBtn,     row,0,1,2);
    grid->addWidget(addGirihBtn,    row,2);
    grid->addWidget(fillVectorsChk, row,3);
    grid->addWidget(importBtn,      row,4);


    QLabel * rlabel = new QLabel("Rot");
    sides = new AQSpinBox;
    sides->setFixedWidth(41);
    sides->setMinimum(1);
    sides->setMaximum(128);
    sides->setValue(config->polySides);

    tileRot = new AQDoubleSpinBox;
    tileRot->setRange(-360.0,360.0);
    tileRot->setDecimals(6);
    tileRot->setValue(0.0);

    girihShapes = new QComboBox();
    girihShapes->addItem("Decagon", "gDecagon");
    girihShapes->addItem("Pentagon","gPentagon");
    girihShapes->addItem("Bowtie",  "gBowtie");
    girihShapes->addItem("Kite",    "gKite");
    girihShapes->addItem("Rhombus", "gRhombus");

    QPushButton * removeExclBtn   = new QPushButton("Remove Excluded (R)");
    QPushButton * exportBtn       = new QPushButton("Export Tile");

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(sides);
    hbox->addStretch();
    hbox->addWidget(rlabel);
    hbox->addWidget(tileRot);

    row++;
    grid->addLayout(hbox,           row,0,1,2);
    grid->addWidget(girihShapes,    row,2);
    grid->addWidget(removeExclBtn,  row,3);
    grid->addWidget(exportBtn,      row,4);

    QPushButton * pbClearTiling   = new QPushButton("Clear Tiling");
    QPushButton * reloadTilingBtn = new QPushButton("Re-load Tiling");

    QHBoxLayout * hbox2 = new QHBoxLayout();
    hbox2->addWidget(pbClearTiling);
    hbox2->addWidget(reloadTilingBtn);

    QPushButton * dupTilingBtn    = new QPushButton("Duplicate Tiling");
    pbRender        = new QPushButton("Render Motifs");
    chkPropagate    = new QCheckBox("Propagate");

    row++;
    grid->addLayout(hbox2,           row,0,1,2);
    grid->addWidget(dupTilingBtn,    row,2);
    grid->addWidget(pbRender,  row,3);
    grid->addWidget(chkPropagate,      row,4);

    chkPropagate->setChecked(true);

    connect(pbClearTiling,     &QPushButton::clicked,  this, &page_tiling_maker::slot_clearTiling);
    connect(reloadTilingBtn,   &QPushButton::clicked,  this, &page_tiling_maker::slot_reloadTiling);
    connect(dupTilingBtn,      &QPushButton::clicked,  this, &page_tiling_maker::slot_duplicateTiling);
    connect(pbRender,          &QPushButton::clicked,  this, [] {Sys::render(RENDER_RESET_MOTIFS);} );
    connect(chkPropagate,      &QCheckBox::clicked,    tilingMaker, &TilingMaker::slot_propagate_changed);

    QGroupBox * actionGroup = new QGroupBox("Actions");
    actionGroup->setLayout(grid);

    connect(addPolyBtn,     &QPushButton::clicked,          this,       [this]() {tilingMaker->addRegularPolygon(); });
    connect(tileRot,        SIGNAL(valueChanged(qreal)),    tilingMaker,SLOT(updatePolygonRot(qreal)));
    connect(sides,          SIGNAL(valueChanged(int)),      tilingMaker,SLOT(updatePolygonSides(int)));

    connect(exportBtn,      &QPushButton::clicked,          this,       &page_tiling_maker::slot_exportPoly);
    connect(importBtn,      &QPushButton::clicked,          this,       &page_tiling_maker::slot_importGirihPoly);
    connect(addGirihBtn,    &QPushButton::clicked,          this,       &page_tiling_maker::slot_addGirihShape);
    connect(fillVectorsChk, &QCheckBox::clicked,            this,       [](bool clickstate) { Sys::tilingMakerView->setFill(clickstate); });
    connect(removeExclBtn,  &QPushButton::clicked,          this,       [this]()            { tilingMaker->removeExcludeds(); emit sig_updateView(); });

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
    QCheckBox    * chkSnapTo    = new QCheckBox("Snap to Grid");
    QCheckBox    * chkSnapOnly  = new QCheckBox("Snap Only");
    QCheckBox    * chkAutoFill  = new QCheckBox("Fill on load");

    stackLabel   = new QLabel("Stack:");

    uint maxW = 61;
    QPushButton  * savStack    = new QPushButton("Save");
    savStack->setMaximumWidth(maxW);
    QPushButton  * undoStack   = new QPushButton("Undo");
    undoStack->setMaximumWidth(maxW);
    QPushButton  * redoStack   = new QPushButton("Redo");
    redoStack->setMaximumWidth(maxW);

    QHBoxLayout * stackL  = new QHBoxLayout();
    stackL->addWidget(stackLabel);
    stackL->addSpacing(5);
    stackL->addWidget(savStack);
    stackL->addSpacing(5);
    stackL->addWidget(undoStack);
    stackL->addSpacing(5);
    stackL->addWidget(redoStack);
    stackL->addStretch();

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
    modeBox->addWidget(reflect,row,2);
    modeBox->addWidget(mirrorX,row,3);
    modeBox->addWidget(mirrorY,row,4);
    modeBox->addWidget(chkAutoFill,row,5);

    row++;
    QHBoxLayout * hb = new QHBoxLayout;
    hb->addWidget(chkSnapOnly);
    hb->addWidget(chkSnapTo);
    hb->addStretch();
    hb->addLayout(stackL);
    modeBox->addLayout(hb,row,0,1,6);

    QVBoxLayout * vbox = new  QVBoxLayout();
    vbox->addLayout(modeBox);
    vbox->addLayout(createSecondControlRow());

    QGroupBox * modeGroup = new QGroupBox("Modes");
    modeGroup->setLayout(vbox);

    // grouping
    mouseModeBtnGroup = new QButtonGroup;
    mouseModeBtnGroup->setExclusive(false);     // allows uncheck

    mouseModeBtnGroup->addButton(drawTrans,     TM_TRANSLATION_VECTOR_MODE);
    mouseModeBtnGroup->addButton(newPoly,       TM_DRAW_POLY_MODE);
    mouseModeBtnGroup->addButton(copyPoly,      TM_COPY_MODE);
    mouseModeBtnGroup->addButton(deletePoly,    TM_DELETE_MODE);
    mouseModeBtnGroup->addButton(includePoly,   TM_INCLUSION_MODE);
    mouseModeBtnGroup->addButton(nomode,        TM_NO_MOUSE_MODE);
    mouseModeBtnGroup->addButton(position,      TM_POSITION_MODE);
    mouseModeBtnGroup->addButton(measure,       TM_MEASURE_MODE);
    mouseModeBtnGroup->addButton(unify,         TM_UNIFY_MODE);
    mouseModeBtnGroup->addButton(editPoly,      TM_EDIT_TILE_MODE);
    mouseModeBtnGroup->addButton(editEdge,      TM_EDGE_CURVE_MODE);
    mouseModeBtnGroup->addButton(mirrorX,       TM_MIRROR_X_MODE);
    mouseModeBtnGroup->addButton(mirrorY,       TM_MIRROR_Y_MODE);
    mouseModeBtnGroup->addButton(reflect,       TM_REFLECT_EDGE);
    mouseModeBtnGroup->addButton(drawConst,     TM_CONSTRUCTION_LINES);
    mouseModeBtnGroup->addButton(decompose,     TM_DECOMPOSE_MODE);

    lastChecked = mouseModeBtnGroup->button(TM_NO_MOUSE_MODE);
    lastChecked->setChecked(true);

    chkAutoFill->setChecked(config->tm_loadFill);
    chkSnapTo->setChecked(config->tm_snapToGrid);
    chkSnapOnly->setChecked(config->tm_snapOnly);

    connect(chkSnapTo,  &QCheckBox::clicked,   this,   [this](bool checked) { config->tm_snapToGrid =  checked;});
    connect(chkSnapOnly,&QCheckBox::clicked,   this,   [this](bool checked) { config->tm_snapOnly =  checked;});
    connect(mouseModeBtnGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, &page_tiling_maker::slot_setModes);
    connect(savStack,   &QPushButton::clicked, tilingMaker, &TilingMaker::slot_stack_save);
    connect(undoStack,  &QPushButton::clicked, tilingMaker, &TilingMaker::slot_stack_undo);
    connect(redoStack,  &QPushButton::clicked, tilingMaker, &TilingMaker::slot_stack_redo);
    connect(chkAutoFill,&QCheckBox::clicked,   this,        &page_tiling_maker::slot_autofill);

    return modeGroup;
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

    chkSingle = new QRadioButton("Singleton");
    chkReps   = new QRadioButton("Fill");

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
    hbox->addStretch();
    hbox->addWidget(chkSingle);
    hbox->addWidget(chkReps);
    hbox->addLayout(repsLayout);
    hbox->addStretch();

    connect(chkSingle, &QCheckBox::clicked, this, &page_tiling_maker::singleton_changed);
    connect(chkReps,   &QCheckBox::clicked, this, &page_tiling_maker::reps_changed);
    connect(xRepMin, SIGNAL(valueChanged(int)),  this, SLOT(slot_set_reps(int)));
    connect(xRepMax, SIGNAL(valueChanged(int)),  this, SLOT(slot_set_reps(int)));
    connect(yRepMin, SIGNAL(valueChanged(int)),  this, SLOT(slot_set_reps(int)));
    connect(yRepMax, SIGNAL(valueChanged(int)),  this, SLOT(slot_set_reps(int)));

    return hbox;
}

QVBoxLayout *page_tiling_maker::createTranslationsRow()
{
    QCheckBox * chkHideVectors = new QCheckBox("Hide onscreen translation vectors");

    QCheckBox * chkShowTranslate = new QCheckBox("Show translation vectors");
    chkShowTranslate->setChecked(!config->tm_hideTranslations);
    connect(chkShowTranslate,&QCheckBox::clicked,    this,   &page_tiling_maker::slot_showTranslations);

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(chkHideVectors);
    hbox->addWidget(chkShowTranslate);
    hbox->addStretch();
    if (config->insightMode)
    {
        QPushButton * swapBtn = new QPushButton("Swap T1/T2");
        hbox->addWidget(swapBtn);
        connect(swapBtn, &QPushButton::clicked,  this,   &page_tiling_maker::slot_swapTrans);
    }

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

    T1len   = new DoubleSpinSet("Len ",0,0,10.0);
    T2len   = new DoubleSpinSet("Len ",0,0,10.0);
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
    T1angle->setSingleStep(0.5);
    T2angle->setSingleStep(0.5);

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
    vbox->addLayout(hbox);
    vbox->addWidget(translationsWidget);

    connect(chkHideVectors, &QCheckBox::clicked, this, &page_tiling_maker::slot_hideVectors);

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

QHBoxLayout * page_tiling_maker::createSecondControlRow()
{
    AQPushButton * pbShowOverlaps = new AQPushButton("Overlaps/Touching");
    AQPushButton * pbShowIncluded = new AQPushButton("Included/Excluded");
    AQPushButton * pbShowUnique   = new AQPushButton("Unique");

    QButtonGroup * bgrp    = new QButtonGroup();
    bgrp->setExclusive(true);
    bgrp->addButton(pbShowOverlaps);
    bgrp->addButton(pbShowIncluded);
    bgrp->addButton(pbShowUnique);

    switch(config->tm_tileColorMode)
    {
    case TILE_COLOR_TOUCHING:
        pbShowOverlaps->setChecked(true);
        break;
    case TILE_COLOR_INCLUDES:
        pbShowIncluded->setChecked(true);
        break;
    case TILE_COLOR_UNIQUE:
        pbShowUnique->setChecked(true);
        break;
    }

    statusLabel  = new QLabel();
    overlapStatus = new QLabel();
    overlapStatus->setStyleSheet("color: green; font-weight: bold;");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(pbShowOverlaps);
    hbox->addWidget(pbShowIncluded);
    hbox->addWidget(pbShowUnique);
    hbox->addStretch();
    hbox->addWidget(statusLabel);
    hbox->addStretch();
    hbox->addWidget(overlapStatus);

    connect(pbShowOverlaps, &QRadioButton::clicked, tilingMaker, [this] { config->tm_tileColorMode = TILE_COLOR_TOUCHING; emit sig_updateView(); });
    connect(pbShowIncluded, &QRadioButton::clicked, tilingMaker, [this] { config->tm_tileColorMode = TILE_COLOR_INCLUDES; emit sig_updateView(); });
    connect(pbShowUnique,   &QRadioButton::clicked, tilingMaker, [this] { config->tm_tileColorMode = TILE_COLOR_UNIQUE;   emit sig_updateView(); });

    return hbox;
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
    tileInfoTable->setStyleSheet("selection-color : black; selection-background-color : LightGreen");

    QStringList qslv;
    qslv << "tile" << "show" << "type"  << "tile-rot" << "tile-scale" << "tile-sides"<< "placed-scale" << "placed-rot" << "placed-X" << "placed-Y" << "CW" << "tile-addr";
    tileInfoTable->setVerticalHeaderLabels(qslv);
    tileInfoTable->horizontalHeader()->setVisible(false);
    tileInfoTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    tileInfoTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QHeaderView * qhv = tileInfoTable->verticalHeader();

    connect(tileInfoTable,   &QTableWidget::customContextMenuRequested, this, &page_tiling_maker::slot_menu);
    connect(tileInfoTable,   &QTableWidget::cellClicked,                this, &page_tiling_maker::slot_cellSelected);
    connect(qhv,             &QHeaderView::sectionClicked,              this, &page_tiling_maker::tableHeaderClicked);

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

QGroupBox * page_tiling_maker::createKbdModes(QButtonGroup * kbdGroup, SMXWidget * smxwidget)
{
    QRadioButton * rbView  = new QRadioButton("Adjust View");
    QRadioButton * rbSTile = new QRadioButton("Adjust Selected Tiling");
    QRadioButton * rbUTile = new QRadioButton("Adjust Unique Tile");
    QRadioButton * rbPTile = new QRadioButton("Adjust Placed Tile");

    // grouping
    kbdGroup->setExclusive(true);

    kbdGroup->addButton(rbView,     TM_MODE_XFORM_ALL);
    kbdGroup->addButton(rbSTile,    TM_MODE_XFORM_TILING);
    kbdGroup->addButton(rbUTile,    TM_MODE_XFORM_UNIQUE_TILE);
    kbdGroup->addButton(rbPTile,    TM_MODE_XFORM_PLACED_TILE);

    connect(kbdGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, [this,kbdGroup] (QAbstractButton* btn) { slot_setKbdMode1(btn, kbdGroup); });
    connect(tilingMaker, &TilingMaker::sig_TMKbdMode, this, &page_tiling_maker::slot_kbdMode1);

    QHBoxLayout * hb = new QHBoxLayout;
    hb->addWidget(smxwidget);
    hb->addStretch();
    hb->addWidget(rbView);
    hb->addWidget(rbSTile);
    hb->addWidget(rbUTile);
    hb->addWidget(rbPTile);

    QGroupBox * gb = new QGroupBox("Keryboard/Mouse Actions");
    gb->setLayout(hb);

    return gb;
}

// kbdGroup selection
void page_tiling_maker::slot_setKbdMode1(QAbstractButton * btn, QButtonGroup * kbdGroup)
{
    int mode = kbdGroup->id(btn);
    eTMKbdMode tmm = static_cast<eTMKbdMode>(mode);

    tilingMaker->setTMKbdMode(tmm);

    if (tmm != TM_MODE_XFORM_ALL)
        emit Sys::viewController->sig_solo(Sys::tilingMakerView.get(),true);
    else
        emit Sys::viewController->sig_solo(Sys::tilingMakerView.get(),false);

    tallyKbdMode();
}

// from canvas setKbdMode
void page_tiling_maker::slot_kbdMode1(eTMKbdMode mode)
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

void page_tiling_maker::initPageStatusString()
{
    const QString s("<body>"
                    "<span style=\"color:rgba(205,102, 25,1.0)\">overlapping</span>  |  "
                    "<span style=\"color:rgba( 25,102,205,1.0)\">touching</span> |  "
                    "<span style=\"color:rgba(255,217,217,1.0)\">included</span>  |  "
                    "<span style=\"color:rgba(217,217,255,1.0)\">excluded</span>  |  "
                    "<span style=\"color:rgba(127,255,127,1.0)\">selected</span>  |  "
                    "<span style=\"color:rgba(  0,255,  0,1.0)\">under-mouse</span>  |  "
                    "<span style=\"color:rgba(206,179,102,1.0)\">dragging</span>"
                    "</body>");
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
    currentTiling = tilingMaker->getSelected();
    refresh(TMR_ALL);
}

void page_tiling_maker::onExit()
{
    clearPageStatus();
}

void page_tiling_maker::slot_refreshMenu(eTileMenuRefresh scope)
{
    refresh(scope);
}

void page_tiling_maker::refresh(eTileMenuRefresh scope)
{
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
        break;
    }
}

void page_tiling_maker::__refreshOther(bool clear)
{
    if (clear)
    {
        tilingMaker->setTilingMakerMouseMode(TM_NO_MOUSE_MODE);
        tilingMaker->clearConstructionLines();
        tileInfo->clear();
    }

    sides->setValue(tilingMaker->getPolygonSides());

    if (viewControl->isEnabled(VIEW_TILING_MAKER))
    {
        Sys::tilingMakerView->forceRedraw();
    }
}

void page_tiling_maker::__refreshTilingSelector()
{
    TilingPtr selected = tilingMaker->getSelected();
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
        tilingsGroup->setTitle(QString("Loaded Tilings (%1) ").arg(tilingList->count()));
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

    TilingPtr tiling = tilingMaker->getSelected();

    QPointF t1 = tiling->hdr().getTrans1();
    QPointF t2 = tiling->hdr().getTrans2();

    QLineF tl1(QPointF(0.0,0.0),t1);
    QLineF tl2(QPointF(0.0,0.0),t2);

    int xMin,xMax,yMin,yMax;
    bool singleton;
    const FillData & fd = tiling->hdr().getCanvasSettings().getFillData();
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

    blockSignals(false);
    blockPage(false);
}

void page_tiling_maker::__refreshTilingTable()
{
    if (!panel->isVisiblePage(this))
        return;

    blockPage(true);
    blockSignals(true);

    TilingPtr tiling = tilingMaker->getSelected();

    PlacedTiles tu;
    if (config->tm_showExcludes)
        tu  = tiling->unit().getAll();
    else
        tu  = tiling->unit().getIncluded();

    int col = 0;
    int count = 0;
    for (const auto & pfp : std::as_const(tu))
    {
        QString inclusion = QString("%1 (%2)").arg((pfp->isIncluded()) ? "Included" : "Excluded").arg(col);
        refreshTableEntry(pfp,col++,inclusion);
        if (++count == TILE_TABLE_LIMIT)
        {
            qWarning() << "page_tiling_maker: tile table count" << tu.count() << "exceeds" << TILE_TABLE_LIMIT;
        }
    }

    tileInfoTable->adjustTableSize(750);

    blockSignals(false);
    blockPage(false);
}

void  page_tiling_maker::onRefresh()
{
    if (currentTiling.lock() != tilingMaker->getSelected())
    {
        currentTiling = tilingMaker->getSelected();
        refresh(TMR_ALL);
    }
    refreshMenuStatus();
}

void page_tiling_maker::refreshMenuStatus()
{
    static eTilingMakerMouseMode oldMode = TM_NO_MOUSE_MODE;

    eTilingMakerMouseMode currentMode = tilingMaker->getTilingMakerMouseMode();
    if (currentMode != oldMode)
    {
        switch (currentMode)
        {
        case TM_INCLUSION_MODE:
            pageStatusString = "Toggle the inclusion of polygons in the tiling by clicking on them with the mouse.";
            break;
        case TM_DRAW_POLY_MODE:
            pageStatusString = "Select a series of vertices clockwise to draw a free-form polygon. (Click on vertices).";
            break;
        case TM_TRANSLATION_VECTOR_MODE:
            pageStatusString = "Use mouse, left-click on a polygon center or vertex, drag to the repititon point. Do this twice for two directions.";
            break;
        default:
            initPageStatusString();
            break;
        }
        setPageStatus();

        oldMode = currentMode;
    }

    // debug info
    if (config->insightMode && chkShowDebug->isChecked())
    {
        QString dbgStatus = tilingMaker->getStatus();
        debugLabel1->setText(dbgStatus);

        QPointF a = Sys::tilingMakerView->getMousePos();
        QPointF b = Sys::tilingMakerView->screenToModel(a);
        QString astring;
        QTextStream ts(&astring);
        ts.setRealNumberPrecision(8);
        ts << "pos: (" << b.x() << ", " << b.y() << ") ";

        PlacedTileSelectorPtr selector = Sys::tilingMakerView->tileSelector();
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

        stackLabel->setText(tilingMaker->getStack().getStackStatus());

        smxwidget1->refresh();
        smxwidget2->refresh();
    }

    // always visible status line
    TilingPtr tp = tilingMaker->getSelected();
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

    QString status = QString("Tiles: %1  Unique: %2  Excluded: %3").arg(tp->unit().numIncluded()).arg(tp->unit().numUnique()).arg(tp->unit().numExcluded());
    statusLabel->setText(status);

    tallyMouseMode();
    tallyKbdMode();

    fillVectorsChk->setChecked(Sys::tm_fill);

    // force tiling maker selection to match table selection
    // FIXME - methinks the menu shoold show the tiling maker state
    PlacedTilePtr selected = tilingMaker->selectedTile();  // or shouild this be clickedSelector()->getPlacedTile();

    if (selected)
    {
        int cols = tileInfoTable->columnCount();
        for (int i=0; i < cols; i++)
        {
            PlacedTilePtr pf = getTileColumn(i);
            if (selected == pf)
            {
                tileInfoTable->setCurrentCell(0,i);
                break;
            }
        }
    }
    else
    {
        tileInfoTable->setCurrentCell(0,-1);
    }

    if (!chkPropagate->isChecked())
        pbRender->setStyleSheet("QPushButton { background-color: yellow; color: red;}");
    else
        pbRender->setStyleSheet("");

    if (tp->getBkgdImage())
    {
        chkBkgd->setChecked(true);
        pbExam->setText("Examine Background Image");
    }
    else
    {
        chkBkgd->setChecked(false);
        pbExam->setText("Add Background Image");
    }
}

void  page_tiling_maker::selectTileColumn(PlacedTilePtr pfp)
{
    int col = getColumn(pfp);
    tileInfoTable->blockSignals(true);
    tileInfoTable->selectColumn(col);
    tileInfoTable->blockSignals(false);
}

bool page_tiling_maker::canExit()
{
    QString txt;
    QString info;
    TilingPtr tp = tilingMaker->getSelected();
    if (tp->unit().numAll())
    {
        if (tp->unit().numIncluded() == 0)
        {
            txt  = "There are no polygons included in the tiling";
            info = "To include polygons, press Cancel and 'Include' the polygons you want in the tiling";
        }
        else
        {
            const FillData & fd = tp->hdr().getCanvasSettings().getFillData();
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

    TilingPtr tp = tilingMaker->getSelected();
    tilingMaker->removeTiling(tp);  // re-creates if becomes empty
    emit sig_reconstructView();

    refresh(TMR_MAKER_TILINGS);
}

void page_tiling_maker::slot_reloadTiling()
{
    TilingPtr oldTiling  = tilingMaker->getSelected();
    VersionedName vname  = oldTiling->getVName();
    VersionedFile vfile  = FileServices::getFile(vname,FILE_TILING);

    tilingMaker->loadTiling(vfile,TILM_RELOAD);

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

    TilingPtr tp = tilingMaker->getSelected();
    Q_ASSERT(tp);

    bool showAll = (config->tm_showExcludes && (tp->unit().numAll() < TILE_TABLE_LIMIT));   // throttle show all for large number pf tiles

    PlacedTiles viewable;
    if (showAll)
        viewable = tp->unit().getAll();
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
    connect(sp,static_cast<void (AQSpinBox::*)(int)>(&AQSpinBox::valueChanged), this,
            [this,col] { slot_sidesChanged(col); });

    BQDoubleSpinBox * dsp = new BQDoubleSpinBox(this,pf);
    dsp->setRange(-360.0,360.0);
    dsp->setDecimals(16);
    dsp->setAlignment(Qt::AlignRight);
    dsp->setValue(tile->getRotation());
    tileInfoTable->setCellWidget(TI_TILE_ROT,col,dsp);
    connect(dsp, static_cast<void (AQDoubleSpinBox::*)(double)>(&AQDoubleSpinBox::valueChanged), this,
            [this,col] { slot_tileRotChanged(col); });

    dsp = new BQDoubleSpinBox(this,pf);
    dsp->setRange(0,128);
    dsp->setDecimals(16);
    dsp->setSingleStep(0.01);
    dsp->setAlignment(Qt::AlignRight);
    dsp->setValue(tile->getScale());
    tileInfoTable->setCellWidget(TI_TILE_SCALE,col,dsp);
    connect(dsp, static_cast<void (AQDoubleSpinBox::*)(double)>(&AQDoubleSpinBox::valueChanged), this,
            [this,col] { slot_tileScaleChanged(col); });

    dsp = new BQDoubleSpinBox(this,pf);
    dsp->setRange(0,128);
    dsp->setDecimals(16);
    dsp->setSingleStep(0.01);
    dsp->setAlignment(Qt::AlignRight);
    dsp->setValue(Transform::scalex(T));
    tileInfoTable->setCellWidget(TI_PLACEMENT_SCALE,col,dsp);
    connect(dsp, static_cast<void (AQDoubleSpinBox::*)(double)>(&AQDoubleSpinBox::valueChanged), this,
            [this,col] { slot_placedScaleChanged(col); });

    dsp = new BQDoubleSpinBox(this,pf);
    dsp->setRange(-360.0,360.0);
    dsp->setDecimals(16);
    dsp->setAlignment(Qt::AlignRight);
    dsp->setValue(qRadiansToDegrees(Transform::rotation(T)));
    tileInfoTable->setCellWidget(TI_PLACEMENT_ROT,col,dsp);
    connect(dsp, static_cast<void (AQDoubleSpinBox::*)(double)>(&AQDoubleSpinBox::valueChanged), this,
            [this,col] { slot_placedRotateChanged(col); });

    dsp = new BQDoubleSpinBox(this,pf);
    dsp->setRange(-100.0,100.0);
    dsp->setDecimals(16);
    dsp->setSingleStep(0.1);
    dsp->setAlignment(Qt::AlignRight);
    dsp->setValue(Transform::transx(T));
    tileInfoTable->setCellWidget(TI_PLACEMENT_X,col,dsp);
    connect(dsp,static_cast<void (AQDoubleSpinBox::*)(double)>(&AQDoubleSpinBox::valueChanged), this,
            [this,col] { slot_placedTranslateChanged(col); });

    dsp = new BQDoubleSpinBox(this,pf);
    dsp->setRange(-100.0,100.0);
    dsp->setDecimals(16);
    dsp->setSingleStep(0.1);
    dsp->setAlignment(Qt::AlignRight);
    dsp->setValue(Transform::transy(T));
    tileInfoTable->setCellWidget(TI_PLACEMENT_Y,col,dsp);
    connect(dsp, static_cast<void (AQDoubleSpinBox::*)(double)>(&AQDoubleSpinBox::valueChanged), this,
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
    AQSpinBox * sp = dynamic_cast<AQSpinBox*>(w);
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
    AQDoubleSpinBox * dsp = dynamic_cast<AQDoubleSpinBox*>(w);
    Q_ASSERT(dsp);
    dsp->setValue(tile->getRotation());

    w = tileInfoTable->cellWidget(TI_TILE_SCALE,col);
    dsp = dynamic_cast<AQDoubleSpinBox*>(w);
    Q_ASSERT(dsp);
    dsp->setValue(tile->getScale());

    w = tileInfoTable->cellWidget(TI_PLACEMENT_SCALE,col);
    dsp = dynamic_cast<AQDoubleSpinBox*>(w);
    Q_ASSERT(dsp);
    dsp->setValue(Transform::scalex(T));

    w = tileInfoTable->cellWidget(TI_PLACEMENT_ROT,col);
    dsp = dynamic_cast<AQDoubleSpinBox*>(w);
    Q_ASSERT(dsp);
    dsp->setValue(qRadiansToDegrees(Transform::rotation(T)));

    w = tileInfoTable->cellWidget(TI_PLACEMENT_X,col);
    dsp = dynamic_cast<AQDoubleSpinBox*>(w);
    Q_ASSERT(dsp);
    dsp->setValue(Transform::transx(T));

    w = tileInfoTable->cellWidget(TI_PLACEMENT_Y,col);
    dsp = dynamic_cast<AQDoubleSpinBox*>(w);
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
    AQSpinBox * sp = dynamic_cast<AQSpinBox*>(cw);
    Q_ASSERT(sp);
    int sides = sp->value();

    PlacedTilePtr pf = getTileColumn(col);
    tilingMaker->selectTile(pf);

    TilePtr tile     = pf->getTile();
    tile->setN(sides);

    if (tilingMaker->getPropagate() && pf->isIncluded())
    {
        ProtoEvent protoEvent;
        protoEvent.event  = PROM_TILE_UNIQUE_NATURE_CHANGED;
        protoEvent.tiling = tilingMaker->getSelected();
        protoEvent.tile   = tile;
        Sys::prototypeMaker->sm_takeUp(protoEvent);
    }

    refresh(TMR_PLACED_TILE);

    emit sig_reconstructView();
}

void page_tiling_maker::slot_tileRotChanged(int col)
{
    if (pageBlocked()) return;

    QWidget * cw  = tileInfoTable->cellWidget(TI_TILE_ROT,col);
    AQDoubleSpinBox * sp = dynamic_cast<AQDoubleSpinBox*>(cw);
    Q_ASSERT(sp);
    qreal rotation = sp->value();

    PlacedTilePtr pf = getTileColumn(col);
    tilingMaker->selectTile(pf);

    TilePtr tile  = pf->getTile();
    tile->setRotate(rotation);

    if (tilingMaker->getPropagate() && pf->isIncluded())
    {
        ProtoEvent protoEvent;
        protoEvent.event  = PROM_TILE_UNIQUE_TRANS_CHANGED;
        protoEvent.tiling = tilingMaker->getSelected();
        protoEvent.tile   = tile;
        Sys::prototypeMaker->sm_takeUp(protoEvent);
    }

    refresh(TMR_PLACED_TILE);

    emit sig_reconstructView();
}

void page_tiling_maker::slot_tileScaleChanged(int col)
{
    if (pageBlocked()) return;

    QWidget * cw  = tileInfoTable->cellWidget(TI_TILE_SCALE,col);
    AQDoubleSpinBox * sp = dynamic_cast<AQDoubleSpinBox*>(cw);
    Q_ASSERT(sp);
    qreal scale = sp->value();

    PlacedTilePtr pf = getTileColumn(col);
    tilingMaker->selectTile(pf);

    TilePtr tile  = pf->getTile();
    tile->setScale(scale);

    if (tilingMaker->getPropagate() && pf->isIncluded())
    {
        ProtoEvent protoEvent;
        protoEvent.event  = PROM_TILE_UNIQUE_TRANS_CHANGED;
        protoEvent.tiling = tilingMaker->getSelected();
        protoEvent.tile   = tile;
        Sys::prototypeMaker->sm_takeUp(protoEvent);
    }

    refresh(TMR_PLACED_TILE);

    emit sig_reconstructView();
}

void page_tiling_maker::slot_placedTranslateChanged(int col)
{
    qWarning() << "slot_placedTranslateChanged";
    if (pageBlocked()) return;

    PlacedTilePtr placedTile = getTileColumn(col);
    tilingMaker->selectTile(placedTile);

    auto widget = tileInfoTable->cellWidget(TI_PLACEMENT_X,col);
    auto dsp    = dynamic_cast<AQDoubleSpinBox*>(widget);
    Q_ASSERT(dsp);
    qreal tx = dsp->value();

    widget = tileInfoTable->cellWidget(TI_PLACEMENT_Y,col);
    dsp    = dynamic_cast<AQDoubleSpinBox*>(widget);
    Q_ASSERT(dsp);
    qreal ty = dsp->value();

    tilingMaker->placedTileSetTranslate(tx,ty);

    if (tilingMaker->getPropagate() && placedTile->isIncluded())
    {        
        ProtoEvent protoEvent;
        protoEvent.event = PROM_TILING_CHANGED;
        protoEvent.tiling = tilingMaker->getSelected();
        Sys::prototypeMaker->sm_takeUp(protoEvent);
    }

    refresh(TMR_PLACED_TILE);

    emit sig_reconstructView();
}

void page_tiling_maker::slot_placedScaleChanged(int col)
{
    qWarning() << "slot_placedScaleChanged col=" << col;
    if (pageBlocked()) return;

    PlacedTilePtr placedTile = getTileColumn(col);
    tilingMaker->selectTile(placedTile);

    QWidget  * cw  = tileInfoTable->cellWidget(TI_PLACEMENT_SCALE,col);
    AQDoubleSpinBox * dsp = dynamic_cast<AQDoubleSpinBox*>(cw);
    Q_ASSERT(dsp);
    qreal scale = dsp->value();
    if (scale <= 0.0 || scale > 128.0)
    {
        return;     // fixes problem with scales of 0 when entering data
    }

    tilingMaker->placedTileSetScale(scale);

    if (tilingMaker->getPropagate() && placedTile->isIncluded())
    {
        ProtoEvent protoEvent;
        protoEvent.event = PROM_TILING_CHANGED;
        protoEvent.tiling = tilingMaker->getSelected();
        Sys::prototypeMaker->sm_takeUp(protoEvent);
    }

    refresh(TMR_PLACED_TILE);

    emit sig_updateView();
}

void page_tiling_maker::slot_placedRotateChanged(int col)
{
    qDebug() << "slot_placedRotateChanged";
    if (pageBlocked()) return;

    PlacedTilePtr placedTile = getTileColumn(col);
    tilingMaker->selectTile(placedTile);

    auto rot_widget = tileInfoTable->cellWidget(TI_PLACEMENT_ROT,col);
    auto rot_dsp = dynamic_cast<AQDoubleSpinBox*>(rot_widget);
    Q_ASSERT(rot_dsp);
    qreal cell_rotation = rot_dsp->value();

    tilingMaker->placedTileSetRotate(cell_rotation);

    if (tilingMaker->getPropagate() && placedTile->isIncluded())
    {
        ProtoEvent protoEvent;
        protoEvent.event = PROM_TILING_CHANGED;
        protoEvent.tiling = tilingMaker->getSelected();
        Sys::prototypeMaker->sm_takeUp(protoEvent);
    }

    refresh(TMR_PLACED_TILE);

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

    refresh(TMR_PLACED_TILE);

    Sys::tilingMakerView->forceRedraw();
}

void page_tiling_maker::slot_set_reps(int val)
{
    Q_UNUSED(val);

    if (pageBlocked()) return;

    TilingPtr tiling = tilingMaker->getSelected();
    if (!tiling) return;

    FillData fdata;
    fdata.set(chkSingle->isChecked(),xRepMin->value(), xRepMax->value(), yRepMin->value(), yRepMax->value());

    CanvasSettings cs = tiling->hdr().getCanvasSettings();
    cs.setFillData(fdata);
    tiling->hdr().setCanvasSettings(cs);

    tilingMaker->updateReps();

    refresh(TMR_TILING_HEADER);

    Sys::tilingMakerView->forceRedraw();
}

void page_tiling_maker::singleton_changed(bool checked)
{
    TilingPtr tiling = tilingMaker->getSelected();
    if (!tiling) return;

    FillData fd;
    if (!checked)
        fd.set(false,-3, 3, -3, 3);
    else
        fd.set(true,0,0,0,0);

    CanvasSettings cs = tiling->hdr().getCanvasSettings();
    cs.setFillData(fd);
    tiling->hdr().setCanvasSettings(cs);

    tilingMaker->updateReps();

    Sys::tilingMakerView->forceRedraw();

    refresh(TMR_TILING_HEADER);
}

void page_tiling_maker::reps_changed(bool checked)
{
    singleton_changed(!checked);
#if 0
    TilingPtr tiling = tilingMaker->getSelected();
    if (!tiling) return;

    FillData fd;
    if (!checked)
        fd.set(false,-3, 3, -3, 3);
    else
        fd.set(true,0,0,0,0);

    CanvasSettings cs = tiling->hdr().getCanvasSettings();
    cs.setFillData(fd);
    tiling->hdr().setCanvasSettings(cs);

    tilingMaker->updateReps();

    refresh(TMR_TILING_HEADER);
#endif
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

    TilingPtr tiling  = tilingMaker->getSelected();
    tiling->hdr().setTranslationVectors(QPointF(x1,y1), QPointF(x2,y2), tiling->hdr().getTranslateOrigin());
    tilingMaker->updateVectors();
    tiling->setTilingViewChanged();

    refresh(TMR_TILING_HEADER);
}

void page_tiling_maker::slot_t1t2LenChanged(double val)
{
    Q_UNUSED(val)

    if (pageBlocked()) return;

    TilingPtr tiling = tilingMaker->getSelected();

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

    TilingPtr tiling = tilingMaker->getSelected();

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

    tilingMaker->select(tp);

    refresh(TMR_TILING);
}

void page_tiling_maker::slot_itemChanged(QListWidgetItem * item)
{
    // TODO - make sure something is always visible
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
    }

    tp->setViewable(checkstate);

    refresh(TMR_TILING);

    emit sig_updateView();
}

void page_tiling_maker::slot_setModes(QAbstractButton * btn)
{
    int mode = mouseModeBtnGroup->id(btn);
    eTilingMakerMouseMode mm = static_cast<eTilingMakerMouseMode>(mode);
    qDebug() << sTilingMakerMouseMode[mm] << btn->isChecked();

    if (btn->isChecked())
        tilingMaker->setTilingMakerMouseMode(mm);
    else
        tilingMaker->setTilingMakerMouseMode(TM_NO_MOUSE_MODE);

    tallyMouseMode();
}

void page_tiling_maker::tallyMouseMode()
{
    // dont need to block signals because connect is based on 'clicked'
    eTilingMakerMouseMode mmode = tilingMaker->getTilingMakerMouseMode();
    QAbstractButton * current   = mouseModeBtnGroup->button(mmode);

    if (current == lastChecked)
    {
        return;
    }

    lastChecked->setChecked(false);
    current->setChecked(true);
    lastChecked = current;
}

void page_tiling_maker::tallyKbdMode()
{
    eTMKbdMode mode = tilingMaker->getTMKbdMode();

    QAbstractButton * button = kbdBtnGroup1->button(mode);
    button->setChecked(true);

    button = kbdBtnGroup2->button(mode);
    button->setChecked(true);
}

void page_tiling_maker::slot_showExludes(bool checked)
{
    config->tm_showExcludes = checked;

    refresh(TMR_TILING_UNIT);
}

void page_tiling_maker::slot_showDebug(bool checked)
{
    config->tm_showDebug = checked;
    debugWidget->setVisible(checked);
    if (!checked)
    {
        stackLabel->setText("Stack:");
    }
}

void page_tiling_maker::slot_showTranslations(bool checked)
{
    config->tm_hideTranslations = !checked;
    translationsWidget->setVisible(checked);
}

void page_tiling_maker::slot_autofill(bool checked)
{
    config->tm_loadFill = checked;
}

// called by clicking on tile info table column
void page_tiling_maker::slot_menu(QPointF spt)
{
    qDebug() << "menu spt=" << spt;

    int col = tileInfoTable->currentColumn();
    PlacedTilePtr pfp = getTileColumn(col);

    QString astr = QString("Side=%1").arg(pfp->getTile()->edgeLen(),0,'g',16);
    qDebug().noquote() << astr;

    QString bstr =  (pfp->isIncluded())  ? "Exclude" : "Include";

    QMenu myMenu(tileInfoTable);

    myMenu.addAction("De Select", this, [this] { tileInfoTable->setCurrentCell(0,-1); tilingMaker->deselectTile(); tilingMaker->resetClickedSelector(); tilingMaker->forceRedraw(); });
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

void page_tiling_maker::slot_table_menu_edit_tile()
{
    // called by click on table column menu
    qDebug() << "page_tiling_maker::slot_menu_edit_tile";
    int           col = tileInfoTable->currentColumn();
    PlacedTilePtr pfp = getTileColumn(col);

    tilingMaker->setTilingMakerMouseMode(TM_EDIT_TILE_MODE);
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

void page_tiling_maker::slot_cellSelected(int row, int col)
{
    Q_UNUSED(row);
    PlacedTilePtr pfp = getTileColumn(col);
    tilingMaker->selectTile(pfp);
    Sys::tilingMakerView->forceRedraw();
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
        return;
    }

    PlacedTilePtr pfp = getTileColumn(col);
    if (!pfp) return;

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
    bool rv = pfp->saveAsGirihShape(vname);

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

    if (viewControl->isEnabled(VIEW_TILING_MAKER))
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
        if (viewControl->isEnabled(VIEW_TILING_MAKER))
        {
            Sys::tilingMakerView->forceRedraw();
        }
    }
}

void page_tiling_maker::tableHeaderClicked(int index)
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

void page_tiling_maker::slot_trim(qreal valX, qreal valY)
{
    TilingPtr tp = tilingMaker->getSelected();
    PlacedTiles ptiles;
    if ( config->tm_showExcludes)
        ptiles = tp->unit().getAll();
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

BQSpinBox::BQSpinBox(page_tiling_maker *parent, PlacedTilePtr ptp) : AQSpinBox()
{
    this->parent = parent;
    tile = ptp;
}

void  BQSpinBox::enterEvent(QEnterEvent *event)
{
    AQSpinBox::enterEvent(event);
    Sys::tilingMaker->selectTile(tile.lock());
    emit parent->sig_updateView();
}

BQDoubleSpinBox::BQDoubleSpinBox(page_tiling_maker *parent, PlacedTilePtr ptp) : AQDoubleSpinBox()
{
    this->parent = parent;
    tile = ptp;
}

void BQDoubleSpinBox::enterEvent(QEnterEvent *event)
{
    AQDoubleSpinBox::enterEvent(event);
    Sys::tilingMaker->selectTile(tile.lock());
    emit parent->sig_updateView();
}
