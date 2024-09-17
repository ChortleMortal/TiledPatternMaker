#include <QComboBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QHeaderView>
#include <QGroupBox>
#include <QMessageBox>
#include <QTextEdit>
#include <QMenu>

#include "gui/panels/page_tiling_maker.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/transform.h"
#include "model/makers/mosaic_maker.h"
#include  "gui/model_editors/tiling_edit/tile_selection.h"
#include "model/makers/tiling_maker.h"
#include "sys/sys/fileservices.h"
#include "sys/qt/utilities.h"
#include "gui/top/controlpanel.h"
#include "model/settings/configuration.h"
#include "model/styles/style.h"
#include "model/tilings/tile.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tiling.h"
#include "model/tilings/tiling_manager.h"
#include "gui/top/view_controller.h"
#include "gui/widgets/dlg_edgepoly_edit.h"
#include "gui/widgets/dlg_listnameselect.h"
#include "gui/widgets/dlg_listselect.h"
#include "gui/widgets/dlg_trim.h"
#include "gui/widgets/layout_sliderset.h"

typedef std::weak_ptr<Tiling>   WeakTilingPtr;

#if (QT_VERSION >= QT_VERSION_CHECK(6,7,0))
#define CBOX_STATECHANGE &QCheckBox::checkStateChanged
#else
#define CBOX_STATECHANGE &QCheckBox::stateChanged
#endif

using std::make_shared;

Q_DECLARE_METATYPE(WeakPlacedTilePtr)

#define TILE_TABLE_LIMIT 30

page_tiling_maker:: page_tiling_maker(ControlPanel * cpanel)  : panel_page(cpanel,PAGE_TILING_MAKER,"Tiling Maker")
{
    if (!config->insightMode)
    {
        setFixedWidth(701);
    }

    tmView = Sys::tilingMakerView;

    QHBoxLayout * controlLayout     = createControlRow();
    QGroupBox   * actionGroup       = createActionsGroup();
    QGroupBox   * modeGroup         = createModesGroup();
    QHBoxLayout * fillRow           = createFillDataRow();
    QTableWidget * table            = createTilingTable();
    translationsWidget              = createTranslationsRow();
    debugWidget                     = createDebugInfo();

    vbox->addLayout(controlLayout);
    vbox->addWidget(actionGroup);
    vbox->addWidget(modeGroup);
    vbox->addLayout(fillRow);
    vbox->addWidget(translationsWidget);
    vbox->addWidget(table);
    vbox->addWidget(debugWidget);
    vbox->addStretch();

    connect(mosaicMaker,  &MosaicMaker::sig_mosaicLoaded,  this, &page_tiling_maker::setup);
    connect(tilingMaker,  &TilingMaker::sig_buildMenu,     this, &page_tiling_maker::slot_buildMenu);
    connect(tilingMaker,  &TilingMaker::sig_refreshMenu,   this, &page_tiling_maker::slot_refreshMenuData);
    connect(tilingMaker,  &TilingMaker::sig_current_tile,  this, &page_tiling_maker::slot_currentTile);

    tileInfoTable->setVisible(!config->tm_hideTable);
    translationsWidget->setVisible(!config->tm_hideTranslations);
    debugWidget->setVisible(config->tm_showDebug);
}

QHBoxLayout * page_tiling_maker::createControlRow()
{
    loadedLabel                   = new QLabel("Loaded Tilings");
    tilingCombo                   = new QComboBox(this);
    tilingCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    QPushButton * dupTilingBtn    = new QPushButton("Duplicate Tiling");
    QPushButton * reloadTilingBtn = new QPushButton("Re-load Tiling");
    QPushButton * pbClearTiling   = new QPushButton("Clear Tiling");
    QCheckBox   * chkPropagate    = new QCheckBox("Propagate changes");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(loadedLabel);
    hbox->addWidget(tilingCombo);
    hbox->addStretch();
    hbox->addWidget(dupTilingBtn);
    hbox->addWidget(reloadTilingBtn);
    hbox->addWidget(pbClearTiling);
    hbox->addStretch();
    hbox->addWidget(chkPropagate);

    chkPropagate->setChecked(tilingMaker->getPropagate());

    connect(tilingCombo,       QOverload<int>::of(&QComboBox::currentIndexChanged),  this, &page_tiling_maker::slot_currentTilingChanged);
    connect(pbClearTiling,     &QPushButton::clicked,  this, &page_tiling_maker::slot_clearTiling);
    connect(reloadTilingBtn,   &QPushButton::clicked,  this, &page_tiling_maker::slot_reloadTiling);
    connect(dupTilingBtn,      &QPushButton::clicked,  this, &page_tiling_maker::slot_duplicateTiling);
    connect(chkPropagate,      &QCheckBox::clicked,    this, &page_tiling_maker::slot_propagate_changed);

    return hbox;
}

