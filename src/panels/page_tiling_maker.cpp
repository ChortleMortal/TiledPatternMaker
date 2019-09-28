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
#include "designs/patterns.h"
#include "base/canvas.h"
#include "base/utilities.h"
#include "base/tiledpatternmaker.h"
#include "style/Style.h"
#include "panels/dlg_feature_edit.h"
#include "panels/dlg_name.h"
#include "base/fileservices.h"
#include "panels/dlg_listselect.h"
#include "panels/dlg_listnameselect.h"
#include "panels/dlg_trim.h"
#include "panels/panel.h"

using std::string;

Q_DECLARE_METATYPE(PlacedFeaturePtr)

page_tiling_maker:: page_tiling_maker(ControlPanel * cpanel)  : panel_page(cpanel,"Tiling Maker")
{
    hideTable  = false;

    // create tiling designer
    tilingMaker = TilingMaker::getInstance();

    createTopGrid();

    AQWidget * widget = createTilingTable();
    vbox->addWidget(widget);

    widget = createTiliingMakerControls();
    vbox->addWidget(widget);

    vbox->addStretch(); // end vbox

    connect(&sidesMapper,  SIGNAL(mapped(int)), this, SLOT(slot_sidesChanged(int)));
    connect(&scaleMapper,  SIGNAL(mapped(int)), this, SLOT(slot_transformChanged(int)));
    connect(&rotMapper,    SIGNAL(mapped(int)), this, SLOT(slot_transformChanged(int)));
    connect(&xMapper,      SIGNAL(mapped(int)), this, SLOT(slot_transformChanged(int)));
    connect(&yMapper,      SIGNAL(mapped(int)), this, SLOT(slot_transformChanged(int)));

    connect(&tile_name,  SIGNAL(editingFinished()), this, SLOT(slot_nameChanged()));
    connect(&tile_author,SIGNAL(editingFinished()), this, SLOT(slot_authorChanged()));
    connect(&tile_desc,  SIGNAL(textChanged()),     this, SLOT(slot_descChanged()));

    connect(maker,  &TiledPatternMaker::sig_loadedTiling,   this,   &page_tiling_maker::slot_loadedTiling);
    connect(maker,  &TiledPatternMaker::sig_loadedXML,      this,   &page_tiling_maker::slot_loadedXML);
    connect(this,   &page_tiling_maker::sig_loadTiling,     maker,  &TiledPatternMaker::slot_loadTiling);

    connect(tilingMaker,      &TilingMaker::sig_buildMenu,      this,  &page_tiling_maker::slot_buildMenu);
    connect(tilingMaker,      &TilingMaker::sig_refreshMenu,    this,  &page_tiling_maker::slot_refreshMenu);
    connect(tilingMaker,      &TilingMaker::sig_current_feature,this,  &page_tiling_maker::slot_currentFeature);

    connect(canvas, &Canvas::sig_deltaScale,    this, &page_tiling_maker::slot_scale);
    connect(canvas, &Canvas::sig_deltaRotate,   this, &page_tiling_maker::slot_rotate);
    connect(canvas, &Canvas::sig_deltaMoveY,    this, &page_tiling_maker::slot_moveY);
    connect(canvas, &Canvas::sig_deltaMoveX,    this, &page_tiling_maker::slot_moveX);
}

void page_tiling_maker::createTopGrid()
{
    QHBoxLayout* hbox1 = new QHBoxLayout;

    ///
    /// Source select row
    ///

    radioLoadedStyleTileView      = new QRadioButton("Style");
    radioWSTileView               = new QRadioButton("Workspace");
    debugLabel                    = new QLabel();
    QPushButton * reloadTilingBtn = new QPushButton("Re-load Tiling");
    QPushButton * pbSave          = new QPushButton("Save XML");
    QPushButton * pbClearWS       = new QPushButton("Clear WS");

    hbox1->addWidget(radioLoadedStyleTileView);
    hbox1->addWidget(radioWSTileView);
    hbox1->addStretch();
    hbox1->addWidget(debugLabel);
    hbox1->addStretch();
    hbox1->addWidget(pbClearWS);
    hbox1->addWidget(reloadTilingBtn);
    hbox1->addWidget(pbSave);

    tilingGroup3.addButton(radioLoadedStyleTileView,TD_STYLE);
    tilingGroup3.addButton(radioWSTileView,TD_WORKSPACE);

    connect(&tilingGroup3,    SIGNAL(buttonClicked(int)), this, SLOT(slot_sourceSelect(int)));
    connect(pbClearWS,        &QPushButton::clicked,      this, &page_tiling_maker::slot_clearWS);
    connect(reloadTilingBtn,  &QPushButton::clicked,      this, &page_tiling_maker::slot_reloadTiling);
    connect(pbSave,           &QPushButton::clicked,      this, &page_tiling_maker::slot_saveTiling);

    ///
    ///  Fill Data Row
    ///

    QHBoxLayout * hbox2 = new QHBoxLayout;

    const int rmin = -1000;
    const int rmax =  1000;

    xRepMin = new SpinSet("xMin",0,rmin,rmax);
    xRepMax = new SpinSet("xMax",0,rmin,rmax);
    yRepMin = new SpinSet("yMin",0,rmin,rmax);
    yRepMax = new SpinSet("yMax",0,rmin,rmax);
    QPushButton * pb =  new QPushButton("Set");

    QComboBox * xformModeCombo = new QComboBox();
    xformModeCombo->insertItem(XF_VIEW,  "XForm View");
    xformModeCombo->insertItem(XF_BKGD,  "XForm Background");
    xformModeCombo->insertItem(XF_TILING,"XForm Tiling");
    xformModeCombo->setCurrentIndex(XF_VIEW);
    tilingMaker->slot_xformMode_changed(XF_VIEW);

    QPushButton * replaceInDesignBtn   = new QPushButton("Replace in Design");
    replaceInDesignBtn->setMinimumWidth(121);

    hbox2->addLayout(xRepMin);
    hbox2->addLayout(xRepMax);
    hbox2->addLayout(yRepMin);
    hbox2->addLayout(yRepMax);
    hbox2->addWidget(pb);
    hbox2->addStretch();
    hbox2->addWidget(xformModeCombo);
    hbox2->addStretch();
    hbox2->addWidget(replaceInDesignBtn);

    connect(pb,                  &QPushButton::clicked, this,  &page_tiling_maker::slot_set_reps);
    connect(replaceInDesignBtn,  &QPushButton::clicked, this,  &page_tiling_maker::slot_replaceTilingInStyles);
    connect(xformModeCombo,      SIGNAL(currentIndexChanged(int)), tilingMaker, SLOT(slot_xformMode_changed(int)));

    ///
    /// Translations row
    ///

    QGridLayout * grid3 = new QGridLayout;
    int row = 0;

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

    grid3->addLayout(t1x,row,0);
    grid3->addLayout(t1y,row,1);
    grid3->addLayout(t2x,row,2);
    grid3->addLayout(t2y,row,3);

    QPushButton * swapBtn = new QPushButton("Swap");
    grid3->addWidget(swapBtn,row,4);

    QObject::connect(t1x,  SIGNAL(valueChanged(double)), this, SLOT(slot_t1t2Changed(double)));
    QObject::connect(t1y,  SIGNAL(valueChanged(double)), this, SLOT(slot_t1t2Changed(double)));
    QObject::connect(t2x,  SIGNAL(valueChanged(double)), this, SLOT(slot_t1t2Changed(double)));
    QObject::connect(t2y,  SIGNAL(valueChanged(double)), this, SLOT(slot_t1t2Changed(double)));
    QObject::connect(swapBtn,  &QPushButton::clicked,    this, &page_tiling_maker::slot_swapTrans);

    vbox->addLayout(hbox1);
    vbox->addLayout(hbox2);
    vbox->addLayout(grid3);
}

