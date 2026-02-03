#include <QGroupBox>
#include <QTextEdit>
#include <QCheckBox>
#include <QToolButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QLabel>

#include "gui/map_editor/map_editor.h"
#include "gui/map_editor/map_editor_db.h"
#include "gui/map_editor/map_editor_map_loader.h"
#include "gui/map_editor/map_editor_map_writer.h"
#include "gui/map_editor/map_selection.h"
#include "gui/panels/page_map_editor.h"
#include "gui/top/controlpanel.h"
#include "gui/top/system_view_controller.h"
#include "gui/viewers/crop_maker_view.h"
#include "gui/viewers/map_editor_view.h"
#include "gui/widgets/crop_widget.h"
#include "gui/widgets/dlg_cleanse.h"
#include "gui/widgets/dlg_listselect.h"
#include "gui/widgets/dlg_name.h"
#include "gui/widgets/dlg_push_select.h"
#include "gui/widgets/layout_sliderset.h"
#include "gui/widgets/smx_widget.h"
#include "model/makers/crop_maker.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/prototype_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/mosaics/mosaic.h"
#include "model/motifs/explicit_map_motif.h"
#include "model/motifs/irregular_motif.h"
#include "model/motifs/radial_motif.h"
#include "model/prototypes/design_element.h"
#include "model/prototypes/prototype.h"
#include "model/settings/configuration.h"
#include "model/tilings/backgroundimage.h"
#include "sys/geometry/crop.h"
#include "sys/geometry/dcel.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/fill_region.h"
#include "sys/geometry/map_cleanser.h"
#include "sys/geometry/map_verifier.h"
#include "sys/qt/utilities.h"
#include "sys/sys/fileservices.h"
#include "sys/tiledpatternmaker.h"

using std::make_shared;

typedef std::shared_ptr<RadialMotif>            RadialPtr;
typedef std::shared_ptr<class BackgroundImage>  BkgdImagePtr;

Q_DECLARE_METATYPE(std::weak_ptr<Style>)

page_map_editor:: page_map_editor(ControlPanel *cpanel)  : panel_page(cpanel,PAGE_MAP_EDITOR,"Map Editor")
{
    maped      = Sys::mapEditor;
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
                constructionBox = createConstructionGroup();

    // arranging the boxes
    QVBoxLayout * left  = new QVBoxLayout;
    QVBoxLayout * mid   = new QVBoxLayout;
    QVBoxLayout * right = new QVBoxLayout;

    left->addWidget(configBox);
    left->addWidget(editBox);
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

    connect(editGroup,  &QButtonGroup::idToggled, this, &page_map_editor::slot_editLayer);
    connect(viewGroup,  &QButtonGroup::idToggled, this, &page_map_editor::slot_viewLayer);

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
    QPushButton * pbCleanse = new QPushButton("Cleanse settings");
    connect(pbCleanse, &QPushButton::clicked, this, &page_map_editor::slot_cleanseSettings);

    SpinSet * lineWidthSpin = new SpinSet("Line Width",3,1,9);
    SpinSet * consWidthSpin = new SpinSet("Cons Width",1,1,9);

    smxWidget = new SMXWidget(Sys::mapEditorView.get(),true,true);

    QCheckBox   * chkLoadCopy = new QCheckBox("Load copies");
    chkLoadCopy->setChecked(config->mapedLoadCopies);

    QCheckBox   * chkLoadOldClines = new QCheckBox("Load old templates");
    chkLoadOldClines->setChecked(config->mapedOldTemplates);

    QVBoxLayout * qvbox = new QVBoxLayout();
    qvbox->addWidget(smxWidget);
    qvbox->addWidget(chkLoadCopy);
    qvbox->addWidget(chkLoadOldClines);
    qvbox->addWidget(pbCleanse);
    qvbox->addLayout(lineWidthSpin);
    qvbox->addLayout(consWidthSpin);

    connect(lineWidthSpin,    &SpinSet::valueChanged, this, &page_map_editor::slot_lineWidthChanged);
    connect(consWidthSpin,    &SpinSet::valueChanged, this, &page_map_editor::slot_consWidthChanged);
    connect(chkLoadCopy,      &QCheckBox::clicked,    this, &page_map_editor::slot_loadCopies);
    connect(chkLoadOldClines, &QCheckBox::clicked,    this, [](bool checked) { Sys::config->mapedOldTemplates = checked; });

    QGroupBox * gBox = new QGroupBox("Settings");
    gBox->setLayout(qvbox);

    return gBox;
}

