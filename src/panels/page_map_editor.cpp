#include <QGroupBox>
#include <QTextEdit>
#include <QCheckBox>
#include <QToolButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>

#include "makers/crop_maker/crop_maker.h"
#include "motifs/explicit_map_motif.h"
#include "panels/page_map_editor.h"
#include "motifs/irregular_motif.h"
#include "motifs/radial_motif.h"
#include "geometry/crop.h"
#include "geometry/dcel.h"
#include "geometry/fill_region.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "makers/map_editor/map_editor.h"
#include "makers/map_editor/map_editor_db.h"
#include "makers/map_editor/map_editor_map_loader.h"
#include "makers/map_editor/map_editor_map_writer.h"
#include "makers/map_editor/map_selection.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "misc/fileservices.h"
#include "misc/utilities.h"
#include "mosaic/design_element.h"
#include "makers/prototype_maker/prototype.h"
#include "panels/controlpanel.h"
#include "qlabel.h"
#include "settings/configuration.h"
#include "tiledpatternmaker.h"
#include "viewers/crop_view.h"
#include "viewers/map_editor_view.h"
#include "viewers/viewcontrol.h"
#include "widgets/crop_widget.h"
#include "widgets/dlg_listselect.h"
#include "widgets/dlg_push_select.h"
#include "widgets/dlg_name.h"
#include "widgets/layout_sliderset.h"

using std::make_shared;

typedef std::shared_ptr<RadialMotif>    RadialPtr;

page_map_editor:: page_map_editor(ControlPanel *cpanel)  : panel_page(cpanel,"Map Editor")
{
    maped      = MapEditor::getInstance();
    meView     = MapEditorView::getInstance();
    cropViewer = CropViewer::getInstance();
    db         = maped->getDb();

    // create group boxes
    editorStatusBox             = createStatusGroup();
    QGroupBox * selectBox       = createMapSelects();
    QGroupBox * mapBox          = createMapGroup();
    QGroupBox * editBox         = createEditGroup();
    QGroupBox * loadBox         = createLoadGroup();
    QGroupBox * viewBox         = createViewGroup();
    QGroupBox * pushBox         = createPushGroup();
    QGroupBox * configBox       = createSettingsGroup();
    QGroupBox * constructionBox = createConstructionGroup();

    // arranging the boxes
    QVBoxLayout * left  = new QVBoxLayout;
    QVBoxLayout * mid   = new QVBoxLayout;
    QVBoxLayout * right = new QVBoxLayout;

    left->addWidget(editBox);
    left->addWidget(configBox);
    left->addStretch();

    mid->addWidget(selectBox);
    mid->addWidget(mapBox);
    mid->addWidget(constructionBox);
    mid->addWidget(pushBox);
    mid->addStretch();

    right->addWidget(loadBox);
    right->addWidget(viewBox);
    right->addStretch();

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addLayout(left);
    hbox->addLayout(mid);
    hbox->addLayout(right);

    // putting it together
    vbox->addLayout(hbox);
    vbox->addWidget(editorStatusBox);
    vbox->addStretch();

    connect(tilingMaker, &TilingMaker::sig_tilingLoaded,   this,   &page_map_editor::slot_tilingLoaded);
    connect(mosaicMaker, &MosaicMaker::sig_mosaicLoaded,   this,   &page_map_editor::slot_mosaicLoaded);

    slot_debugChk(config->mapedStatusBox);

    slot_editLayer(LAYER_1,true);

    cropDlg = nullptr;
}

QGroupBox * page_map_editor::createMapSelects()
{
    QGridLayout * grid = new QGridLayout;
    grid->setVerticalSpacing(0);
    grid->setColumnMinimumWidth(1,300);

    QLabel * editL    = new QLabel("Edit");
    compositeEChk     = new QCheckBox("  Composite");
    layer1EChk        = new QCheckBox("  Layer 1");
    layer2EChk        = new QCheckBox("  Layer 2");
    layer3EChk        = new QCheckBox("  Layer 3");

    editGroup = new QButtonGroup();
    editGroup->addButton(compositeEChk,COMPOSITE);
    editGroup->addButton(layer1EChk,LAYER_1);
    editGroup->addButton(layer2EChk,LAYER_2);
    editGroup->addButton(layer3EChk,LAYER_3);

    QLabel * viewL    = new QLabel("View");
    compositeVChk     = new QCheckBox;
    layer1VChk        = new QCheckBox;
    layer2VChk        = new QCheckBox;
    layer3VChk        = new QCheckBox;

    viewGroup = new QButtonGroup();
    viewGroup->setExclusive(false);
    viewGroup->addButton(compositeVChk,COMPOSITE);
    viewGroup->addButton(layer1VChk,LAYER_1);
    viewGroup->addButton(layer2VChk,LAYER_2);
    viewGroup->addButton(layer3VChk,LAYER_3);

    int col = 0;
    grid->setColumnStretch(col,0);
    grid->addWidget(editL,0,col);
    grid->addWidget(compositeEChk,1,col);
    grid->addWidget(layer1EChk,2,col);
    grid->addWidget(layer2EChk,3,col);
    grid->addWidget(layer3EChk,4,col);

    col = 1;
    grid->setColumnStretch(col,0);
    grid->addWidget(viewL,0,col);
    grid->addWidget(compositeVChk,1,col);
    grid->addWidget(layer1VChk,2,col);
    grid->addWidget(layer2VChk,3,col);
    grid->addWidget(layer3VChk,4,col);

    connect(editGroup,     &QButtonGroup::idToggled, this, &page_map_editor::slot_editLayer);
    connect(viewGroup,     &QButtonGroup::idToggled, this, &page_map_editor::slot_viewLayer);

    QGroupBox * gb = new QGroupBox("Map Selects");
    gb->setLayout(grid);
    return gb;
}

QGroupBox   * page_map_editor::createStatusGroup()
{
    statusBox = new QTextEdit();

    QVBoxLayout * qvbox = new QVBoxLayout();
    qvbox->addWidget(statusBox);

    QGroupBox * gBox = new QGroupBox();
    //gBox->setMinimumWidth(531);
    gBox->setMaximumHeight(181);
    gBox->setLayout(qvbox);

    return gBox;
}

QGroupBox   * page_map_editor::createSettingsGroup()
{
    DoubleSpinSet * sensitivity = new DoubleSpinSet("Snap-to", config->mapedMergeSensitivity,1e-8, 100.0);
    sensitivity->setSingleStep(1e-4);
    sensitivity->setValue(config->mapedMergeSensitivity); //dunno why needs to be done again

    connect(sensitivity, &DoubleSpinSet::valueChanged, this, &page_map_editor::slot_mergeSensitivityA);

    DoubleSpinSet * lineWidthSpin = new DoubleSpinSet("Line Width",3.0,1.0,10.0);
    DoubleSpinSet * consWidthSpin = new DoubleSpinSet("Cons Width",1.0,1.0,10.0);

    QVBoxLayout * qvbox = new QVBoxLayout();
    qvbox->addLayout(sensitivity);
    qvbox->addLayout(lineWidthSpin);
    qvbox->addLayout(consWidthSpin);

    connect(lineWidthSpin,  &DoubleSpinSet::valueChanged, this, &page_map_editor::slot_lineWidthChanged);
    connect(consWidthSpin,  &DoubleSpinSet::valueChanged, this, &page_map_editor::slot_consWidthChanged);

    QGroupBox * gBox = new QGroupBox("Settings");
    gBox->setLayout(qvbox);

    return gBox;
}