AQWidget * page_tiling_maker::createTilingTable()
{
    // tile info table
    tileInfoTable = new QTableWidget();
    tileInfoTable->setSortingEnabled(false);
    tileInfoTable->setColumnCount(10);
    tileInfoTable->setSelectionMode(QAbstractItemView::SingleSelection);
    tileInfoTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    tileInfoTable->setContextMenuPolicy(Qt::CustomContextMenu);
    tileInfoTable->setStyleSheet("selection-color : white; selection-background-color : green");
    tileInfoTable->setMaximumHeight(600);
    QStringList qslh;
    qslh << "index" << "sides" << "scale" << "rot" << "X" << "Y" << "reg" << "CW" << "feat" << "loc";
    tileInfoTable->setHorizontalHeaderLabels(qslh);
    tileInfoTable->verticalHeader()->setVisible(false);
    tileInfoTable->setColumnWidth(0,10);
    tileInfoTable->setColumnWidth(1,10);

    QHeaderView * qhv = tileInfoTable->horizontalHeader();

    tile_desc.setFixedHeight(70);
    featureInfo.setFixedHeight(50);

    chk_hideTable                 = new QCheckBox("Hide table");
    chk_autoFill                  = new QCheckBox("Auto-fill on load");
    chk_showOverlaps              = new QCheckBox("Show overlaps/touching");
    QCheckBox * chk_all_features  = new QCheckBox("All features in table");
    chk_all_features->setChecked(config->showAllFeatures);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->setSpacing(7);

    hbox->addWidget(chk_hideTable);
    hbox->addWidget(chk_all_features);
    hbox->addWidget(chk_autoFill);
    hbox->addWidget(chk_showOverlaps);

    AQVBoxLayout * vlayout = new AQVBoxLayout();
    vlayout->addLayout(hbox);
#ifdef LAYER_XFORM_INFO
    vlayout->addSpacing(3);
    vlayout->addLayout(&layerXform);
    vlayout->addSpacing(5);
    vlayout->addLayout(&layerDeltas);
#endif
    vlayout->addSpacing(11);
    vlayout->addWidget(tileInfoTable);
    vlayout->addSpacing(11);
    vlayout->addWidget(&featureInfo);
    vlayout->addSpacing(3);
    vlayout->addWidget(&tile_desc);
    vlayout->addSpacing(3);
    vlayout->addWidget(&tile_author);
    vlayout->addSpacing(3);
    vlayout->addWidget(&tile_name);
    vlayout->addSpacing(5);

    AQWidget * widget = new AQWidget();
    widget->setLayout(vlayout);

    connect(chk_hideTable,  &QCheckBox::clicked,     this,   &page_tiling_maker::slot_hideTable);
    connect(chk_all_features,&QCheckBox::clicked,    this,   &page_tiling_maker::slot_all_features);
    connect(chk_showOverlaps,&QCheckBox::clicked,    tilingMaker, &TilingMaker::slot_showOverlaps);
    connect(tileInfoTable,   &QTableWidget::customContextMenuRequested, this, &page_tiling_maker::slot_menu);
    connect(qhv,             &QHeaderView::sectionClicked, this, &page_tiling_maker::tableHeaderClicked);

    return widget;
}

void page_tiling_maker::slot_menu(QPointF spt)
{
   qDebug() << "menu spt=" << spt;

   QMenu myMenu(tileInfoTable);

   myMenu.addAction("Edit Feature", this, SLOT(slot_menu_edit_feature()));

   int row = tileInfoTable->currentRow();
   PlacedFeaturePtr pfp = getFeatureRow(row);
   QString str =  (tilingMaker->getInTiling().contains(pfp))  ? "Exclude" : "Include";
   myMenu.addAction(str, this, SLOT(slot_menu_includePlaced()));

   myMenu.addAction("Remove Feature", this, SLOT(slot_remove_clicked()));

   myMenu.exec(tileInfoTable->viewport()->mapToGlobal(spt.toPoint()));
}

void page_tiling_maker::slot_menu_edit_feature()
{
    qDebug() << "selected edit feature";
    int              row = tileInfoTable->currentRow();
    PlacedFeaturePtr pfp = getFeatureRow(row);
    FeaturePtr        fp = pfp->getFeature();
    QTransform         t = pfp->getTransform();

    DlgFeatureEdit * fe  = new DlgFeatureEdit(fp->getEdgePoly(),t,this);
    fe->show();
    fe->raise();
    fe->activateWindow();

    connect(fe,&DlgFeatureEdit::sig_currentPoint,tilingMaker, &TilingMaker::setFeatureEditPoint,Qt::UniqueConnection);
}

void page_tiling_maker::slot_menu_includePlaced()
{
    int row = tileInfoTable->currentRow();
    PlacedFeaturePtr pfp = getFeatureRow(row);

    TilingSelectionPtr tsp = make_shared<TilingSelection>(INTERIOR,pfp);
    tilingMaker->toggleInclusion(tsp);
}

