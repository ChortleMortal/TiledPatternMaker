#include <QApplication>
#include <QButtonGroup>
#include <QFileDialog>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QRadioButton>
#include <QTextEdit>
#include <QToolButton>

#include "gui/map_editor/map_editor_db.h"
#include "gui/map_editor/map_editor_map_loader.h"
#include "gui/map_editor/map_editor_map_writer.h"
#include "gui/panels/page_map_editor.h"
#include "gui/top/controlpanel.h"
#include "gui/top/view_controller.h"
#include "gui/viewers/crop_maker_view.h"
#include "gui/viewers/map_editor_view.h"
#include "gui/widgets/crop_widget.h"
#include "gui/widgets/dlg_cleanse.h"
#include "gui/widgets/dlg_listselect.h"
#include "gui/widgets/dlg_name.h"
#include "gui/widgets/dlg_push_select.h"
#include "gui/widgets/dlg_saveversion.h"
#include "gui/widgets/layout_sliderset.h"
#include "gui/widgets/smx_widget.h"
#include "model/makers/crop_maker.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/motif_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/mosaics/mosaic.h"
#include "model/motifs/explicit_map_motif.h"
#include "model/motifs/adaptive_motif.h"
#include "model/motifs/radial_motif.h"
#include "model/prototypes/tile_motif.h"
#include "model/prototypes/prototype.h"
#include "model/settings/configuration.h"
#include "model/tilings/backgroundimage.h"
#include "sys/geometry/crop.h"
#include "sys/geometry/map_cleanser.h"
#include "sys/geometry/map_verifier.h"
#include "sys/qt/utilities.h"
#include "sys/sys/fileservices.h"
#include "sys/tiledpatternmaker.h"

using std::make_shared;

typedef std::shared_ptr<RadialMotif>            RadialPtr;

typedef std::weak_ptr<class Style>              WeakStylePtr;

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
    QVBoxLayout * right = new QVBoxLayout;
    left->setContentsMargins(0,0,0,0);
    right->setContentsMargins(0,0,0,0);

    QVBoxLayout * leftLeft   = new QVBoxLayout;
    QVBoxLayout * leftRight   = new QVBoxLayout;
    leftLeft->setContentsMargins(0,0,0,0);
    leftRight->setContentsMargins(0,0,0,0);

    QHBoxLayout * leftBot = new QHBoxLayout;
    leftBot->setContentsMargins(0,0,0,0);
    leftBot->addLayout(leftLeft);
    leftBot->addLayout(leftRight);

    leftLeft->addWidget(editBox);
    leftLeft->addStretch();

    leftRight->addWidget(mapBox);
    leftRight->addWidget(constructionBox);
    leftRight->addWidget(pushBox);
    leftRight->addWidget(configBox);
    leftRight->addStretch();

    left->addWidget(selectBox);
    left->addLayout(leftBot);

    right->addWidget(loadBox);
    right->addWidget(viewBox);
    right->addStretch();

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addLayout(left);
    hbox->addLayout(right);

    // putting it together
    vbox->addLayout(hbox);
    vbox->addWidget(editorStatusBox);
    vbox->addStretch();

    connect(tilingMaker, &TilingMaker::sig_tilingLoaded,   this,   &page_map_editor::slot_tilingLoaded);
    connect(mosaicMaker, &MosaicMaker::sig_mosaicLoaded,   this,   &page_map_editor::slot_mosaicLoaded);

    slot_debugChk(config->mapedStatusBox);

    slot_editLayer((int)LayerId::L1,true);

    cropDlg = nullptr;
}