QGroupBox * page_map_editor::createMapGroup()
{
    QToolButton * pbVerifyMap = new QToolButton();
    QToolButton * pbMakeExplicit = new QToolButton();
    QToolButton * pbDivideEdges = new QToolButton();
    QToolButton * pbJoinEdges = new QToolButton();
    QToolButton * pbCleanNeighbours = new QToolButton();
    QToolButton * pbRemoveFloatingVerts = new QToolButton();
    QToolButton * pbRemoveSingleVerts = new QToolButton();
    QToolButton * pbRemoveZombieEdges = new QToolButton();
                  pbCleanseVertices = new QToolButton();
    QToolButton * pbRebuildNeigbours = new QToolButton();
    QToolButton * pbCleanseMap = new QToolButton();
    QToolButton * pbDumpMap = new QToolButton();
                  chkEditCrop = new QCheckBox();
                  pbEmbedCrop = new QToolButton();
                  pbApplyCrop = new QToolButton();

    chkEditCrop->setStyleSheet("padding-left: 17px;");

    defaultStyle = pbVerifyMap->styleSheet();

    pbVerifyMap->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    pbMakeExplicit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    pbDumpMap->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    pbDivideEdges->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    pbJoinEdges->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    pbCleanNeighbours->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    pbCleanseVertices->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    pbRebuildNeigbours->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    pbRemoveFloatingVerts->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    pbRemoveSingleVerts->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    pbRemoveZombieEdges->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    pbCleanseMap->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    chkEditCrop->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    pbEmbedCrop->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    pbApplyCrop->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    pbMakeExplicit->setText("Make Explicit");
    pbVerifyMap->setText("Verify Map");
    pbDivideEdges->setText("Divide\nIntersecting Edges");
    pbJoinEdges->setText("Join\nColinear Edges");
    pbCleanNeighbours->setText("Clean\nNeighbours");
    pbRebuildNeigbours->setText("Rebuild Neighbours");
    pbCleanseVertices->setText("Cleanse Vertices");
    pbRemoveFloatingVerts->setText("Remove\nFloating Vertices");
    pbRemoveSingleVerts->setText("Remove\nSingle Vertices");
    pbRemoveZombieEdges->setText("Remove\nZombie Edges");

    pbCleanseMap->setText("Cleanse Map");
    pbDumpMap->setText("Dump Map");
    chkEditCrop->setText("Edit Crop");
    pbEmbedCrop->setText("Embed Crop");
    pbApplyCrop->setText("Crop Outside");

    // Map Box
    QGridLayout * mapGrid = new QGridLayout;
    int row = 0;
    mapGrid->addWidget(pbVerifyMap,row,0);
    mapGrid->addWidget(pbMakeExplicit,row,1);
    mapGrid->addWidget(pbDumpMap,row,2);

    row++;
    mapGrid->addWidget(pbRemoveZombieEdges,row,0);
    mapGrid->addWidget(pbRemoveFloatingVerts,row,1);
    mapGrid->addWidget(pbRemoveSingleVerts,row,2);

    row++;
    mapGrid->addWidget(pbDivideEdges,row,0);
    mapGrid->addWidget(pbJoinEdges,row,1);
    mapGrid->addWidget(pbCleanNeighbours,row,2);

    row++;
    mapGrid->addWidget(pbRebuildNeigbours,row,2);

    row++;
    mapGrid->addWidget(pbCleanseVertices,row,0,1,2);
    mapGrid->addWidget(pbCleanseMap,row,2);

    row++;
    mapGrid->addWidget(chkEditCrop,row,0);
    mapGrid->addWidget(pbEmbedCrop,row,1);
    mapGrid->addWidget(pbApplyCrop,row,2);

    QGroupBox * mapBox  = new QGroupBox("Map");
    mapBox->setLayout(mapGrid);

    connect(pbVerifyMap,            &QToolButton::clicked,  this,   &page_map_editor::slot_verify);
    connect(pbMakeExplicit,         &QToolButton::clicked,  this,   &page_map_editor::slot_convertToExplicit);
    connect(pbDivideEdges,          &QToolButton::clicked,  this,   &page_map_editor::slot_divideIntersectingEdges);
    connect(pbJoinEdges,            &QToolButton::clicked,  this,   &page_map_editor::slot_joinColinearEdges);
    connect(pbCleanNeighbours,      &QToolButton::clicked,  this,   &page_map_editor::slot_cleanNeighbours);
    connect(pbCleanseVertices,      &QToolButton::clicked,  this,   &page_map_editor::slot_cleanVertices);
    connect(pbRebuildNeigbours,     &QToolButton::clicked,  this,   &page_map_editor::slot_rebuildNeighbours);
    connect(pbCleanseMap,           &QToolButton::clicked,  this,   &page_map_editor::slot_cleanseMap);
    connect(pbRemoveFloatingVerts,  &QToolButton::clicked,  this,   &page_map_editor::slot_removeUnconnectedVertices);
    connect(pbRemoveSingleVerts,    &QToolButton::clicked,  this,   &page_map_editor::slot_removeSingleConnectVertices);
    connect(pbRemoveZombieEdges,    &QToolButton::clicked,  this,   &page_map_editor::slot_removeZombieEdges);
    connect(pbDumpMap,              &QToolButton::clicked,  this,   &page_map_editor::slot_dumpMap);
    connect(chkEditCrop,            &QCheckBox::clicked,    this,   &page_map_editor::slot_editCrop);
    connect(pbEmbedCrop,            &QToolButton::clicked,  this,   &page_map_editor::slot_embedCrop);
    connect(pbApplyCrop,            &QToolButton::clicked,  this,   &page_map_editor::slot_applyCrop);

    return mapBox;
}

