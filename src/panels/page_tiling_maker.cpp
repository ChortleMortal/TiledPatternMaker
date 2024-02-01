#include <QComboBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QHeaderView>
#include <QGroupBox>
#include <QMessageBox>
#include <QTextEdit>
#include <QMenu>

#include "panels/page_tiling_maker.h"
#include "geometry/transform.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "makers/tiling_maker/tile_selection.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "misc/fileservices.h"
#include "misc/utilities.h"
#include "panels/controlpanel.h"
#include "settings/configuration.h"
#include "style/style.h"
#include "tile/tile.h"
#include "tile/placed_tile.h"
#include "tile/tiling.h"
#include "tile/tiling_manager.h"
#include "viewers/view_controller.h"
#include "widgets/dlg_edgepoly_edit.h"
#include "widgets/dlg_listnameselect.h"
#include "widgets/dlg_listselect.h"
#include "widgets/dlg_trim.h"
#include "widgets/layout_sliderset.h"

typedef std::weak_ptr<Tiling>   WeakTilingPtr;

using std::make_shared;

Q_DECLARE_METATYPE(WeakPlacedTilePtr)

page_tiling_maker:: page_tiling_maker(ControlPanel * cpanel)  : panel_page(cpanel,PAGE_TILING_MAKER,"Tiling Maker")
{
    tmView = TilingMakerView::getInstance();

    QHBoxLayout * controlLayout     = createControlRow();
    QGroupBox   * actionGroup       = createActionsGroup();
    QGroupBox   * modeGroup         = createModesGroup();
    QHBoxLayout * fillRow           = createFillDataRow();
    QHBoxLayout * tableControl      = createTableControlRow();
    QTableWidget * table            = createTilingTable();
    translationsWidget              = createTranslationsRow();
    debugWidget                     = createDebugInfo();

    vbox->addLayout(controlLayout);
    vbox->addWidget(actionGroup);
    vbox->addWidget(modeGroup);
    vbox->addLayout(fillRow);
    vbox->addLayout(tableControl);
    vbox->addWidget(translationsWidget);
    vbox->addWidget(table);
    vbox->addWidget(debugWidget);
    vbox->addStretch();

    connect(mosaicMaker,  &MosaicMaker::sig_mosaicLoaded,  this, &page_tiling_maker::setup);
    connect(tilingMaker,  &TilingMaker::sig_buildMenu,     this, &page_tiling_maker::slot_buildMenu);
    connect(tilingMaker,  &TilingMaker::sig_refreshMenu,   this, &page_tiling_maker::slot_refreshMenuData);
    connect(tilingMaker,  &TilingMaker::sig_current_tile,  this, &page_tiling_maker::slot_currentTile);

    tileInfoTable->setVisible(!config->tm_hideTable);
    translationsWidget->setVisible(!config->tm_hideTable);
    debugWidget->setVisible(config->tm_showDebug);
}

QHBoxLayout * page_tiling_maker::createControlRow()
{
    loadedLabel                   = new QLabel("Loaded Tilings");
    tilingCombo                   = new QComboBox();
    tilingCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    QPushButton * reloadTilingBtn = new QPushButton("Re-load Tiling");
    QPushButton * pbClearTiling   = new QPushButton("Clear Tiling");
    QCheckBox * chkPropagate      = new QCheckBox("Propagate changes");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(loadedLabel);
    hbox->addWidget(tilingCombo);
    hbox->addStretch();
    hbox->addWidget(reloadTilingBtn);
    hbox->addWidget(pbClearTiling);
    hbox->addStretch();
    hbox->addWidget(chkPropagate);

    chkPropagate->setChecked(tilingMaker->getPropagate());

    connect(tilingCombo,       SIGNAL(currentIndexChanged(int)),  this, SLOT(slot_currentTilingChanged(int)));
    connect(pbClearTiling,     &QPushButton::clicked,  this, &page_tiling_maker::slot_clearTiling);
    connect(reloadTilingBtn,   &QPushButton::clicked,  this, &page_tiling_maker::slot_reloadTiling);
    connect(chkPropagate,      &QCheckBox::clicked,    this, &page_tiling_maker::slot_propagate_changed);

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

    QWidget * widget = new QWidget();
    widget->setContentsMargins(0,0,0,0);
    widget->setLayout(grid);

    return widget;
}