QGroupBox * page_tiling_maker::createActionsGroup()
{
    // actions
    QPushButton * addPolyBtn      = new QPushButton("Add Regular Polygon (A)");
    QPushButton * addGirihBtn     = new QPushButton("Add Girih Shape");
    fillVectorsChk  = new QCheckBox(  "Fill/Repeat (F)");
    QPushButton * removeExclBtn   = new QPushButton("Remove Excluded (R)");

    QPushButton * exportBtn       = new QPushButton("Export Tile");
    QPushButton * importBtn       = new QPushButton("Import Tile");

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

    QLabel * rlabel = new QLabel("Rot");

    QHBoxLayout * hbox1 = new QHBoxLayout();
    hbox1->addWidget(addPolyBtn);

    QHBoxLayout * hbox2 = new QHBoxLayout();
    hbox2->addWidget(sides);
    hbox2->addWidget(rlabel);
    hbox2->addWidget(tileRot);

    QVBoxLayout * vbox1 = new QVBoxLayout;
    vbox1->addLayout(hbox1);
    vbox1->addLayout(hbox2);

    QVBoxLayout * vbox2 = new QVBoxLayout;
    vbox2->addWidget(addGirihBtn);
    vbox2->addWidget(girihShapes);

    QVBoxLayout * vbox3 = new QVBoxLayout;
    vbox3->addWidget(fillVectorsChk);
    vbox3->addWidget(removeExclBtn);

    QVBoxLayout * vbox4 = new QVBoxLayout;
    vbox4->addWidget(importBtn);
    vbox4->addWidget(exportBtn);

    QHBoxLayout * hbox3 = new QHBoxLayout();
    hbox3->addLayout(vbox1);
    hbox3->addStretch();
    hbox3->addLayout(vbox2);
    hbox3->addStretch();
    hbox3->addLayout(vbox3);
    hbox3->addStretch();
    hbox3->addLayout(vbox4);

    QGroupBox * actionGroup = new QGroupBox("Actions");
    actionGroup->setLayout(hbox3);

    connect(addPolyBtn,     &QPushButton::clicked,          this, [this]() {tilingMaker->addRegularPolygon(); });
    connect(tileRot,        SIGNAL(valueChanged(qreal)),tilingMaker, SLOT(updatePolygonRot(qreal)));
    connect(sides,          SIGNAL(valueChanged(int)),  tilingMaker, SLOT(updatePolygonSides(int)));

    connect(exportBtn,      &QPushButton::clicked,            this,     &page_tiling_maker::slot_exportPoly);
    connect(importBtn,      &QPushButton::clicked,            this,     &page_tiling_maker::slot_importGirihPoly);
    connect(addGirihBtn,    &QPushButton::clicked,            this,     &page_tiling_maker::slot_addGirihShape);
    connect(fillVectorsChk, &QCheckBox::clicked,             this,   [this](bool clickstate) { tmView->setFill(clickstate); });
    connect(removeExclBtn,  &QPushButton::clicked,           this,   [this]()                { tilingMaker->removeExcludeds(); emit sig_updateView(); });

    return actionGroup;
}

QGroupBox * page_tiling_maker::createModesGroup()
{
    AQPushButton * nomode       = new AQPushButton("No Mode (ESC)");
    AQPushButton * drawTrans    = new AQPushButton("Set Repeat Points (F3)");
    AQPushButton * newPoly      = new AQPushButton("Draw Polygon (F4)");
    AQPushButton * copyPoly     = new AQPushButton("Copy Polygon (F5)");
    AQPushButton * deletePoly   = new AQPushButton("Delete Polygon (F6)");
    AQPushButton * includePoly  = new AQPushButton("Include/Exclude (F7)");
    AQPushButton * position     = new AQPushButton("Show Position (F8)");
    AQPushButton * measure      = new AQPushButton("Measure (F9)");
    AQPushButton * unify        = new AQPushButton("Unify (F10)");
    AQPushButton * editPoly     = new AQPushButton("Edit Tile (F11)");
    AQPushButton * mirrorX      = new AQPushButton("Mirror X");
    AQPushButton * mirrorY      = new AQPushButton("Mirror Y");
    AQPushButton * reflect      = new AQPushButton("Reflect Edge");
    AQPushButton * curveEdge    = new AQPushButton("Edit Curved Edge (F12)");
    AQPushButton * drawConst    = new AQPushButton("Draw Construction Lines");
    QCheckBox    * chkSnapTo    = new QCheckBox("Snap to Grid");
    statusLabel  = new QLabel();
    overlapStatus = new QLabel();
    overlapStatus->setStyleSheet("color: green");
    stackLabel  = new QLabel("Stack :");

    uint maxW = 61;
    QPushButton  * savStack    = new QPushButton("Save");
    savStack->setMaximumWidth(maxW);
    QPushButton  * undoStack   = new QPushButton("Undo");
    undoStack->setMaximumWidth(maxW);
    QPushButton  * redoStack   = new QPushButton("Redo");
    redoStack->setMaximumWidth(maxW);

    AQHBoxLayout * statusL = new AQHBoxLayout();
    statusL->addWidget(statusLabel);
    statusL->addStretch();
    statusL->addWidget(overlapStatus);
    statusL->addStretch();

    AQHBoxLayout * stackL  = new AQHBoxLayout();
    stackL->addWidget(stackLabel);
    stackL->addSpacing(3);
    stackL->addWidget(savStack);
    stackL->addSpacing(5);
    stackL->addWidget(undoStack);
    stackL->addSpacing(5);
    stackL->addWidget(redoStack);

    QGridLayout * modeBox = new QGridLayout();

    int row=0;
    modeBox->addWidget(nomode,row,0);
    modeBox->addWidget(drawTrans,row,1);
    modeBox->addWidget(newPoly,row,2);
    modeBox->addWidget(copyPoly,row,3);
    modeBox->addWidget(deletePoly,row,4);

    row++;
    modeBox->addWidget(includePoly,row,0);
    modeBox->addWidget(position,row,1);
    modeBox->addWidget(measure,row,2);
    modeBox->addWidget(unify,row,3);
    modeBox->addWidget(editPoly,row,4);

    row++;
    modeBox->addWidget(mirrorX,row,0);
    modeBox->addWidget(mirrorY,row,1);
    modeBox->addWidget(reflect,row,2);
    modeBox->addWidget(curveEdge,row,3);
    modeBox->addWidget(drawConst,row,4);

    row++;
    QHBoxLayout * hb = new QHBoxLayout;
    hb->addWidget(chkSnapTo);
    hb->addLayout(statusL);
    hb->addLayout(stackL);
    //modeBox->addWidget(chkSnapTo,row,0);
    //modeBox->addLayout(statusL,row,1,1,2);
    //modeBox->addLayout(stackL,row,3,1,2);
    modeBox->addLayout(hb,row,0,1,5);

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
    mouseModeBtnGroup->addButton(curveEdge,     TM_EDGE_CURVE_MODE);
    mouseModeBtnGroup->addButton(mirrorX,       TM_MIRROR_X_MODE);
    mouseModeBtnGroup->addButton(mirrorY,       TM_MIRROR_Y_MODE);
    mouseModeBtnGroup->addButton(reflect,       TM_REFLECT_EDGE);
    mouseModeBtnGroup->addButton(drawConst,     TM_CONSTRUCTION_LINES);

    lastChecked = mouseModeBtnGroup->button(TM_NO_MOUSE_MODE);
    lastChecked->setChecked(true);

    chkSnapTo->setChecked(config->snapToGrid);

    connect(chkSnapTo,  &QCheckBox::clicked,    this,   [this](bool checked) { config->snapToGrid =  checked;});
    connect(mouseModeBtnGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, &page_tiling_maker::slot_setModes);
    connect(savStack,   &QPushButton::clicked, tilingMaker, &TilingMaker::slot_stack_save);
    connect(undoStack,  &QPushButton::clicked, tilingMaker, &TilingMaker::slot_stack_undo);
    connect(redoStack,  &QPushButton::clicked, tilingMaker, &TilingMaker::slot_stack_redo);

    return modeGroup;
}