QGroupBox * page_map_editor::createEditGroup()
{
    DoubleSpinSet * radiusSpin    = new DoubleSpinSet("Radius",0.25,0.0,2.0);

    DoubleSpinSet * angleSpin    = new DoubleSpinSet("Angle",30,-360.0,360.0);
    angleSpin->setSingleStep(0.5);

    DoubleSpinSet * lenSpin    = new DoubleSpinSet("lLngth",1.0,0.01,9.0);
    angleSpin->setSingleStep(0.1);

    QRadioButton * modeNone         = new QRadioButton("No Mode (ESC)");
    QRadioButton * modeDrawLine     = new QRadioButton("Draw Lines (F3)");
    QRadioButton * modeConstruction = new QRadioButton("Construct Lines (F4)");
    QRadioButton * modeDelLine      = new QRadioButton("Delete Selected (F5)");
    QRadioButton * modeSplitLine    = new QRadioButton("Split Edges (F6)");
    QRadioButton * modeExtendLineP1 = new QRadioButton("Extend Line P1 (F7)");
    QRadioButton * modeExtendLineP2 = new QRadioButton("Extend Line P2");
    QRadioButton * modeConstrCicle  = new QRadioButton("Construct Circles (F9)");
    QRadioButton * modeCreateLine   = new QRadioButton("CreateLine");

    modeGroup = new QButtonGroup();
    modeGroup->addButton(modeNone,           MAPED_MOUSE_NONE);
    modeGroup->addButton(modeDrawLine,       MAPED_MOUSE_DRAW_LINE);
    modeGroup->addButton(modeConstrCicle,    MAPED_MOUSE_CONSTRUCTION_CIRCLES);
    modeGroup->addButton(modeDelLine,        MAPED_MOUSE_DELETE);
    modeGroup->addButton(modeSplitLine,      MAPED_MOUSE_SPLIT_LINE);
    modeGroup->addButton(modeConstruction,   MAPED_MOUSE_CONSTRUCTION_LINES);
    modeGroup->addButton(modeExtendLineP1,   MAPED_MOUSE_EXTEND_LINE_P1);
    modeGroup->addButton(modeExtendLineP2,   MAPED_MOUSE_EXTEND_LINE_P2);
    modeGroup->addButton(modeCreateLine,     MAPED_MOUSE_CREATE_LINE);

    modeGroup->button(maped->getMouseMode())->setChecked(true);

    // Edit Box
    QVBoxLayout * editBox = new QVBoxLayout;

    editBox->addWidget(modeNone);
    editBox->addWidget(modeDrawLine);
    editBox->addWidget(modeConstrCicle);
    editBox->addLayout(radiusSpin);
    editBox->addSpacing(3);
    editBox->addWidget(modeConstruction);
    editBox->addWidget(modeDelLine);

    editBox->addWidget(modeSplitLine);
    editBox->addWidget(modeExtendLineP1);
    editBox->addWidget(modeExtendLineP2);

    editBox->addWidget(modeCreateLine);
    editBox->addLayout(angleSpin);
    editBox->addLayout(lenSpin);

    editBox->addStretch();

    QGroupBox * editGBox = new QGroupBox("Edit");
    editGBox->setLayout(editBox);

    radiusSpin->setValue(config->mapedRadius);
    angleSpin->setValue(config->mapedAngle);
    lenSpin->setValue(config->mapedLen);

    connect(radiusSpin,     &DoubleSpinSet::valueChanged, this, &page_map_editor::slot_radiusChanged);
    connect(angleSpin,      &DoubleSpinSet::valueChanged, this, &page_map_editor::slot_createAngleChanged);
    connect(lenSpin,        &DoubleSpinSet::valueChanged, this, &page_map_editor::slot_createLenChanged);
    connect(modeGroup,      &QButtonGroup::idToggled,     this, &page_map_editor::slot_setMouseMode);

    return editGBox;
}

QGroupBox * page_map_editor::createViewGroup()
{
    QCheckBox * showBoundsChk   = new QCheckBox("Show boundaries");
                showConsChk     = new QCheckBox("Show construct lines");
    QCheckBox * showMapChk      = new QCheckBox("Show map");
    QCheckBox * showPtsChk      = new QCheckBox("Show points");
    QCheckBox * showMidPtsChk   = new QCheckBox("Show mid-points");
    QCheckBox * showDirPtsChk   = new QCheckBox("Show directions");
    QCheckBox * showArcCtrChk   = new QCheckBox("Show arc centres");
    animateChk                  = new QCheckBox("Animate Load");
    QCheckBox * showStatusChk   = new QCheckBox("Show Status");


    QRadioButton * viewMap      = new QRadioButton("Map");
                   viewDCEL     = new QRadioButton("DCEL");

    showDirPtsChk->setStyleSheet("padding-left:15px");
    showArcCtrChk->setStyleSheet("padding-left:15px");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(viewMap);
    hbox->addWidget(viewDCEL);

    QFrame    * line1 = new QFrame();
    line1->setFrameShape(QFrame::HLine);
    line1->setFrameShadow(QFrame::Sunken);
    line1->setLineWidth(1);

    QFrame    * line2 = new QFrame();
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Sunken);
    line2->setLineWidth(1);

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addWidget(line2);
    vbox->addWidget(showMapChk);
    vbox->addWidget(showDirPtsChk);
    vbox->addWidget(showArcCtrChk);
    vbox->addWidget(showPtsChk);
    vbox->addWidget(showMidPtsChk);
    vbox->addWidget(showBoundsChk);
    vbox->addWidget(showConsChk);
    vbox->addWidget(animateChk);
    vbox->addWidget(showStatusChk);
    vbox->addStretch();

    QGroupBox * gbox = new QGroupBox("View");
    gbox->setLayout(vbox);

    auto db = maped->getDb();

    showMapChk->setChecked(db->showMap);
    showBoundsChk->setChecked(db->showBoundaries);
    showConsChk->setChecked(db->showConstructionLines);
    showPtsChk->setChecked(db->showPoints);
    showMidPtsChk->setChecked(db->showMidPoints);
    showDirPtsChk->setChecked(db->showDirnPoints);
    showArcCtrChk->setChecked(db->showArcCentre);
    animateChk->setChecked(false);
    showStatusChk->setChecked(config->mapedStatusBox);

    if (config->mapEditorMode == MAPED_MODE_MAP)
        viewMap->setChecked(true);
    else
        viewDCEL->setChecked(true);

    connect(showBoundsChk,  &QCheckBox::clicked,    this, &page_map_editor::slot_showBounds);
    connect(showConsChk,    &QCheckBox::clicked,    this, &page_map_editor::slot_showCons);
    connect(showMapChk,     &QCheckBox::clicked,    this, &page_map_editor::slot_showMap);
    connect(showPtsChk,     &QCheckBox::clicked,    this, &page_map_editor::slot_showPoints);
    connect(showMidPtsChk,  &QCheckBox::clicked,    this, &page_map_editor::slot_showMidPoints);
    connect(showDirPtsChk,  &QCheckBox::clicked,    this, &page_map_editor::slot_showDirPoints);
    connect(showArcCtrChk,  &QCheckBox::clicked,    this, &page_map_editor::slot_showArcCentre);
    connect(showStatusChk,  &QCheckBox::clicked,    this, &page_map_editor::slot_debugChk);
    connect(viewMap,        &QRadioButton::clicked, this, &page_map_editor::slot_chkViewMap);
    connect(viewDCEL,       &QRadioButton::clicked, this, &page_map_editor::slot_chkViewDCEL);

    return gbox;
}