AQWidget * page_tiling_maker::createTiliingMakerControls()
{

    // actions
    QPushButton * addPolyBtn      = new QPushButton("Add regular poly (A)");
    QPushButton * addGirihBtn     = new QPushButton("Add girih shape");
    QPushButton * exclAllBtn      = new QPushButton("Exclude all (E)");
    QPushButton * fillVectorsBtn  = new QPushButton("Fill using Vectors (F)");
    QPushButton * removeExclBtn   = new QPushButton("Remove Excluded (R)");
    QPushButton * clearVectorsBtn = new QPushButton("Clear Vectors (V)");

    QPushButton * exportBtn       = new QPushButton("Export Poly");
    QPushButton * importBtn       = new QPushButton("Import Poly");

    sides = new QSpinBox;
    sides->setFixedWidth(41);
    sides->setMinimum(3);
    sides->setMaximum(128);

    girihShapes = new QComboBox();
    girihShapes->addItem("Decagon", "gDecagon");
    girihShapes->addItem("Pentagon","gPentagon");
    girihShapes->addItem("Bowtie",  "gBowtie");
    girihShapes->addItem("Kite",    "gKite");
    girihShapes->addItem("Rhombus", "gRhombus");

    QGridLayout  * actionBox = new QGridLayout;

    int row = 0;
    actionBox->addWidget(addPolyBtn,row,0);
    actionBox->addWidget(sides,row,1,Qt::AlignLeft);
    actionBox->addWidget(fillVectorsBtn,row,2);
    actionBox->addWidget(removeExclBtn,row,3);
    actionBox->addWidget(importBtn,row,4);

    row++;
    actionBox->addWidget(addGirihBtn,row,0);
    actionBox->addWidget(girihShapes,row,1);
    actionBox->addWidget(clearVectorsBtn,row,2);
    actionBox->addWidget(exclAllBtn,row,3);
    actionBox->addWidget(exportBtn,row,4);

    QGroupBox * actionGroup = new QGroupBox("Actions");
    actionGroup->setLayout(actionBox);

    //
    // background
    //

    QPushButton * loadBkgdBtn     = new QPushButton("Load Background");
    QPushButton * adjustBtn       = new QPushButton("Adjust perspective");
    QPushButton * saveAdjustedBtn = new QPushButton("Save Adjusted");

    showBkgd        = new QCheckBox("Show Background");
    perspectiveBkgd = new QCheckBox("Perspective");
    transformBkgd   = new QCheckBox("Xform");
    hideTiling      = new QCheckBox("Hide Tiling");

    QGridLayout * bkgGrid = new QGridLayout;

    row=0;
    bkgGrid->addWidget(loadBkgdBtn,row,0);
    bkgGrid->addLayout(&bkgdLayout,row,1,1,4);
    bkgGrid->addWidget(adjustBtn,row,5);

    row++;
    bkgGrid->addWidget(showBkgd,row,0);
    bkgGrid->addWidget(perspectiveBkgd,row,1);
    bkgGrid->addWidget(transformBkgd,row,2);
    bkgGrid->addWidget(hideTiling,row,3);
    bkgGrid->addWidget(saveAdjustedBtn,row,4);

    bkgdGroup = new QGroupBox("Background Image");
    bkgdGroup->setLayout(bkgGrid);

    //
    // modes
    //

    QRadioButton * nomode       = new QRadioButton("No Mode (ESC)");
    QRadioButton * drawTrans    = new QRadioButton("Translation Vectors (F3)");
    QRadioButton * newPoly      = new QRadioButton("Create Polygon (F4)");
    QRadioButton * copyPoly     = new QRadioButton("Copy Polygon (F5)");
    QRadioButton * deletePoly   = new QRadioButton("Delete Polygon (F6)");
    QRadioButton * includePoly  = new QRadioButton("Include/Exclude (F7)");
    QRadioButton * position     = new QRadioButton("Show Position (F8)");
    QRadioButton * measure      = new QRadioButton("Measure (F9)");
    QRadioButton * perspective  = new QRadioButton("Bkgd Perspective (F10)");
    QRadioButton * editPoly     = new QRadioButton("Edit Feature (F11)");
    QRadioButton * mirrorX      = new QRadioButton("Mirror X");
    QRadioButton * mirrorY      = new QRadioButton("Mirror Y");
    QRadioButton * curveEdge    = new QRadioButton("Curve Edge (F12)");
    QRadioButton * flattenEdge  = new QRadioButton("Flatten Edge");
    QCheckBox    * convexEdge   = new QCheckBox("Convex Edge");

    QGridLayout * modeBox = new QGridLayout();

    row=0;
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
    modeBox->addWidget(flattenEdge,row,2);
    modeBox->addWidget(convexEdge,row,3);
    modeBox->addWidget(curveEdge,row,4);

    QGroupBox * modeGroup = new QGroupBox("Modes");
    modeGroup->setLayout(modeBox);

    // grouping
    mouseModeBtnGroup = new QButtonGroup;
    mouseModeBtnGroup->addButton(drawTrans,     TRANSLATION_VECTOR_MODE);
    mouseModeBtnGroup->addButton(newPoly,       DRAW_POLY_MODE);
    mouseModeBtnGroup->addButton(copyPoly,      COPY_MODE);
    mouseModeBtnGroup->addButton(deletePoly,    DELETE_MODE);
    mouseModeBtnGroup->addButton(includePoly,   INCLUSION_MODE);
    mouseModeBtnGroup->addButton(nomode,        NO_MOUSE_MODE);
    mouseModeBtnGroup->addButton(position,      POSITION_MODE);
    mouseModeBtnGroup->addButton(measure,       MEASURE_MODE);
    mouseModeBtnGroup->addButton(perspective,   BKGD_SKEW_MODE);
    mouseModeBtnGroup->addButton(editPoly,      EDIT_FEATURE_MODE);
    mouseModeBtnGroup->addButton(curveEdge,     CURVE_EDGE_MODE);
    mouseModeBtnGroup->addButton(flattenEdge,   FLATTEN_EDGE_MODE);
    mouseModeBtnGroup->addButton(mirrorX,       MIRROR_X_MODE);
    mouseModeBtnGroup->addButton(mirrorY,       MIRROR_Y_MODE);

    AQVBoxLayout * avbox = new AQVBoxLayout;
    avbox->addWidget(modeGroup);
    avbox->addWidget(actionGroup);
    avbox->addWidget(bkgdGroup);

    AQWidget * widget = new AQWidget();
    widget->setLayout(avbox);

    connect(loadBkgdBtn,    &QPushButton::clicked,            this,    &page_tiling_maker::slot_loadBackground);
    connect(adjustBtn,      &QPushButton::clicked,            this,    &page_tiling_maker::slot_adjustBackground);
    connect(saveAdjustedBtn,&QPushButton::clicked,            this,    &page_tiling_maker::slot_saveAdjustedBackground);
    connect(exportBtn,      &QPushButton::clicked,            this,    &page_tiling_maker::slot_exportPoly);
    connect(importBtn,      &QPushButton::clicked,            this,    &page_tiling_maker::slot_importPoly);
    connect(&bkgdLayout,    &LayoutTransform::xformChanged,   this,    &page_tiling_maker::slot_bkgdTransformChanged);
    connect(showBkgd,       &QAbstractButton::clicked,        this,    &page_tiling_maker::slot_bkgdImageChanged);
    connect(perspectiveBkgd,&QAbstractButton::clicked,        this,    &page_tiling_maker::slot_bkgdImageChanged);
    connect(transformBkgd,  &QAbstractButton::clicked,        this,    &page_tiling_maker::slot_bkgdImageChanged);
    connect(addGirihBtn,    &QPushButton::clicked,            this,    &page_tiling_maker::slot_addGirihShape);

    connect(hideTiling,     &QCheckBox::clicked,              tilingMaker,&TilingMaker::hide);
    connect(convexEdge,     &QCheckBox::clicked,              tilingMaker,&TilingMaker::setConvexEdge);

    connect(mouseModeBtnGroup, SIGNAL(buttonClicked(int)),     this,        SLOT(slot_setModes(int)));
    connect(sides,             SIGNAL(valueChanged(int)),      tilingMaker,    SLOT(updatePolygonSides(int)));

    connect(addPolyBtn,       &QPushButton::clicked,    tilingMaker,    &TilingMaker::addRegularPolygon);
    connect(clearVectorsBtn,  &QPushButton::clicked,    tilingMaker,    &TilingMaker::clearTranslation);
    connect(fillVectorsBtn,   &QPushButton::clicked,    tilingMaker,    &TilingMaker::fillUsingTranslations);
    connect(removeExclBtn,    &QPushButton::clicked,    tilingMaker,    &TilingMaker::removeExcluded);
    connect(exclAllBtn,       &QPushButton::clicked,    tilingMaker,    &TilingMaker::excludeAll);

    sides->setValue(8);
    convexEdge->setChecked(true);

    return widget;
}