QGroupBox * page_map_editor::createMapGroup()
{
    QToolButton * pbVerifyMap           = new QToolButton();
    QToolButton * pbMakeExplicit        = new QToolButton();
    QToolButton * pbDivideEdges         = new QToolButton();
    QToolButton * pbJoinEdges           = new QToolButton();
    QToolButton * pbRemoveFloatingVerts = new QToolButton();
    QToolButton * pbRemoveSingleVerts   = new QToolButton();
    QToolButton * pbRemoveZombieEdges   = new QToolButton();
                  pbCleanseVertices     = new QToolButton();
                  pbCleanseMap          = new QToolButton();
    QToolButton * pbDumpMap             = new QToolButton();
                  pbApplyCrop           = new QToolButton();
                  pbEmbedCrop           = new QToolButton();
                  chkEditCrop           = new QCheckBox();

    chkEditCrop->setStyleSheet("padding-left: 17px;");

    defaultStyle = pbVerifyMap->styleSheet();

    pbVerifyMap->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    pbMakeExplicit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    pbDumpMap->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    pbDivideEdges->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    pbJoinEdges->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    pbCleanseVertices->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

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
    pbCleanseVertices->setText("Coalesce Vertices");
    pbRemoveFloatingVerts->setText("Remove\nFloating Vertices");
    pbRemoveSingleVerts->setText("Remove\nSingle Vertices");
    pbRemoveZombieEdges->setText("Coalesce\nEdges");

    pbCleanseMap->setText("Cleanse\nMap");
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
    mapGrid->addWidget(pbRemoveFloatingVerts,row,0);
    mapGrid->addWidget(pbRemoveSingleVerts,row,1);
    mapGrid->addWidget(pbCleanseVertices,row,2);

    row++;
    mapGrid->addWidget(pbDivideEdges,row,0);
    mapGrid->addWidget(pbJoinEdges,row,1);
    mapGrid->addWidget(pbRemoveZombieEdges,row,2);

    row++;
    mapGrid->addWidget(pbCleanseMap,row,0);

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
    connect(pbCleanseVertices,      &QToolButton::clicked,  this,   &page_map_editor::slot_cleanVertices);
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
    DoubleSpinSet * radiusSpin    = new DoubleSpinSet("Radius",0.25,0.0,8.0);

    DoubleSpinSet * angleSpin    = new DoubleSpinSet("Angle",30,-360.0,360.0);
    angleSpin->setSingleStep(0.5);

    DoubleSpinSet * lenSpin    = new DoubleSpinSet("lLngth",1.0,0.01,9.0);
    angleSpin->setSingleStep(0.1);

    QRadioButton * modeNone         = new QRadioButton("No Mode (ESC)");
    QRadioButton * modeDrawLine     = new QRadioButton("Draw Map Lines (F2)");
    QRadioButton * modeCreateLine   = new QRadioButton("Create Map Lines (F3)");
    QRadioButton * modeConstruction = new QRadioButton("Construction Lines (F4)");
    QRadioButton * modeConstrCicle  = new QRadioButton("Construction Circles (F5)");
    QRadioButton * modeSplitLine    = new QRadioButton("Split Edges (F6)");
    QRadioButton * modeExtendLineP1 = new QRadioButton("Extend Line P1 (F7)");
    QRadioButton * modeExtendLineP2 = new QRadioButton("Extend Line P2 (F8)");
    QRadioButton * modeDelLine      = new QRadioButton("Delete Selected (F9)");

    modeGroup = new QButtonGroup();
    modeGroup->addButton(modeNone,           MAPED_MOUSE_NONE);
    modeGroup->addButton(modeDrawLine,       MAPED_MOUSE_DRAW_LINE);
    modeGroup->addButton(modeCreateLine,     MAPED_MOUSE_CREATE_LINE);
    modeGroup->addButton(modeConstrCicle,    MAPED_MOUSE_CONSTRUCTION_CIRCLES);
    modeGroup->addButton(modeDelLine,        MAPED_MOUSE_DELETE);
    modeGroup->addButton(modeSplitLine,      MAPED_MOUSE_SPLIT_LINE);
    modeGroup->addButton(modeConstruction,   MAPED_MOUSE_CONSTRUCTION_LINES);
    modeGroup->addButton(modeExtendLineP1,   MAPED_MOUSE_EXTEND_LINE_P1);
    modeGroup->addButton(modeExtendLineP2,   MAPED_MOUSE_EXTEND_LINE_P2);

    modeGroup->button(maped->getMapedMouseMode())->setChecked(true);

    // Edit Box
    QVBoxLayout * editBox = new QVBoxLayout;

    editBox->addWidget(modeNone);
    editBox->addWidget(modeDrawLine);
    editBox->addWidget(modeCreateLine);
    editBox->addLayout(angleSpin);
    editBox->addLayout(lenSpin);
    editBox->addWidget(modeConstruction);
    editBox->addWidget(modeConstrCicle);
    editBox->addLayout(radiusSpin);
    editBox->addWidget(modeSplitLine);
    editBox->addWidget(modeExtendLineP1);
    editBox->addWidget(modeExtendLineP2);
    editBox->addWidget(modeDelLine);
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
    QPushButton * unload       = new QPushButton("Unload All");
    QPushButton * loadMosaic   = new QPushButton("Load Style Map");
    QPushButton * loadProto    = new QPushButton("Load Proto Map");
    QPushButton * loadMotif    = new QPushButton("Load Selected Motifs");
    QPushButton * loadTileUnit = new QPushButton("Load Unit Tiling");
    QPushButton * loadTileRep  = new QPushButton("Load Repeated Tiling");
    QPushButton * loadFile     = new QPushButton("Load Map File");
    QPushButton * createMap    = new QPushButton("Create New Map");

    validStyles = new QComboBox();

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addWidget(unload);
    vbox->addWidget(validStyles);
    vbox->addWidget(loadMosaic);
    vbox->addWidget(loadProto);
    vbox->addWidget(loadMotif);
    vbox->addWidget(loadTileRep);
    vbox->addWidget(loadTileUnit);
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

    updateStyleSelections();

    return gbox;
}