QGroupBox * page_map_editor::createLoadGroup()
{
    QPushButton * unload       = new QPushButton("Unload");
    QPushButton * loadMosaic   = new QPushButton("Load Mosaic Prototype");
    QPushButton * loadProto    = new QPushButton("Load Motif Prototype");
    QPushButton * loadMotif    = new QPushButton("Load Selected Motifs");
    QPushButton * loadTileUnit = new QPushButton("Load Unit Tiling");
    QPushButton * loadTileRep  = new QPushButton("Load Repeated Tiling");
    QPushButton * loadFile     = new QPushButton("Load Map File");
    QPushButton * createMap    = new QPushButton("Create New Map");

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addWidget(unload);
    vbox->addWidget(loadMosaic);
    vbox->addWidget(loadProto);
    vbox->addWidget(loadMotif);
    vbox->addWidget(loadTileUnit);
    vbox->addWidget(loadTileRep);
    vbox->addWidget(loadFile);
    vbox->addWidget(createMap);

    QGroupBox * gbox = new QGroupBox("Load");
    gbox->setLayout(vbox);

    connect(loadMosaic, &QPushButton::clicked, this, &page_map_editor::slot_loadMosaicPrototype);
    connect(loadProto,  &QPushButton::clicked, this, &page_map_editor::slot_loadMotifPrototype);
    connect(loadMotif,  &QPushButton::clicked, this, &page_map_editor::slot_loadMotif);
    connect(loadTileUnit,&QPushButton::clicked,this, &page_map_editor::slot_loadTileUnit);
    connect(loadTileRep,&QPushButton::clicked, this, &page_map_editor::slot_loadTileRep);
    connect(loadFile,   &QPushButton::clicked, this, &page_map_editor::slot_loadMapFile);
    connect(createMap,  &QPushButton::clicked, this, &page_map_editor::slot_createMap);
    connect(unload,     &QPushButton::clicked, this, &page_map_editor::slot_unloadMaps);

    return gbox;
}

QGroupBox * page_map_editor::createPushGroup()
{
    QPushButton * pbPushMap  = new QPushButton("Push Map");
    QPushButton * pbSaveMap  = new QPushButton("Save Edit Map");
    QLabel      * dummy1     = new QLabel;

    QGridLayout * pushLayout = new QGridLayout;
    pushLayout->addWidget(pbPushMap,0,0);
    pushLayout->addWidget(dummy1,0,1);
    pushLayout->addWidget(pbSaveMap,0,2);

    QGroupBox * pushBox = new QGroupBox("Push");
    pushBox->setLayout(pushLayout);

    connect(pbPushMap, &QPushButton::clicked,  this,   &page_map_editor::slot_pushMap);
    connect(pbSaveMap, &QPushButton::clicked,  this,   &page_map_editor::slot_saveMapToFile);

    return pushBox;
}

QGroupBox * page_map_editor::createConstructionGroup()
{
    QPushButton * pbRestoreConstructs   = new QPushButton();
    QPushButton * pbUndoConstructs      = new QPushButton();
    QPushButton * pbRedoConstructs      = new QPushButton();
    QPushButton * pbClearConstructs     = new QPushButton();
    QPushButton * pbSaveTemplate        = new QPushButton();
    QPushButton * pbLoadTemplate        = new QPushButton();

    pbRestoreConstructs->setText("Restore C-Lines");
    pbUndoConstructs->setText("  Undo C-Lines");
    pbRedoConstructs->setText("  Redo C-Lines");
    pbClearConstructs->setText("Clear C-Lines");
    pbSaveTemplate->setText("Save C-Line Template");
    pbLoadTemplate->setText("Load C-Line Template");

    pbUndoConstructs->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowLeft));
    pbRedoConstructs->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowRight));

    QGridLayout * templateGrid = new QGridLayout;
    int row = 0;
    templateGrid->addWidget(pbRestoreConstructs,row,0);
    templateGrid->addWidget(pbUndoConstructs,row,1);
    templateGrid->addWidget(pbRedoConstructs,row,2);

    row++;
    templateGrid->addWidget(pbSaveTemplate,row,0);
    templateGrid->addWidget(pbLoadTemplate,row,1);
    templateGrid->addWidget(pbClearConstructs,row,2);

    QGroupBox * templateBox = new QGroupBox("Construction Lines");
    templateBox->setLayout(templateGrid);

    connect(pbRestoreConstructs,    &QPushButton::clicked,  this,   &page_map_editor::slot_popstash);
    connect(pbUndoConstructs,       &QPushButton::clicked,  this,   &page_map_editor::slot_undoConstructionLines);
    connect(pbRedoConstructs,       &QPushButton::clicked,  this,   &page_map_editor::slot_redoConstructionLines);
    connect(pbClearConstructs,      &QPushButton::clicked,  this,   &page_map_editor::slot_clearConstructionLines);
    connect(pbSaveTemplate,         &QPushButton::clicked,  this,   &page_map_editor::slot_saveTemplate);
    connect(pbLoadTemplate,         &QPushButton::clicked,  this,   &page_map_editor::slot_loadTemplate);

    return templateBox;
}

void  page_map_editor::onEnter()
{
static QString msg("<body>"
                   "<font color=magenta>tile</font>  |  "
                   "<span style=\"color:rgb(255,127,0)\">motif boundary</span>  |  "
                   "<font color=yellow>ext boundary</font>  |  "
                   "<font color=blue>vertex</font>  |  "
                   "<font color=yellow>pt</font>  |  "
                   "<font color=#800000>mid-edge</font>  |  "  // dark red
                   "<font color=#808000>mid-line</font>  |  "  // dark yellow
                    "</body>");

    panel->pushPanelStatus(msg);

    modeGroup->blockSignals(true);
    modeGroup->button(maped->getMouseMode())->setChecked(true);
    modeGroup->blockSignals(false);

    cropViewer->init(db);
}

void  page_map_editor::onExit()
{
    panel->popPanelStatus();

    db->resetMouseInteraction();

    cropViewer->setShowCrop(false);
    if (cropDlg)
    {
        cropDlg->close();
        cropDlg = nullptr;
    }
}

void page_map_editor::onRefresh()
{
    static bool oldCropEditState = false;

    bool currentCropEditState = cropViewer->getShowCrop();
    if (currentCropEditState != oldCropEditState)
    {
        if (currentCropEditState)
        {
            panel->pushPanelStatus("Left Click on corner to resize crop, or drag to move");
        }
        else
        {
            panel->popPanelStatus();
        }
        oldCropEditState = currentCropEditState;
    }

    tallySelects();

    tallyCropButtons();

    if (config->mapedStatusBox)
    {
        refreshStatusBox();
    }

    MapPtr map = db->getMap(COMPOSITE);
    if (map)
        compositeVChk->setText(map->namedSummary());
    else
        compositeVChk->setText("No Map");

    map = db->getMap(LAYER_1);
    if (map)
        layer1VChk->setText(QString("%1 %2").arg(sMapEditorMapType[db->getMapType(map)]).arg(map->summary()));
    else
        layer1VChk->setText("No map");

    map = db->getMap(LAYER_2);
    if (map)
        layer2VChk->setText(QString("%1 %2").arg(sMapEditorMapType[db->getMapType(map)]).arg(map->summary()));
    else
        layer2VChk->setText("No map");

    map = db->getMap(LAYER_3);
    if (map)
        layer3VChk->setText(QString("%1 %2").arg(sMapEditorMapType[db->getMapType(map)]).arg(map->summary()));
    else
        layer3VChk->setText("No map");

    modeGroup->button(maped->getMouseMode())->setChecked(true);

    pbCleanseVertices->setText(QString("Cleanse Vertices Sensitivity : %1").arg(config->mapedMergeSensitivity));

    if (viewDCEL->isChecked())
        viewDCEL->setStyleSheet("background-color:yellow; color:red;");
    else
        viewDCEL->setStyleSheet("");

    showConsChk->setChecked(db->showConstructionLines);
}