void page_tiling_maker::slot_all_features(bool checked)
{
    config->showAllFeatures = checked;
    buildMenu();
}

void page_tiling_maker::slot_setModes(int mode)
{
    eMouseMode mm = static_cast<eMouseMode>(mode);
    tilingMaker->setMouseMode(mm);

    QString txt;
    switch (mm)
    {
    case INCLUSION_MODE:
        txt = "Toggle the inclusion of polygons in the tiling by clicking on them with the mouse.";
        break;
    case DRAW_POLY_MODE:
        txt = "Select a series of vertices counter-clockwise to draw a free-form polygon. (Click on vertices).";
        break;
    case TRANSLATION_VECTOR_MODE:
        txt = "Select the two translation vectors used to tile the plane, using the mouse. (Drag with the mouse).";
        break;
    case BKGD_SKEW_MODE:
        txt = "Click to select four points on background image. Then press 'Adjust Perspective' to fix camera skew.";
        break;
    case NO_MOUSE_MODE:
    case COPY_MODE:
    case DELETE_MODE:
    case POSITION_MODE:
    case MEASURE_MODE:
    case EDIT_FEATURE_MODE:
    case CURVE_EDGE_MODE:
    case MIRROR_X_MODE:
    case MIRROR_Y_MODE:
    case FLATTEN_EDGE_MODE:
        break;
    }
    panel->setStatus(txt);
}

void page_tiling_maker::clear()
{
    tile_desc.blockSignals(true);
    tile_desc.clear();
    tile_desc.blockSignals(false);
    tile_name.clear();
    tile_author.clear();
    featureInfo.clear();

    tileInfoTable->clearContents();
    tileInfoTable->setMaximumHeight(401);

    t1x->blockSignals(true);
    t1y->blockSignals(true);
    t2x->blockSignals(true);
    t2y->blockSignals(true);

    t1x->setValue(0);
    t1y->setValue(0);
    t2x->setValue(0);
    t2y->setValue(0);

    t1x->blockSignals(false);
    t1y->blockSignals(false);
    t2x->blockSignals(false);
    t2y->blockSignals(false);
}

void  page_tiling_maker::refreshPage()
{
    TilingPtr tiling = workspace->getLoadedStyles().getTiling();
    QString txt = "Style ";
    txt        += addr(tiling.get());
    if (tiling)
    {
        txt += " ";
        txt += tiling->getName();
    }
    radioLoadedStyleTileView->setText(txt);

    tiling = workspace->getTiling();
    txt  = "Workspace ";
    txt += addr(tiling.get());
    if (tiling)
    {
        txt += " ";
        txt += tiling->getName();
    }
    radioWSTileView->setText(txt);

    tilingGroup3.button(config->tilingMakerViewer)->setChecked(true);
    mouseModeBtnGroup->button(tilingMaker->getMouseMode())->setChecked(true);

    sides->setValue(tilingMaker->getPolygonSides());

#ifdef DEBUG_MOUSE_POS
    QPointF a = designer->sMousePos;
    QPointF b = designer->screenToWorld(a);
    QString astring;
    QTextStream ts(&astring);
    ts << "pos: (" << a.x() << ", " << a.y() << ") ("
                   << b.x() << ", " << b.y() << ")";
    debugLabel->setText(astring);
#else
    QString status = tilingMaker->getStatus();
    debugLabel->setText(status);
#endif

#ifdef LAYER_XFORM_INFO
    QTransform t = designer->getLayerTransform();
    layerXform.setTransform(t);

    Xform xf = designer->getDeltas();
    layerDeltas.setX(xf.translateX);
    layerDeltas.setY(xf.translateY);
    layerDeltas.setScale(xf.scale);
    layerDeltas.setRot(xf.rotation);
#endif
}

void  page_tiling_maker::onExit()
{
    canvas->setKbdMode(KBD_MODE_TRANSFORM);
}