QGroupBox * page_map_editor::createPushGroup()
{
    QPushButton * pbPushMap  = new QPushButton("Push Edit Map");
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

    pbRestoreConstructs->setText("Restore C-lines");
    pbUndoConstructs->setText("Undo C-lines");
    pbRedoConstructs->setText("Redo C-lines");
    pbClearConstructs->setText("Clear C-lines");
    pbSaveTemplate->setText("Save C-lines");
    pbLoadTemplate->setText("Load C-lines");

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

void  page_map_editor::initPageStatusString()
{
       QString msg( "<body>"
                    "<font color=magenta>tile</font>  |  "
                    "<span style=\"color:rgb(255,127,0)\">motif boundary</span>  |  "
                    "<font color=yellow>ext boundary</font>  |  "
                    "<font color=blue>vertex</font>  |  "
                    "<font color=yellow>pt</font>  |  "
                    "<font color=#800000>mid-edge</font>  |  "  // dark red
                    "<font color=#808000>mid-line</font>  |  "  // dark yellow
                    "</body>");

    pageStatusString = msg;
}

void  page_map_editor::onEnter()
{
    updateStyleSelections();

    setPageStatus();

    modeGroup->blockSignals(true);
    modeGroup->button(maped->getMapedMouseMode())->setChecked(true);
    modeGroup->blockSignals(false);

    Sys::cropMakerView->aquire(db,CM_MAPED);
}

void  page_map_editor::onExit()
{
    clearPageStatus();

    db->resetMouseInteraction();

    Sys::cropMakerView->setShowCrop(CM_MAPED,false);
    Sys::cropMakerView->release(CM_MAPED);

    if (cropDlg)
    {
        cropDlg->close();
        cropDlg = nullptr;
    }
}

void page_map_editor::onRefresh()
{
    static bool oldCropEditState = false;

    bool currentCropEditState = Sys::cropMakerView->getShowCrop(CM_MAPED);
    if (currentCropEditState != oldCropEditState)
    {
        if (currentCropEditState)
        {
            pageStatusString = "Left Click on corner to resize crop, or drag to move";
        }
        else
        {
            initPageStatusString();
        }
        setPageStatus();
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
        compositeVChk->setText(map->summary());
    else
        compositeVChk->setText("No Map");

    map = db->getMap(LAYER_1);
    if (map)
        layer1VChk->setText(QString("%1 %2").arg(sMapEditorMapType[db->getMapType(map)],map->info()));
    else
        layer1VChk->setText("No map");

    map = db->getMap(LAYER_2);
    if (map)
        layer2VChk->setText(QString("%1 %2").arg(sMapEditorMapType[db->getMapType(map)],map->info()));
    else
        layer2VChk->setText("No map");

    map = db->getMap(LAYER_3);
    if (map)
        layer3VChk->setText(QString("%1 %2").arg(sMapEditorMapType[db->getMapType(map)],map->info()));
    else
        layer3VChk->setText("No map");

    modeGroup->button(maped->getMapedMouseMode())->setChecked(true);

    pbCleanseVertices->setText(QString("Coalesce Verts\nSensitivity: %1").arg(config->mapedMergeSensitivity));
    pbCleanseMap->setText(QString("Cleanse Map\n0x%1").arg(QString::number(config->mapedCleanseLevel,16)));

    if (viewDCEL->isChecked())
        viewDCEL->setStyleSheet("background-color:yellow; color:red;");
    else
        viewDCEL->setStyleSheet("");

    QString astring = "Construction Lines";
    int  count =  db->constructionLines.count() + db->constructionCircles.count();
    if (count)
    {
        astring += QString(": %1  Circles:%2").arg(db->constructionLines.count()).arg(db->constructionCircles.count());
    }
    constructionBox->setTitle(astring);

    showConsChk->setChecked(db->showConstructionLines);

    smxWidget->refresh();
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
        txt << QString("Editor mode: %1  Crop Mode: %2 %3 %4").arg(sMapEditorMode[mode]).arg(crop->getContentString()).arg(emb).arg(app);
    }
    else
    {
        txt << QString("Editor mode: %1  Crop Mode: NONE").arg(sMapEditorMode[mode]);
    }

    MapPtr map                = db->getEditMap();
    eMapEditorMapType mapType = db->getMapType(map);
    MapEditorLayer & layer    = db->getLayer(map);
    txt << QString("Map status: %1").arg(sMapEditorMapType[mapType]);

    if (map)
        txt << QString("Map: %1  %2 ").arg(Utils::addr(map.get())).arg(map->summary());
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
        for (auto & wdelp : std::as_const(db->getDesignElements()))
        {
            DELPtr delp = wdelp.lock();
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

    case MAPED_LOADED_FROM_PROTOTMAKER:
    {
        ProtoPtr pp = layer.getProto();
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

    case MAPED_LOADED_FROM_STYLE:
        txt << "Mosaic";
        break;

    case MAPED_LOADED_FROM_TILING_UNIT:
    case MAPED_LOADED_FROM_TILING_REPEATED:
    {
        TilingPtr tp = layer.getTiling();
        if (tp)
        {
            txt << tp->info();
        }
    }
        break;

    case MAPED_LOADED_FROM_FILE:
    case MAPED_LOADED_FROM_FILE_MOTIF:
    case MAPED_TYPE_CREATED:
    case MAPED_TYPE_CROP:
    case MAPED_TYPE_NONE:
        break;
    }

    QString mi;
    MapMouseActionPtr mma = db->getMouseInteraction();
    if (mma)
        mi = mma->desc;
    else
        mi = "no interaction";

    QPointF a = Sys::mapEditorView->getMousePos();
    QPointF b = Sys::mapEditorView->screenToModel(a);
    QString astring;
    QTextStream ts(&astring);
    ts << "   pos: (" << a.x() << ", " << a.y() << ") ("
                      << b.x() << ", " << b.y() << ")";
    mi += astring;

    txt << QString("%1 : %2").arg(sMapEditorMouseMode[maped->getMapedMouseMode()]).arg(mi);

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
            txt << crop->getContentString();
        }
    }

    MapEditorSelection * selector = maped->getSelector();
    SelectionSet set = selector->getCurrentSelections();
    if (set.size())
    {
        str.clear();
        for (auto & sel : std::as_const(set))
        {
            eMapSelection type  = sel->getType();
            str += QString("Selection=%1").arg(sMapSelection[type]);
            switch(type)
            {
            case MAP_VERTEX:
            {
                auto vert  = sel->getVertex();
                QPointF pt = vert->pt;
                str += QString(" (%1,%2)\n").arg(pt.x()).arg(pt.y());
            }   break;

            case MAP_POINT:
            {
                QPointF pt = sel->getPoint();
                str += QString(" (%1,%2)\n").arg(pt.x()).arg(pt.y());
            }   break;

            case MAP_CIRCLE:
            {
                QPointF pt   = sel->getPoint();
                CirclePtr c  = sel->getCircle();
                if (c)
                {
                    str += QString(" (%1,%2) center=(%4,%5) radius=%3\n").arg(pt.x()).arg(pt.y()).arg(c->radius).arg(c->centre.x()).arg(c->centre.y());
                }
            }   break;

            case MAP_LINE:
            {
                QLineF line = sel->getLine();
                str += QString(" [(%1,%2)(%3,%4)\n").arg(line.p1().x()).arg(line.p1().y()).arg(line.p2().x()).arg(line.p2().y());
            }   break;

            case MAP_EDGE:
                EdgePtr e = sel->getEdge();
                str += QString(" [(%1,%2)(%3,%4) %5").arg(e->v1->pt.x()).arg(e->v1->pt.y()).arg(e->v2->pt.x()).
                       arg(e->v2->pt.y()).arg((e->isCurve()) ? "curve" : "line");
                if (e->isCurve())
                {
                    auto  & ad = e->getArcData();
                    QString s2 =QString(" s=%1 e=%2 span=%3").arg(ad.start()).arg(ad.end()).arg(ad.span());
                    str += s2;
                }
                str += "\n";
            }   break;
        }
    }
    else
    {
        str = "No selection";
    }
    txt << str;

    statusBox->setText(txt.join("\n"));
}