QHBoxLayout * page_tiling_maker::createFillDataRow()
{
    const int rmin = -99;
    const int rmax =  99;

    chkSingle      = new QCheckBox("Single");
    QCheckBox * chkHideVectors = new QCheckBox("Hide");
    QCheckBox * showTranslate        = new QCheckBox("Translations");

    xRepMin = new AQSpinBox();
    xRepMax = new AQSpinBox();
    yRepMin = new AQSpinBox();
    yRepMax = new AQSpinBox();

    xRepMin->setRange(rmin,rmax);
    xRepMax->setRange(rmin,rmax);
    yRepMin->setRange(rmin,rmax);
    yRepMax->setRange(rmin,rmax);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(new QLabel("Fill:"));
    hbox->addWidget(new QLabel("T1 Min"));
    hbox->addWidget(xRepMin);
    hbox->addWidget(new QLabel("T1 Max"));
    hbox->addWidget(xRepMax);
    hbox->addWidget(new QLabel("T2 Min"));
    hbox->addWidget(yRepMin);
    hbox->addWidget(new QLabel("T2 Max"));
    hbox->addWidget(yRepMax);
    hbox->addSpacing(5);
    hbox->addWidget(chkSingle);
    hbox->addWidget(chkHideVectors);
    hbox->addStretch();
    hbox->addWidget(showTranslate);
    if (config->insightMode)
    {
        QPushButton * swapBtn = new QPushButton("Swap");
        swapBtn->setMaximumWidth(51);
        hbox->addWidget(swapBtn);
        connect(swapBtn, &QPushButton::clicked,  this,   &page_tiling_maker::slot_swapTrans);
    }

    showTranslate->setChecked(!config->tm_hideTranslations);

    connect(chkSingle,      &QCheckBox::clicked, this, &page_tiling_maker::singleton_changed);
    connect(chkHideVectors, &QCheckBox::clicked, this, &page_tiling_maker::slot_hideVectors);
    connect(showTranslate,  &QCheckBox::clicked, this, &page_tiling_maker::slot_showTranslations);
    connect(xRepMin, SIGNAL(valueChanged(int)),  this, SLOT(slot_set_reps(int)));
    connect(xRepMax, SIGNAL(valueChanged(int)),  this, SLOT(slot_set_reps(int)));
    connect(yRepMin, SIGNAL(valueChanged(int)),  this, SLOT(slot_set_reps(int)));
    connect(yRepMax, SIGNAL(valueChanged(int)),  this, SLOT(slot_set_reps(int)));

    return hbox;
}

QWidget * page_tiling_maker::createTranslationsRow()
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

    connect(t1x,  &DoubleSpinSet::valueChanged, this, &page_tiling_maker::slot_t1t2Changed);
    connect(t1y,  &DoubleSpinSet::valueChanged, this, &page_tiling_maker::slot_t1t2Changed);
    connect(t2x,  &DoubleSpinSet::valueChanged, this, &page_tiling_maker::slot_t1t2Changed);
    connect(t2y,  &DoubleSpinSet::valueChanged, this, &page_tiling_maker::slot_t1t2Changed);

    connect(T1len,   &DoubleSpinSet::valueChanged, this, &page_tiling_maker::slot_t1t2LenChanged);
    connect(T2len,   &DoubleSpinSet::valueChanged, this, &page_tiling_maker::slot_t1t2LenChanged);
    connect(T1angle, &DoubleSpinSet::valueChanged, this, &page_tiling_maker::slot_t1t2AngleChanged);
    connect(T2angle, &DoubleSpinSet::valueChanged, this, &page_tiling_maker::slot_t1t2AngleChanged);

    translationWidget = new QWidget();
    translationWidget->setContentsMargins(0,0,0,0);
    translationWidget->setLayout(grid);

    return translationWidget;
}

