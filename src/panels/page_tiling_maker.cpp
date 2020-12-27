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

#include "panels/page_tiling_maker.h"
#include "base/fileservices.h"
#include "base/tiledpatternmaker.h"
#include "base/utilities.h"
#include "geometry/transform.h"
#include "makers/decoration_maker/decoration_maker.h"
#include "makers/motif_maker/motif_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "panels/dlg_edgepoly_edit.h"
#include "panels/dlg_listnameselect.h"
#include "panels/dlg_listselect.h"
#include "panels/dlg_name.h"
#include "panels/dlg_trim.h"
#include "panels/panel.h"
#include "style/style.h"
#include "tile/backgroundimage.h"
#include "tile/tiling_manager.h"
#include "viewers/view.h"
#include "viewers/viewcontrol.h"

Q_DECLARE_METATYPE(WeakPlacedFeaturePtr)

page_tiling_maker:: page_tiling_maker(ControlPanel * cpanel)  : panel_page(cpanel,"Tiling Maker"), bkgdLayout("Bkgd Xform")
{
    tilingMaker = TilingMaker::getInstance(); // creates tiling designer

    QHBoxLayout * controlLayout = createControlRow();
    vbox->addLayout(controlLayout);

    QGroupBox * actionGroup = createActionsGroup();
    vbox->addWidget(actionGroup);

    QGroupBox * modeGroup = createModesGroup();
    vbox->addWidget(modeGroup);

    QHBoxLayout * fillRow = createFillDataRow();
    vbox->addLayout(fillRow);

    QHBoxLayout * tableControl = createTableControlRow();
    vbox->addLayout(tableControl);

    translationsWidget = createTranslationsRow();
    vbox->addWidget(translationsWidget);

    AQTableWidget * table = createTilingTable();
    vbox->addWidget(table);

    debugWidget = createDebugInfo();
    vbox->addWidget(debugWidget);

    bkgdGroup = createBackgroundGroup();
    vbox->addWidget(bkgdGroup);

    connect(&f_sidesMapper,SIGNAL(mapped(int)), this, SLOT(slot_sidesChanged(int)));
    connect(&f_rotMapper,  SIGNAL(mapped(int)), this, SLOT(slot_f_rotChanged(int)));
    connect(&f_scaleMapper,SIGNAL(mapped(int)), this, SLOT(slot_f_scaleChanged(int)));
    connect(&scaleMapper,  SIGNAL(mapped(int)), this, SLOT(slot_transformChanged(int)));
    connect(&rotMapper,    SIGNAL(mapped(int)), this, SLOT(slot_transformChanged(int)));
    connect(&xMapper,      SIGNAL(mapped(int)), this, SLOT(slot_transformChanged(int)));
    connect(&yMapper,      SIGNAL(mapped(int)), this, SLOT(slot_transformChanged(int)));
    connect(&showMapper,   SIGNAL(mapped(int)), this, SLOT(slot_showFeatureChanged(int)));

    connect(theApp,  &TiledPatternMaker::sig_mosaicLoaded,  this, &page_tiling_maker::onEnter);
    connect(tilingMaker,  &TilingMaker::sig_buildMenu,      this, &page_tiling_maker::slot_buildMenu);
    connect(tilingMaker,  &TilingMaker::sig_refreshMenu,    this, &page_tiling_maker::slot_refreshMenuData);
    connect(tilingMaker,  &TilingMaker::sig_current_feature,this, &page_tiling_maker::slot_currentFeature);

    tileInfoTable->setVisible(!config->tm_hideTable);
    translationsWidget->setVisible(!config->tm_hideTable);
    debugWidget->setVisible(config->tm_showDebug);
}

QHBoxLayout * page_tiling_maker::createControlRow()
{
    QLabel * label                = new QLabel("Loaded Tilings:");
    tilingCombo                   = new QComboBox();
    tilingCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    QPushButton * reloadTilingBtn = new QPushButton("Re-load Tiling");
    QPushButton * pbClearMakers   = new QPushButton("Clear Makers");
    QPushButton * pbClearTiling   = new QPushButton("Clear Tiling");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(label);
    hbox->addWidget(tilingCombo);
    hbox->addStretch();
    hbox->addWidget(reloadTilingBtn);
    hbox->addWidget(pbClearTiling);
    hbox->addWidget(pbClearMakers);
    hbox->addStretch();
    hbox->addStretch();

    connect(tilingCombo,       SIGNAL(currentIndexChanged(int)),  this, SLOT(slot_currentTilingChanged(int)));
    connect(pbClearMakers,     &QPushButton::clicked,  this, &page_tiling_maker::slot_clearMakers);
    connect(pbClearTiling,     &QPushButton::clicked,  this, &page_tiling_maker::slot_clearTiling);
    connect(reloadTilingBtn,   &QPushButton::clicked,  this, &page_tiling_maker::slot_reloadTiling);

    return hbox;
}

AQWidget * page_tiling_maker::createTranslationsRow()
{

    t1x = new DoubleSpinSet("T1-X",0,-100.0,100.0);
    t1y = new DoubleSpinSet("T1-Y",0,-100.0,100.0);
    t2x = new DoubleSpinSet("T2-X",0,-100.0,100.0);
    t2y = new DoubleSpinSet("T2-Y",0,-100.0,100.0);
    t1x->setAlignment(Qt::AlignLeft);
    t1y->setAlignment(Qt::AlignLeft);
    t2x->setAlignment(Qt::AlignLeft);
    t2y->setAlignment(Qt::AlignLeft);
    t1x->setDecimals(16);
    t1y->setDecimals(16);
    t2x->setDecimals(16);
    t2y->setDecimals(16);

    QGridLayout * grid = new QGridLayout;
    int row = 0;
    grid->addLayout(t1x,row,0);
    grid->addLayout(t1y,row,1);
    grid->addLayout(t2x,row,2);
    grid->addLayout(t2y,row,3);

    QObject::connect(t1x,  &DoubleSpinSet::sig_valueChanged, this, &page_tiling_maker::slot_t1t2Changed);
    QObject::connect(t1y,  &DoubleSpinSet::sig_valueChanged, this, &page_tiling_maker::slot_t1t2Changed);
    QObject::connect(t2x,  &DoubleSpinSet::sig_valueChanged, this, &page_tiling_maker::slot_t1t2Changed);
    QObject::connect(t2y,  &DoubleSpinSet::sig_valueChanged, this, &page_tiling_maker::slot_t1t2Changed);

    AQWidget * widget = new AQWidget();
    widget->setLayout(grid);

    return widget;
}