void  page_tiling_maker::onEnter()
{
    emit sig_viewWS();

    clear();

    int id = tilingMaker->getMouseMode();
    mouseModeBtnGroup->button(id)->setChecked(true);

    tilingGroup3.button(config->tilingMakerViewer)->setChecked(true);

    TilingPtr tiling = getSourceTiling();
    if (!tiling)
    {
        // if workspace is selected then make a new tiling;
        if (config->tilingMakerViewer== TD_WORKSPACE)
        {
            tiling = make_shared<Tiling>();
            workspace->setTiling(tiling);
        }
        else
        {
            // nothing to work on
            Q_ASSERT(config->tilingMakerViewer== TD_STYLE);
            tilingMaker->clearDesignerData();
            lastTiling.reset();
            return;
        }
    }

    if (tiling != lastTiling)
    {
        // only set tiling if it has changed
        lastTiling = tiling;
        tilingMaker->setTiling(tiling);
    }

    tilingMaker->viewRectChanged();

    useBackground(tiling);

    buildMenu();

    if (config->viewerType == VIEW_TILIING_MAKER)
    {
        tilingMaker->forceRedraw();
    }
}

void page_tiling_maker::slot_buildMenu()
{
    buildMenu();
}

void page_tiling_maker::slot_refreshMenu()
{
    refreshMenuData();
}

void page_tiling_maker::buildMenu()
{
    qDebug() << "page_tiling_maker::buildMenu";
    tileInfoTable->clearContents();
    tileInfoTable->setRowCount(0);

    int row = 0;
    QVector<PlacedFeaturePtr> & pfeatures = (config->showAllFeatures) ? tilingMaker->getAllFeatures() : tilingMaker->getInTiling();
    for (auto pfp : pfeatures)
    {
        QString inclusion = (tilingMaker->getInTiling().contains(pfp)) ? "included" : "excluded";
        buildTableEntry(pfp,row++,inclusion);
    }

    connect(tileInfoTable, SIGNAL(cellClicked(int,int)),  this, SLOT(slot_cellSelected(int,int)),Qt::UniqueConnection);

    tileInfoTable->resizeColumnToContents(2);
    tileInfoTable->resizeColumnToContents(3);
    tileInfoTable->resizeColumnToContents(4);
    tileInfoTable->resizeColumnToContents(5);
    tileInfoTable->resizeColumnToContents(6);
    tileInfoTable->resizeColumnToContents(7);
    tileInfoTable->resizeColumnToContents(8);
    tileInfoTable->resizeColumnToContents(9);
    adjustTableSize(tileInfoTable);
    updateGeometry();

    if (tileInfoTable->height() > 401)
    {
        tileInfoTable->setFixedHeight(401);
    }

    tileInfoTable->setVisible(!hideTable);

    refreshMenuData();
}

void page_tiling_maker::refreshMenuData()
{
    qDebug() << "page_tiling_maker::refreshMenuData";
    TilingPtr tiling = tilingMaker->getTiling();
    if (!tiling) return;

    tile_name.setText(tiling->getName());
    tile_author.setText(tiling->getAuthor());
    tile_desc.blockSignals(true);
    tile_desc.setText(tiling->getDescription());
    tile_desc.blockSignals(false);

    QPointF t1 = tilingMaker->getTrans1();
    QPointF t2 = tilingMaker->getTrans2();
    tiling->setTrans1(t1);
    tiling->setTrans2(t2);

    t1x->blockSignals(true);
    t1y->blockSignals(true);
    t2x->blockSignals(true);
    t2y->blockSignals(true);

    t1x->setValue(t1.x());
    t1y->setValue(t1.y());
    t2x->setValue(t2.x());
    t2y->setValue(t2.y());

    t1x->blockSignals(false);
    t1y->blockSignals(false);
    t2x->blockSignals(false);
    t2y->blockSignals(false);

    int xMin,xMax,yMin,yMax;
    tiling->getFillData().get(xMin ,xMax,yMin,yMax);
    xRepMin->setValue(xMin);
    xRepMax->setValue(xMax);
    yRepMin->setValue(yMin);
    yRepMax->setValue(yMax);

    int row = 0;
    QVector<PlacedFeaturePtr> & pfeatures = (config->showAllFeatures) ? tilingMaker->getAllFeatures() : tilingMaker->getInTiling();
    for (auto pfp : pfeatures)
    {
        QString inclusion = (tilingMaker->getInTiling().contains(pfp)) ? "included" : "excluded";
        refreshTableEntry(pfp,row++,inclusion);
    }


    row = tileInfoTable->currentRow();
    if (row >= 0)
    {
        PlacedFeaturePtr pfp = getFeatureRow(row);
        updateFeaturePointInfo(pfp);
    }
}

void page_tiling_maker::buildTableEntry(PlacedFeaturePtr pf, int row, QString inclusion)
{
    FeaturePtr feature  = pf->getFeature();
    QTransform T        = pf->getTransform();

    tileInfoTable->setRowCount(row+1);

    QTableWidgetItem * twi = new QTableWidgetItem(QString::number(row));
    twi->setData(Qt::UserRole,QVariant::fromValue(pf));
    tileInfoTable->setItem(row,TI_INDEX,twi);

    QSpinBox * sp = new QSpinBox;
    sp->setValue(feature->numPoints());
    tileInfoTable->setCellWidget(row,TI_SIDES,sp);
    QObject::connect(sp, SIGNAL(valueChanged(int)), &sidesMapper, SLOT(map()));
    sidesMapper.setMapping(sp,row);

    QDoubleSpinBox * dsp = new QDoubleSpinBox;
    dsp->setRange(0,128);
    dsp->setDecimals(16);
    dsp->setSingleStep(0.01);
    dsp->setAlignment(Qt::AlignRight);
    dsp->setValue(Transform::scalex(T));
    tileInfoTable->setCellWidget(row,TI_SCALE,dsp);
    QObject::connect(dsp, SIGNAL(valueChanged(double)), &scaleMapper, SLOT(map()));
    scaleMapper.setMapping(dsp,row);

    dsp = new QDoubleSpinBox;
    dsp->setRange(-360.0,360.0);
    dsp->setDecimals(16);
    dsp->setAlignment(Qt::AlignRight);
    dsp->setValue(qRadiansToDegrees(Transform::rotation(T)));
    tileInfoTable->setCellWidget(row,TI_ROT,dsp);
    QObject::connect(dsp, SIGNAL(valueChanged(double)), &rotMapper, SLOT(map()));
    rotMapper.setMapping(dsp,row);

    dsp = new QDoubleSpinBox;
    dsp->setRange(-100.0,100.0);
    dsp->setDecimals(16);
    dsp->setAlignment(Qt::AlignRight);
    dsp->setValue(Transform::transx(T));
    tileInfoTable->setCellWidget(row,TI_X,dsp);
    QObject::connect(dsp, SIGNAL(valueChanged(double)), &xMapper, SLOT(map()));
    xMapper.setMapping(dsp,row);

    dsp = new QDoubleSpinBox;
    dsp->setRange(-100.0,100.0);
    dsp->setDecimals(16);
    dsp->setAlignment(Qt::AlignRight);
    dsp->setValue(Transform::transy(T));
    tileInfoTable->setCellWidget(row,TI_Y,dsp);
    QObject::connect(dsp, SIGNAL(valueChanged(double)), &yMapper, SLOT(map()));
    yMapper.setMapping(dsp,row);

    QString regular = feature->isRegular() ? "Y" : "N";
    twi = new QTableWidgetItem(regular);
    tileInfoTable->setItem(row,TI_REGULAR,twi);

    QString cw = feature->isClockwise() ? "Y" : "N";
    twi = new QTableWidgetItem(cw);
    tileInfoTable->setItem(row,TI_CW,twi);

    twi = new QTableWidgetItem(addr(feature.get()));
    tileInfoTable->setItem(row,TI_FEAT_ADDR,twi);

    twi = new QTableWidgetItem(inclusion);
    tileInfoTable->setItem(row,TI_LOCATION,twi);
}