QHBoxLayout * page_tiling_maker::createSecondControlRow()
{
    QHBoxLayout * hbox = new QHBoxLayout;

    QCheckBox    * chk_autoFill    = new QCheckBox("Fill on load");
    QRadioButton * showOverlapsRad = new QRadioButton("Show Overlaps/Touching");
    QRadioButton * showIncludedRad = new QRadioButton("Show Included/Excluded");

    chk_autoFill->setChecked(config->tm_loadFill);
    if (config->tm_showOverlaps)
        showOverlapsRad->setChecked(true);
    else
        showIncludedRad->setChecked(true);

    if (config->insightMode)
    {
        QCheckBox * chk_showTable     = new QCheckBox("Show Table");
        QCheckBox * chk_showExludes   = new QCheckBox("View Excluded");
                    chk_showDebug     = new QCheckBox("Show Debug");

        chk_showTable->setChecked(!config->tm_hideTable);
        chk_showExludes->setChecked(config->tm_showExcludes);
        chk_showDebug->setChecked(config->tm_showDebug);

        hbox->addWidget(chk_autoFill);
        hbox->addWidget(chk_showExludes);
        hbox->addWidget(showOverlapsRad);
        hbox->addWidget(showIncludedRad);
        hbox->addStretch();
        hbox->addWidget(chk_showTable);
        hbox->addWidget(chk_showDebug);

        connect(chk_showTable,   &QCheckBox::clicked,    this,   &page_tiling_maker::slot_showTable);
        connect(chk_showExludes, &QCheckBox::clicked,    this,   &page_tiling_maker::slot_showExludes);
        connect(chk_showDebug,   &QCheckBox::clicked,    this,   &page_tiling_maker::slot_showDebug);
    }
    else
    {
        config->tm_hideTable    = true;
        config->tm_showExcludes = false;
        config->tm_showDebug    = false;

        hbox->addWidget(chk_autoFill);
        hbox->addWidget(showOverlapsRad);
        hbox->addWidget(showIncludedRad);
        hbox->addStretch();
    }

    connect(chk_autoFill,    &QCheckBox::clicked,    this,   &page_tiling_maker::slot_autofill);
    connect(showOverlapsRad, &QRadioButton::clicked, tilingMaker, [this](bool checked) { config->tm_showOverlaps =  checked; emit sig_updateView(); });
    connect(showIncludedRad, &QRadioButton::clicked, tilingMaker, [this](bool checked) { config->tm_showOverlaps = !checked; emit sig_updateView(); });

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
    qslv << "tile" << "show" << "type"  << "tile-rot" << "tile-scale" << "tile-sides"<< "placed-scale" << "placed-rot" << "placed-X" << "placed-Y" << "CW" << "addr";
    tileInfoTable->setVerticalHeaderLabels(qslv);
    tileInfoTable->horizontalHeader()->setVisible(false);
    tileInfoTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    tileInfoTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QHeaderView * qhv = tileInfoTable->verticalHeader();

    connect(tileInfoTable,   &QTableWidget::customContextMenuRequested, this, &page_tiling_maker::slot_menu);
    connect(qhv,             &QHeaderView::sectionClicked, this, &page_tiling_maker::tableHeaderClicked);

    return tileInfoTable;
}