void page_map_editor::updateStyleSelections()
{
    validStyles->clear();
    auto mosaic     = mosaicMaker->getMosaic();
    StyleSet styles = mosaic->getStyleSet();
    for (StylePtr style : styles)
    {
        if (style->getStyleMap())
        {
            QString name = style->getStyleDesc();
            WeakStylePtr wsp = style;
            validStyles->addItem(name,QVariant::fromValue(wsp));
        }
    }
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
    emit sig_updateView();
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
    emit sig_updateView();
}

void page_map_editor::slot_mosaicLoaded(VersionedFile vfile)
{
    qDebug() << "page_map_editor: loaded -" << vfile.getVersionedName().get();
    maped->initStashFrom(vfile.getVersionedName());
    slot_mosaicChanged();
}

void page_map_editor::slot_mosaicChanged()
{
    updateStyleSelections();

    if (viewControl->isEnabled(VIEW_MAP_EDITOR))
    {
        eMapEditorMapType mtype = db->getMapType(db->getEditMap());
        switch (mtype)
        {
        case MAPED_LOADED_FROM_STYLE:
            //maped->loadFromMosaicStyle(styleBox->value());
            break;

        case MAPED_LOADED_FROM_PROTOTMAKER:
            maped->loadFromfPrototypeMaker();
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
        case MAPED_TYPE_NONE:
        case MAPED_TYPE_COMPOSITE:
        case MAPED_TYPE_COMPOSITE_MOTIF:
        case MAPED_TYPE_CREATED:
        case MAPED_TYPE_CROP:
            break;
        }
    }
}