QHBoxLayout * page_tiling_maker::createFillDataRow()
{
    QHBoxLayout * hbox = new QHBoxLayout;

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

    hbox->addWidget(chkSingle);
    hbox->addSpacing(5);
    hbox->addWidget(new QLabel("Repetitions:  xMin"));
    hbox->addWidget(xRepMin);
    hbox->addWidget(new QLabel("xMax"));
    hbox->addWidget(xRepMax);
    hbox->addWidget(new QLabel("yMin"));
    hbox->addWidget(yRepMin);
    hbox->addWidget(new QLabel("yMax"));
    hbox->addWidget(yRepMax);
    hbox->addStretch();
    if (config->insightMode)
    {
        QPushButton * swapBtn = new QPushButton("Swap T1/T2");
        hbox->addWidget(swapBtn);
        connect(swapBtn, &QPushButton::clicked,  this,   &page_tiling_maker::slot_swapTrans);
    }
    hbox->addStretch();

    connect(chkSingle, &QCheckBox::clicked, this, &page_tiling_maker::singleton_changed);
    connect(xRepMin, SIGNAL(valueChanged(int)), this,  SLOT(slot_set_reps(int)));
    connect(xRepMax, SIGNAL(valueChanged(int)), this,  SLOT(slot_set_reps(int)));
    connect(yRepMin, SIGNAL(valueChanged(int)), this,  SLOT(slot_set_reps(int)));
    connect(yRepMax, SIGNAL(valueChanged(int)), this,  SLOT(slot_set_reps(int)));

    return hbox;
}

QHBoxLayout * page_tiling_maker::createTableControlRow()
{
    QHBoxLayout * hbox = new QHBoxLayout;

    QCheckBox    * chk_autoFill    = new QCheckBox("Auto-fill on load");
    QRadioButton * showOverlapsRad = new QRadioButton("Show overlaps/touching");
    QRadioButton * showIncludedRad = new QRadioButton("Show included/excluded");

    if (config->insightMode)
    {
        QCheckBox * chk_showTable     = new QCheckBox("Table");
        chk_showTable->setChecked(!config->tm_hideTable);

        QCheckBox * chk_allTiles      = new QCheckBox("Show excluded");
                    chk_showDebug     = new QCheckBox("Debug");
        chk_allTiles->setChecked(config->tm_showAllTiles);

        chk_allTiles->setChecked(config->tm_showAllTiles);
        chk_showDebug->setChecked(config->tm_showDebug);

        hbox->addWidget(chk_showTable);
        hbox->addWidget(chk_autoFill);
        hbox->addWidget(chk_allTiles);
        hbox->addWidget(showOverlapsRad);
        hbox->addWidget(showIncludedRad);
        hbox->addWidget(chk_showDebug);
        hbox->addStretch();

        connect(chk_showTable,   &QCheckBox::clicked,    this,   &page_tiling_maker::slot_showTable);
        connect(chk_allTiles    ,&QCheckBox::clicked,    this,   &page_tiling_maker::slot_all_tiles);
        connect(chk_showDebug,   &QCheckBox::clicked,    this,   &page_tiling_maker::slot_showDebug);
    }
    else
    {
        config->tm_hideTable    = true;
        config->tm_showAllTiles = false;
        config->tm_showDebug    = false;

        hbox->addWidget(chk_autoFill);
        hbox->addWidget(showOverlapsRad);
        hbox->addWidget(showIncludedRad);
        hbox->addStretch();
    }

    chk_autoFill->setChecked(config->tm_autofill);
    if (config->tm_showOverlaps)
        showOverlapsRad->setChecked(true);
    else
        showIncludedRad->setChecked(true);

    connect(chk_autoFill,    &QCheckBox::clicked,    this,   &page_tiling_maker::slot_autofill);
    connect(showOverlapsRad, &QRadioButton::clicked, tilingMaker, [this](bool checked) { config->tm_showOverlaps =  checked; view->update(); });
    connect(showIncludedRad, &QRadioButton::clicked, tilingMaker, [this](bool checked) { config->tm_showOverlaps = !checked; view->update(); });

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
    QHBoxLayout * debhbox = new QHBoxLayout;
    debhbox->addWidget(debugLabel1);
    debhbox->addSpacing(9);
    debhbox->addWidget(debugLabel2);
    debhbox->addStretch();
    QVBoxLayout * debvbox = new QVBoxLayout;
    debvbox->addSpacing(5);
    debvbox->addWidget(tileInfo);
    debvbox->addLayout(debhbox);
    debvbox->addSpacing(7);

    debugWidget = new QWidget();
    debugWidget->setContentsMargins(0,0,0,0);
    debugWidget->setLayout(debvbox);

    return debugWidget;
}