QHBoxLayout * page_tiling_maker::createTableControlRow()
{
    QHBoxLayout * hbox = new QHBoxLayout;

    QCheckBox * chk_autoFill     = new QCheckBox("Auto-fill on load");
    QCheckBox * chk_showOverlaps = new QCheckBox("Show overlaps/touching");

    if (config->insightMode)
    {
        QCheckBox * chk_showTable     = new QCheckBox("Show table");
        QCheckBox * chk_showDebug     = new QCheckBox("Show debug");
        QCheckBox * chk_all_features  = new QCheckBox("Show Excluded");
        chk_all_features->setChecked(config->tm_showAllFeatures);
        QPushButton * swapBtn         = new QPushButton("Swap T1/T2");

        chk_showTable->setChecked(!config->tm_hideTable);
        chk_all_features->setChecked(config->tm_showAllFeatures);
        chk_showDebug->setChecked(config->tm_showDebug);

        hbox->addWidget(chk_showTable);
        hbox->addWidget(chk_all_features);
        hbox->addWidget(chk_autoFill);
        hbox->addWidget(chk_showOverlaps);
        hbox->addWidget(chk_showDebug);
        hbox->addStretch();
        hbox->addWidget(swapBtn);

        connect(chk_showTable,   &QCheckBox::clicked,    this,   &page_tiling_maker::slot_showTable);
        connect(chk_all_features,&QCheckBox::clicked,    this,   &page_tiling_maker::slot_all_features);
        connect(chk_showDebug,   &QCheckBox::clicked,    this,   &page_tiling_maker::slot_showDebug);
        connect(swapBtn,         &QPushButton::clicked,  this,   &page_tiling_maker::slot_swapTrans);
    }
    else
    {
        config->tm_hideTable       = true;
        config->tm_showAllFeatures = false;
        config->tm_showDebug       = false;

        hbox->addStretch();
        hbox->addWidget(chk_autoFill);
        hbox->addWidget(chk_showOverlaps);
        hbox->addStretch();
    }

    chk_autoFill->setChecked(config->tm_autofill);
    chk_showOverlaps->setChecked(config->tm_showOverlaps);

    connect(chk_autoFill,    &QCheckBox::clicked,    this,   &page_tiling_maker::slot_autofill);
    connect(chk_showOverlaps,&QCheckBox::clicked, tilingMaker, &TilingMaker::slot_showOverlaps);

    return hbox;
}

QHBoxLayout * page_tiling_maker::createFillDataRow()
{
    QHBoxLayout * hbox = new QHBoxLayout;

    const int rmin = -99;
    const int rmax =  99;

    xRepMin = new QSpinBox();
    xRepMax = new QSpinBox();
    yRepMin = new QSpinBox();
    yRepMax = new QSpinBox();

    xRepMin->setRange(rmin,rmax);
    xRepMax->setRange(rmin,rmax);
    yRepMin->setRange(rmin,rmax);
    yRepMax->setRange(rmin,rmax);

    hbox->addWidget(new QLabel("Repetitions:  xMin"));
    hbox->addWidget(xRepMin);
    hbox->addWidget(new QLabel("xMax"));
    hbox->addWidget(xRepMax);
    hbox->addWidget(new QLabel("yMin"));
    hbox->addWidget(yRepMin);
    hbox->addWidget(new QLabel("yMax"));
    hbox->addWidget(yRepMax);
    hbox->addStretch();

    connect(xRepMin, SIGNAL(valueChanged(int)), this,  SLOT(slot_set_reps(int)));
    connect(xRepMax, SIGNAL(valueChanged(int)), this,  SLOT(slot_set_reps(int)));
    connect(yRepMin, SIGNAL(valueChanged(int)), this,  SLOT(slot_set_reps(int)));
    connect(yRepMax, SIGNAL(valueChanged(int)), this,  SLOT(slot_set_reps(int)));

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
    qslv << "feature" << "show" << "type" << "feat-sides" << "feat-rot" << "feat-scale" << "scale" << "rot" << "X" << "Y" << "CW" << "addr";
    tileInfoTable->setVerticalHeaderLabels(qslv);
    tileInfoTable->horizontalHeader()->setVisible(false);
    tileInfoTable->setMaximumWidth(880);
    tileInfoTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    tileInfoTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QHeaderView * qhv = tileInfoTable->verticalHeader();

    connect(tileInfoTable,   &QTableWidget::customContextMenuRequested, this, &page_tiling_maker::slot_menu);
    connect(qhv,             &QHeaderView::sectionClicked, this, &page_tiling_maker::tableHeaderClicked);

    return tileInfoTable;
}

AQWidget * page_tiling_maker::createDebugInfo()
{
    ///  Debug Status    ///
    debugLabel1  = new QLabel;
    debugLabel2  = new QLabel;
    featureInfo  = new QTextEdit;
    featureInfo->setFixedHeight(49);
    QHBoxLayout * debhbox = new QHBoxLayout;
    debhbox->addWidget(debugLabel1);
    debhbox->addSpacing(9);
    debhbox->addWidget(debugLabel2);
    debhbox->addStretch();
    QVBoxLayout * debvbox = new QVBoxLayout;
    debvbox->addSpacing(5);
    debvbox->addWidget(featureInfo);
    debvbox->addLayout(debhbox);
    debvbox->addSpacing(7);

    debugWidget = new AQWidget;
    debugWidget->setLayout(debvbox);

    return debugWidget;
}