void page_map_editor::refreshStatusBox()
{
    QStringList txt;
    QString     str;

    eMapEditorMode mode = config->mapEditorMode;

    CropPtr crop = db->getCrop();
    if (crop)
    {
        QString emb = (crop->getEmbed()) ? "Embed" : QString();
        QString app = (crop->getApply()) ? "ApplY" : QString();
        txt << QString("Editor mode: %1  Crop Mode: %2 %3 %4").arg(sMapEditorMode[mode]).arg(crop->getCropString()).arg(emb).arg(app);
    }
    else
    {
        txt << QString("Editor mode: %1  Crop Mode: NONE").arg(sMapEditorMode[mode]);
    }

    MapPtr map = db->getEditMap();
    eMapEditorMapType mapType = db->getMapType(map);
    txt << QString("Map status: %1").arg(sMapEditorMapType[mapType]);

    if (map)
        txt << QString("Map: %1  %2 ").arg(Utils::addr(map.get())).arg(map->namedSummary());
    else
        txt <<  "No edit map";

    if (mode == MAPED_MODE_DCEL)
    {
        DCELPtr dp = db->getActiveDCEL();
        if (dp)
            txt << QString("DCEL: vertices = %1 half-edges = %2 faces = %3").arg(dp->getVertices().size()).arg(dp->getEdges().size()).arg(dp->getFaceSet().size());
        else
            txt <<  "DCEL: NONE";
    }

    switch (mapType)
    {
    case MAPED_TYPE_COMPOSITE:
    case MAPED_TYPE_COMPOSITE_MOTIF:
        break;

    case MAPED_LOADED_FROM_MOTIF:
    {
        for (auto & wdelp : db->getDesignElements())
        {
            DesignElementPtr delp = wdelp.lock();
            if (delp)
            {
                MotifPtr motif = delp->getMotif();
                TilePtr  tile  = delp->getTile();

                txt << QString("DesignElement = %1").arg(Utils::addr(delp.get()));
                txt << QString("Tile = %1").arg(Utils::addr(tile.get()));
                txt << QString("Motif = %1 %2").arg(Utils::addr(motif.get())).arg(motif->getMotifTypeString());
            }
            else
            {
                txt << "No DesignElement from WS";
            }
        }
    }
        break;

    case MAPED_LOADED_FROM_MOTIF_PROTOTYPE:
    {
        ProtoPtr pp = db->getMotifPrototype();
        if (pp)
        {
            CropPtr cr;
            if (pp)
            {
                cr = pp->getCrop();
            }
            txt << QString("Protoype = %1 Crop = %2").arg(Utils::addr(pp.get())).arg(Utils::addr(cr.get()));
        }
    }
        break;

    case MAPED_LOADED_FROM_MOSAIC:
        txt << "Mosaic";
        break;

    case MAPED_LOADED_FROM_TILING_UNIT:
    case MAPED_LOADED_FROM_TILING_REPEATED:
    {
        TilingPtr tp =  db->getTiling();
        if (tp)
        {
            txt << tp->dump();
        }
    }
        break;

    case MAPED_LOADED_FROM_FILE:
    case MAPED_LOADED_FROM_FILE_MOTIF:
    case MAPED_TYPE_CREATED:
    case MAPED_TYPE_CROP:
    case MAPED_TYPE_UNKNOWN:
        break;
    }

    QString mi;
    MapMouseActionPtr mma = db->getMouseInteraction();
    if (mma)
        mi = mma->desc;
    else
        mi = "no interaction";

    MapEditorView * view = MapEditorView::getInstance();
    QPointF a = view->getMousePos();
    QPointF b = view->screenToWorld(a);
    QString astring;
    QTextStream ts(&astring);
    ts << "   pos: (" << a.x() << ", " << a.y() << ") ("
                      << b.x() << ", " << b.y() << ")";
    mi += astring;

    txt << QString("%1 : %2").arg(sMapEditorMouseMode[maped->getMouseMode()]).arg(mi);

    if (crop)
    {
        if (crop->getCropType() == CROP_RECTANGLE)
        {
            QRectF rect = crop->getRect();
            txt <<  QString("Crop  Rect: %1 %2 %3 %4 vert=%5 aspect=%6")
                .arg(rect.x()).arg(rect.y()).arg(rect.width()).arg(rect.height())
                .arg(crop->getAspectVertical()).arg(crop->getAspect());
        }
        else
        {
            txt << crop->getCropString();
        }
    }

    MapEditorSelection * selector = maped->getSelector();
    SelectionSet set = selector->getCurrentSelections();
    if (set.size())
    {
        str.clear();
        for (auto it=set.begin(); it != set.end(); it++)
        {
            auto sel = *it;
            eMapSelection type  = sel->getType();
            str += QString("Selection=%1").arg(sMapSelection[type]);
            switch(type)
            {
            case MAP_VERTEX:
            case MAP_POINT:
            {
                QPointF pt = sel->getPoint();
                str += QString(" (%1,%2)\n").arg(pt.x()).arg(pt.y());
                break;
            }
            case MAP_CIRCLE:
            {
                QPointF pt  = sel->getPoint();
                auto c      = sel->getCircle();
                str += QString(" (%1,%2) center=(%4,%5) radius=%3\n").arg(pt.x()).arg(pt.y()).arg(c.radius).arg(c.centre.x()).arg(c.centre.y());
                break;
            }
            case MAP_EDGE:
            case MAP_LINE:
            {
                QLineF line = sel->getLine();
                str += QString(" [(%1,%2)(%3,%4)\n").arg(line.p1().x()).arg(line.p1().y()).arg(line.p2().x()).arg(line.p2().y());
                break;
            }
            }
        }
    }
    else
    {
        str = "No selection";
    }
    txt << str;

    statusBox->setText(txt.join("\n"));
}

void page_map_editor::slot_chkViewMap(bool checked)
{
    config->mapEditorMode = (checked) ? MAPED_MODE_MAP : MAPED_MODE_DCEL;
    if (config->mapEditorMode == MAPED_MODE_DCEL)
    {
        auto map = db->getEditMap();
        bool rv = maped->useExistingDCEL(map);
        if (!rv)
        {
            rv = maped->createLocalDCEL(map);
            if (!rv)
            {
                QMessageBox box(this);
                box.setIcon(QMessageBox::Warning);
                box.setText("There is no map = cannot view DCEL");
                box.exec();
            }
        }
    }
    updateView();
}

void page_map_editor::slot_chkViewDCEL(bool checked)
{
    config->mapEditorMode = (checked) ? MAPED_MODE_DCEL : MAPED_MODE_MAP;
    if (config->mapEditorMode == MAPED_MODE_DCEL)
    {
        auto map = db->getEditMap();
        bool rv  = maped->useExistingDCEL(map);
        if (!rv)
        {
            rv = maped->createLocalDCEL(map);
            if (!rv)
            {
                QMessageBox box(this);
                box.setIcon(QMessageBox::Warning);
                box.setText("There is no map = cannot view DCEL");
                box.exec();
            }
        }
    }
    updateView();
}