void page_tiling_maker::refreshTableEntry(PlacedFeaturePtr pf, int row, QString inclusion)
{
    if (row >= tileInfoTable->rowCount())
    {
        qDebug() << "table row " << row << "exceeds count" << tileInfoTable->rowCount();
        return;
    }

    FeaturePtr feature  = pf->getFeature();
    QTransform T        = pf->getTransform();

    QWidget * w = tileInfoTable->cellWidget(row,TI_SIDES);
    if (!w)
    {
        qDebug() << "table row not found";
        return;
    }

    QSpinBox * sp = dynamic_cast<QSpinBox*>(w);
    Q_ASSERT(sp);
    sp->setValue(feature->numPoints());

    w = tileInfoTable->cellWidget(row,TI_SCALE);
    QDoubleSpinBox * dsp = dynamic_cast<QDoubleSpinBox*>(w);
    Q_ASSERT(dsp);
    dsp->setValue(Transform::scalex(T));

    w = tileInfoTable->cellWidget(row,TI_ROT);
    dsp = dynamic_cast<QDoubleSpinBox*>(w);
    Q_ASSERT(dsp);
    dsp->setValue(qRadiansToDegrees(Transform::rotation(T)));

    w = tileInfoTable->cellWidget(row,TI_X);
    dsp = dynamic_cast<QDoubleSpinBox*>(w);
    Q_ASSERT(dsp);
    dsp->setValue(Transform::transx(T));

    w = tileInfoTable->cellWidget(row,TI_Y);
    dsp = dynamic_cast<QDoubleSpinBox*>(w);
    Q_ASSERT(dsp);
    dsp->setValue(Transform::transy(T));

    QString regular = feature->isRegular() ? "Y" : "N";
    QTableWidgetItem * twi = tileInfoTable->item(row,TI_REGULAR);
    twi->setText(regular);

    QString cw = feature->isClockwise() ? "Y" : "N";
    twi = tileInfoTable->item(row,TI_CW);
    twi->setText(cw);

    twi = tileInfoTable->item(row,TI_FEAT_ADDR);
    twi->setText(addr(feature.get()));

    twi = tileInfoTable->item(row,TI_LOCATION);
    twi->setText(inclusion);
}

PlacedFeaturePtr page_tiling_maker::getFeatureRow(int row)
{
    PlacedFeaturePtr pf;
    if (row == -1) return pf;

    QTableWidgetItem * twi = tileInfoTable->item(row,TI_INDEX);
    if (twi)
    {
        QVariant var = twi->data(Qt::UserRole);
        if (var.canConvert<PlacedFeaturePtr>())
        {
            pf = var.value<PlacedFeaturePtr>();
        }
        Q_ASSERT(pf);
    }
    return pf;
}

void page_tiling_maker::slot_sidesChanged(int row)
{
    QWidget * cw  = tileInfoTable->cellWidget(row,TI_SIDES);
    QSpinBox * sp = dynamic_cast<QSpinBox*>(cw);
    Q_ASSERT(sp);
    int sides = sp->value();
    FeaturePtr f2 = make_shared<Feature>(sides);

    PlacedFeaturePtr pf = getFeatureRow(row);
    pf->setFeature(f2);

    tilingMaker->forceRedraw();
    emit sig_tilingChanged();
}

void page_tiling_maker::slot_transformChanged(int row)
{
    QWidget        * cw  = tileInfoTable->cellWidget(row,TI_SCALE);
    QDoubleSpinBox * dsp = dynamic_cast<QDoubleSpinBox*>(cw);
    Q_ASSERT(dsp);
    qreal scale = dsp->value();
    if (scale <= 0.0 || scale > 128.0)
    {
        return;     // fixes problem with scales of 0 when entering data
    }

    cw  = tileInfoTable->cellWidget(row,TI_ROT);
    dsp = dynamic_cast<QDoubleSpinBox*>(cw);
    Q_ASSERT(dsp);
    qreal rotation = qDegreesToRadians(dsp->value());

    cw  = tileInfoTable->cellWidget(row,TI_X);
    dsp = dynamic_cast<QDoubleSpinBox*>(cw);
    Q_ASSERT(dsp);
    qreal tx = dsp->value();

    cw  = tileInfoTable->cellWidget(row,TI_Y);
    dsp = dynamic_cast<QDoubleSpinBox*>(cw);
    Q_ASSERT(dsp);
    qreal ty  = dsp->value();

    //Transform t = Transform::compose(scale, rotation, QPointF(tx,ty));
    QTransform t = QTransform().scale(scale,scale) * QTransform().rotateRadians(rotation) * QTransform::fromTranslate(tx,ty);
    qDebug().noquote() << "row=" << row << "T=" << Transform::toInfoString(t);

    PlacedFeaturePtr pf = getFeatureRow(row);
    pf->setTransform(t);

    canvas->invalidate();
    emit sig_tilingChanged();
}