QGroupBox * page_map_editor::createMapSelects()
{
    QLabel * unloadLabel = new QLabel("");
    QPushButton * pbComp = new QPushButton("Unload");
    QPushButton * pbL1   = new QPushButton("Unload");
    QPushButton * pbL2   = new QPushButton("Unload");
    QPushButton * pbL3   = new QPushButton("Unload");

    unloadGroup = new QButtonGroup();
    unloadGroup->addButton(pbComp,(int)LayerId::Composite);
    unloadGroup->addButton(pbL1,(int)LayerId::L1);
    unloadGroup->addButton(pbL2,(int)LayerId::L2);
    unloadGroup->addButton(pbL3,(int)LayerId::L3);

    QLabel * editL    = new QLabel("Edit");
    compositeEChk     = new QCheckBox("  Composite");
    layer1EChk        = new QCheckBox("  Layer 1");
    layer2EChk        = new QCheckBox("  Layer 2");
    layer3EChk        = new QCheckBox("  Layer 3");

    QString compositeColorCss = QString("color: %1;").arg(maped->getLayer(LayerId::Composite).getColor().name());
    QString layer1ColorCss    = QString("color: %1;").arg(maped->getLayer(LayerId::L1).getColor().name());
    QString layer2ColorCss    = QString("color: %1;").arg(maped->getLayer(LayerId::L2).getColor().name());
    QString layer3ColorCss    = QString("color: %1;").arg(maped->getLayer(LayerId::L3).getColor().name());

    compositeEChk->setStyleSheet(compositeColorCss);
    layer1EChk->setStyleSheet(layer1ColorCss);
    layer2EChk->setStyleSheet(layer2ColorCss);
    layer3EChk->setStyleSheet(layer3ColorCss);

    pbComp->setStyleSheet(compositeColorCss);
    pbL1->setStyleSheet(layer1ColorCss);
    pbL2->setStyleSheet(layer2ColorCss);
    pbL3->setStyleSheet(layer3ColorCss);

    editGroup = new QButtonGroup();
    editGroup->addButton(compositeEChk,(int)LayerId::Composite);
    editGroup->addButton(layer1EChk,(int)LayerId::L1);
    editGroup->addButton(layer2EChk,(int)LayerId::L2);
    editGroup->addButton(layer3EChk,(int)LayerId::L3);

    QLabel * viewL    = new QLabel("View");
    compositeVChk     = new QCheckBox;
    layer1VChk        = new QCheckBox;
    layer2VChk        = new QCheckBox;
    layer3VChk        = new QCheckBox;

    compositeVChk->setStyleSheet(compositeColorCss);
    layer1VChk->setStyleSheet(layer1ColorCss);
    layer2VChk->setStyleSheet(layer2ColorCss);
    layer3VChk->setStyleSheet(layer3ColorCss);

    viewGroup = new QButtonGroup();
    viewGroup->setExclusive(false);
    viewGroup->addButton(compositeVChk,(int)LayerId::Composite);
    viewGroup->addButton(layer1VChk,(int)LayerId::L1);
    viewGroup->addButton(layer2VChk,(int)LayerId::L2);
    viewGroup->addButton(layer3VChk,(int)LayerId::L3);

    QVBoxLayout * vb2 = new QVBoxLayout;
    vb2->addWidget(unloadLabel);
    vb2->addWidget(pbComp);
    vb2->addWidget(pbL1);
    vb2->addWidget(pbL2);
    vb2->addWidget(pbL3);

    QVBoxLayout * vb0 = new QVBoxLayout;
    vb0->addWidget(editL);
    vb0->addWidget(compositeEChk);
    vb0->addWidget(layer1EChk);
    vb0->addWidget(layer2EChk);
    vb0->addWidget(layer3EChk);

    QVBoxLayout * vb1 = new QVBoxLayout;
    vb1->addWidget(viewL,0);
    vb1->addWidget(compositeVChk);
    vb1->addWidget(layer1VChk);
    vb1->addWidget(layer2VChk);
    vb1->addWidget(layer3VChk);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addLayout(vb0);
    hbox->addLayout(vb1);
    hbox->addStretch();
    hbox->addLayout(vb2);

    connect(editGroup,  &QButtonGroup::idToggled, this, &page_map_editor::slot_editLayer);
    connect(viewGroup,  &QButtonGroup::idToggled, this, &page_map_editor::slot_viewLayer);
    connect(unloadGroup,&QButtonGroup::idClicked, this, &page_map_editor::slot_unloadLayer);

    QGroupBox * gb = new QGroupBox("Map Selects");
    gb->setLayout(hbox);
    return gb;
}

QGroupBox * page_map_editor::createStatusGroup()
{
    statusTxt =  new ResizingTextEdit;

    connect(statusTxt->document(), &QTextDocument::contentsChanged, this, [this] { statusTxt->updateGeometry();} );

    QVBoxLayout * vbox = new QVBoxLayout();
    vbox->addWidget(statusTxt);

    QGroupBox * gBox = new QGroupBox();
    gBox->setLayout(vbox);

    return gBox;
}


QGroupBox   * page_map_editor::createSettingsGroup()
{
    SpinSet * lineWidthSpin = new SpinSet("Line Width",3,1,9);
    SpinSet * consWidthSpin = new SpinSet("Cons Width",1,1,9);

    smxWidget = new SMXWidget(Sys::mapEditorView.get(),true,true);

    QCheckBox   * chkLoadCopy = new QCheckBox("Load copies");
    chkLoadCopy->setChecked(config->mapedLoadCopies);

    QHBoxLayout * hb1 = new QHBoxLayout;
    hb1->addWidget(chkLoadCopy);
    hb1->addStretch();

    QHBoxLayout * hb2 = new QHBoxLayout;
    hb2->addLayout(lineWidthSpin);
    hb2->addLayout(consWidthSpin);
    hb2->addStretch();

    QVBoxLayout * qvbox = new QVBoxLayout();
    qvbox->addWidget(smxWidget);
    qvbox->addLayout(hb1);
    qvbox->addLayout(hb2);

    connect(lineWidthSpin,    &SpinSet::valueChanged, this, &page_map_editor::slot_lineWidthChanged);
    connect(consWidthSpin,    &SpinSet::valueChanged, this, &page_map_editor::slot_consWidthChanged);
    connect(chkLoadCopy,      &QCheckBox::clicked,    this, &page_map_editor::slot_loadCopies);

    QGroupBox * gBox = new QGroupBox("Settings");
    gBox->setLayout(qvbox);

    return gBox;
}