QGroupBox * page_tiling_maker::createActionsGroup()
{
    // actions
    BQPushButton * addPolyBtn      = new BQPushButton("Add Regular Polygon (A)");
    BQPushButton * addGirihBtn     = new BQPushButton("Add Girih Shape");
    BQPushButton * fillVectorsBtn  = new BQPushButton("Fill using Repeat Points (F)");
    BQPushButton * removeExclBtn   = new BQPushButton("Remove Excluded (R)");

    BQPushButton * exportBtn       = new BQPushButton("Export Tile");
    BQPushButton * importBtn       = new BQPushButton("Import Tile");

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
    girihShapes->setStyleSheet("padding-left: 35px;");
    girihShapes->addItem("Decagon", "gDecagon");
    girihShapes->addItem("Pentagon","gPentagon");
    girihShapes->addItem("Bowtie",  "gBowtie");
    girihShapes->addItem("Kite",    "gKite");
    girihShapes->addItem("Rhombus", "gRhombus");

    QLabel * rlabel = new QLabel("Rot");

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(sides);
    hbox->addWidget(rlabel);
    hbox->addWidget(tileRot);


    QGridLayout  * actionBox = new QGridLayout;

    int row = 0;
    actionBox->addWidget(addPolyBtn,row,0);
    actionBox->addWidget(addGirihBtn,row,1);
    actionBox->addWidget(fillVectorsBtn,row,2);
    actionBox->addWidget(importBtn,row,3);

    row++;
    actionBox->addLayout(hbox,row,0);
    actionBox->addWidget(girihShapes,row,1);
    actionBox->addWidget(removeExclBtn,row,2);
    actionBox->addWidget(exportBtn,row,3);

    QGroupBox * actionGroup = new QGroupBox("Actions");
    actionGroup->setLayout(actionBox);

    connect(exportBtn,      &QPushButton::clicked,            this,     &page_tiling_maker::slot_exportPoly);
    connect(importBtn,      &QPushButton::clicked,            this,     &page_tiling_maker::slot_importPoly);
    connect(addGirihBtn,    &QPushButton::clicked,            this,     &page_tiling_maker::slot_addGirihShape);
    connect(sides,          SIGNAL(valueChanged(int)),  tilingMaker,    SLOT(updatePolygonSides(int)));
    connect(tileRot,        SIGNAL(valueChanged(qreal)),tilingMaker,    SLOT(updatePolygonRot(qreal)));
    connect(addPolyBtn,       &QPushButton::clicked,    tilingMaker,    &TilingMaker::addRegularPolygon);
    connect(fillVectorsBtn,   &QPushButton::clicked,    tilingMaker,    &TilingMaker::slot_fillUsingTranslations);
    connect(removeExclBtn,    &QPushButton::clicked,    tilingMaker,    &TilingMaker::removeExcluded);

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
    modeBox->addWidget(chkSnapTo,row,0);
    modeBox->addWidget(statusLabel,row,1,1,2);
    modeBox->addWidget(overlapStatus,row,3,1,2);

    QGroupBox * modeGroup = new QGroupBox("Modes");
    modeGroup->setLayout(modeBox);

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

    chkSnapTo->setChecked(config->snapToGrid);

    connect(chkSnapTo, &QCheckBox::clicked, this, [this](bool checked) { config->snapToGrid =  checked;});
    connect(mouseModeBtnGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, &page_tiling_maker::slot_setModes);

    return modeGroup;
}

/////////////////////////////////////
///
///  Common slots
///
/////////////////////////////////////