QGroupBox * page_tiling_maker::createActionsGroup()
{
    // actions
    BQPushButton * addPolyBtn      = new BQPushButton("Add Regular Polygon (A)");
    BQPushButton * addGirihBtn     = new BQPushButton("Add Girih Shape");
    BQPushButton * exclAllBtn      = new BQPushButton("Exclude All (E)");
    BQPushButton * fillVectorsBtn  = new BQPushButton("Fill using Repeat Points (F)");
    BQPushButton * removeExclBtn   = new BQPushButton("Remove Excluded (R)");
    BQPushButton * clearVectorsBtn = new BQPushButton("Erase Repeat Points (X)");

    BQPushButton * exportBtn       = new BQPushButton("Export Polygon");
    BQPushButton * importBtn       = new BQPushButton("Import Polygon");

    sides = new QSpinBox;
    sides->setFixedWidth(41);
    sides->setMinimum(3);
    sides->setMaximum(128);
    sides->setValue(config->polySides);

    featRot = new QDoubleSpinBox;
    featRot->setRange(-360.0,360.0);
    featRot->setDecimals(6);
    featRot->setValue(0.0);

    girihShapes = new QComboBox();
    girihShapes->addItem("Decagon", "gDecagon");
    girihShapes->addItem("Pentagon","gPentagon");
    girihShapes->addItem("Bowtie",  "gBowtie");
    girihShapes->addItem("Kite",    "gKite");
    girihShapes->addItem("Rhombus", "gRhombus");

    QLabel * rlabel = new QLabel("Rot");

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(rlabel);
    hbox->addWidget(featRot);

    QHBoxLayout * hbox2 = new QHBoxLayout();
    hbox2->addWidget(sides);
    hbox2->addWidget(addPolyBtn);

    QGridLayout  * actionBox = new QGridLayout;

    int row = 0;
    actionBox->addLayout(hbox2,row,0);
    actionBox->addLayout(hbox,row,1);
    actionBox->addWidget(fillVectorsBtn,row,2);
    actionBox->addWidget(removeExclBtn,row,3);
    actionBox->addWidget(importBtn,row,4);

    row++;
    actionBox->addWidget(girihShapes,row,0);
    actionBox->addWidget(addGirihBtn,row,1);
    actionBox->addWidget(clearVectorsBtn,row,2);
    actionBox->addWidget(exclAllBtn,row,3);
    actionBox->addWidget(exportBtn,row,4);

    QGroupBox * actionGroup = new QGroupBox("Actions");
    actionGroup->setLayout(actionBox);

    connect(exportBtn,      &QPushButton::clicked,            this,    &page_tiling_maker::slot_exportPoly);
    connect(importBtn,      &QPushButton::clicked,            this,    &page_tiling_maker::slot_importPoly);
    connect(addGirihBtn,    &QPushButton::clicked,            this,    &page_tiling_maker::slot_addGirihShape);
    connect(sides,          SIGNAL(valueChanged(int)),  tilingMaker,    SLOT(updatePolygonSides(int)));
    connect(featRot,        SIGNAL(valueChanged(qreal)),tilingMaker, SLOT(updatePolygonRot(qreal)));
    connect(addPolyBtn,       &QPushButton::clicked,    tilingMaker,    &TilingMaker::addRegularPolygon);
    connect(clearVectorsBtn,  &QPushButton::clicked,    tilingMaker,    &TilingMaker::clearTranslationVectors);
    connect(fillVectorsBtn,   &QPushButton::clicked,    tilingMaker,    &TilingMaker::fillUsingTranslations);
    connect(removeExclBtn,    &QPushButton::clicked,    tilingMaker,    &TilingMaker::removeExcluded);
    connect(exclAllBtn,       &QPushButton::clicked,    tilingMaker,    &TilingMaker::excludeAll);

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
    AQPushButton * perspective  = new AQPushButton("Bkgd Perspective (F10)");
    AQPushButton * editPoly     = new AQPushButton("Edit Feature (F11)");
    AQPushButton * mirrorX      = new AQPushButton("Mirror X");
    AQPushButton * mirrorY      = new AQPushButton("Mirror Y");
    AQPushButton * curveEdge    = new AQPushButton("Edit Curved Edge (F12)");
    AQPushButton * drawConst    = new AQPushButton("Draw Construction Lines");
    QCheckBox    * chkSnapTo    = new QCheckBox("Snap to Grid");

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
    modeBox->addWidget(perspective,row,3);
    modeBox->addWidget(editPoly,row,4);

    row++;
    modeBox->addWidget(mirrorX,row,0);
    modeBox->addWidget(mirrorY,row,1);
    modeBox->addWidget(curveEdge,row,3);
    modeBox->addWidget(drawConst,row,4);

    row++;
    modeBox->addWidget(chkSnapTo,row,0);

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
    mouseModeBtnGroup->addButton(perspective,   TM_BKGD_SKEW_MODE);
    mouseModeBtnGroup->addButton(editPoly,      TM_EDIT_FEATURE_MODE);
    mouseModeBtnGroup->addButton(curveEdge,     TM_EDGE_CURVE_MODE);
    mouseModeBtnGroup->addButton(mirrorX,       TM_MIRROR_X_MODE);
    mouseModeBtnGroup->addButton(mirrorY,       TM_MIRROR_Y_MODE);
    mouseModeBtnGroup->addButton(drawConst,     TM_CONSTRUCTION_LINES);

    connect(chkSnapTo, &QCheckBox::clicked, tilingMaker, &TilingMaker::slot_snapTo);
    connect(mouseModeBtnGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, &page_tiling_maker::slot_setModes);

    return modeGroup;
}

QGroupBox * page_tiling_maker::createBackgroundGroup()
{
    QPushButton * loadBkgdBtn     = new QPushButton("Load Background");
    QPushButton * adjustBtn       = new QPushButton("Adjust perspective");
    QPushButton * saveAdjustedBtn = new QPushButton("Save Adjusted");

    chk_showBkgd    = new QCheckBox("Show Background");
    chk_adjustBkgd  = new QCheckBox("Perspective");
    chk_hideTiling  = new QCheckBox("Hide Tiling");

    QHBoxLayout * bkh2 = new QHBoxLayout();
    bkh2->addWidget(loadBkgdBtn);
    bkh2->addWidget(chk_showBkgd);
    bkh2->addWidget(chk_adjustBkgd);
    bkh2->addWidget(chk_hideTiling);
    bkh2->addWidget(adjustBtn);
    bkh2->addWidget(saveAdjustedBtn);
    bkh2->addStretch();

    QVBoxLayout * bkg = new QVBoxLayout();
    bkg->addLayout(&bkgdLayout);
    bkg->addLayout(bkh2);

    QGroupBox * bkgdGroup  = new QGroupBox("Background Image");
    bkgdGroup->setLayout(bkg);

    connect(loadBkgdBtn,    &QPushButton::clicked,            this,    &page_tiling_maker::slot_loadBackground);
    connect(adjustBtn,      &QPushButton::clicked,            this,    &page_tiling_maker::slot_adjustBackground);
    connect(saveAdjustedBtn,&QPushButton::clicked,            this,    &page_tiling_maker::slot_saveAdjustedBackground);

    connect(&bkgdLayout,    &LayoutTransform::xformChanged,   this,    &page_tiling_maker::slot_setBkgdXform);
    connect(chk_showBkgd,   &QAbstractButton::clicked,        this,    &page_tiling_maker::slot_setBkgd);
    connect(chk_adjustBkgd, &QAbstractButton::clicked,        this,    &page_tiling_maker::slot_setBkgd);

    connect(chk_hideTiling, &QCheckBox::clicked,       tilingMaker,    &TilingMaker::hideTiling);

    return bkgdGroup;
}

/////////////////////////////////////
///
///  Common slots
///
/////////////////////////////////////