void page_map_editor::slot_mosaicLoaded(QString name)
{
    qDebug() << "page_map_editor: loaded -" << name;
    maped->initStashFrom(name);
    slot_mosaicChanged();
}

void page_map_editor::slot_mosaicChanged()
{
    if (view->isEnabled(VIEW_MAP_EDITOR))
    {
        eMapEditorMapType mtype = db->getMapType(db->getEditMap());
        switch (mtype)
        {
        case MAPED_LOADED_FROM_MOSAIC:
            maped->loadMosaicPrototype();
            break;

        case MAPED_LOADED_FROM_MOTIF_PROTOTYPE:
            maped->loadMotifPrototype();
            break;

        case MAPED_LOADED_FROM_TILING_UNIT:
            maped->loadTilingUnit();
            break;

        case MAPED_LOADED_FROM_TILING_REPEATED:
            maped->loadTilingRepeated();
            break;

        case MAPED_LOADED_FROM_MOTIF:
            maped->loadSelectedMotifs();
            break;

        case MAPED_LOADED_FROM_FILE:
        case MAPED_LOADED_FROM_FILE_MOTIF:
        case MAPED_TYPE_UNKNOWN:
        case MAPED_TYPE_COMPOSITE:
        case MAPED_TYPE_COMPOSITE_MOTIF:
        case MAPED_TYPE_CREATED:
        case MAPED_TYPE_CROP:
            break;
        }
    }
}

void page_map_editor::slot_tilingLoaded (QString name)
{
    Q_UNUSED(name);

    if (db->getMapType(db->getEditMap()) == MAPED_LOADED_FROM_TILING_UNIT)
    {
        maped->loadTilingUnit();
    }
    else if (db->getMapType(db->getEditMap()) == MAPED_LOADED_FROM_TILING_REPEATED)
    {
        maped->loadTilingRepeated();
    }
}

void page_map_editor::slot_convertToExplicit()
{
    MapEditorLayer & layer = db->getEditLayer();
    DesignElementPtr delp  = layer.getDel();
    if (delp)
    {
        Q_ASSERT(db->isMotif(layer.getLayerMapType()));
        MotifPtr motif = delp->getMotif();
        if (motif)
        {
            auto ep = std::dynamic_pointer_cast<IrregularMotif>(motif);
            if (ep)
            {
                QMessageBox box(this);
                box.setIcon(QMessageBox::Warning);
                box.setText("Ignoring - Motif is already explicit");
                box.exec();
                return;
            }

            int sides = 10;
            RadialPtr rp = std::dynamic_pointer_cast<RadialMotif>(motif);
            if (rp)
            {
                sides = rp->getN();
            }
            MapPtr map = layer.getMapedLayerMap();
            if (map)
            {
                ep = make_shared<ExplicitMapMotif>(map);
                ep->setN(sides);
                delp->setMotif(ep);
            }
        }
    }
    else
    {
        // TODO - this was not a design element - so lets make one from scratch and push to to the
        // motif maker
        qWarning("Not yet implemented");
    }
}

void page_map_editor::slot_verify()
{
    MapPtr map = db->getEditMap();
    if (!map) return;

    bool oldConf = config->verifyMaps;
    config->verifyMaps = true;

    if (map->verify(true))
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Information);
        box.setText("Verified OK");
        box.exec();
    }
    else
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Verification FAILED");
        box.exec();
    }

    config->verifyMaps = oldConf;
}

void page_map_editor::slot_divideIntersectingEdges()
{
    MapPtr map = db->getEditMap();
    if (!map) return;
    map->cleanse(divideupIntersectingEdges);
    updateView();
}

void page_map_editor::slot_joinColinearEdges()
{
    MapPtr map = db->getEditMap();
    if (!map) return;
    map->cleanse(joinupColinearEdges);
    updateView();
}

void page_map_editor::slot_cleanNeighbours()
{
    MapPtr map = db->getEditMap();
    if (!map) return;
    map->cleanse(cleanupNeighbours);
    updateView();
}

void page_map_editor::slot_cleanVertices()
{
    // this is not a general cleanse operation since it uses the tolernace defined here
    maped->cleanupMapPoints();
    updateView();
}

void page_map_editor::slot_removeUnconnectedVertices()
{
    MapPtr map = db->getEditMap();
    if (!map) return;
    map->cleanse(badVertices_0);
    updateView();
}

void page_map_editor::slot_removeSingleConnectVertices()
{
    MapPtr map = db->getEditMap();
    if (!map) return;
    map->cleanse(badVertices_1);
    updateView();
}

void page_map_editor::slot_removeZombieEdges()
{
    MapPtr map = db->getEditMap();
    if (!map) return;
    map->cleanse(badEdges);
    updateView();
}

void page_map_editor::slot_rebuildNeighbours()
{
    MapPtr map = db->getEditMap();
    if (!map) return;
    map->resetNeighbourMap();
    map->getNeighbourMap(); // rebuilds
    updateView();
}

void page_map_editor::slot_setMouseMode(int mode, bool checked)
{
    if (checked)
    {
        eMapEditorMouseMode mm = static_cast<eMapEditorMouseMode>(mode);
        maped->setMapedMouseMode(mm);
    }
}

void page_map_editor::slot_showBounds(bool show)
{
    db->showBoundaries = show;
    updateView();
}

void page_map_editor::slot_showCons(bool show)
{
    db->showConstructionLines = show;
    updateView();
}

void page_map_editor::slot_showMap(bool show)
{
    db->showMap = show;
    updateView();
}

void page_map_editor::slot_showPoints(bool show)
{
    db->showPoints = show;
    updateView();
}

void page_map_editor::slot_showMidPoints(bool show)
{
    db->showMidPoints = show;
    updateView();
}

void page_map_editor::slot_showDirPoints(bool show)
{
    db->showDirnPoints = show;
    updateView();
}

void page_map_editor::slot_showArcCentre(bool show)
{
    db->showArcCentre = show;
    updateView();
}

void page_map_editor::slot_debugChk(bool on)
{
    config->mapedStatusBox = on;
    if (!on)
    {
        statusBox->hide();
    }
    else
    {
        statusBox->show();
    }
}

void page_map_editor::slot_popstash()
{
    maped->loadCurrentStash();
    updateView();
}

void page_map_editor::slot_undoConstructionLines()
{
    db->getStash()->getPrev();
    maped->loadCurrentStash();
    updateView();
}

void page_map_editor::slot_redoConstructionLines()
{
    db->getStash()->getNext();
    maped->loadCurrentStash();
    updateView();
}

void page_map_editor::slot_clearConstructionLines()
{
    // this does not affect the stash
    maped->getDb()->constructionLines.clear();
    maped->getDb()->constructionCircles.clear();
    updateView();
}

void page_map_editor::slot_dumpMap()
{
    MapPtr m = db->getEditMap();
    if (m)
    {
        m->dumpMap(true);
    }
    else
    {
        qDebug() << " Map not found!";
    }
}