void  page_tiling_maker::onEnter()
{
static QString msg("<body>"
                   "<span style=\"color:rgb(205,102, 25)\">overlapping</span>  |  "
                   "<span style=\"color:rgb( 25,102,205)\">touching</span> |  "
                   "<span style=\"color:rgb(255,217,217)\">included</span>  |  "
                   "<span style=\"color:rgb(217,217,255)\">excluded</span>  |  "
                   "<span style=\"color:rgb(127,255,127)\">under-mouse</span>  |  "
                   "<span style=\"color:rgb(  0,255,  0)\">selected</span>  |  "
                   "<span style=\"color:rgb(206,179,102)\">dragging</span>"
                   "</body>");

    panel->pushPanelStatus(msg);

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

void  page_tiling_maker::onExit()
{
    panel->popPanelStatus();
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
            panel->popPanelStatus();
            break;
        default:
            break;
        }

        switch (currentMode)
        {
        case TM_INCLUSION_MODE:
            txt = "Toggle the inclusion of polygons in the tiling by clicking on them with the mouse.";
            panel->pushPanelStatus(txt);
            break;
        case TM_DRAW_POLY_MODE:
            txt = "Select a series of vertices clockwise to draw a free-form polygon. (Click on vertices).";
            panel->pushPanelStatus(txt);
            break;
        case TM_TRANSLATION_VECTOR_MODE:
            txt = "Use mouse, left-click on a polygon center or vertex, drag to the repititon point. Do this twice for two directions.";
            panel->pushPanelStatus(txt);
            break;
        default:
            break;
        }

        oldMode = currentMode;
    }

    if (config->insightMode && chk_showDebug->isChecked())
    {
        QString dbgStatus = tilingMaker->getStatus();
        debugLabel1->setText(dbgStatus);

        QPointF a = tmView->getMousePos();
        QPointF b = tmView->screenToWorld(a);
        QString astring;
        QTextStream ts(&astring);
        ts << "pos: (" << b.x() << ", " << b.y() << ")";

        TileSelectorPtr tsp = tilingMaker->getCurrentSelection();
        if (tsp)
        {
            QPointF c = tsp->getModelPoint();
            ts << "  sel: (" << c.x() << ", " << c.y() << ")";
        }
        debugLabel2->setText(astring);
    }

    if (tp->hasIntrinsicOverlaps())
    {
        overlapStatus->setText("Intrinsic overlaps");
    }
    else if (tp->hasTiledOverlaps())
    {
        overlapStatus->setText("Tiled overlaps");
    }
    else
    {
        overlapStatus->setText("");
    }


    QString status = QString("Tiles: %1  Unique: %2  Excluded: %3").arg(tp->getInTiling().count()).arg(tilingMaker->getUniqueTiles().count()).arg(tilingMaker->numExcluded());
    statusLabel->setText(status);

    tallyMouseMode();
}