void page_map_editor::slot_tilingLoaded (VersionedFile vfile)
{
    Q_UNUSED(vfile);

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
    DELPtr delp  = layer.getDel();
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

    MapVerifier mv(map);
    if (mv.verify(true))
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
    MapCleanser mc(map);
    mc.cleanse(divideupIntersectingEdges,config->mapedMergeSensitivity);
    emit sig_updateView();
}

void page_map_editor::slot_joinColinearEdges()
{
    MapPtr map = db->getEditMap();
    if (!map) return;
    MapCleanser mc(map);
    mc.cleanse(joinupColinearEdges,config->mapedMergeSensitivity);
    emit sig_updateView();
}

void page_map_editor::slot_cleanVertices()
{
    auto map = db->getEditMap();
    if (!map) return;

    qDebug() << map->summary();
    MapCleanser mc(map);
    mc.cleanse(coalescePoints,config->mapedMergeSensitivity);
    qDebug() << map->summary();
    emit sig_updateView();
}

void page_map_editor::slot_removeUnconnectedVertices()
{
    MapPtr map = db->getEditMap();
    if (!map) return;
    MapCleanser mc(map);
    mc.cleanse(badVertices_0,config->mapedMergeSensitivity);
    emit sig_updateView();
}

void page_map_editor::slot_removeSingleConnectVertices()
{
    MapPtr map = db->getEditMap();
    if (!map) return;
    MapCleanser mc(map);
    mc.cleanse(badVertices_1,config->mapedMergeSensitivity);
    emit sig_updateView();
}

void page_map_editor::slot_removeZombieEdges()
{
    MapPtr map = db->getEditMap();
    if (!map) return;
    MapCleanser mc(map);
    mc.cleanse(coalesceEdges,config->mapedMergeSensitivity);
    emit sig_updateView();
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
    emit sig_updateView();
}

void page_map_editor::slot_showCons(bool show)
{
    db->showConstructionLines = show;
    emit sig_updateView();
}

void page_map_editor::slot_showMap(bool show)
{
    db->showMap = show;
    emit sig_updateView();
}

void page_map_editor::slot_showPoints(bool show)
{
    db->showPoints = show;
    emit sig_updateView();
}

void page_map_editor::slot_showMidPoints(bool show)
{
    db->showMidPoints = show;
    emit sig_updateView();
}

void page_map_editor::slot_showDirPoints(bool show)
{
    db->showDirnPoints = show;
    emit sig_updateView();
}

void page_map_editor::slot_showArcCentre(bool show)
{
    db->showArcCentre = show;
    emit sig_updateView();
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
    emit sig_updateView();
}

void page_map_editor::slot_undoConstructionLines()
{
    maped->loadPrevStash();
    emit sig_updateView();
}

void page_map_editor::slot_redoConstructionLines()
{
    maped->loadNextStash();
    emit sig_updateView();
}

void page_map_editor::slot_clearConstructionLines()
{
    // this does not affect the stash
    maped->getDb()->constructionLines.clear();
    maped->getDb()->constructionCircles.clear();
    emit sig_updateView();
}