QGroupBox * page_map_editor::createMapGroup()
{
    QPushButton * pbVerifyMap       = new QPushButton("Make Explicit");
    QPushButton * pbMakeExplicit    = new QPushButton("Verify Map");
    QPushButton * pbDumpMap         = new QPushButton("Dump Map");

    QPushButton  * pbCleanse        = new QPushButton("Cleanse Settings");
    QPushButton * pbCleanseMap      = new QPushButton("Cleans Map");

    pbApplyCrop                     = new QPushButton("Crop Outside");
    pbEmbedCrop                     = new QPushButton("Embed Crop");
    chkEditCrop                     = new QCheckBox("Edit Crop");

    chkEditCrop->setStyleSheet("padding-left: 17px;");

    defaultStyle = pbVerifyMap->styleSheet();

    pbVerifyMap->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    pbMakeExplicit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    pbDumpMap->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);


    pbCleanseMap->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    chkEditCrop->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    pbEmbedCrop->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    pbApplyCrop->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Map Box

    QGridLayout * mapGrid = new QGridLayout;
    int row = 0;
    mapGrid->addWidget(pbVerifyMap,row,0);
    mapGrid->addWidget(pbMakeExplicit,row,1);
    mapGrid->addWidget(pbDumpMap,row,2);

    row++;
    mapGrid->addWidget(pbCleanse,row,0);
    mapGrid->addWidget(pbCleanseMap,row,1);

    row++;
    mapGrid->addWidget(chkEditCrop,row,0);
    mapGrid->addWidget(pbEmbedCrop,row,1);
    mapGrid->addWidget(pbApplyCrop,row,2);

    QGroupBox * mapBox  = new QGroupBox("Map");
    mapBox->setLayout(mapGrid);

    connect(pbCleanseMap,           &QToolButton::clicked,  this,   &page_map_editor::slot_cleanseMap);
    connect(pbCleanse,              &QPushButton::clicked,  this,   &page_map_editor::slot_cleanseSettings);
    connect(pbVerifyMap,            &QToolButton::clicked,  this,   &page_map_editor::slot_verify);
    connect(pbMakeExplicit,         &QToolButton::clicked,  this,   &page_map_editor::slot_convertToExplicit);
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
    modeGroup->addButton(modeNone,           (int)MapEditor::MouseMode::None);
    modeGroup->addButton(modeDrawLine,       (int)MapEditor::MouseMode::DrawLine);
    modeGroup->addButton(modeCreateLine,     (int)MapEditor::MouseMode::CreateLine);
    modeGroup->addButton(modeConstrCicle,    (int)MapEditor::MouseMode::ConstructionCircles);
    modeGroup->addButton(modeDelLine,        (int)MapEditor::MouseMode::Delete);
    modeGroup->addButton(modeSplitLine,      (int)MapEditor::MouseMode::SplitLine);
    modeGroup->addButton(modeConstruction,   (int)MapEditor::MouseMode::ConstructionLines);
    modeGroup->addButton(modeExtendLineP1,   (int)MapEditor::MouseMode::ExtendP1);
    modeGroup->addButton(modeExtendLineP2,   (int)MapEditor::MouseMode::ExtendP2);

    modeGroup->button((int)maped->getMapedMouseMode())->setChecked(true);

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
    QCheckBox * showTileCtrChk  = new QCheckBox("Show tile centres");
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
    vbox->addWidget(showTileCtrChk);
    vbox->addWidget(showConsChk);
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
    showTileCtrChk->setChecked(db->showTileCentre);
    showStatusChk->setChecked(config->mapedStatusBox);

    if (config->mapEditorMode == MapEditor::EditorMode::Map)
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
    connect(showTileCtrChk, &QCheckBox::clicked,    this, &page_map_editor::slot_showTileCentre);
    connect(showStatusChk,  &QCheckBox::clicked,    this, &page_map_editor::slot_debugChk);
    connect(viewMap,        &QRadioButton::clicked, this, &page_map_editor::slot_chkViewMap);
    connect(viewDCEL,       &QRadioButton::clicked, this, &page_map_editor::slot_chkViewDCEL);

    return gbox;
}