QWidget * page_tiling_maker::createDebugInfo()
{
    ///  Debug Status    ///
    debugLabel1  = new QLabel;
    debugLabel2  = new QLabel;

    tileInfo  = new QTextEdit;
    tileInfo->setFixedHeight(49);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(debugLabel1);
    hbox->addSpacing(9);
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




/////////////////////////////////////
///
///  Common slots
///
/////////////////////////////////////

QString  page_tiling_maker::getPageStatus()
{
    return QString("<body>"
                   "<span style=\"color:rgb(205,102, 25)\">overlapping</span>  |  "
                   "<span style=\"color:rgb( 25,102,205)\">touching</span> |  "
                   "<span style=\"color:rgb(255,217,217)\">included</span>  |  "
                   "<span style=\"color:rgb(217,217,255)\">excluded</span>  |  "
                   "<span style=\"color:rgb(127,255,127)\">under-mouse</span>  |  "
                   "<span style=\"color:rgb(  0,255,  0)\">selected</span>  |  "
                   "<span style=\"color:rgb(206,179,102)\">dragging</span>"
                   "</body>");
}

void  page_tiling_maker::onEnter()
{
    setup();
}

void page_tiling_maker::setup()
{
    tallyMouseMode();

    sides->setValue(tilingMaker->getPolygonSides());

    blockPage(true);

    tileInfo->clear();
    tileInfoTable->clearContents();     // done in build menu

    t1x->setValue(0);
    t1y->setValue(0);
    t2x->setValue(0);
    t2y->setValue(0);

    blockPage(false);

    TilingPtr tiling = tilingMaker->getSelected();
    loadTilingCombo(tiling);
    tiling->resetOverlaps();
    buildMenu();

    tilingMaker->setTilingMakerMouseMode(TM_NO_MOUSE_MODE);
    tilingMaker->clearConstructionLines();

    if (viewControl->isEnabled(VIEW_TILING_MAKER))
    {
        tmView->forceRedraw();
    }
}

void  page_tiling_maker::onRefresh()
{
    static WeakTilingPtr wtp;
    static eTilingMakerMouseMode oldMode = TM_NO_MOUSE_MODE;

    TilingPtr tp = tilingMaker->getSelected();
    if (wtp.lock() != tp)
    {
        wtp = tp;
        setup();
    }

    QString txt;
    eTilingMakerMouseMode currentMode = tilingMaker->getTilingMakerMouseMode();
    if (currentMode != oldMode)
    {
        switch (oldMode)
        {
        case TM_INCLUSION_MODE:
        case TM_DRAW_POLY_MODE:
        case TM_TRANSLATION_VECTOR_MODE:
            panel->setStatus(getPageStatus());
            break;
        default:
            break;
        }

        switch (currentMode)
        {
        case TM_INCLUSION_MODE:
            txt = "Toggle the inclusion of polygons in the tiling by clicking on them with the mouse.";
            panel->setStatus(txt);
            break;
        case TM_DRAW_POLY_MODE:
            txt = "Select a series of vertices clockwise to draw a free-form polygon. (Click on vertices).";
            panel->setStatus(txt);
            break;
        case TM_TRANSLATION_VECTOR_MODE:
            txt = "Use mouse, left-click on a polygon center or vertex, drag to the repititon point. Do this twice for two directions.";
            panel->setStatus(txt);
            break;
        default:
            break;
        }

        oldMode = currentMode;
    }

    // debug info
    if (config->insightMode && chk_showDebug->isChecked())
    {
        QString dbgStatus = tilingMaker->getStatus();
        debugLabel1->setText(dbgStatus);

        QPointF a = tmView->getMousePos();
        QPointF b = tmView->screenToModel(a);
        QString astring;
        QTextStream ts(&astring);
        ts << "pos: (" << b.x() << ", " << b.y() << ")";

        PlacedTileSelectorPtr tsp = tilingMaker->getCurrentSelection();
        if (tsp)
        {
            QPointF c = tsp->getModelPoint();
            ts << "  sel: (" << c.x() << ", " << c.y() << ")";
        }
        debugLabel2->setText(astring);

        astring.clear();
        auto selector = tmView->getTileSelector();
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

        QString sl = "Stack :" + tilingMaker->getStack().getStackStatus();
        stackLabel->setText(sl);
    }

    // always visible status line
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

    QString status = QString("Tiles: %1  Unique: %2  Excluded: %3").arg(tp->numIncluded()).arg(tp->numUnique()).arg(tp->numExcluded());
    statusLabel->setText(status);

    tallyMouseMode();

    fillVectorsChk->setChecked(Sys::tm_fill);
}

bool page_tiling_maker::canExit()
{
    QString txt;
    QString info;
    TilingPtr tp = tilingMaker->getSelected();
    if (tp->numAll())
    {
        if (tp->numIncluded() == 0)
        {
            txt  = "There are no polygons included in the tiling";
            info = "To include polygons, press Cancel and 'Include' the polygons you want in the tiling";
        }
        else
        {
            const FillData & fd = tp->getCanvasSettings().getFillData();
            int minX, maxX, minY, maxY;
            bool singleton;
            fd.get(singleton,minX, maxX, minY, maxY);

            if (!singleton)
            {
                QPointF t1    = tp->getData().getTrans1();
                QPointF t2    = tp->getData().getTrans2();

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
    viewControl->removeAllImages();

    TilingPtr tp = tilingMaker->getSelected();
    tilingMaker->removeTiling(tp);  // re-creates if becomes empty
    emit sig_reconstructView();

    setup();
    //view->dump(true);
}

void page_tiling_maker::slot_reloadTiling()
{
    eTILM_Event event    = TILM_RELOAD;
    TilingPtr oldTiling  = tilingMaker->getSelected();
    VersionedName vname  = oldTiling->getName();
    VersionedFile vfile  = FileServices::getFile(vname,FILE_TILING);

    TilingManager tm;
    TilingPtr newTiling = tm.loadTiling(vfile,event);
    if (!newTiling)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText(QString("Could not load <%1>").arg(vname.get()));
        box.setInformativeText("Reload failed");
        box.exec();
        return;
    }

    emit tilingMaker->sig_tilingLoaded(vfile);
    emit sig_reconstructView();
}

void page_tiling_maker::slot_duplicateTiling()
{
    tilingMaker->duplicateSelectedTiling();

    QMessageBox box(this);
    box.setText("Duplicate added");
    box.exec();
}

/////////////////////////////////////
///
///  Build Menu
///
/////////////////////////////////////

void page_tiling_maker::slot_buildMenu()
{
    buildMenu();
}

void page_tiling_maker::buildMenu()
{
    blockPage(true);

    //qDebug() << "page_tiling_maker::buildMenu";
    tileInfoTable->clearContents();
    tileInfoTable->setColumnCount(0);

    int col = 0;
    TilingPtr tp = tilingMaker->getSelected();
    Q_ASSERT(tp);
    bool showAll = (config->tm_showExcludes && (tp->numAll() < TILE_TABLE_LIMIT));   // throttle show all for large number pf tiles
    const TilingPlacements viewable = tp->getTilingUnitPlacements(showAll);

    for (const auto & pfp : std::as_const(viewable))
    {
        QString inclusion = QString("%1 (%2)").arg(pfp->isIncluded() ? "Included" : "Excluded").arg(col);
        buildTableEntry(pfp,col++,inclusion);
    }

    connect(tileInfoTable, SIGNAL(cellClicked(int,int)),  this, SLOT(slot_cellSelected(int,int)),Qt::UniqueConnection);

    for (int i=0; i < tileInfoTable->columnCount(); i++)
    {
        tileInfoTable->resizeColumnToContents(i);
    }
    tileInfoTable->adjustTableSize(750);

    updateGeometry();

    tileInfoTable->setVisible(!config->tm_hideTable);

    refreshMenuData();

    blockPage(false);
}

void page_tiling_maker::slot_refreshMenuData()
{
    refreshMenuData();
}

void page_tiling_maker::refreshMenuData()
{
    if (!panel->isVisiblePage(this))
    {
        return;
    }

    //qDebug() << "page_tiling_maker::refreshMenuData";

    blockPage(true);

    TilingPtr tiling = tilingMaker->getSelected();

    QPointF t1 = tiling->getData().getTrans1();
    QPointF t2 = tiling->getData().getTrans2();

    QLineF tl1(QPointF(0.0,0.0),t1);
    QLineF tl2(QPointF(0.0,0.0),t2);

    int xMin,xMax,yMin,yMax;
    bool singleton;
    const FillData & fd = tiling->getCanvasSettings().getFillData();
    fd.get(singleton, xMin ,xMax,yMin,yMax);

    blockSignals(true);

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
    }
    else
    {
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

    const TilingPlacements & tu = tiling->getTilingUnitPlacements(config->tm_showExcludes);
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

    blockPage(false);

    tallyMouseMode();
}

void page_tiling_maker::buildTableEntry(PlacedTilePtr pf, int col, QString inclusion)
{
    TilePtr tile  = pf->getTile();
    QTransform T  = pf->getTransform();

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
    connect(cb, CBOX_STATECHANGE, this, [this,col] { slot_showTileChanged(col); });

    AQSpinBox * sp = new AQSpinBox;
    sp->setValue(tile->numPoints());
    sp->setReadOnly(!tile->isRegular());
    tileInfoTable->setCellWidget(TI_TILE_SIDES,col,sp);
    connect(sp,static_cast<void (AQSpinBox::*)(int)>(&AQSpinBox::valueChanged), this,
            [this,col] { slot_sidesChanged(col); });

    AQDoubleSpinBox *dsp = new AQDoubleSpinBox;
    dsp->setRange(-360.0,360.0);
    dsp->setDecimals(16);
    dsp->setAlignment(Qt::AlignRight);
    dsp->setValue(tile->getRotation());
    tileInfoTable->setCellWidget(TI_TILE_ROT,col,dsp);
    connect(dsp, static_cast<void (AQDoubleSpinBox::*)(double)>(&AQDoubleSpinBox::valueChanged), this,
            [this,col] { slot_tileRotChanged(col); });

    dsp = new AQDoubleSpinBox;
    dsp->setRange(0,128);
    dsp->setDecimals(16);
    dsp->setSingleStep(0.01);
    dsp->setAlignment(Qt::AlignRight);
    dsp->setValue(tile->getScale());
    tileInfoTable->setCellWidget(TI_TILE_SCALE,col,dsp);
    connect(dsp, static_cast<void (AQDoubleSpinBox::*)(double)>(&AQDoubleSpinBox::valueChanged), this,
            [this,col] { slot_tileScaleChanged(col); });

    dsp = new AQDoubleSpinBox;
    dsp->setRange(0,128);
    dsp->setDecimals(16);
    dsp->setSingleStep(0.01);
    dsp->setAlignment(Qt::AlignRight);
    dsp->setValue(Transform::scalex(T));
    tileInfoTable->setCellWidget(TI_PLACEMENT_SCALE,col,dsp);
    connect(dsp, static_cast<void (AQDoubleSpinBox::*)(double)>(&AQDoubleSpinBox::valueChanged), this,
            [this,col] { slot_placedScaleChanged(col); });

    dsp = new AQDoubleSpinBox;
    dsp->setRange(-360.0,360.0);
    dsp->setDecimals(16);
    dsp->setAlignment(Qt::AlignRight);
    dsp->setValue(qRadiansToDegrees(Transform::rotation(T)));
    tileInfoTable->setCellWidget(TI_PLACEMENT_ROT,col,dsp);
    connect(dsp, static_cast<void (AQDoubleSpinBox::*)(double)>(&AQDoubleSpinBox::valueChanged), this,
            [this,col] { slot_placedRotateChanged(col); });

    dsp = new AQDoubleSpinBox;
    dsp->setRange(-100.0,100.0);
    dsp->setDecimals(16);
    dsp->setSingleStep(0.1);
    dsp->setAlignment(Qt::AlignRight);
    dsp->setValue(Transform::transx(T));
    tileInfoTable->setCellWidget(TI_PLACEMENT_X,col,dsp);
    connect(dsp,static_cast<void (AQDoubleSpinBox::*)(double)>(&AQDoubleSpinBox::valueChanged), this,
            [this,col] { slot_placedTranslateChanged(col); });

    dsp = new AQDoubleSpinBox;
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
    QTransform T  = pf->getTransform();

    QString type;
    if (tile->isRegular())
        type = "Regular";
    else
        type = "Irregular";
    QTableWidgetItem * twi = new QTableWidgetItem(type);
    twi->setData(Qt::UserRole,QVariant::fromValue(WeakPlacedTilePtr(pf)));
    tileInfoTable->setItem(TI_TYPE_PFP,col,twi);

    QWidget * w = tileInfoTable->cellWidget(TI_TILE_SIDES,col);
    if (!w)
    {
        qDebug() << "table col not found";
        return;
    }
    AQSpinBox * sp = dynamic_cast<AQSpinBox*>(w);
    Q_ASSERT(sp);
    sp->setValue(tile->numPoints());

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
    TilePtr tile     = pf->getTile();

    tile->setN(sides);

    if (pf->isIncluded())
    {
        tilingMaker->pushTileToPrototypeMaker(PROM_TILE_NUM_SIDES_CHANGED,tile);
    }

    refreshMenuData();

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

    TilePtr tile  = pf->getTile();
    tile->setRotation(rotation);

    if (pf->isIncluded())
    {
        tilingMaker->pushTileToPrototypeMaker(PROM_TILE_ROTATION_CHANGED,tile);
    }

    refreshMenuData();

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

    TilePtr tile  = pf->getTile();
    tile->setScale(scale);

    if (pf->isIncluded())
    {
        tilingMaker->pushTileToPrototypeMaker(PROM_TILE_SCALE_CHANGED,tile);
    }

    refreshMenuData();

    emit sig_reconstructView();
}

void page_tiling_maker::slot_placedTranslateChanged(int col)
{
    qWarning() << "slot_placedTranslateChanged";
    if (pageBlocked()) return;

    PlacedTilePtr placedTile = getTileColumn(col);
    tilingMaker->setCurrentPlacedTile(placedTile);

    auto widget = tileInfoTable->cellWidget(TI_PLACEMENT_X,col);
    auto dsp    = dynamic_cast<AQDoubleSpinBox*>(widget);
    Q_ASSERT(dsp);
    qreal tx = dsp->value();

    widget = tileInfoTable->cellWidget(TI_PLACEMENT_Y,col);
    dsp    = dynamic_cast<AQDoubleSpinBox*>(widget);
    Q_ASSERT(dsp);
    qreal ty = dsp->value();

    tilingMaker->placedTileSetTranslate(tx,ty);

    if (placedTile->isIncluded())
    {
        tilingMaker->pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
    }

    emit sig_reconstructView();
}

void page_tiling_maker::slot_placedScaleChanged(int col)
{
    qWarning() << "slot_placedScaleChanged col=" << col;
    if (pageBlocked()) return;

    PlacedTilePtr placedTile = getTileColumn(col);
    tilingMaker->setCurrentPlacedTile(placedTile);

    QWidget  * cw  = tileInfoTable->cellWidget(TI_PLACEMENT_SCALE,col);
    AQDoubleSpinBox * dsp = dynamic_cast<AQDoubleSpinBox*>(cw);
    Q_ASSERT(dsp);
    qreal scale = dsp->value();
    if (scale <= 0.0 || scale > 128.0)
    {
        return;     // fixes problem with scales of 0 when entering data
    }

    tilingMaker->placedTileSetScale(scale);

    if (placedTile->isIncluded())
    {
        tilingMaker->pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
    }

    emit sig_updateView();
}

void page_tiling_maker::slot_placedRotateChanged(int col)
{
    qDebug() << "slot_placedRotateChanged";
    if (pageBlocked()) return;

    PlacedTilePtr placedTile = getTileColumn(col);
    tilingMaker->setCurrentPlacedTile(placedTile);

    auto rot_widget = tileInfoTable->cellWidget(TI_PLACEMENT_ROT,col);
    auto rot_dsp = dynamic_cast<AQDoubleSpinBox*>(rot_widget);
    Q_ASSERT(rot_dsp);
    qreal cell_rotation = rot_dsp->value();

    tilingMaker->placedTileSetRotate(cell_rotation);

    if (placedTile->isIncluded())
    {
        tilingMaker->pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
    }

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

    tile->setShow(checked);

    tmView->forceRedraw();
}

void page_tiling_maker::slot_set_reps(int val)
{
    Q_UNUSED(val);

    if (pageBlocked()) return;

    TilingPtr tiling = tilingMaker->getSelected();
    if (!tiling) return;
    
    FillData fdata;
    fdata.set(chkSingle->isChecked(),xRepMin->value(), xRepMax->value(), yRepMin->value(), yRepMax->value());

    CanvasSettings cs = tiling->getCanvasSettings();
    cs.setFillData(fdata);
    tiling->setCanvasSettings(cs);

    tilingMaker->updateReps();
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

    CanvasSettings cs = tiling->getCanvasSettings();
    cs.setFillData(fd);
    tiling->setCanvasSettings(cs);

    tilingMaker->updateReps();

    refreshMenuData();
}

void page_tiling_maker::slot_propagate_changed(bool checked)
{
    tilingMaker->setPropagate(checked);
    if (checked)
    {
        auto tiling = tilingMaker->getSelected();
        tilingMaker->sm_takeUp(tiling,TILM_RELOAD);
    }
}

void page_tiling_maker::slot_hideVectors(bool checked)
{
    tmView->hideVectors(checked);
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
    tiling->setTranslationVectors(QPointF(x1,y1), QPointF(x2,y2), tiling->getTranslateOrigin());

    tilingMaker->updateVectors();

    refreshMenuData();
}

void page_tiling_maker::slot_t1t2LenChanged(double val)
{
    Q_UNUSED(val)

    if (pageBlocked()) return;

    TilingPtr tiling = tilingMaker->getSelected();

    QPointF t1 = tiling->getData().getTrans1();
    QPointF t2 = tiling->getData().getTrans2();

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

    tiling->setTranslationVectors(QPointF(x1,y1),QPointF(x2,y2),tiling->getTranslateOrigin());

    tilingMaker->updateVectors();

    refreshMenuData();
}

void page_tiling_maker::slot_t1t2AngleChanged(double val)
{
    Q_UNUSED(val)

    if (pageBlocked()) return;

    TilingPtr tiling = tilingMaker->getSelected();

    QPointF t1 = tiling->getData().getTrans1();
    QPointF t2 = tiling->getData().getTrans2();

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

    tiling->setTranslationVectors(QPointF(x1,y1),QPointF(x2,y2),tiling->getTranslateOrigin());

    tilingMaker->updateVectors();

    refreshMenuData();
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

void page_tiling_maker::loadTilingCombo(TilingPtr selected)
{
    tilingCombo->blockSignals(true);
    tilingCombo->clear();
    const QVector<TilingPtr> & tilings = tilingMaker->getTilings();
    for (auto& tiling : std::as_const(tilings))
    {
        tilingCombo->addItem(tiling->getName().get());
    }
    int index = tilingCombo->findText(selected->getName().get());
    tilingCombo->setCurrentIndex(index);
    tilingCombo->blockSignals(false);

    loadedLabel->setText(QString("Loaded Tilings (%1) ").arg(tilingCombo->count()));
}

void page_tiling_maker::slot_currentTilingChanged(int index)
{
    Q_UNUSED(index);
    QString name = tilingCombo->currentText();
    TilingPtr tp = tilingMaker->findTilingByName(name);

    tilingMaker->select(tp);

    setup();
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

void page_tiling_maker::slot_showTable(bool checked)
{
    config->tm_hideTable = !checked;
    tileInfoTable->setVisible(checked);
    buildMenu();
}

void page_tiling_maker::slot_showExludes(bool checked)
{
    config->tm_showExcludes = checked;
    buildMenu();
}

void page_tiling_maker::slot_showDebug(bool checked)
{
    config->tm_showDebug = checked;
    debugWidget->setVisible(checked);
    if (!checked)
    {
        stackLabel->setText("Stack :");
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

void page_tiling_maker::slot_menu(QPointF spt)
{
    qDebug() << "menu spt=" << spt;

    int col = tileInfoTable->currentColumn();
    PlacedTilePtr pfp = getTileColumn(col);

    QString astr = QString("Side=%1").arg(pfp->getTile()->edgeLen(),0,'g',16);
    qDebug().noquote() << astr;

    QString bstr =  (pfp->isIncluded())  ? "Exclude" : "Include";

    QMenu myMenu(tileInfoTable);

    myMenu.addAction("Edit Tile", this, &page_tiling_maker::slot_menu_edit_tile);
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

void page_tiling_maker::slot_menu_edit_tile()
{
    qDebug() << "page_tiling_maker::slot_menu_edit_tile";
    int           col = tileInfoTable->currentColumn();
    PlacedTilePtr pfp = getTileColumn(col);
    TilePtr      tile = pfp->getTile();
    QTransform      t = pfp->getTransform();

    DlgEdgePolyEdit * fe  = new DlgEdgePolyEdit(tile,t);
    fe->show();
    fe->raise();
    fe->activateWindow();

    auto tmView = Sys::tilingMakerView;
    connect(fe,          &DlgEdgePolyEdit::sig_currentPoint, tmView, &TilingMakerView::slot_setTileEditPoint);
    connect(tilingMaker, &TilingMaker::sig_refreshMenu,      fe,     &DlgEdgePolyEdit::display);
}

void page_tiling_maker::slot_menu_includePlaced()
{
    int col = tileInfoTable->currentColumn();
    PlacedTilePtr pfp = getTileColumn(col);
    
    PlacedTileSelectorPtr tsp = make_shared<InteriorTilleSelector>(pfp);
    tilingMaker->toggleInclusion(tsp);    
}

void page_tiling_maker::slot_delete_clicked()
{
    int col = tileInfoTable->currentColumn();
    PlacedTilePtr pf = getTileColumn(col);
    tilingMaker->deletePlacedTile(pf);
    buildMenu();
}

void page_tiling_maker::slot_convert_tile_regularity()
{
    int col          = tileInfoTable->currentColumn();
    PlacedTilePtr pf = getTileColumn(col);
    TilePtr tile     = pf->getTile();

    tilingMaker->flipTileRegularity(tile);
    buildMenu();
}

void page_tiling_maker::slot_uniquify_clicked()
{
    int col = tileInfoTable->currentColumn();
    PlacedTilePtr pf = getTileColumn(col);
    TilePtr fp = pf->getTile();
    TilePtr fp2 = fp->recreate();  // creates a new tile same as other
    pf->setTile(fp2);

    tilingMaker->pushTilingToPrototypeMaker(PROM_TILES_ADDED);
    buildMenu();
}

void page_tiling_maker::slot_cellSelected(int row, int col)
{
    Q_UNUSED(row)
    PlacedTilePtr pfp = getTileColumn(col);
    tilingMaker->setCurrentPlacedTile(pfp);

    auto selector = make_shared<InteriorTilleSelector>(pfp);
    tmView->setTileSelector(selector);
    tmView->forceRedraw();
}

QString page_tiling_maker::getTileInfo(PlacedTilePtr pfp)
{
    TilePtr          fp  = pfp->getTile();
    QPolygonF      poly  = fp->getPoints();
    QTransform        t  = pfp->getTransform();
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

void page_tiling_maker::slot_currentTile(PlacedTilePtr pfp)
{
    int col = -1;
    if (pfp)
    {
        col = getColumn(pfp);
    }

    if (col == -1)
    {
        // deselect
        tileInfoTable->clearSelection();
        tileInfo->clear();
    }
    else
    {
        // de-select
        tileInfoTable->setFocus();
        tileInfoTable->selectColumn(col);
    }
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
            tilingMaker->addNewPlacedTile(pfp);
        }
    }

    buildMenu();

    if (viewControl->isEnabled(VIEW_TILING_MAKER))
    {
        tmView->forceRedraw();
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
        tilingMaker->addNewPlacedTile(pfp);
        buildMenu();
        if (viewControl->isEnabled(VIEW_TILING_MAKER))
        {
            tmView->forceRedraw();
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
    const TilingPlacements ptiles = tp->getTilingUnitPlacements(config->tm_showExcludes);
    for (const auto & pfp : std::as_const(ptiles))
    {
        QTransform t1 = pfp->getTransform();
        QTransform t2 = QTransform::fromTranslate(valX,valY);
        pfp->setTransform(t1*t2);
    }
    tmView->forceRedraw();
    refreshMenuData();
}