void page_map_editor::slot_dumpMap()
{
    MapPtr m = db->getEditMap();
    if (m)
    {
        m->dump(true);
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
            crop = db->MosaicCropMaker::getCrop();
            if (!crop)
            {
                // creae one
                crop = db->createCrop();
            }
            Q_ASSERT(crop);
            // make a local copy
            db->setCrop(crop);
        }

        Sys::cropMakerView->setShowCrop(CM_MAPED,true);

        cropDlg = new CropDlg(db->getCrop());
        cropDlg->show();

        connect(cropDlg->cw, &CropWidget::sig_cropModified, this, [this]() { emit sig_updateView(); } );
        connect(cropDlg->cw, &CropWidget::sig_cropChanged,  this, [this]() { emit sig_reconstructView(); } );
        connect(cropDlg,     &CropDlg::sig_dlg_done,        this,  &page_map_editor::slot_completeCrop);

        emit sig_reconstructView();
    }
    else
    {
        Sys::cropMakerView->setShowCrop(CM_MAPED,false);
        if (cropDlg)
        {
            cropDlg->close();
            cropDlg = nullptr;
        }
    }
}

void page_map_editor::slot_completeCrop()
{
    Sys::cropMakerView->setShowCrop(CM_MAPED,false);
    cropDlg = nullptr;
    chkEditCrop->blockSignals(true);
    chkEditCrop->setChecked(false);
    chkEditCrop->blockSignals(false);
    emit sig_reconstructView();
}

void page_map_editor::slot_embedCrop()
{
    if (!Sys::cropMakerView->getShowCrop(CM_MAPED))
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
        layer.setLayer(map,MAPED_TYPE_CROP);
        if (layer.getId() == COMPOSITE )
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

    emit sig_reconstructView();
}

void page_map_editor::slot_applyCrop()
{
    if (!Sys::cropMakerView->getShowCrop(CM_MAPED))
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

    emit sig_reconstructView();
}

void page_map_editor::slot_cleanseMap()
{
    auto db    = maped->getDb();
    MapPtr map = db->getEditMap();
    if (!map) return;

    MapCleanser mc(map);
    mc.cleanse(config->mapedCleanseLevel,config->mapedMergeSensitivity);

    switch(db->getMapType(map))
    {
    case MAPED_LOADED_FROM_STYLE:
    {
        MosaicPtr mosaic = mosaicMaker->getMosaic();
        if (mosaic)
        {
            ProtoPtr proto = mosaic->getPrototypes().first();
            if (proto)
            {
                proto->setCleanseLevel(config->mapedCleanseLevel);
                proto->setCleanseSensitivity(config->mapedMergeSensitivity);
            }
        }
    } break;

    case MAPED_LOADED_FROM_PROTOTMAKER:
    {
        ProtoPtr proto = prototypeMaker->getSelectedPrototype();
        if (proto)
        {
                proto->setCleanseLevel(config->mapedCleanseLevel);
                proto->setCleanseSensitivity(config->mapedMergeSensitivity);
        }
    } break;

    case MAPED_LOADED_FROM_TILING_UNIT:
    case MAPED_LOADED_FROM_TILING_REPEATED:
    case MAPED_LOADED_FROM_MOTIF:
    case MAPED_LOADED_FROM_FILE:
    case MAPED_LOADED_FROM_FILE_MOTIF:
    case MAPED_TYPE_NONE:
    case MAPED_TYPE_COMPOSITE:
    case MAPED_TYPE_COMPOSITE_MOTIF:
    case MAPED_TYPE_CREATED:
    case MAPED_TYPE_CROP:
        break;
    }

    emit sig_updateView();
}