QGroupBox * page_map_editor::createLoadGroup()
{
    QPushButton * loadMosaic   = new QPushButton("Load Style Map");
    QPushButton * loadProto    = new QPushButton("Load Proto Map");
    QPushButton * loadMotifUnit= new QPushButton("Load Motif Unit");
    QPushButton * loadMotif    = new QPushButton("Load Selected Motifs");
    QPushButton * loadTileUnit = new QPushButton("Load Tiling Unit");
    QPushButton * loadTileRep  = new QPushButton("Load Repeated Tiling");
    QPushButton * loadFile     = new QPushButton("Load Map File");
    QPushButton * createMap    = new QPushButton("Create New Map");
    QPushButton * unload       = new QPushButton("Unload All");

    validStyles = new QComboBox();

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addWidget(loadMosaic);
    vbox->addWidget(validStyles);
    vbox->addWidget(loadProto);
    vbox->addWidget(loadMotifUnit);
    vbox->addWidget(loadMotif);
    vbox->addWidget(loadTileRep);
    vbox->addWidget(loadTileUnit);
    vbox->addWidget(loadFile);
    vbox->addWidget(createMap);
    vbox->addWidget(unload);

    QGroupBox * gbox = new QGroupBox("Import");
    gbox->setLayout(vbox);

    importGroup = new QButtonGroup();
    importGroup->addButton(loadMosaic,      (int)MapType::Style);
    importGroup->addButton(loadProto,       (int)MapType::Prototype);
    importGroup->addButton(loadMotifUnit,   (int)MapType::MotifUnit);
    importGroup->addButton(loadMotif,       (int)MapType::Motif);
    importGroup->addButton(loadTileRep,     (int)MapType::TilingFull);
    importGroup->addButton(loadTileUnit,    (int)MapType::TilingUnit);
    importGroup->addButton(loadFile,        (int)MapType::File);
    importGroup->addButton(createMap,       (int)MapType::Created);
    importGroup->addButton(unload,          (int)MapType::None);

    connect(importGroup,    &QButtonGroup::idClicked, this, &page_map_editor::slot_import);

    updateStyleSelections();

    return gbox;
}

QGroupBox * page_map_editor::createPushGroup()
{
    QPushButton * pbPushMap    = new QPushButton("Push Edit Map");
    QPushButton * pbConvertMap = new QPushButton("Convert to Fixed Map");
    QPushButton * pbSaveMap    = new QPushButton("Save Edit Map");

    QGridLayout * pushLayout = new QGridLayout;
    pushLayout->addWidget(pbPushMap,0,0);
    pushLayout->addWidget(pbConvertMap,0,1);
    pushLayout->addWidget(pbSaveMap,0,2);

    QGroupBox * pushBox = new QGroupBox("Export");
    pushBox->setLayout(pushLayout);

    connect(pbPushMap,    &QPushButton::clicked,  this,   &page_map_editor::slot_pushMap);
    connect(pbConvertMap, &QPushButton::clicked,  this,   &page_map_editor::slot_convertToFixedMap);
    connect(pbSaveMap,    &QPushButton::clicked,  this,   &page_map_editor::slot_saveMapToFile);

    return pushBox;
}