void page_tiling_maker::slot_t1t2Changed(double val)
{
    Q_UNUSED(val)

    qreal x1 = t1x->value();
    qreal y1 = t1y->value();
    qreal x2 = t2x->value();
    qreal y2 = t2y->value();

    TilingPtr tiling = tilingMaker->getTiling();
    if (!tiling) return;

    tiling->setTrans1(QPointF(x1,y1));
    tiling->setTrans2(QPointF(x2,y2));

    tilingMaker->fixupTranslate();

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

void page_tiling_maker::slot_nameChanged()
{
    TilingPtr tiling = tilingMaker->getTiling();
    if (!tiling) return;
    tiling->setName(tile_name.text());
}

void page_tiling_maker::slot_authorChanged()
{
    TilingPtr tiling = tilingMaker->getTiling();
    if (!tiling) return;
    tiling->setAuthor(tile_author.text());
}

void page_tiling_maker::slot_descChanged()
{
    TilingPtr tiling = tilingMaker->getTiling();
    if (!tiling) return;
    tiling->setDescription(tile_desc.toPlainText());
}

void page_tiling_maker::slot_saveTiling()
{
    if (tile_name.text().isEmpty())
    {
        QMessageBox box(this);
        box.setText("Nothing to save");
        box.exec();
        return;
    }

    tilingMaker->updatePlacedFeaturesFromData();

    TilingPtr tiling = tilingMaker->getTiling();
    tiling->setViewTransform(tilingMaker->getDeltas());
    workspace->saveTiling(tile_name.text(),tiling);
}

void page_tiling_maker::slot_reloadTiling()
{
    TilingPtr tiling = tilingMaker->getTiling();
    if (!tiling) return;

    QString name = tiling->getName();
    emit sig_loadTiling(name);

}

// called by push button: Replace in Design
// puts the tiling into the loaded style set
void page_tiling_maker::slot_replaceTilingInStyles()
{
    TilingPtr tiling = tilingMaker->getTiling();
    if (!tiling) return;

    QList<FeaturePtr> qlfp = tiling->getUniqueFeatures();

    // update local tiling
    tilingMaker->updatePlacedFeaturesFromData();

    // update styles
    StyledDesign     & sd = workspace->getLoadedStyles();
    const StyleSet & sset = sd.getStyleSet();
    QVector<PrototypePtr> pps;
    for (auto stylePtr : sset)
    {
        PrototypePtr pp = stylePtr->getPrototype();
        if (pps.contains(pp))
        {
            continue;
        }
        pps.push_back(pp);
        pp->setTiling(tiling);
        pp->setFeaturesReversed(qlfp);
        pp->resetProtoMap();
        pp->createProtoMap();
    }

    for (auto stylePtr : sset)
    {
        stylePtr->resetStyleRepresentation();
        stylePtr->createStyleRepresentation();
    }

    emit sig_viewWS();
    emit sig_tilingChanged();
    buildMenu();
}

void page_tiling_maker::slot_sourceSelect(int id)
{
    config->tilingMakerViewer = eTilingMakerView(id);
    onEnter();
    emit sig_tilingChanged();
}

void page_tiling_maker::slot_remove_clicked()
{
    TilingPtr tiling = tilingMaker->getTiling();
    if (!tiling) return;

    int row = tileInfoTable->currentRow();
    PlacedFeaturePtr pf = getFeatureRow(row);

    tilingMaker->removeFeature(pf);
    buildMenu();
    emit sig_tilingChanged();
}

void page_tiling_maker::slot_set_reps()
{
    TilingPtr tiling = tilingMaker->getTiling();
    if (!tiling) return;

    FillData fd;
    fd.set(xRepMin->value(), xRepMax->value(), yRepMin->value(), yRepMax->value());
    tiling->setFillData(fd);

    slot_replaceTilingInStyles();
}

void page_tiling_maker::slot_cellSelected(int row, int col)
{
    Q_UNUSED(col)
    PlacedFeaturePtr pfp = getFeatureRow(row);
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
    featureInfo.setText(astring);
}

TilingPtr page_tiling_maker::getSourceTiling()
{
    TilingPtr tiling;
    switch (config->tilingMakerViewer)
    {
    case TD_STYLE:
        tiling = workspace->getLoadedStyles().getTiling();
        break;
    case TD_WORKSPACE:
        tiling = workspace->getTiling();
        break;
    }
    return tiling;
}

void page_tiling_maker::slot_currentFeature(int index)
{
    QVector<PlacedFeaturePtr> & all  = tilingMaker->getAllFeatures();
    PlacedFeaturePtr pf = all[index];

    if (config->showAllFeatures)
    {
        tileInfoTable->setFocus();
        tileInfoTable->selectRow(index);
        updateFeaturePointInfo(pf);
    }
    else
    {
        QVector<PlacedFeaturePtr> & intil = tilingMaker->getInTiling();
        if (intil.contains(pf))
        {
            index = intil.indexOf(pf);
            tileInfoTable->setFocus();
            tileInfoTable->selectRow(index);
            updateFeaturePointInfo(pf);
        }
        else
        {
            tileInfoTable->clearSelection();
            featureInfo.clear();
        }
    }
}

void page_tiling_maker::slot_loadedXML(QString name)
{
    Q_UNUSED(name)
    onEnter();
}

void page_tiling_maker::slot_loadedTiling (QString name)
{
    Q_UNUSED(name)
    onEnter();
    if (chk_autoFill->isChecked())
    {
        tilingMaker->fillUsingTranslations();
    }
}

void page_tiling_maker::slot_hideTable(bool checked)
{
    hideTable = checked;
    tileInfoTable->setVisible(!checked);
}

void page_tiling_maker::slot_clearWS()
{
    TilingPtr tp;
    workspace->setTiling(tp);
    onEnter();
}

void page_tiling_maker::useBackground(TilingPtr tiling)
{
    BackgroundImage & bi = tiling->getBackground();
    bkgdLayout.setX(bi.x);
    bkgdLayout.setY(bi.y);
    bkgdLayout.setScale(bi.scale);
    bkgdLayout.setRot(bi.rot);

    if (!bi.bkgdName.isEmpty())
    {
        showBkgd->setChecked(true);
        transformBkgd->setChecked(true);

        bool rv = bi.loadImageUsingName();
        if (!rv)
        {
            QMessageBox box;
            box.setIcon(QMessageBox::Critical);
            box.setText(QString("Error loading background image: %1 - FAILED").arg(bi.bkgdName));
            box.exec();
        }

        if (!bi.perspective.isIdentity())
        {
            perspectiveBkgd->setChecked(true);
            bi.adjustBackground();
        }
        else
        {
            perspectiveBkgd->setChecked(false);
        }
        slot_bkgdImageChanged();
    }
    else
    {
        showBkgd->setChecked(false);
        transformBkgd->setChecked(false);
        perspectiveBkgd->setChecked(false);
    }

    bkgdGroup->setTitle(QString("Background Image: %1").arg(bi.bkgdName));
}

void page_tiling_maker::slot_loadBackground()
{
    TilingPtr tiling = tilingMaker->getTiling();
    if (!tiling)
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Warning);
        box.setText("There is no tiling to load into.  Please select Worskpace to begin");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    QString bkgdDir = config->rootMediaDir + "bkgd_photos/";
    QString filename = QFileDialog::getOpenFileName(nullptr,"Select image file",bkgdDir, "Image Files (*.png *.jpg *.bmp *.heic)");
    if (filename.isEmpty()) return;

    // load
    BackgroundImage & bi = tiling->getBackground();
    bool rv = bi.loadAndCopy(filename);
    if (!rv) return;

    // update gui
    bkgdLayout.setTransform(bi.getTransform());
    showBkgd->setChecked(true);
    transformBkgd->setChecked(true);
    QFileInfo info(filename);
    QString name = info.fileName();
    bkgdGroup->setTitle(QString("Background Image: %1").arg(name));

    slot_bkgdImageChanged();
}