void page_map_editor::slot_cleansed()
{
    emit sig_updateView();
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

void page_map_editor::slot_lineWidthChanged(uint r)
{
    Sys::mapEditorView->mapLineWidth = r;
    emit sig_updateView();
}

void page_map_editor::slot_consWidthChanged(uint r)
{
    Sys::mapEditorView->constructionLineWidth = r;
    emit sig_updateView();
}

void page_map_editor::slot_cleanseSettings()
{
    MapPtr map = maped->getDb()->getEditMap();  // could be null

    uint level = config->mapedCleanseLevel;
    qreal sens = config->mapedMergeSensitivity;

    DlgCleanse dlg(map,level,sens,this);
    connect(&dlg, &DlgCleanse::sig_cleansed,this,&page_map_editor::slot_cleansed, Qt::QueuedConnection);

    int rv = dlg.exec();
    if (rv != QDialog::Accepted)
        return;

    // puts cleanse settings in config not for any map
    level = dlg.fromCheckboxes();
    sens  = dlg.getSsnsitivity();
    config->mapedCleanseLevel     = level;
    config->mapedMergeSensitivity = sens;
}

void page_map_editor::slot_loadCopies(bool show)
{
    config->mapedLoadCopies = show;
}

void page_map_editor::slot_saveTemplate()
{
    // saves current stash to the given name
    QString name = lastNamedTemplate.getVersionedName().get();

    DlgName dlg;
    dlg.setWindowTitle("Save Construction Line Template");
    dlg.newEdit->setText(name);
    int retval = dlg.exec();

    if (retval == QDialog::Rejected)
    {
        qDebug() << "Canceled";
        return;
    }
    Q_ASSERT(retval == QDialog::Accepted);

    name = dlg.newEdit->text();
    if (!name.isEmpty())
    {
        VersionedName vname(name);
        maped->saveTemplate(vname);
    }
    else
        qInfo() << "No template selected";
}

void page_map_editor::slot_loadTemplate()
{
    VersionFileList xfl;

    if (config->mapedOldTemplates)
        xfl = FileServices::getFiles(FILE_TEMPLATE);
    else
        xfl = FileServices::getFiles(FILE_TEMPLATE2);

    DlgListSelect dlg(xfl,this);
    dlg.setWindowTitle("Load Construction Line Template");
    int retval = dlg.exec();
    if (retval == QDialog::Rejected)
    {
        qDebug() << "Canceled";
        return;
    }
    Q_ASSERT(retval == QDialog::Accepted);

    VersionedFile xfile = dlg.selected;
    if (xfile.isEmpty())
    {
        qInfo() << "No template selected";
        return;
    }

    db->showConstructionLines = true;

    maped->loadTemplate(xfile, animateChk->isChecked());

    lastNamedTemplate = xfile;
}

void page_map_editor::slot_loadMosaicPrototype()
{
    int index = validStyles->currentIndex();
    QVariant v = validStyles->itemData(index);
    WeakStylePtr wsp =  v.value<WeakStylePtr>();
    StylePtr style = wsp.lock();
    if (style)
    {
        maped->loadFromMosaicStyle(style);
        panel->delegateView(VIEW_MAP_EDITOR,true);
    }
}

void page_map_editor::slot_loadMotifPrototype()
{
    maped->loadFromfPrototypeMaker();
    panel->delegateView(VIEW_MAP_EDITOR,true);
}

void page_map_editor::slot_loadMotif()
{
    maped->loadSelectedMotifs();
    panel->delegateView(VIEW_MAP_EDITOR,true);
}

void page_map_editor::slot_loadTileUnit()
{
    maped->loadTilingUnit();
    panel->delegateView(VIEW_MAP_EDITOR,true);
}

void page_map_editor::slot_loadTileRep()
{
    maped->loadTilingRepeated();
    panel->delegateView(VIEW_MAP_EDITOR,true);
}

void page_map_editor::slot_loadMapFile()
{
    qDebug() << "page_map_editor::slot_loadMap";

    VersionFileList xfiles = FileServices::getFiles(FILE_MAP);

    DlgListSelect dlg(xfiles,this);
    if (!lastNamedMap.isEmpty())
    {
        dlg.list->selectItemByName(lastNamedMap.get());
    }
    int retval = dlg.exec();
    if (retval == QDialog::Rejected)
    {
        qDebug() << "Canceled";
        return;
    }
    Q_ASSERT(retval == QDialog::Accepted);

    VersionedName vname = dlg.selected.getVersionedName();

    if (vname.isEmpty())
    {
        qInfo() << "No map selected";
        return;
    }

    lastNamedMap = vname;

    VersionedFile xfile = FileServices::getFile(vname,FILE_MAP);
    if (xfile.isEmpty())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Map NOT loaded");
        box.exec();
        return;
    }

    MapEditorMapLoader loader;
    MapPtr loadedMap  = loader.loadMosaicMap(xfile);
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
    auto bip = loader.getBackground();

    qDebug().noquote() << loadedMap->summary();

    maped->loadFromMap(loadedMap,maptype);
    maped->getDb()->setBackgroundImage(bip);    // persists

    Sys::mapEditorView->setModelXform(xf,false,Sys::nextSigid());

    if (bip)
        Sys::currentBkgImage = BKGD_IMAGE_MAPED;

    tallySelects();

    panel->delegateView(VIEW_MAP_EDITOR,true);

    if (viewControl->isEnabled(VIEW_MAP_EDITOR))
    {
        viewControl ->setSize(loader.getViewSize());
    }

    emit sig_updateView();

    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    box.setText("Map loaded OK");
    box.exec();
}

void page_map_editor::slot_createMap()
{
    MapPtr amap = make_shared<Map>(QString("Local"));
    maped->loadFromMap(amap,MAPED_TYPE_CREATED);
    panel->delegateView(VIEW_MAP_EDITOR,true);
}

void page_map_editor::slot_unloadMaps()
{
    maped->unload();

    //maped->setCanvasXform(vcontrol->getCurrentXform());
    //QSize sz = view->frameSettings.getCropSize(VIEW_MAP_EDITOR);
    //maped->getSys::mapEditorView()->setCenterScreenUnits(QRect(QPoint(0,0),sz).center());

    emit sig_updateView();
}