QGroupBox * page_map_editor::createConstructionGroup()
{
    QPushButton * pbClearConstructs     = new QPushButton("Clear C-lines");
    QPushButton * pbSaveTemplate        = new QPushButton("Save C-lines");
    QPushButton * pbLoadTemplate        = new QPushButton("Load C-lines");

    QGridLayout * templateGrid = new QGridLayout;
    templateGrid->addWidget(pbSaveTemplate,0,0);
    templateGrid->addWidget(pbLoadTemplate,0,1);
    templateGrid->addWidget(pbClearConstructs,0,2);

    QGroupBox * templateBox = new QGroupBox("Construction Lines");
    templateBox->setLayout(templateGrid);

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
    modeGroup->button((int)maped->getMapedMouseMode())->setChecked(true);
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

    refreshStatusBox();

    MapPtr map = maped->getMap(LayerId::Composite);
    if (map)
        compositeVChk->setText(map->summary());
    else
        compositeVChk->setText("No Map");

    map = maped->getMap(LayerId::L1);
    if (map)
        layer1VChk->setText(QString("%1 %2").arg(sMapEditorMapType[(int)maped->getMapType(map)],map->info()));
    else
        layer1VChk->setText("No map");

    map = maped->getMap(LayerId::L2);
    if (map)
        layer2VChk->setText(QString("%1 %2").arg(sMapEditorMapType[(int)maped->getMapType(map)],map->info()));
    else
        layer2VChk->setText("No map");

    map = maped->getMap(LayerId::L3);
    if (map)
        layer3VChk->setText(QString("%1 %2").arg(sMapEditorMapType[(int)maped->getMapType(map)],map->info()));
    else
        layer3VChk->setText("No map");

    modeGroup->button((int)maped->getMapedMouseMode())->setChecked(true);

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
    if (!config->mapedStatusBox)
        return;

    QStringList txt;

    MapEditor::EditorMode mode = config->mapEditorMode;

    // Crop
    CropPtr crop = db->getCrop();
    if (crop)
    {
        QString emb = (crop->getEmbed()) ? "Embed" : QString();
        QString app = (crop->getApply()) ? "ApplY" : QString();
        txt << QString("Editor mode: %1  Crop Mode: %2 %3 %4").arg(sMapEditorMode[(int)mode]).arg(crop->getContentString()).arg(emb).arg(app);
    }
    else
    {
        txt << QString("Editor mode: %1  Crop Mode: NONE").arg(sMapEditorMode[(int)mode]);
    }

    // deals with Edit map
    MapPtr map              = maped->getEditMap();
    MapType mapType         = maped->getMapType(map);
    MapEditorLayer & layer  = maped->getLayer(map);
    txt << QString("Map status: %1").arg(sMapEditorMapType[(int)mapType]);

    if (map)
        txt << QString("Map: %1  %2 ").arg(Utils::addr(map.get())).arg(map->summary());
    else
        txt <<  "No edit map";

    if (mode == MapEditor::EditorMode::DCEL)
    {
        DCELPtr dp = db->getActiveDCEL();
        if (dp)
            txt << QString("DCEL: vertices = %1 half-edges = %2 faces = %3").arg(dp->getVertices().size()).arg(dp->getEdges().size()).arg(dp->getFaceSet().size());
        else
            txt <<  "DCEL: NONE";
    }

    switch (mapType)
    {
    case MapType::Composite:
    case MapType::CompositeMotif:
        break;

    case MapType::Motif:
    case MapType::MotifUnit:
    {
        MapedMapPtr mmp   = layer.getWorking();
        TimoMap * timoMap = static_cast<TimoMap*>(mmp.get());
        auto timos        = timoMap->getTileMotifs();
        if (timos.isEmpty())
        {
            txt << "No TileMotif";
        }
        for (auto & timo : std::as_const(timos))
        {
            MotifPtr motif = timo->getMotif();
            TilePtr  tile  = timo->getTile();

            txt << QString("TileMotif = %1").arg(Utils::addr(timo.get()));
            txt << QString("Tile = %1").arg(Utils::addr(tile.get()));
            txt << QString("Motif = %1 %2").arg(Utils::addr(motif.get())).arg(motif->getMotifTypeString());
        }
    }   break;

    case MapType::Prototype:
    {
        MapedMapPtr mmp     = layer.getWorking();
        ProtoMap * protoMap = static_cast<ProtoMap*>(mmp.get());
        ProtoPtr pp = protoMap->getPrototype();
        if (pp)
        {
            CropPtr cr = pp->getCrop();
            txt << QString("Protoype = %1 Crop = %2").arg(Utils::addr(pp.get())).arg(Utils::addr(cr.get()));
        }
    }   break;

    case MapType::Style:
        txt << "Style";
        break;

    case MapType::TilingUnit:
    case MapType::TilingFull:
    {
        MapedMapPtr mmp       = layer.getWorking();
        TilingMap * tilingMap = static_cast<TilingMap*>(mmp.get());
        TilingPtr tp = tilingMap->getTiling();
        if (tp)
        {
            txt << tp->info();
        }
    }   break;

    case MapType::File:
    case MapType::FileMotif:
    case MapType::Created:
    case MapType::Crop:
    case MapType::None:
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

    txt << QString("%1 : %2").arg(sMapEditorMouseMode[(int)maped->getMapedMouseMode()]).arg(mi);

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
    QString     str;
    if (set.size())
    {
        for (auto & sel : std::as_const(set))
        {
            MapSelection::Type type  = sel->getType();
            str += QString("Selection=%1").arg(sMapSelection[(int)type]);
            switch(type)
            {
            case MapSelection::Type::MAP_VERTEX:
            {
                auto vert  = sel->getVertex();
                QPointF pt = vert->pt;
                str += QString(" (%1,%2)\n").arg(pt.x()).arg(pt.y());
            }   break;

            case MapSelection::Type::MAP_POINT:
            {
                QPointF pt = sel->getPoint();
                str += QString(" (%1,%2)\n").arg(pt.x()).arg(pt.y());
            }   break;

            case MapSelection::Type::MAP_CIRCLE:
            {
                QPointF pt   = sel->getPoint();
                CirclePtr c  = sel->getCircle();
                if (c)
                {
                    str += QString(" (%1,%2) center=(%4,%5) radius=%3\n").arg(pt.x()).arg(pt.y()).arg(c->radius).arg(c->centre.x()).arg(c->centre.y());
                }
            }   break;

            case MapSelection::Type::MAP_LINE:
            {
                QLineF line = sel->getLine();
                str += QString(" [(%1,%2)(%3,%4)\n").arg(line.p1().x()).arg(line.p1().y()).arg(line.p2().x()).arg(line.p2().y());
            }   break;

            case MapSelection::Type::MAP_EDGE:
            {
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
    }
    else
    {
        str = "No selection";
    }
    txt << str;

    statusTxt->setText(txt.join("\n"));
}

void page_map_editor::updateStyleSelections()
{
    validStyles->clear();
    auto mosaic             = mosaicMaker->getMosaic();
    const StyleSet & styles = mosaic->getStyleSet();
    for (const StylePtr & style : styles)
    {
        QString name     = style->getStyleDesc();
        WeakStylePtr wsp = style;
        validStyles->addItem(name,QVariant::fromValue(wsp));
        validStyles->setItemData(validStyles->count() - 1, Qt::AlignCenter, Qt::TextAlignmentRole);
    }
}

void page_map_editor::slot_chkViewMap(bool checked)
{
    config->mapEditorMode = (checked) ? MapEditor::EditorMode::Map : MapEditor::EditorMode::DCEL;
    if (config->mapEditorMode ==  MapEditor::EditorMode::DCEL)
    {
        auto map = maped->getEditMap();
        bool rv = maped->useExistingDCEL(map);
        if (!rv)
        {
            rv = maped->createLocalDCEL(map);
            if (!rv && !map)
            {
                // a malformed map is reported by the DCEL itself
                QMessageBox box(this);
                box.setIcon(QMessageBox::Warning);
                box.setText("There is no map - cannot view DCEL");
                box.exec();
            }
        }
    }
    emit sig_updateView();
}

void page_map_editor::slot_chkViewDCEL(bool checked)
{
    config->mapEditorMode = (checked) ? MapEditor::EditorMode::DCEL : MapEditor::EditorMode::Map;
    if (config->mapEditorMode == MapEditor::EditorMode::DCEL)
    {
        auto map = maped->getEditMap();
        bool rv  = maped->useExistingDCEL(map);
        if (!rv)
        {
            rv = maped->createLocalDCEL(map);
            if (!rv && !map)
            {
                // a malformed map is reported by the DCEL itself
                QMessageBox box(this);
                box.setIcon(QMessageBox::Warning);
                box.setText("There is no map - cannot view DCEL");
                box.exec();
            }
        }
    }
    emit sig_updateView();
}

void page_map_editor::slot_mosaicLoaded(VersionedFile vfile)
{
    qDebug() << "page_map_editor: loaded -" << vfile.getVersionedName().get();

    slot_mosaicChanged();
}

void page_map_editor::slot_mosaicChanged()
{
    // unload what is there now, but reload maps to same state if posiible
    // if in copy mode, no need to change

    updateStyleSelections();    // alwyas needs to be done

    if (config->mapedLoadCopies)
    {
        return;     // if using pointers to source then do thing
    }

    QVector<MapEditorMap::MapType> usedTypes;
    auto layers = maped->gerUsedComposableLayers();
    for (const MapEditorLayer * layer : std::as_const(layers))
    {
        MapEditorMap::MapType mtype = layer->getLayerMapType();
        usedTypes.push_back(mtype);

        slot_unloadLayer((int)layer->getId());
    }

    for (auto & mapType : usedTypes)
    {
        slot_import((int)mapType);
    }
}

void page_map_editor::slot_tilingLoaded (VersionedFile vfile)
{
    slot_mosaicLoaded(vfile);
}

void page_map_editor::slot_convertToExplicit()
{
    MapEditorLayer & layer       = maped->getEditLayer();
    MapEditorMap::MapType mtype  = layer.getLayerMapType();
    if (mtype  != MapEditorMap::MapType::Motif  &&  mtype != MapEditorMap::MapType::MotifUnit )
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Ignoring - No motif to convert");
        box.exec();
        return;
    }

    MapedMapPtr memp   = layer.getWorking();
    TimoMap * mm       = static_cast<TimoMap*>(memp.get());
    TileMotifSet timos = mm->getTileMotifs();
    if (timos.size() > 1)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Ignoring. Only one motif should be selected");
        box.exec();
        return;
    }

    TileMotifPtr timo = timos.first();
    MotifPtr motif = timo->getMotif();
    if (motif)
    {
        auto ep = std::dynamic_pointer_cast<AdaptiveMotif>(motif);
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
            timo->setMotif(ep);
        }
    }
    else
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Ignoring. No motif found");
        box.exec();
        return;
    }
}