void page_map_editor::slot_editCrop(bool checked)
{
    if (checked)
    {
        CropPtr crop = db->getCrop();
        if (!crop)
        {
            // try to get one from the mosaic
            crop = db->CropMaker::getCrop();
            if (!crop)
            {
                // creae one
                crop = db->createCrop();
            }
            Q_ASSERT(crop);
            // make a local copy
            db->setCrop(crop);
        }

        cropViewer->setShowCrop(true);

        cropDlg = new CropDlg(db->getCrop());
        cropDlg->show();

        connect(cropDlg->cw, &CropWidget::sig_cropModified, this, [this]() { view->update(); } );
        connect(cropDlg->cw, &CropWidget::sig_cropChanged,  this, [this]() { emit sig_refreshView(); } );
        connect(cropDlg,     &CropDlg::sig_dlg_done,        this,  &page_map_editor::slot_completeCrop);

        emit sig_refreshView();
    }
    else
    {
        cropViewer->setShowCrop(false);
        if (cropDlg)
        {
            cropDlg->close();
            cropDlg = nullptr;
        }
    }
}

void page_map_editor::slot_completeCrop()
{
    cropViewer->setShowCrop(false);
    cropDlg = nullptr;
    chkEditCrop->blockSignals(true);
    chkEditCrop->setChecked(false);
    chkEditCrop->blockSignals(false);
    emit sig_refreshView();
}

void page_map_editor::slot_embedCrop()
{
    if (!cropViewer->getShowCrop())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Embed Crop : FAILED\n\nPress 'Edit Crop' before embedding");
        box.exec();
        return;
    }

    // merges the crop rectangle into the map
    MapEditorLayer & layer = db->getEditLayer();
    MapPtr map = layer.getMapedLayerMap();

    bool rv = true;
    if (!map)
    {
        map = make_shared<Map>("Crop Map");
        auto type = maped->getDb()->insertLayer(map,MAPED_TYPE_CROP);
        if (type == COMPOSITE )
        {
            rv = false;
        }
    }

    if (rv)
    {
        rv = db->embedCrop(map);
    }

    if (!rv)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Embed Crop : FAILED");
        box.exec();
    }

    emit sig_refreshView();
}

void page_map_editor::slot_applyCrop()
{
    if (!cropViewer->getShowCrop())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Apply Crop : FAILED\n\nPress 'Edit Crop' before embedding");
        box.exec();
        return;
    }

    // deletes everything outside of the crop rectangle
    MapPtr map = db->getEditMap();

    bool rv = db->cropMap(map);
    if (!rv)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Crop Map : FAILED");
        box.exec();
    }

    emit sig_refreshView();
}

void page_map_editor::slot_cleanseMap()
{
    MapPtr map = maped->getDb()->getEditMap();
    if (!map) return;
    map->cleanse(default_cleanse);
    updateView();
}

void page_map_editor::slot_radiusChanged(qreal r)
{
    config->mapedRadius = r;
}

void page_map_editor::slot_createAngleChanged(qreal angle)
{
    config->mapedAngle = angle;
}

void page_map_editor::slot_createLenChanged(qreal len)
{
    config->mapedLen = len;
}

void page_map_editor::slot_lineWidthChanged(qreal r)
{
    meView->mapLineWidth = r;
    updateView();
}

void page_map_editor::slot_consWidthChanged(qreal r)
{
    meView->constructionLineWidth = r;
    updateView();
}

void page_map_editor::slot_mergeSensitivityA(qreal r)
{
    config->mapedMergeSensitivity = r;
}

void page_map_editor::slot_mergeSensitivityB(int sens)
{
    static qreal rvals[] = {1e-2,1e-1,1e-2,1e-3,1e-4,1e-5,1e-6,1e-7,1e-8,1e-9,1e-10,1e-11,1e-12,1e-13,1e-14,1e-15,1e-16};

    qreal r = 1e-2;
    if (sens >0 && sens <=16)
    {
        r = rvals[sens];
    }

   config->mapedMergeSensitivity = r;
}

void page_map_editor::slot_saveTemplate()
{
    DlgName dlg;
    dlg.newEdit->setText(lastNamedTemplate);
    int retval = dlg.exec();
    if (retval == QDialog::Rejected)
    {
        qDebug() << "Canceled";
        return;
    }
    Q_ASSERT(retval == QDialog::Accepted);
    QString name = dlg.newEdit->text();
    if (!name.isEmpty())
    {
        lastNamedTemplate = name;
        maped->getDb()->getStash()->saveTemplate(name);
    }
    else
        qInfo() << "No template selected";
}

void page_map_editor::slot_loadTemplate()
{
    QStringList ts = FileServices::getTemplates();

    DlgListSelect dlg(ts,this);
    int retval = dlg.exec();
    if (retval == QDialog::Rejected)
    {
        qDebug() << "Canceled";
        return;
    }
    Q_ASSERT(retval == QDialog::Accepted);
    QString label = dlg.selectedFile;
    if (label.isEmpty())
    {
        qInfo() << "No template selected";
        return;
    }

    QStringList  qsl = label.split('+');
    QString name     = qsl[0];
    name = name.trimmed();
    maped->loadNamedStash(name, animateChk->isChecked());

    QFileInfo fi(name);
    lastNamedTemplate = fi.baseName();
}

void page_map_editor::slot_loadMosaicPrototype()
{
    maped->loadMosaicPrototype();
}

void page_map_editor::slot_loadMotifPrototype()
{
    maped->loadMotifPrototype();
}

void page_map_editor::slot_loadMotif()
{
    maped->loadSelectedMotifs();
}

void page_map_editor::slot_loadTileUnit()
{
    maped->loadTilingUnit();
}

void page_map_editor::slot_loadTileRep()
{
    maped->loadTilingRepeated();
}

void page_map_editor::slot_loadMapFile()
{
    qDebug() << "page_map_editor::slot_loadMap";

    QStringList ts = FileServices::getMaps();

    VersionList vList;
    vList.create(ts);
    QStringList ts2 = vList.recompose();

    DlgListSelect dlg(ts2,this);
    if (!lastNamedMap.isEmpty())
    {
        dlg.list->selectItemByName(lastNamedMap);
    }
    int retval = dlg.exec();
    if (retval == QDialog::Rejected)
    {
        qDebug() << "Canceled";
        return;
    }
    Q_ASSERT(retval == QDialog::Accepted);
    QString filename = dlg.selectedFile;
    if (filename.isEmpty())
    {
        qInfo() << "No map selected";
        return;
    }

    lastNamedMap = filename;

    filename.prepend(config->mapsDir);
    filename.append(".xml");
    MapEditorMapLoader loader;
    MapPtr loadedMap  = loader.loadMosaicMap(filename);
    if (!loadedMap)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Map NOT loaded");
        box.exec();
        return;
    }
    eMapEditorMapType maptype = loader.getType();

    Xform xf = loader.getXform();

    qDebug().noquote() << loadedMap->namedSummary();

    maped->loadFromMap(loadedMap,maptype);
    meView->setCanvasXform(xf);

    tallySelects();

    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    box.setText("Map loaded OK");
    box.exec();
}