void page_map_editor::slot_pushMap()
{
    qDebug() << "page_map_editor::slot_pushMap()";

    bool brv = false;

    QString msg = "Undefined";

    MapEditorLayer & layer = db->getEditLayer();
    auto map   = layer.getMapedLayerMap();
    if (!map || map->isEmpty())
    {
        brv = false;
    }
    else
    {
#if 0
        eMapEditorMapType type = maped->getDb()->getMapType(map);
        switch (type)
        {
        case MAPED_LOADED_FROM_MOSAIC:
            msg = "Mosaic";
            brv= maped->pushToMosaic(layer);
            break;

        case MAPED_LOADED_FROM_MOTIF_PROTOTYPE:
        case MAPED_LOADED_FROM_MOTIF:
        case MAPED_LOADED_FROM_FILE_MOTIF:
        case MAPED_TYPE_COMPOSITE_MOTIF:
            msg = "Motif";
            brv =  maped->pushToMotif(map);
            break;

        case MAPED_LOADED_FROM_TILING_UNIT:
            msg = "Tiling";
            brv = maped->pushToTiling(map,false);
            break;

        case MAPED_LOADED_FROM_TILING_REPEATED:
        {
            QMessageBox box(panel);
            box.setIcon(QMessageBox::Warning);
            box.setWindowTitle("Pushing a complete tiling");
            box.setText("This is NOT RECOMMENDED. Proceed art your peril.");
            box.setInformativeText("The Map Editor can be used to examinine a complete tiling, "
                                   "but you should only push a tiling unit, not the complete tiling");
            box.setStandardButtons(QMessageBox::Apply | QMessageBox::Cancel);
            box.setDefaultButton(QMessageBox::Cancel);
            int rv2 = box.exec();
            if (rv2 == QMessageBox::Cancel)
            {
                return;
            }

            msg = "Tiling";
            brv = maped->pushToTiling(map,false);
        }   break;

        case MAPED_TYPE_UNKNOWN:
            msg = "Unknown";
            break;

        case MAPED_TYPE_CROP:
            msg = "Crop";
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
                {
                    msg = "Mosaic";
                    brv= maped->pushToMosaic(layer);
                }   break;
                case 2:
                {
                    msg = "Motif";
                    brv =  maped->pushToMotif(map);

                }   break;
                case 3:
                    msg = "Tiling";
                    brv = maped->pushToTiling(map,false);
                    break;
                }
            }
            else
            {
                return;
            }
        }   break;
        }
#else
        DlgPushSelect dlg(this);
        int rv2 = dlg.exec();
        if (rv2 == QDialog::Accepted)
        {
            switch (dlg.retval)
            {
            case 1:
            {
                msg = "Mosaic";
                maped->pushToMosaic(layer);
                brv = true;     // always does something
            }   break;
            case 2:
            {
                msg = "Motif";
                brv =  maped->pushToMotif(layer);

            }   break;
            case 3:
                msg = "Tiling";
                brv = maped->pushToTiling(layer,false);
                break;
            }
        }
#endif
    }

    QMessageBox box(this);
    if (brv)
    {
        box.setIcon(QMessageBox::Information);
        QString str = QString("Pushed to %1 - OK").arg(msg);
        box.setText(str);
    }
    else
    {
        box.setIcon(QMessageBox::Warning);
        QString str = QString("Push to %1 - FAILED").arg(msg);
        box.setText(str);
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

    VersionedFile xfile = FileServices::getFile(lastNamedMap,FILE_MAP);
    QString path;
    if (xfile.isEmpty())
        path = Sys::mapsDir;
    else
        path = xfile.getPathOnly();

    QString filename = QFileDialog::getSaveFileName(this, "Save Map File",path,"Map (*.xml)",nullptr,QFileDialog::DontConfirmOverwrite);
    if (filename.isEmpty())
        return;

    xfile.setFromFullPathname(filename);

    if (QFile::exists(xfile.getPathedName()))
    {
        // file already exists
        QMessageBox msgBox(panel);
        msgBox.setText(QString("The map %1 already exists").arg(xfile.getVersionedName().get()));
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

        if (msgBox.clickedButton() == bump)
        {

            VersionedName vn  = FileServices::getNextVersion(FILE_MAP,xfile.getVersionedName()); // appends a version
            xfile.updateFromVersionedName(vn);
        }
        else
        {
            Q_UNUSED(save)
        }
    }

    MapEditorMapWriter writer;
    bool rv = writer.writeXML(xfile,db);
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
    emit sig_updateView();
}

void page_map_editor::slot_editLayer(int id, bool checked)
{
    if (checked)
        db->setEditSelect(eMapedLayer(id));
    else
        db->setEditSelect(NO_MAP);

    if (Sys::viewController->isEnabled(VIEW_MAP_EDITOR))
    {
        emit sig_updateView();
    }
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
    if (Sys::cropMakerView->getShowCrop(CM_MAPED))
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