void  page_tiling_maker::onEnter()
{
    tallyMouseMode();

    sides->setValue(tilingMaker->getPolygonSides());

    blockPage(true);

    featureInfo->clear();
    tileInfoTable->clearContents();     // done in build menu

    t1x->setValue(0);
    t1y->setValue(0);
    t2x->setValue(0);
    t2y->setValue(0);

    blockPage(false);

    TilingPtr tiling = tilingMaker->getSelected();
    displayBackgroundStatus(tiling);
    loadTilingCombo(tiling);
    buildMenu();

    tilingMaker->clearConstructionLines();

    if (config->viewerType == VIEW_TILING_MAKER)
    {
        tilingMaker->forceRedraw();
    }
}

void  page_tiling_maker::onExit()
{
    panel->hidePanelStatus();
}

void  page_tiling_maker::refreshPage()
{
    static WeakTilingPtr    wtp;

    TilingPtr tp = tilingMaker->getSelected();
    if (wtp.lock() != tp)
    {
        wtp = tp;
        onEnter();
    }

    QString status = tilingMaker->getStatus();
    debugLabel1->setText(status);

    QPointF a = tilingMaker->getMousePos();
    QPointF b = tilingMaker->screenToWorld(a);
    QString astring;
    QTextStream ts(&astring);
    ts << "pos: (" << b.x() << ", " << b.y() << ")";

    displayBackgroundStatus(tp);

    TilingSelectorPtr tsp = tilingMaker->getCurrentSelection();
    if (tsp)
    {
        QPointF c = tsp->getModelPoint();
        ts << "  sel: (" << c.x() << ", " << c.y() << ")";
    }
    debugLabel2->setText(astring);

    tallyMouseMode();
}