void page_tiling_maker::slot_bkgdImageChanged()
{
    TilingPtr tiling = tilingMaker->getTiling();
    if (!tiling) return;

    BackgroundImage & bi = tiling->getBackground();
    bi.x     = bkgdLayout.getX();
    bi.y     = bkgdLayout.getY();
    bi.scale = bkgdLayout.getScale();
    bi.rot   = bkgdLayout.getRot();

    if (config->viewerType != VIEW_TILIING_MAKER)
    {
        return;
    }

    bi.bkgdImageChanged(showBkgd->isChecked(),
                        perspectiveBkgd->isChecked(),
                        transformBkgd->isChecked());
}

void page_tiling_maker::slot_bkgdTransformChanged()
{
    TilingPtr tiling = tilingMaker->getTiling();
    if (!tiling) return;

    BackgroundImage & bi = tiling->getBackground();
    bi.x     = bkgdLayout.getX();
    bi.y     = bkgdLayout.getY();
    bi.scale = bkgdLayout.getScale();
    bi.rot   = bkgdLayout.getRot();

    if (config->viewerType != VIEW_TILIING_MAKER)
    {
        return;
    }

    bi.bkgdTransformChanged(transformBkgd->isChecked());
}

void page_tiling_maker::slot_adjustBackground()
{
    if (tilingMaker->getMouseMode() != BKGD_SKEW_MODE)
        return;

    EdgePoly & waccum = tilingMaker->getAccumW();
    if (waccum.size() != 4)
        return;

    TilingPtr tiling = tilingMaker->getTiling();
    if (!tiling) return;


    BackgroundImage & bi = tiling->getBackground();
    bi.adjustBackground(
        tilingMaker->worldToScreen(waccum[0]->getV1()->getPosition()),
        tilingMaker->worldToScreen(waccum[1]->getV1()->getPosition()),
        tilingMaker->worldToScreen(waccum[2]->getV1()->getPosition()),
        tilingMaker->worldToScreen(waccum[3]->getV1()->getPosition()));


    tilingMaker->setMouseMode(NO_MOUSE_MODE);

    perspectiveBkgd->setChecked(true);

    slot_bkgdImageChanged();
}

void page_tiling_maker::slot_saveAdjustedBackground()
{
    TilingPtr tiling = tilingMaker->getTiling();
    if (!tiling) return;

    BackgroundImage & bi = tiling->getBackground();

    QString oldname = bi.bkgdName;

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
    bool rv = bi.saveAdjusted(newName);

    QMessageBox box;
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
    int row = tileInfoTable->currentRow();
    if (row < 0)
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Warning);
        box.setText("Please select a row in the table");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    PlacedFeaturePtr pfp = getFeatureRow(row);
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
    QMessageBox box;
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
            QTransform t;
            tilingMaker->addToAllPlacedFeatures(pfp);
        }
    }
    buildMenu();
}

void page_tiling_maker::slot_addGirihShape()
{
    QString name = girihShapes->currentData().toString();
    PlacedFeaturePtr pfp = make_shared<PlacedFeature>();
    bool rv =  pfp->loadFromGirihShape(name);
    if (rv)
    {
        QTransform t;
        tilingMaker->addToAllPlacedFeatures(pfp);
        buildMenu();
    }
}

void page_tiling_maker::slot_moveX(int amount)
{
    qDebug() << "page_tiling_maker::slot_moveX" << amount;
    if (canvas->getKbdMode() == KBD_MODE_BKGD)
    {
        bkgdLayout.bumpX(amount);
    }
    else if (canvas->getKbdMode() == KBD_MODE_DATA)
    {
        tilingMaker->allDeltaX(amount);
    }
}

void page_tiling_maker::slot_moveY(int amount)
{
    qDebug() << "page_tiling_maker::slot_moveY" << amount;
    if (canvas->getKbdMode() == KBD_MODE_BKGD)
    {
        bkgdLayout.bumpY(amount);
    }
    else if (canvas->getKbdMode() == KBD_MODE_DATA)
    {
        tilingMaker->allDeltaY(amount);
    }
}

void page_tiling_maker::slot_rotate(int amount)
{
    qDebug() << "page_tiling_maker::slot_rotate" << amount;
    if (canvas->getKbdMode() == KBD_MODE_BKGD)
    {
        bkgdLayout.bumpRot(amount);
    }
    else if (canvas->getKbdMode() == KBD_MODE_DATA)
    {
        tilingMaker->allDeltaRotate(amount);
    }
}

void page_tiling_maker::slot_scale(int amount)
{
    qDebug() << "page_tiling_maker::slot_scale" << amount;
    if (canvas->getKbdMode() == KBD_MODE_BKGD)
    {
        bkgdLayout.bumpScale(-amount);
    }
    else if (canvas->getKbdMode() == KBD_MODE_DATA)
    {
        tilingMaker->allDeltaScale(amount);
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
    QVector<PlacedFeaturePtr> & pfeatures = (config->showAllFeatures) ? tilingMaker->getAllFeatures() : tilingMaker->getInTiling();
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