bool page_tiling_maker::canExit()
{
    QString txt;
    QString info;
    const PlacedTiles & alltiles = tmView->getAllTiles();
    if (alltiles.size())
    {
        TilingPtr tp = tilingMaker->getSelected();
        const PlacedTiles & tiles = tp->getInTiling();
        if (tiles.size() == 0)
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
    emit sig_refreshView();

    setup();
    //view->dump(true);
}

void page_tiling_maker::slot_reloadTiling()
{
    eTILM_Event event    = TILM_RELOAD;
    TilingPtr oldTiling  = tilingMaker->getSelected();
    QString name         = oldTiling->getTitle();

    TilingManager tm;
    TilingPtr newTiling = tm.loadTiling(name,event);
    if (!newTiling)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText(QString("Could not load <%1>").arg(name));
        box.setInformativeText("Reload failed");
        box.exec();
        return;
    }

    emit tilingMaker->sig_tilingLoaded(name);
    emit sig_refreshView();
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
    const PlacedTiles & ptiles = (config->tm_showAllTiles) ? tmView->getAllTiles() : tilingMaker->getInTiling();
    for (const auto & pfp : std::as_const(ptiles))
    {
        QString inclusion = QString("%1 (%2)").arg((tilingMaker->isIncluded(pfp)) ? "Included" : "Excluded").arg(col);
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

    int col = 0;
    const PlacedTiles & ptiles = (config->tm_showAllTiles) ? tmView->getAllTiles() : tiling->getInTiling();
    for (const auto & pfp : std::as_const(ptiles))
    {
        QString inclusion = QString("%1 (%2)").arg((tilingMaker->isIncluded(pfp)) ? "Included" : "Excluded").arg(col);
        refreshTableEntry(pfp,col++,inclusion);
    }

    col = tileInfoTable->currentColumn();
    if (col >= 0)
    {
        PlacedTilePtr pfp = getTileColumn(col);
        updateTilePointInfo(pfp);
    }

    blockPage(false);

    tallyMouseMode();
}

void page_tiling_maker::buildTableEntry(PlacedTilePtr pf, int col, QString inclusion)
{
    TilePtr tile  = pf->getTile();
    QTransform T        = pf->getTransform();

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
    connect(cb, &QCheckBox::stateChanged, this,
            [this,col] { slot_showTileChanged(col); });

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
    if (col >= tileInfoTable->columnCount())
    {
        qDebug() << "page_tiling_maker table col " << col << "exceeds count" << tileInfoTable->columnCount();
        return;
    }

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

    if (tilingMaker->isIncluded(pf))
    {
        tilingMaker->pushTileToPrototypeMaker(PROM_TILE_NUM_SIDES_CHANGED,tile);
    }

    refreshMenuData();

    emit sig_refreshView();
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

    if (tilingMaker->isIncluded(pf))
    {
        tilingMaker->pushTileToPrototypeMaker(PROM_TILE_ROTATION_CHANGED,tile);
    }

    refreshMenuData();

    emit sig_refreshView();
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

    if (tilingMaker->isIncluded(pf))
    {
        tilingMaker->pushTileToPrototypeMaker(PROM_TILE_SCALE_CHANGED,tile);
    }

    refreshMenuData();

    emit sig_refreshView();
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

    if (tilingMaker->isIncluded(placedTile))
    {
        tilingMaker->pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
    }

    emit sig_refreshView();
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

    if (tilingMaker->isIncluded(placedTile))
    {
        tilingMaker->pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
    }

    view->update();
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

    if (tilingMaker->isIncluded(placedTile))
    {
        tilingMaker->pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
    }

    view->update();
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

    viewControl->getCanvas().setFillData(fdata);

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

    viewControl->getCanvas().setFillData(fd);

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

void page_tiling_maker::slot_t1t2Changed(double val)
{
    Q_UNUSED(val)

    if (pageBlocked()) return;

    qreal x1 = t1x->value();
    qreal y1 = t1y->value();
    qreal x2 = t2x->value();
    qreal y2 = t2y->value();

    TilingPtr tiling = tilingMaker->getSelected();
    tiling->setTranslationVectors(QPointF(x1,y1), QPointF(x2,y2));

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

    tiling->setTranslationVectors(QPointF(x1,y1),QPointF(x2,y2));

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

    tiling->setTranslationVectors(QPointF(x1,y1),QPointF(x2,y2));

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
        tilingCombo->addItem(tiling->getTitle());
    }
    int index = tilingCombo->findText(selected->getTitle());
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
    static QAbstractButton * lastChecked = nullptr;
    if (!lastChecked)
    {
        // one time initialization
        lastChecked = mouseModeBtnGroup->button(TM_NO_MOUSE_MODE);
        lastChecked->blockSignals(true);
        lastChecked->setChecked(true);
        lastChecked->blockSignals(false);
    }

    eTilingMakerMouseMode mmode = tilingMaker->getTilingMakerMouseMode();
    QAbstractButton * current = mouseModeBtnGroup->button(mmode);

    if (current == lastChecked)
    {
        return;
    }

    lastChecked->blockSignals(true);
    lastChecked->setChecked(false);
    lastChecked->blockSignals(false);

    current->blockSignals(true);
    current->setChecked(true);
    current->blockSignals(false);

    lastChecked = current;


}

void page_tiling_maker::slot_showTable(bool checked)
{
    config->tm_hideTable = !checked;
    tileInfoTable->setVisible(checked);
    translationsWidget->setVisible(checked);
    buildMenu();
}

void page_tiling_maker::slot_all_tiles(bool checked)
{
    config->tm_showAllTiles = checked;
    buildMenu();
}

void page_tiling_maker::slot_showDebug(bool checked)
{
    config->tm_showDebug = checked;
    debugWidget->setVisible(checked);
}

void page_tiling_maker::slot_autofill(bool checked)
{
    config->tm_autofill = checked;
}

void page_tiling_maker::slot_menu(QPointF spt)
{
    qDebug() << "menu spt=" << spt;

    int col = tileInfoTable->currentColumn();
    PlacedTilePtr pfp = getTileColumn(col);

    QString astr = QString("Side=%1").arg(pfp->getTile()->edgeLen(),0,'g',16);
    qDebug().noquote() << astr;

    QString bstr =  (tilingMaker->isIncluded(pfp))  ? "Exclude" : "Include";

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

    auto tmView = TilingMakerView::getInstance();
    connect(fe,          &DlgEdgePolyEdit::sig_currentPoint, tmView, &TilingMakerView::slot_setTileEditPoint, Qt::UniqueConnection);
    connect(tilingMaker, &TilingMaker::sig_refreshMenu,      fe,     &DlgEdgePolyEdit::display,      Qt::UniqueConnection);
}

void page_tiling_maker::slot_menu_includePlaced()
{
    int col = tileInfoTable->currentColumn();
    PlacedTilePtr pfp = getTileColumn(col);
    
    TileSelectorPtr tsp = make_shared<InteriorTilleSelector>(pfp);
    tilingMaker->toggleInclusion(tsp);    
}

void page_tiling_maker::slot_delete_clicked()
{
    int col = tileInfoTable->currentColumn();
    PlacedTilePtr pf = getTileColumn(col);
    tilingMaker->deleteTile(pf);
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

    tilingMaker->pushTilingToPrototypeMaker(PROM_TILING_ADDED);
    buildMenu();
}

void page_tiling_maker::slot_cellSelected(int row, int col)
{
    Q_UNUSED(row)
    PlacedTilePtr pfp = getTileColumn(col);
    updateTilePointInfo(pfp);
    tilingMaker->setCurrentPlacedTile(pfp);
    tmView->forceRedraw();
}

void page_tiling_maker::updateTilePointInfo(PlacedTilePtr pfp)
{
    TilePtr          fp  = pfp->getTile();
    QPolygonF      poly  = fp->getPoints();
    QTransform        t  = pfp->getTransform();
    QString astring;
    astring = Utils::addr(pfp.get()) + "  " + Transform::toInfoString(t) + "\n";
    for (int i=0; i < poly.size(); i++)
    {
        QPointF pt = poly.at(i);
        QString bstring = "[" + QString::number(pt.x(),'g',16) + "," + QString::number(pt.y(),'g',16) + "]  ";
        astring += bstring;
    }
    tileInfo->setText(astring);
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
        updateTilePointInfo(pfp);
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
    QStringList ts = FileServices::getPolys();

    DlgListNameSelect dlg(ts);
    dlg.setWindowTitle("Poly Name");
    int retval = dlg.exec();
    if (retval == QDialog::Rejected)
    {
        qDebug() << "Canceled";
        return;
    }
    Q_ASSERT(retval == QDialog::Accepted);
    QString name = dlg.newEdit->text();

    // export
    bool rv = pfp->saveAsGirihShape(name);

    // report
    QMessageBox box(this);
    if (rv)
    {
        box.setIcon(QMessageBox::Information);
        box.setText(QString("Saved: %1 - OK").arg(name));
    }
    else
    {
        box.setIcon(QMessageBox::Critical);
        box.setText(QString("Error saving: %1 - FAILED").arg(name));
    }
    box.exec();
}

void page_tiling_maker::slot_importPoly()
{
    QStringList ts = FileServices::getPolys();

    GirihListSelect dlg(ts,this);
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
        QString name = *it;
        PlacedTilePtr pfp = make_shared<PlacedTile>();
        bool rv =  pfp->loadFromGirihShape(name);
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
    QString name = girihShapes->currentData().toString();
    PlacedTilePtr pfp = make_shared<PlacedTile>();
    bool rv =  pfp->loadFromGirihShape(name);
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
    const PlacedTiles & ptiles = (config->tm_showAllTiles) ? tmView->getAllTiles() : tilingMaker->getInTiling();
    for (const auto & pfp : std::as_const(ptiles))
    {
        QTransform t1 = pfp->getTransform();
        QTransform t2 = QTransform::fromTranslate(valX,valY);
        pfp->setTransform(t1*t2);
    }
    tmView->forceRedraw();
    refreshMenuData();
}