void page_map_editor::slot_createMap()
{
    MapPtr amap = make_shared<Map>(QString("New local map"));
    maped->loadFromMap(amap,MAPED_TYPE_CREATED);
}

void page_map_editor::slot_unloadMaps()
{
    maped->unload();

    //maped->setCanvasXform(vcontrol->getCurrentXform());
    //QSize sz = view->frameSettings.getCropSize(VIEW_MAP_EDITOR);
    //maped->getMapedView()->setCenterScreenUnits(QRect(QPoint(0,0),sz).center());

    updateView();
}

void page_map_editor::slot_pushMap()
{
    qDebug() << "page_map_editor::slot_pushMap()";

    bool rv = false;

    MapEditorLayer & layer = db->getEditLayer();
    auto map   = layer.getMapedLayerMap();
    if (!map || map->isEmpty())
    {
        rv = false;
    }
    else
    {
        switch (maped->getDb()->getMapType(map))
        {
        case MAPED_LOADED_FROM_MOSAIC:
            rv= maped->pushToMosaic(layer);
            break;

        case MAPED_LOADED_FROM_MOTIF_PROTOTYPE:
        case MAPED_LOADED_FROM_MOTIF:
        case MAPED_LOADED_FROM_FILE_MOTIF:
        case MAPED_TYPE_COMPOSITE_MOTIF:
            rv =  maped->convertToMotif(map);
            break;

        case MAPED_LOADED_FROM_TILING_UNIT:
        case MAPED_LOADED_FROM_TILING_REPEATED:
            rv = maped->convertToTiling(map,false);
            break;

        case MAPED_TYPE_UNKNOWN:
        case MAPED_TYPE_CROP:
            break;

        case MAPED_LOADED_FROM_FILE:
        case MAPED_TYPE_CREATED:
        case MAPED_TYPE_COMPOSITE:
        {
            DlgPushSelect dlg(this);
            int rv2 = dlg.exec();
            if (rv2 == QDialog::Accepted)
            {
                switch (dlg.retval)
                {
                case 1:
                    rv= maped->pushToMosaic(layer);
                    break;
                case 2:
                    rv =  maped->convertToMotif(map);
                    break;
                case 3:
                    rv = maped->convertToTiling(map,false);
                    break;
                }
            }
            else
            {
                Q_ASSERT(rv2 == QDialog::Rejected);
                rv = false;
            }
        }
            break;
        }
    }

    QMessageBox box(this);
    if (rv)
    {
        box.setIcon(QMessageBox::Information);
        box.setText("Pushed OK");
    }
    else
    {
        box.setIcon(QMessageBox::Warning);
        box.setText("Push FAILED");
    }
    box.exec();
}

void page_map_editor::slot_saveMapToFile()
{
    MapPtr map = db->getEditMap();
    if (!map)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("There is no map. Map NOT saved");
        box.exec();
        return;
    }

    if (map->isEmpty())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Map is empty. Map NOT saved");
        box.exec();
        return;
    }

    QString dir = config->mapsDir;
    QString defaultFile = dir;
    if (!lastNamedMap.isEmpty())
    {
        defaultFile = dir + lastNamedMap + ".xml";
    }
    QString filename = QFileDialog::getSaveFileName(this, "Save Map File",defaultFile,"Map (*.xml)",nullptr,QFileDialog::DontConfirmOverwrite);
    if (filename.isEmpty()) return;

    if (QFile::exists(filename))
    {
        // file already exists
        QString name = filename.remove(dir);
        name = name.remove(".xml");
        lastNamedMap = name;

        QMessageBox msgBox(ControlPanel::getInstance());
        msgBox.setText(QString("The map %1 already exists").arg(name));
        msgBox.setInformativeText("Do you want to bump version (Bump) or overwrite (Save)?");
        QPushButton * bump   = msgBox.addButton("Bump",QMessageBox::ApplyRole);
        QPushButton * save   = msgBox.addButton(QMessageBox::Save);
        QPushButton * cancel = msgBox.addButton(QMessageBox::Cancel);
        msgBox.setDefaultButton(bump);
        msgBox.exec();

        if (msgBox.clickedButton() == cancel)
        {
            return;
        }
        else if (msgBox.clickedButton() == bump)
        {
            name = FileServices::getNextVersion(FILE_MAP,name); // appends a version
        }
        else
        {
            Q_UNUSED(save)
        }

        lastNamedMap = name;
        filename = dir + name + ".xml";
    }

    MapEditorMapWriter writer(meView);
    bool rv = writer.writeXML(filename,map,maped->getDb()->getMapType(maped->getDb()->getEditMap()));
    if (rv)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Information);
        box.setText("Map saved OK");
        box.exec();
    }
    else
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Map NOT saved");
        box.exec();
    }
}


void page_map_editor::slot_viewLayer(int id, bool checked)
{
    eMapedLayer eid = eMapedLayer(id);
    db->setViewSelect(eid,checked);
    if (checked)
        db->setEditSelect(eid);
    updateView();
}

void page_map_editor::slot_editLayer(int id, bool checked)
{
    if (checked)
        db->setEditSelect(eMapedLayer(id));
    else
        db->setEditSelect(NO_MAP);
    updateView();
}

void page_map_editor::tallySelects()
{
    viewGroup->blockSignals(true);

    if (db->isViewSelected(COMPOSITE))
    {
        compositeVChk->setChecked(true);
        layer1VChk->setChecked(false);
        layer2VChk->setChecked(false);
        layer3VChk->setChecked(false);
    }
    else
    {
        compositeVChk->setChecked(false);
        bool chk = db->isViewSelected(LAYER_1);
        layer1VChk->setChecked(chk);
        chk = db->isViewSelected(LAYER_2);
        layer2VChk->setChecked(chk);
        chk = db->isViewSelected(LAYER_3);
        layer3VChk->setChecked(chk);
    }

    viewGroup->blockSignals(false);

    // this is needed
    editGroup->blockSignals(true);

    compositeEChk->setChecked(false);
    layer1EChk->setChecked(false);
    layer2EChk->setChecked(false);
    layer3EChk->setChecked(false);

    switch (db->getEditSelect())
    {
    case NO_MAP:
        break;

    case LAYER_1:
        layer1EChk->setChecked(true);
        break;

    case LAYER_2:
        layer2EChk->setChecked(true);
        break;

    case LAYER_3:
        layer3EChk->setChecked(true);
        break;

    case COMPOSITE:
        compositeEChk->setChecked(true);
        break;
    }

    editGroup->blockSignals(false);
}

void page_map_editor::tallyCropButtons()
{
    if (cropViewer->getShowCrop())
    {
        pbEmbedCrop->setStyleSheet("QToolButton { background-color: yellow; color: red;}");
        pbApplyCrop->setStyleSheet("QToolButton { background-color: yellow; color: red;}");
    }
    else
    {
        pbEmbedCrop->setStyleSheet(defaultStyle);
        pbApplyCrop->setStyleSheet(defaultStyle);
    }
}