bool page_tiling_maker::canExit()
{
    QString txt;
    QString info;
    QVector<PlacedFeaturePtr> & allfeatures = tilingMaker->getAllFeatures();
    if (allfeatures.size())
    {
        QVector<PlacedFeaturePtr> & features = tilingMaker->getInTiling();
        if (features.size() == 0)
        {
            txt  = "There are no polygons included in the tiling";
            info = "To include polygons, press Cancel and 'Include' the polygons you want in the tiling";
        }
        else if (tilingMaker->getSelected()->getTrans1().isNull() && tilingMaker->getSelected()->getTrans2().isNull())
        {
            txt  = "There are no Repeat Points for the tiling";
            info = "To set the Repeat Points, press Cancel and then press F3 to enter 'Set Repeat Points' mode";
        }
    }
    if (!txt.isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(txt);
        msgBox.setInformativeText(info);
        QPushButton * save   = msgBox.addButton(QMessageBox::Ignore);
        QPushButton * cancel = msgBox.addButton(QMessageBox::Cancel);
        msgBox.setDefaultButton(cancel);
        msgBox.exec();
        if (msgBox.clickedButton() == cancel)
        {
            return false;
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

void page_tiling_maker::slot_clearMakers()
{
    // clears everything
    vcontrol->resetAllMakers();

    onEnter();
    //view->dump(true);
}

void page_tiling_maker::slot_clearTiling()
{
    // clears current tiling
    TilingPtr tp = tilingMaker->getSelected();
    tilingMaker->removeTiling(tp);  // re-creates if becomes empty
    emit sig_refreshView();

    onEnter();
    //view->dump(true);
}

void page_tiling_maker::slot_reloadTiling()
{
    int num              = tilingMaker->numTilings();
    eSM_Event mode = (num > 1) ? SM_RELOAD_MULTI : SM_RELOAD_SINGLE;
    TilingPtr oldTiling  = tilingMaker->getSelected();
    QString name         = oldTiling->getName();

    TilingManager tm;
    TilingPtr newTiling = tm.loadTiling(name,mode);
    if (!newTiling)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Reload failed");
        box.exec();
        return;
    }
    newTiling->setState(TILING_LOADED);

    emit theApp->sig_tilingLoaded(name);
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
    QVector<PlacedFeaturePtr> & pfeatures = (config->tm_showAllFeatures) ? tilingMaker->getAllFeatures() : tilingMaker->getInTiling();
    for (auto& pfp : pfeatures)
    {
        QString inclusion = (tilingMaker->isIncluded(pfp)) ? "Included" : "Excluded";
        buildTableEntry(pfp,col++,inclusion);
    }

    connect(tileInfoTable, SIGNAL(cellClicked(int,int)),  this, SLOT(slot_cellSelected(int,int)),Qt::UniqueConnection);

    for (int i=0; i < tileInfoTable->columnCount(); i++)
    {
        tileInfoTable->resizeColumnToContents(i);
    }
    tileInfoTable->adjustTableSize(690);

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

    QPointF t1 = tiling->getTrans1();
    QPointF t2 = tiling->getTrans2();

    t1x->setValue(t1.x());
    t1y->setValue(t1.y());
    t2x->setValue(t2.x());
    t2y->setValue(t2.y());

    int xMin,xMax,yMin,yMax;
    const FillData & fd = tiling->getFillData();
    fd.get(xMin ,xMax,yMin,yMax);
    xRepMin->setValue(xMin);
    xRepMax->setValue(xMax);
    yRepMin->setValue(yMin);
    yRepMax->setValue(yMax);

    int col = 0;
    QVector<PlacedFeaturePtr> & pfeatures = (config->tm_showAllFeatures) ? tilingMaker->getAllFeatures() : tilingMaker->getInTiling();
    for (auto& pfp : pfeatures)
    {
        QString inclusion = (tilingMaker->isIncluded(pfp)) ? "Included" : "Excluded";
        refreshTableEntry(pfp,col++,inclusion);
    }

    col = tileInfoTable->currentColumn();
    if (col >= 0)
    {
        PlacedFeaturePtr pfp = getFeatureColumn(col);
        updateFeaturePointInfo(pfp);
    }

    blockPage(false);

    tallyMouseMode();
}


void page_tiling_maker::buildTableEntry(PlacedFeaturePtr pf, int col, QString inclusion)
{
    FeaturePtr feature  = pf->getFeature();
    QTransform T        = pf->getTransform();

    tileInfoTable->setColumnCount(col+1);

    QString type;
    if (feature->isRegular())
        type = "Regular";
    else
        type = "Irregular";
    QTableWidgetItem * twi = new QTableWidgetItem(type);
    twi->setData(Qt::UserRole,QVariant::fromValue(WeakPlacedFeaturePtr(pf)));
    tileInfoTable->setItem(TI_TYPE_PFP,col,twi);

    QCheckBox * cb = new QCheckBox("Show");
    cb->setStyleSheet("padding-left: 10px;");
    tileInfoTable->setCellWidget(TI_SHOW,col,cb);
    connect(cb, SIGNAL(stateChanged(int)), &showMapper, SLOT(map()));
    showMapper.setMapping(cb,col);

    QSpinBox * sp = new QSpinBox;
    sp->setValue(feature->numPoints());
    sp->setReadOnly(!feature->isRegular());
    tileInfoTable->setCellWidget(TI_FEAT_SIDES,col,sp);
    connect(sp, SIGNAL(valueChanged(int)), &f_sidesMapper, SLOT(map()));
    f_sidesMapper.setMapping(sp,col);

    QDoubleSpinBox *dsp = new QDoubleSpinBox;
    dsp->setRange(-360.0,360.0);
    dsp->setDecimals(16);
    dsp->setAlignment(Qt::AlignRight);
    dsp->setValue(feature->getRotation());
    tileInfoTable->setCellWidget(TI_FEAT_ROT,col,dsp);
    connect(dsp, SIGNAL(valueChanged(double)), &f_rotMapper, SLOT(map()));
    f_rotMapper.setMapping(dsp,col);

    dsp = new QDoubleSpinBox;
    dsp->setRange(0,128);
    dsp->setDecimals(16);
    dsp->setSingleStep(0.01);
    dsp->setAlignment(Qt::AlignRight);
    dsp->setValue(feature->getScale());
    tileInfoTable->setCellWidget(TI_FEAT_SCALE,col,dsp);
    connect(dsp, SIGNAL(valueChanged(double)), &f_scaleMapper, SLOT(map()));
    f_scaleMapper.setMapping(dsp,col);

    dsp = new QDoubleSpinBox;
    dsp->setRange(0,128);
    dsp->setDecimals(16);
    dsp->setSingleStep(0.01);
    dsp->setAlignment(Qt::AlignRight);
    dsp->setValue(Transform::scalex(T));
    tileInfoTable->setCellWidget(TI_SCALE,col,dsp);
    connect(dsp, SIGNAL(valueChanged(double)), &scaleMapper, SLOT(map()));
    scaleMapper.setMapping(dsp,col);

    dsp = new QDoubleSpinBox;
    dsp->setRange(-360.0,360.0);
    dsp->setDecimals(16);
    dsp->setAlignment(Qt::AlignRight);
    dsp->setValue(qRadiansToDegrees(Transform::rotation(T)));
    tileInfoTable->setCellWidget(TI_ROT,col,dsp);
    connect(dsp, SIGNAL(valueChanged(double)), &rotMapper, SLOT(map()));
    rotMapper.setMapping(dsp,col);

    dsp = new QDoubleSpinBox;
    dsp->setRange(-100.0,100.0);
    dsp->setDecimals(16);
    dsp->setAlignment(Qt::AlignRight);
    dsp->setValue(Transform::transx(T));
    tileInfoTable->setCellWidget(TI_X,col,dsp);
    connect(dsp, SIGNAL(valueChanged(double)), &xMapper, SLOT(map()));
    xMapper.setMapping(dsp,col);

    dsp = new QDoubleSpinBox;
    dsp->setRange(-100.0,100.0);
    dsp->setDecimals(16);
    dsp->setAlignment(Qt::AlignRight);
    dsp->setValue(Transform::transy(T));
    tileInfoTable->setCellWidget(TI_Y,col,dsp);
    connect(dsp, SIGNAL(valueChanged(double)), &yMapper, SLOT(map()));
    yMapper.setMapping(dsp,col);

    QString cw = feature->isClockwise() ? "Clockwise" : "Anticlockwise";
    twi = new QTableWidgetItem(cw);
    tileInfoTable->setItem(TI_CW,col,twi);

    twi = new QTableWidgetItem(addr(feature.get()));
    tileInfoTable->setItem(TI_FEAT_ADDR,col,twi);

    twi = new QTableWidgetItem(inclusion);
    tileInfoTable->setItem(TI_LOCATION,col,twi);
}

void page_tiling_maker::refreshTableEntry(PlacedFeaturePtr pf, int col, QString inclusion)
{
    if (col >= tileInfoTable->columnCount())
    {
        qDebug() << "table col " << col << "exceeds count" << tileInfoTable->columnCount();
        return;
    }

    FeaturePtr feature  = pf->getFeature();
    QTransform T        = pf->getTransform();

    QString type;
    if (feature->isRegular())
        type = "Regular";
    else
        type = "Irregular";
    QTableWidgetItem * twi = new QTableWidgetItem(type);
    twi->setData(Qt::UserRole,QVariant::fromValue(WeakPlacedFeaturePtr(pf)));
    tileInfoTable->setItem(TI_TYPE_PFP,col,twi);

    QWidget * w = tileInfoTable->cellWidget(TI_FEAT_SIDES,col);
    if (!w)
    {
        qDebug() << "table col not found";
        return;
    }
    QSpinBox * sp = dynamic_cast<QSpinBox*>(w);
    Q_ASSERT(sp);
    sp->setValue(feature->numPoints());

    w = tileInfoTable->cellWidget(TI_SHOW,col);
    QCheckBox * cb = dynamic_cast<QCheckBox*>(w);
    cb->blockSignals(true);
    cb->setChecked(pf->show());
    cb->blockSignals(false);

    w = tileInfoTable->cellWidget(TI_FEAT_ROT,col);
    QDoubleSpinBox * dsp = dynamic_cast<QDoubleSpinBox*>(w);
    Q_ASSERT(dsp);
    dsp->setValue(feature->getRotation());

    w = tileInfoTable->cellWidget(TI_FEAT_SCALE,col);
    dsp = dynamic_cast<QDoubleSpinBox*>(w);
    Q_ASSERT(dsp);
    dsp->setValue(feature->getScale());

    w = tileInfoTable->cellWidget(TI_SCALE,col);
    dsp = dynamic_cast<QDoubleSpinBox*>(w);
    Q_ASSERT(dsp);
    dsp->setValue(Transform::scalex(T));

    w = tileInfoTable->cellWidget(TI_ROT,col);
    dsp = dynamic_cast<QDoubleSpinBox*>(w);
    Q_ASSERT(dsp);
    dsp->setValue(qRadiansToDegrees(Transform::rotation(T)));

    w = tileInfoTable->cellWidget(TI_X,col);
    dsp = dynamic_cast<QDoubleSpinBox*>(w);
    Q_ASSERT(dsp);
    dsp->setValue(Transform::transx(T));

    w = tileInfoTable->cellWidget(TI_Y,col);
    dsp = dynamic_cast<QDoubleSpinBox*>(w);
    Q_ASSERT(dsp);
    dsp->setValue(Transform::transy(T));

    QString cw = feature->isClockwise() ? "Clockwise" : "Anticlockwise";
    twi = tileInfoTable->item(TI_CW,col);
    twi->setText(cw);

    twi = tileInfoTable->item(TI_FEAT_ADDR,col);
    twi->setText(addr(feature.get()));

    twi = tileInfoTable->item(TI_LOCATION,col);
    twi->setText(inclusion);
}

PlacedFeaturePtr page_tiling_maker::getFeatureColumn(int col)
{
    PlacedFeaturePtr pf;
    if (col == -1) return pf;

    QTableWidgetItem * twi = tileInfoTable->item(TI_TYPE_PFP,col);
    if (twi)
    {
        QVariant var = twi->data(Qt::UserRole);
        if (var.canConvert<WeakPlacedFeaturePtr>())
        {
            WeakPlacedFeaturePtr wkpf = var.value<WeakPlacedFeaturePtr>();
            pf = wkpf.lock();
        }
    }
    return pf;
}

/////////////////////////////////////
///
///  Private Slots
///
/////////////////////////////////////

void page_tiling_maker::slot_sidesChanged(int col)
{
    if (pageBlocked()) return;

    QWidget * cw  = tileInfoTable->cellWidget(TI_FEAT_SIDES,col);
    QSpinBox * sp = dynamic_cast<QSpinBox*>(cw);
    Q_ASSERT(sp);
    int sides = sp->value();

    PlacedFeaturePtr pf = getFeatureColumn(col);
    FeaturePtr feature  = pf->getFeature();
    if (!feature->isRegular())
    {
        return;
    }
    feature->setN(sides);

    if (tilingMaker->isIncluded(pf))
    {
        pushTilingToMotifMaker(SM_FEATURE_CHANGED);
    }

    refreshMenuData();

    emit sig_refreshView();
}

void page_tiling_maker::slot_f_rotChanged(int col)
{
    if (pageBlocked()) return;

    QWidget * cw  = tileInfoTable->cellWidget(TI_FEAT_ROT,col);
    QDoubleSpinBox * sp = dynamic_cast<QDoubleSpinBox*>(cw);
    Q_ASSERT(sp);
    qreal rotation = sp->value();

    PlacedFeaturePtr pf = getFeatureColumn(col);
    FeaturePtr feature  = pf->getFeature();
    feature->setRotation(rotation);

    if (tilingMaker->isIncluded(pf))
    {
        pushTilingToMotifMaker(SM_FEATURE_CHANGED);
    }

    refreshMenuData();

    emit sig_refreshView();
}

void page_tiling_maker::slot_f_scaleChanged(int col)
{
    if (pageBlocked()) return;

    QWidget * cw  = tileInfoTable->cellWidget(TI_FEAT_SCALE,col);
    QDoubleSpinBox * sp = dynamic_cast<QDoubleSpinBox*>(cw);
    Q_ASSERT(sp);
    qreal scale = sp->value();

    PlacedFeaturePtr pf = getFeatureColumn(col);
    FeaturePtr feature  = pf->getFeature();
    feature->setScale(scale);

    if (tilingMaker->isIncluded(pf))
    {
        pushTilingToMotifMaker(SM_FEATURE_CHANGED);
    }

    refreshMenuData();

    emit sig_refreshView();
}

void page_tiling_maker::slot_transformChanged(int col)
{
    if (pageBlocked()) return;

    QWidget        * cw  = tileInfoTable->cellWidget(TI_SCALE,col);
    QDoubleSpinBox * dsp = dynamic_cast<QDoubleSpinBox*>(cw);
    Q_ASSERT(dsp);
    qreal scale = dsp->value();
    if (scale <= 0.0 || scale > 128.0)
    {
        return;     // fixes problem with scales of 0 when entering data
    }

    cw  = tileInfoTable->cellWidget(TI_ROT,col);
    dsp = dynamic_cast<QDoubleSpinBox*>(cw);
    Q_ASSERT(dsp);
    qreal rotation = qDegreesToRadians(dsp->value());

    cw  = tileInfoTable->cellWidget(TI_X,col);
    dsp = dynamic_cast<QDoubleSpinBox*>(cw);
    Q_ASSERT(dsp);
    qreal tx = dsp->value();

    cw  = tileInfoTable->cellWidget(TI_Y,col);
    dsp = dynamic_cast<QDoubleSpinBox*>(cw);
    Q_ASSERT(dsp);
    qreal ty  = dsp->value();

    //Transform t = Transform::compose(scale, rotation, QPointF(tx,ty));
    QTransform t = QTransform().scale(scale,scale) * QTransform().rotateRadians(rotation) * QTransform::fromTranslate(tx,ty);
    qDebug().noquote() << "col=" << col << "T=" << Transform::toInfoString(t);

    PlacedFeaturePtr placedFeature = getFeatureColumn(col);
    placedFeature->setTransform(t);

    if (tilingMaker->isIncluded(placedFeature))
    {
        pushTilingToMotifMaker(SM_FEATURE_CHANGED);
    }

    emit sig_refreshView();
}

void page_tiling_maker::slot_showFeatureChanged(int col)
{
    QWidget   * cw = tileInfoTable->cellWidget(TI_SHOW,col);
    QCheckBox * cb = dynamic_cast<QCheckBox*>(cw);
    Q_ASSERT(cb);
    bool checked = cb->isChecked();
    qDebug() << "col=" << col << "checked:" << checked;

    PlacedFeaturePtr feature = getFeatureColumn(col);
    feature->setShow(checked);

    tilingMaker->forceRedraw();
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

    tiling->setTrans1(QPointF(x1,y1));
    tiling->setTrans2(QPointF(x2,y2));

    tilingMaker->updateVisibleVectors();

    if (tilingMaker->getAllFeatures().size() > tilingMaker->getInTiling().size())
    {
        tilingMaker->fillUsingTranslations();
    }
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
    for (auto& tiling : tilings)
    {
        tilingCombo->addItem(tiling->getName());
    }
    int index = tilingCombo->findText(selected->getName());
    tilingCombo->setCurrentIndex(index);
    tilingCombo->blockSignals(false);
}

void page_tiling_maker::slot_currentTilingChanged(int index)
{
    Q_UNUSED(index);
    QString name = tilingCombo->currentText();
    TilingPtr tp = tilingMaker->findTilingByName(name);

    tilingMaker->select(tp);

    onEnter();
}

void page_tiling_maker::slot_setModes(QAbstractButton * btn)
{
    int mode = mouseModeBtnGroup->id(btn);
    eTMMouseMode mm = static_cast<eTMMouseMode>(mode);
    qDebug() << sTMMouseMode[mm] << btn->isChecked();

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

    eTMMouseMode mmode = tilingMaker->getTilingMakerMouseMode();
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

    panel->hidePanelStatus();

    QString txt;
    switch (tilingMaker->getTilingMakerMouseMode())
    {
    case TM_INCLUSION_MODE:
        txt = "Toggle the inclusion of polygons in the tiling by clicking on them with the mouse.";
        panel->showPanelStatus(txt);
        break;
    case TM_DRAW_POLY_MODE:
        txt = "Select a series of vertices clockwise to draw a free-form polygon. (Click on vertices).";
        panel->showPanelStatus(txt);
        break;
    case TM_TRANSLATION_VECTOR_MODE:
        txt = "Use mouse, left-click on a polygon center or vertex, drag to the repititon point. Do this twice for two directions.";
        panel->showPanelStatus(txt);
        break;
    case TM_BKGD_SKEW_MODE:
        txt = "Click to select four points on background image. Then press 'Adjust Perspective' to fix camera skew.";
        panel->showPanelStatus(txt);
        break;
    case TM_NO_MOUSE_MODE:
    case TM_COPY_MODE:
    case TM_DELETE_MODE:
    case TM_POSITION_MODE:
    case TM_MEASURE_MODE:
    case TM_EDIT_FEATURE_MODE:
    case TM_EDGE_CURVE_MODE:
    case TM_MIRROR_X_MODE:
    case TM_MIRROR_Y_MODE:
    case TM_CONSTRUCTION_LINES:
        break;
    }
}

void page_tiling_maker::slot_showTable(bool checked)
{
    config->tm_hideTable = !checked;
    tileInfoTable->setVisible(checked);
    translationsWidget->setVisible(checked);
    buildMenu();
}

void page_tiling_maker::slot_all_features(bool checked)
{
    config->tm_showAllFeatures = checked;
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

    QMenu myMenu(tileInfoTable);

    myMenu.addAction("Edit Feature", this, &page_tiling_maker::slot_menu_edit_feature);

    int col = tileInfoTable->currentColumn();
    PlacedFeaturePtr pfp = getFeatureColumn(col);
    QString str =  (tilingMaker->isIncluded(pfp))  ? "Exclude" : "Include";
    myMenu.addAction(str, this, &page_tiling_maker::slot_menu_includePlaced);

    if (pfp->getFeature()->isRegular())
    {
        myMenu.addAction("Make Irregular", this, &page_tiling_maker::slot_make_irregular_clicked);
    }
    myMenu.addAction("Delete Feature", this, &page_tiling_maker::slot_delete_clicked);
    myMenu.addAction("Uniquify Feature", this, &page_tiling_maker::slot_uniquify_clicked);

    myMenu.exec(tileInfoTable->viewport()->mapToGlobal(spt.toPoint()));
}

void page_tiling_maker::slot_menu_edit_feature()
{
    qDebug() << "selected edit feature";
    int              col = tileInfoTable->currentColumn();
    PlacedFeaturePtr pfp = getFeatureColumn(col);
    FeaturePtr        fp = pfp->getFeature();
    QTransform         t = pfp->getTransform();

    DlgEdgePolyEdit * fe  = new DlgEdgePolyEdit(fp->getEdgePoly(),t);
    fe->show();
    fe->raise();
    fe->activateWindow();

    connect(fe,          &DlgEdgePolyEdit::sig_currentPoint, tilingMaker, &TilingMaker::setFeatureEditPoint, Qt::UniqueConnection);
    connect(tilingMaker, &TilingMaker::sig_refreshMenu,      fe,          &DlgEdgePolyEdit::display,         Qt::UniqueConnection);
}

void page_tiling_maker::slot_menu_includePlaced()
{
    int col = tileInfoTable->currentColumn();
    PlacedFeaturePtr pfp = getFeatureColumn(col);

    TilingSelectorPtr tsp = make_shared<InteriorTilingSelector>(pfp);
    tilingMaker->toggleInclusion(tsp);    
}

void page_tiling_maker::slot_delete_clicked()
{
    int col = tileInfoTable->currentColumn();
    PlacedFeaturePtr pf = getFeatureColumn(col);
    tilingMaker->deleteFeature(pf);
    buildMenu();
}

void page_tiling_maker::slot_make_irregular_clicked()
{
    int col             = tileInfoTable->currentColumn();
    PlacedFeaturePtr pf = getFeatureColumn(col);
    FeaturePtr fp       = pf->getFeature();
    fp->setRegular(false);

    pushTilingToMotifMaker(SM_TILING_CHANGED);
    buildMenu();
}

void page_tiling_maker::slot_uniquify_clicked()
{
    int col = tileInfoTable->currentColumn();
    PlacedFeaturePtr pf = getFeatureColumn(col);
    FeaturePtr fp = pf->getFeature();
    FeaturePtr fp2 = fp->recreate();  // creates a new feature same as other
    pf->setFeature(fp2);

    pushTilingToMotifMaker(SM_TILING_CHANGED);
    buildMenu();
}

void page_tiling_maker::slot_set_reps(int val)
{
    Q_UNUSED(val);

    if (pageBlocked()) return;

    TilingPtr tiling = tilingMaker->getSelected();
    if (!tiling) return;

    FillData fd;
    fd.set(xRepMin->value(), xRepMax->value(), yRepMin->value(), yRepMax->value());

    tiling->setFillData(fd);
    vcontrol->setFillData(fd);

    emit sig_refreshView();
}

void page_tiling_maker::slot_cellSelected(int row, int col)
{
    Q_UNUSED(row)
    PlacedFeaturePtr pfp = getFeatureColumn(col);
    updateFeaturePointInfo(pfp);
    tilingMaker->setCurrentFeature(pfp);
    tilingMaker->forceRedraw();
}

void page_tiling_maker::updateFeaturePointInfo(PlacedFeaturePtr pfp)
{
    FeaturePtr       fp  = pfp->getFeature();
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
    featureInfo->setText(astring);
}

void page_tiling_maker::slot_currentFeature(int index)
{
    QVector<PlacedFeaturePtr> & all  = tilingMaker->getAllFeatures();
    PlacedFeaturePtr pf = all[index];

    if (config->tm_showAllFeatures)
    {
        tileInfoTable->setFocus();
        tileInfoTable->selectColumn(index);
        updateFeaturePointInfo(pf);
    }
    else
    {
        if (tilingMaker->isIncluded(pf))
        {
            QVector<PlacedFeaturePtr> & intil = tilingMaker->getInTiling();
            index = intil.indexOf(pf);
            tileInfoTable->setFocus();
            tileInfoTable->selectColumn(index);
            updateFeaturePointInfo(pf);
        }
        else
        {
            tileInfoTable->clearSelection();
            featureInfo->clear();
        }
    }
}

void page_tiling_maker::displayBackgroundStatus(TilingPtr tiling)
{
    if (!tiling)
    {
        return;
    }

    if (!refresh)
    {
        return;
    }

    BkgdImgPtr bi = tiling->getBackground();
    if (bi)
    {
        Xform xform = bi->getBkgdXform();
        bkgdLayout.setTransform(xform);

        chk_adjustBkgd->setChecked(bi->bAdjustPerspective);
        chk_showBkgd->setChecked(bi->bShowBkgd);
        QString name = bi->getName();
        if (name.isEmpty()) name = "none";
        bkgdGroup->setTitle(QString("Background Image:  %1").arg(name));
    }
    else
    {
        bkgdGroup->setTitle(QString("Background Image:  none"));
        bkgdLayout.init();
    }
}

void page_tiling_maker::slot_loadBackground()
{
    TilingPtr tiling = tilingMaker->getSelected();

    QString bkgdDir = config->rootMediaDir + "bkgd_photos/";
    QString filename = QFileDialog::getOpenFileName(nullptr,"Select image file",bkgdDir, "Image Files (*.png *.jpg *.bmp *.heic)");
    if (filename.isEmpty()) return;

    // load
    BkgdImgPtr bi = tiling->getBackground();
    if (!bi)
    {
        bi = make_shared<BackgroundImage>();
    }
    bool rv = bi->import(filename);
    if (rv)
    {
        tiling->setBackground(bi);

        chk_adjustBkgd->setChecked(false);
        chk_showBkgd->setChecked(true);

        slot_setBkgd();

        displayBackgroundStatus(tiling);
    }
}

void page_tiling_maker::slot_setBkgd()
{
    TilingPtr tiling = tilingMaker->getSelected();

    BkgdImgPtr bi = tiling->getBackground();
    if (bi)
    {
        Xform xform = bi->getBkgdXform();
        xform.setTransform(bkgdLayout.getQTransform());
        bi->updateBkgdXform(xform);

        bi->bkgdImageChanged(chk_showBkgd->isChecked(),
                             chk_adjustBkgd->isChecked());
    }

    if (config->viewerType == VIEW_TILING_MAKER)
    {
        emit sig_refreshView();
    }
}

void page_tiling_maker::slot_setBkgdXform()
{
    TilingPtr tiling = tilingMaker->getSelected();

    BkgdImgPtr bi = tiling->getBackground();
    if (bi)
    {
        Xform xform = bi->getBkgdXform();
        xform.setTransform(bkgdLayout.getQTransform());
        bi->updateBkgdXform(xform);
    }

    if (config->viewerType == VIEW_TILING_MAKER)
    {
        emit sig_refreshView();
    }
}

void page_tiling_maker::slot_adjustBackground()
{
    if (tilingMaker->getTilingMakerMouseMode() != TM_BKGD_SKEW_MODE)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Please press F10 or select Bkgd perspective");
        box.exec();
        return;
    }

    EdgePoly & waccum = tilingMaker->getAccumW();
    if (waccum.size() != 4)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Please select four points to skew perspective");
        box.exec();
        return;
    }

    TilingPtr tiling = tilingMaker->getSelected();

    BkgdImgPtr bi = tiling->getBackground();
    bi->adjustBackground(
        tilingMaker->worldToScreen(waccum[0]->getV1()->getPosition()),
        tilingMaker->worldToScreen(waccum[1]->getV1()->getPosition()),
        tilingMaker->worldToScreen(waccum[2]->getV1()->getPosition()),
        tilingMaker->worldToScreen(waccum[3]->getV1()->getPosition()));


    displayBackgroundStatus(tiling);
    tilingMaker->setTilingMakerMouseMode(TM_NO_MOUSE_MODE);

    if (config->viewerType == VIEW_TILING_MAKER)
    {
        emit sig_refreshView();
    }
}