void page_map_editor::slot_verify()
{
    MapPtr map = maped->getEditMap();
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

void page_map_editor::slot_setMouseMode(int mode, bool checked)
{
    if (checked)
    {
        MapEditor::MouseMode mm = static_cast<MapEditor::MouseMode>(mode);
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

void page_map_editor::slot_showTileCentre(bool show)
{
    db->showTileCentre = show;
    emit sig_updateView();
}

void page_map_editor::slot_debugChk(bool on)
{
    config->mapedStatusBox = on;
    if (!on)
    {
        editorStatusBox->hide();
    }
    else
    {
        editorStatusBox->show();
    }
}

void page_map_editor::slot_clearConstructionLines()
{
    db->constructionLines.clear();
    db->constructionCircles.clear();
    db->selectedCLine = -1;
    maped->stash();
    emit sig_updateView();
}

void page_map_editor::slot_dumpMap()
{
    MapPtr m = maped->getEditMap();
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
    MapEditorLayer & layer = maped->getEditLayer();
    MapPtr map = layer.getMapedLayerMap();

    bool rv = true;
    if (!map)
    {
        map = make_shared<Map>("Crop Map");
        layer.setLayer(map,MapType::Crop);
        if (layer.getId() == LayerId::Composite )
        {
            rv = false;
        }
    }

    if (rv)
    {
        rv = maped->embedCrop(map);
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
    MapPtr map = maped->getEditMap();

    bool rv = maped->cropMap(map);
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
    MapPtr map = maped->getEditMap();
    if (!map) return;

    MapCleanser mc(map);
    mc.cleanse(config->mapedCleanseLevel,config->mapedMergeSensitivity);

    switch(maped->getMapType(map))
    {
    case MapType::Style:
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

    case MapType::Prototype:
    {
        auto protos = motifMaker->protoData.getPrototypes();
        for( auto & proto : protos)
        {
            proto->setCleanseLevel(config->mapedCleanseLevel);
            proto->setCleanseSensitivity(config->mapedMergeSensitivity);
        }
    } break;

    default:
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Not currently supported");
        box.setDetailedText("Can only apply Map Cleansing for Styles and Prototype");
        box.exec();
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
    MapPtr map = maped->getEditMap();  // could be null

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

    xfl = FileServices::getFiles(FILE_TEMPLATE);

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

    maped->loadTemplate(xfile);

    lastNamedTemplate = xfile;
}

void page_map_editor::slot_import(int mapTypeId)
{
    MapEditorMap::MapType id = static_cast<MapEditorMap::MapType>(mapTypeId);
    switch(id)
    {
    case MapType::Style:
    {
        int index = validStyles->currentIndex();
        QVariant v = validStyles->itemData(index);
        WeakStylePtr wsp =  v.value<WeakStylePtr>();
        StylePtr style = wsp.lock();
        if (style)
        {
            maped->loadFromMosaicStyle(style);
        }
    } break;

    case MapType::Prototype:
        maped->loadFromfPrototypeMaker();
        break;

    case MapType::Motif:
        maped->loadSelectedMotifs();
        break;

    case MapType::MotifUnit:
        maped->loadMotifUnit();
        break;

    case MapType::TilingUnit:
        maped->loadTilingUnit();
        break;

    case MapType::TilingFull:
        maped->loadTilingRepeated();
        break;

    case MapType::File:
        importMapFile();
        break;

    case MapType::Created:
    {
        MapPtr amap = make_shared<Map>(QString("Local"));
        maped->loadFromMap(amap,id);
    }   break;

    case MapType::None:
        maped->unload();
        emit sig_updateView();
        break;

    case MapType::FileMotif:
    case MapType::Composite:
    case MapType::CompositeMotif:
    case MapType::Crop:
        return;
    }

    panel->delegateView(Type::MapEditor,true);
}

void page_map_editor::importMapFile()
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

    MapType maptype = loader.getType();

    Xform xf = loader.getXform();

    qDebug().noquote() << loadedMap->summary();

    bool rv = maped->loadFromMap(loadedMap,maptype);
    if (!rv)
        return;     // failed

    Sys::mapEditorView->setModelXform(xf,false,Sys::nextSigid());

    tallySelects();

    panel->delegateView(Type::MapEditor,true);

    if (viewControl->isEnabled(Type::MapEditor))
    {
        viewControl ->setTakerViewSize(loader.getViewSize());
    }

    emit sig_updateView();

    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    box.setText("Map loaded OK");
    box.exec();
}

void page_map_editor::slot_pushMap()
{
    qDebug() << "page_map_editor::slot_pushMap()";

    bool brv = false;

    QString msg = "Undefined";

    MapEditorLayer & layer = maped->getEditLayer();
    auto map   = layer.getMapedLayerMap();
    if (!map || map->isEmpty())
    {
        brv = false;
    }
    else
    {
        using MapType = MapEditorMap::MapType;
        MapType ltype = layer.getLayerMapType();
        switch (ltype)
        {
        case MapType::Style:
        case MapType::Prototype:
            msg = "Mosaic";
            brv = maped->pushToMosaic(layer);
            break;

        case MapType::Motif:
        case MapType::MotifUnit:
        case MapType::FileMotif:
        case MapType::CompositeMotif:
            msg = "Motif";
            brv = maped->pushToMotif(layer);
            break;

        case MapType::TilingUnit:
            msg = "Tiling";
            brv = maped->pushToTiling(layer,false);
            break;

        case MapType::TilingFull:
        {
            QMessageBox box(this);
            box.setIcon(QMessageBox::Warning);
            box.setWindowTitle("Pushing a complete tiling");
            box.setText("This is NOT RECOMMENDED. Proceed at your peril.");
            box.setInformativeText("The Map Editor can be used to examine a complete tiling, "
                                   "but you should only push a tiling unit, not the complete tiling");
            box.setStandardButtons(QMessageBox::Apply | QMessageBox::Cancel);
            box.setDefaultButton(QMessageBox::Cancel);
            int rv2 = box.exec();
            if (rv2 == QMessageBox::Cancel)
            {
                return;
            }

            msg = "Tiling";
            brv = maped->pushToTiling(layer,false);
        }   break;

        case MapType::File:
        case MapType::Created:
        case MapType::Composite:
        {
            DlgPushSelect dlg(this);
            int rv2 = dlg.exec();
            if (rv2 == QDialog::Accepted)
            {
                switch (dlg.retval)
                {
                case 1:
                    msg = "Mosaic";
                    brv = maped->pushToMosaic(layer);
                    break;
                case 2:
                    msg = "Motif";
                    brv = maped->pushToMotif(layer);
                    break;
                case 3:
                    msg = "Tiling";
                    brv = maped->pushToTiling(layer,false);
                    break;
                }
            }
            else
            {
                return;
            }
        }   break;

        case MapType::Crop:
            msg = "Crop";
            break;

        case MapType::None:
            msg = "Unknown";
            break;
        }
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

void page_map_editor::slot_convertToFixedMap()
{
    qDebug() << "page_map_editor::slot_convertToFixedMap()";

    MapEditorLayer & layer = maped->getEditLayer();
    auto map = layer.getMapedLayerMap();

    bool brv = false;

    if (map && !map->isEmpty())
    {
        using MapType = MapEditorMap::MapType;
        MapType ltype = layer.getLayerMapType();
        if (ltype == MapType::Style || ltype == MapType::Prototype)
        {
            brv = maped->pushToMosaic(layer, true);
        }
    }

    QMessageBox box(this);
    if (brv)
    {
        box.setIcon(QMessageBox::Information);
        box.setText("Converted to fixed map - OK");
    }
    else
    {
        box.setIcon(QMessageBox::Warning);
        box.setText("Convert to fixed map - FAILED (only applies to a loaded Style or Prototype map)");
    }
    box.exec();
}

void page_map_editor::slot_saveMapToFile()
{
    MapPtr map = maped->getEditMap();
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
        DlgSaveVersion msgBox(panel);
        msgBox.setText1(QString("Map <%1> already exists.").arg(xfile.getVersionedName().get()));
        int rv = msgBox.exec();

        if (rv == QDialog::Rejected)
        {
            return;
        }

        if (rv != QDialog::Accepted)
        {
            // bumps version
            VersionedName vn = msgBox.bumpedName(FILE_MAP,xfile.getVersionedName());
            xfile.updateFromVersionedName(vn);
        }
    }

    MapEditorMapWriter writer;
    bool rv = writer.writeXML(xfile,maped);

    QStringList unverifiedMaps = writer.getUnverifiedMaps();
    if (rv && !unverifiedMaps.isEmpty())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("These maps failed verification:\n" + unverifiedMaps.join("\n"));
        box.setInformativeText("Cleanse them and save again?");
        box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        if (box.exec() == QMessageBox::Yes)
        {
            MapEditorMapWriter writer2;
            writer2.setCleanseMaps(true);
            rv = writer2.writeXML(xfile,maped);
        }
    }

    if (rv)
    {
        lastNamedMap = xfile.getVersionedName();

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
    LayerId eid = LayerId(id);
    maped->setViewSelect(eid,checked);
    if (checked)
        maped->setEditSelect(eid);
    emit sig_updateView();
}

void page_map_editor::slot_editLayer(int id, bool checked)
{
    LayerId eid = LayerId(id);
    if (checked)
    {
        maped->setEditSelect(eid);
        maped->setViewSelect(eid,true);
    }
    else
    {
        maped->setEditSelect(LayerId::None);
    }
    if (Sys::systemViewController->isEnabled(Type::MapEditor))
    {
        emit sig_updateView();
    }
}

void page_map_editor::slot_unloadLayer(int id)
{
    LayerId eid = LayerId(id);
    maped->getLayer(eid).reset();
    emit sig_updateView();
}

void page_map_editor::tallySelects()
{
    viewGroup->blockSignals(true);

    if (maped->isViewSelected(LayerId::Composite))
    {
        compositeVChk->setChecked(true);
        layer1VChk->setChecked(false);
        layer2VChk->setChecked(false);
        layer3VChk->setChecked(false);
    }
    else
    {
        compositeVChk->setChecked(false);
        bool chk = maped->isViewSelected(LayerId::L1);
        layer1VChk->setChecked(chk);
        chk = maped->isViewSelected(LayerId::L2);
        layer2VChk->setChecked(chk);
        chk = maped->isViewSelected(LayerId::L3);
        layer3VChk->setChecked(chk);
    }

    viewGroup->blockSignals(false);

    // this is needed
    editGroup->blockSignals(true);

    compositeEChk->setChecked(false);
    layer1EChk->setChecked(false);
    layer2EChk->setChecked(false);
    layer3EChk->setChecked(false);

    switch (maped->getEditSelect())
    {
    case LayerId::None:
        break;

    case LayerId::L1:
        layer1EChk->setChecked(true);
        break;

    case LayerId::L2:
        layer2EChk->setChecked(true);
        break;

    case LayerId::L3:
        layer3EChk->setChecked(true);
        break;

    case LayerId::Composite:
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