void page_tiling_maker::slot_saveAdjustedBackground()
{
    TilingPtr tiling  = tilingMaker->getSelected();

    BkgdImgPtr bi   = tiling->getBackground();
    QString oldname = bi->getName();

    DlgName dlg;
    dlg.newEdit->setText(oldname);
    int retval = dlg.exec();
    if (retval == QDialog::Rejected)
    {
        qDebug() << "Canceled";
        return;
    }

    Q_ASSERT(retval == QDialog::Accepted);
    QString newName = dlg.newEdit->text();

    // save
    bool rv = bi->saveAdjusted(newName);

    QMessageBox box(this);
    if (rv)
    {
        box.setIcon(QMessageBox::Information);
        box.setText("OK");
    }
    else
    {
        box.setIcon(QMessageBox::Warning);
        box.setText("FAILED");
    }
    box.exec();
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

    PlacedFeaturePtr pfp = getFeatureColumn(col);
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

    GirihListSelect dlg(ts);
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
        qInfo() << "No feature selected";
        return;
    }

    for (auto it = qsl.begin(); it != qsl.end(); it++)
    {
        QString name = *it;
        PlacedFeaturePtr pfp = make_shared<PlacedFeature>();
        bool rv =  pfp->loadFromGirihShape(name);
        if (rv)
        {
            tilingMaker->addNewPlacedFeature(pfp);
        }
    }

    buildMenu();

    if (config->viewerType == VIEW_TILING_MAKER)
    {
        tilingMaker->forceRedraw();
    }
}

void page_tiling_maker::slot_addGirihShape()
{
    QString name = girihShapes->currentData().toString();
    PlacedFeaturePtr pfp = make_shared<PlacedFeature>();
    bool rv =  pfp->loadFromGirihShape(name);
    if (rv)
    {
        QTransform t;
        tilingMaker->addNewPlacedFeature(pfp);
        buildMenu();
        if (config->viewerType == VIEW_TILING_MAKER)
        {
            tilingMaker->forceRedraw();
        }
    }
}

void page_tiling_maker::tableHeaderClicked(int index)
{
    qDebug() << "index=" << index;
    if (index == TI_X || index == TI_Y)
    {
        DlgTrim * trim = new DlgTrim(this);
        connect (trim, &DlgTrim::sig_apply, this, &page_tiling_maker::slot_trim);
        trim->setModal(false);
        trim->show();
    }
}

void page_tiling_maker::slot_trim(qreal valX, qreal valY)
{
    QVector<PlacedFeaturePtr> & pfeatures = (config->tm_showAllFeatures) ? tilingMaker->getAllFeatures() : tilingMaker->getInTiling();
    for (auto it = pfeatures.begin(); it != pfeatures.end(); it++)
    {
        PlacedFeaturePtr pfp = *it;
        QTransform t1 = pfp->getTransform();
        QTransform t2 = QTransform::fromTranslate(valX,valY);
        pfp->setTransform(t1*t2);
    }
    tilingMaker->forceRedraw();
    refreshMenuData();
}

void page_tiling_maker::pushTilingToMotifMaker(eSM_Event event)
{
    tilingMaker->sm_take(tilingMaker->getSelected(),event);
}




