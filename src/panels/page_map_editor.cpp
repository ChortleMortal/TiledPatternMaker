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

#include "panels/page_map_editor.h"
#include "makers/map_editor/map_editor.h"
#include "panels/layout_sliderset.h"
#include "panels/dlg_listselect.h"
#include "panels/dlg_crop.h"
#include "panels/dlg_name.h"
#include "base/border.h"
#include "base/fileservices.h"
#include "base/mosaic_loader.h"
#include "base/mosaic_writer.h"
#include "base/tiledpatternmaker.h"
#include "base/utilities.h"
#include "geometry/dcel.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "tapp/explicit_figure.h"
#include "tapp/radial_figure.h"
#include "viewers/viewcontrol.h"
#include "tapp/prototype.h"
#include "tapp/design_element.h"
#include "geometry/crop.h"
#include "tile/feature.h"
#include "tile/placed_feature.h"

using std::make_shared;

typedef std::shared_ptr<RadialFigure>    RadialPtr;


page_map_editor:: page_map_editor(ControlPanel *cpanel)  : panel_page(cpanel,"Map Editor")
{
    maped = MapEditor::getSharedInstance();

    QRadioButton * mapEdNone   = new QRadioButton("None");
    QRadioButton * mapEdMosaic = new QRadioButton("Mosaic");
    QRadioButton * mapEdProto  = new QRadioButton("Prototype");
    QRadioButton * mapEdFigure = new QRadioButton("Figure");
    QRadioButton * mapEdLocal  = new QRadioButton("Local");
    QRadioButton * mapEdTiling = new QRadioButton("Tiling");
    QRadioButton * mapEdDCEL   = new QRadioButton("DCEL");
    QRadioButton * mapEdFilledTiling = new QRadioButton("Filled Tiling");

    // map editor group
    mapEdModeGroup.addButton(mapEdNone,         MAPED_MODE_NONE);
    mapEdModeGroup.addButton(mapEdMosaic,       MAPED_MODE_MOSAIC);
    mapEdModeGroup.addButton(mapEdProto,        MAPED_MODE_PROTO);
    mapEdModeGroup.addButton(mapEdFigure,       MAPED_MODE_FIGURE);
    mapEdModeGroup.addButton(mapEdLocal,        MAPED_MODE_LOCAL);
    mapEdModeGroup.addButton(mapEdFilledTiling, MAPED_MODE_FILLED_TILING);
    mapEdModeGroup.addButton(mapEdTiling,       MAPED_MODE_TILING);
    mapEdModeGroup.addButton(mapEdDCEL,         MAPED_MODE_DCEL);
    mapEdModeGroup.button(config->mapEditorMode)->setChecked(true);

    editorStatusBox = new QGroupBox("Map Editor Status");
    editorStatusBox->setMinimumWidth(531);
    editorStatusBox->setCheckable(true);
    editorStatusBox->setChecked(config->mapedStatusBox);

    QVBoxLayout * qvbox = new QVBoxLayout();
    qvbox->addWidget(&statusBox);
    editorStatusBox->setLayout(qvbox);

    QToolButton * pbVerifyMap = new QToolButton();
    QToolButton * pbMakeExplicit = new QToolButton();
    QToolButton * pbClearMap= new QToolButton();
    QToolButton * pbDivideEdges = new QToolButton();
    QToolButton * pbJoinEdges = new QToolButton();
    QToolButton * pbCleanNeighbours = new QToolButton();
    QToolButton * pbRemoveFloatingVerts = new QToolButton();
    QToolButton * pbRemoveSingleVerts = new QToolButton();
    QToolButton * pbRemoveZombieEdges = new QToolButton();
    QToolButton * pbRebuildNeigbours = new QToolButton();
    QToolButton * pbSortVertices = new QToolButton();
    QToolButton * pbSortEdges = new QToolButton();
    QToolButton * pbDumpMap = new QToolButton();
    QToolButton * pbCreateMap = new QToolButton();
    QToolButton * pbLoadMap = new QToolButton();
    QToolButton * pbSaveMap = new QToolButton();
                  pbCropMap = new QToolButton();
                  pbMaskMap = new QToolButton();

    QToolButton * pbRestoreConstructs= new QToolButton();
    QToolButton * pbUndoConstructs= new QToolButton();
    QToolButton * pbRedoConstructs= new QToolButton();
    QToolButton * pbClearConstructs = new QToolButton();
    QToolButton * pbCleanseMap = new QToolButton();
    QToolButton * pbCreateDCEL = new QToolButton();
    QToolButton * pbSaveTemplate= new QToolButton();
    QToolButton * pbLoadTemplate= new QToolButton();

    pal = pbVerifyMap->palette();
    palPink = pal;
    palPink.setColor(QPalette::Button, QColor(0xffb6c1));
    palYellow = palPink;
    palYellow.setColor(QPalette::Button, QColor(Qt::yellow));
    pbClearMap->setPalette(palPink);

    pbCreateMap->setPalette(palPink);
    pbLoadMap->setPalette(palPink);
    pbSaveMap->setPalette(palPink);
    //pbCropMap->setPalette(palYellow);
    //pbMaskMap->setPalette(palYellow);

    pbMakeExplicit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbVerifyMap->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbDivideEdges->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbJoinEdges->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbCleanNeighbours->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbRebuildNeigbours->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbSortVertices->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbSortEdges->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbRemoveFloatingVerts->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbRemoveSingleVerts->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbRemoveZombieEdges->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    pbRestoreConstructs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbUndoConstructs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbRedoConstructs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbClearConstructs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbSaveTemplate->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbLoadTemplate->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    pbCleanseMap->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbCreateDCEL->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbClearMap->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbDumpMap->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbCropMap->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbMaskMap->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbCreateMap->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbLoadMap->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbSaveMap->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    pbMakeExplicit->setText("Make Explicit");
    pbVerifyMap->setText("Verify Map");
    pbDivideEdges->setText("Divide\nIntersecting Edges");
    pbJoinEdges->setText("Join\nColinear Edges");
    pbCleanNeighbours->setText("Clean Neighbours");
    pbRebuildNeigbours->setText("Rebuild Neighbours");
    pbSortVertices->setText("Sort Vertices");
    pbSortEdges->setText("Sort Edges");
    pbRemoveFloatingVerts->setText("Remove\nFloating Vertices");
    pbRemoveSingleVerts->setText("Remove\nSingle Vertices");
    pbRemoveZombieEdges->setText("Remove\nZombie Edges");
    pbRestoreConstructs->setText("Restore\nConstruct Lines");
    pbUndoConstructs->setText("Undo\nConstruct Lines");
    pbRedoConstructs->setText("Redo\nConstruct Lines");
    pbClearConstructs->setText("Clear\nConstruct Lines");
    pbSaveTemplate->setText("Save Template");
    pbLoadTemplate->setText("Load Template");
    pbClearMap->setText("Clear Map");
    pbCleanseMap->setText("Cleanse Map");
    pbCreateDCEL->setText("Create DCEL");
    pbDumpMap->setText("Dump Map");
    pbCropMap->setText("Apply Border");
    pbMaskMap->setText("Apply Mask");
    pbCreateMap->setText("Create Map");
    pbLoadMap->setText("Load Map");
    pbSaveMap->setText("Save Map");

    pbUndoConstructs->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    pbRedoConstructs->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    pbUndoConstructs->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowLeft));
    pbRedoConstructs->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowRight));

    //QToolButton * pbReplaceInStyle  = new QToolButton();
    QToolButton * pbRender          = new QToolButton();
    QToolButton * pbPushToTiling    = new QToolButton();

    //pbReplaceInStyle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbRender->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbPushToTiling->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    //pbReplaceInStyle->setText("Push Proto to Mosaic");
    pbRender->setText("Render");
    pbPushToTiling->setText("Push to Tiling");

    QRadioButton * modeNone         = new QRadioButton("No Mode (ESC)");
    QRadioButton * modeDrawLine     = new QRadioButton("Draw Lines (F3)");
    QRadioButton * modeConstruction = new QRadioButton("Construct Lines (F4)");
    QRadioButton * modeDelLine      = new QRadioButton("Delete Selected (F5)");
    QRadioButton * modeSplitLine    = new QRadioButton("Split Edges (F6)");
    QRadioButton * modeExtendLine   = new QRadioButton("Extend Line (F7)");
    QRadioButton * modeCrop         = new QRadioButton("Create Crop (F8)");
    QRadioButton * modeConstrCicle  = new QRadioButton("Construct Circles (F9)");
    QRadioButton * modeConstrBorder = new QRadioButton("Create Border (F10)");
    QRadioButton * modeEditBorder   = new QRadioButton("Edit Border");

    modeGroup.addButton(modeNone,           MAPED_MOUSE_NONE);
    modeGroup.addButton(modeDrawLine,       MAPED_MOUSE_DRAW_LINE);
    modeGroup.addButton(modeConstrCicle,    MAPED_MOUSE_CONSTRUCTION_CIRCLES);
    modeGroup.addButton(modeDelLine,        MAPED_MOUSE_DELETE);
    modeGroup.addButton(modeSplitLine,      MAPED_MOUSE_SPLIT_LINE);
    modeGroup.addButton(modeConstruction,   MAPED_MOUSE_CONSTRUCTION_LINES);
    modeGroup.addButton(modeExtendLine,     MAPED_MOUSE_EXTEND_LINE);
    modeGroup.addButton(modeCrop,           MAPED_MOUSE_CREATE_CROP);
    modeGroup.addButton(modeConstrBorder,   MAPED_MOUSE_CREATE_BORDER);
    modeGroup.addButton(modeEditBorder,     MAPED_MOUSE_EDIT_BORDER);

    modeGroup.button(maped->getMouseMode())->setChecked(true);

    QCheckBox * hideConsChk     = new QCheckBox("Hide construct lines");
    QCheckBox * hideMapChk      = new QCheckBox("Hide map");
    QCheckBox * hidePtsChk      = new QCheckBox("Hide points");
    QCheckBox * hideMidPtsChk   = new QCheckBox("Hide mid-points");
    animateChk                  = new QCheckBox("Animate Load");

    DoubleSpinSet * lineWidthSpin = new DoubleSpinSet("Line Width",3.0,1.0,10.0);
    DoubleSpinSet * consWidthSpin = new DoubleSpinSet("Cons Width",1.0,1.0,10.0);
    DoubleSpinSet * radiusSpin    = new DoubleSpinSet("Radius",0.25,0.0,2.0);
    radiusSpin->setPrecision(8);
    radiusSpin->setSingleStep(0.05);

    // View Box
    QVBoxLayout * vbox2 = new QVBoxLayout;
    vbox2->addWidget(mapEdNone);
    vbox2->addWidget(mapEdMosaic);
    vbox2->addWidget(mapEdProto);
    vbox2->addWidget(mapEdFigure);
    vbox2->addWidget(mapEdFilledTiling);
    vbox2->addWidget(mapEdTiling);
    vbox2->addWidget(mapEdLocal);
    vbox2->addWidget(mapEdDCEL);
    vbox2->addSpacing(9);
    vbox2->addWidget(hideConsChk);
    vbox2->addWidget(hideMapChk);
    vbox2->addWidget(hidePtsChk);
    vbox2->addWidget(hideMidPtsChk);
    vbox2->addWidget(animateChk);
    vbox2->addLayout(lineWidthSpin);
    vbox2->addLayout(consWidthSpin);
    vbox2->addStretch();

    QGroupBox * viewBox = new QGroupBox("View");
    viewBox->setLayout(vbox2);

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
    mapGrid->addWidget(pbSortVertices,row,0);
    mapGrid->addWidget(pbSortEdges,row,1);
    mapGrid->addWidget(pbRebuildNeigbours,row,2);

    row++;
    mapGrid->addWidget(pbCleanseMap,row,1);
    mapGrid->addWidget(pbCreateDCEL,row,2);

    row++;
    mapGrid->addWidget(pbClearMap,row,0);
    mapGrid->addWidget(pbCreateMap,row,1);
    mapGrid->addWidget(pbLoadMap,row,2);

    row++;
    mapGrid->addWidget(pbCropMap,row,0);
    mapGrid->addWidget(pbMaskMap,row,1);
    mapGrid->addWidget(pbSaveMap,row,2);

    QGroupBox * mapBox  = new QGroupBox("Map");
    mapBox->setLayout(mapGrid);

    // Template box
    QGridLayout * templateGrid = new QGridLayout;
    row = 0;
    templateGrid->addWidget(pbRestoreConstructs,row,0);
    templateGrid->addWidget(pbUndoConstructs,row,1);
    templateGrid->addWidget(pbRedoConstructs,row,2);

    row++;
    templateGrid->addWidget(pbSaveTemplate,row,0);
    templateGrid->addWidget(pbLoadTemplate,row,1);
    templateGrid->addWidget(pbClearConstructs,row,2);

    QGroupBox * templateBox = new QGroupBox("Construction Line Templates");
    templateBox->setLayout(templateGrid);

    // Edit Box
    QVBoxLayout * editBox = new QVBoxLayout;

    editBox->addWidget(modeNone);
    editBox->addWidget(modeDrawLine);
    editBox->addWidget(modeConstrCicle);
    editBox->addLayout(radiusSpin);
    editBox->addWidget(modeConstruction);
    editBox->addWidget(modeDelLine);

    editBox->addWidget(modeSplitLine);
    editBox->addWidget(modeExtendLine);
    editBox->addWidget(modeCrop);
    editBox->addWidget(modeConstrBorder);
    editBox->addWidget(modeEditBorder);

    editBox->addStretch();

    QGroupBox * editGBox = new QGroupBox("Edit");
    editGBox->setLayout(editBox);

    // Push Box
    QHBoxLayout * pushLayout = new QHBoxLayout;

    //pushLayout->addWidget(pbReplaceInStyle);
    pushLayout->addWidget(pbRender);
    pushLayout->addWidget(pbPushToTiling);

    QGroupBox * pushBox = new QGroupBox("Push");
    pushBox->setLayout(pushLayout);

    // arranging the boxes
    QGridLayout * boxLayout = new QGridLayout();

    boxLayout->addWidget(editGBox,   0,0,3,1);
    boxLayout->addWidget(mapBox,     0,1);
    boxLayout->addWidget(viewBox,    0,2,3,1);
    boxLayout->addWidget(templateBox,1,1);
    boxLayout->addWidget(pushBox,    2,1);

    // putting it together
    vbox->addWidget(editorStatusBox);
    vbox->addLayout(boxLayout);

    connect(vcontrol, &ViewControl::sig_selected_dele_changed, this, &page_map_editor::slot_selected_dele_changed);

    connect(theApp,  &TiledPatternMaker::sig_tilingLoaded,   this,   &page_map_editor::slot_tilingLoaded);
    connect(theApp,  &TiledPatternMaker::sig_mosaicLoaded,   this,   &page_map_editor::slot_mosaicLoaded);

    connect(pbVerifyMap,            &QToolButton::clicked,  this,   &page_map_editor::slot_verify);
    connect(pbMakeExplicit,         &QToolButton::clicked,  this,   &page_map_editor::slot_convertToExplicit);
    connect(pbClearMap,             &QToolButton::clicked,  this,   &page_map_editor::slot_clearMap);
    connect(pbDivideEdges,          &QToolButton::clicked,  this,   &page_map_editor::slot_divideIntersectingEdges);
    connect(pbJoinEdges,            &QToolButton::clicked,  this,   &page_map_editor::slot_joinColinearEdges);
    connect(pbCleanNeighbours,      &QToolButton::clicked,  this,   &page_map_editor::slot_cleanNeighbours);
    connect(pbRebuildNeigbours,     &QToolButton::clicked,  this,   &page_map_editor::slot_rebuildNeighbours);
    connect(pbSortVertices,         &QToolButton::clicked,  this,   &page_map_editor::slot_sortVertices);
    connect(pbSortEdges,            &QToolButton::clicked,  this,   &page_map_editor::slot_sortEdges);
    connect(pbCleanseMap,           &QToolButton::clicked,  this,   &page_map_editor::slot_cleanseMap);
    connect(pbCreateDCEL,           &QToolButton::clicked,  this,   &page_map_editor::slot_createDCEL);
    connect(pbRemoveFloatingVerts,  &QToolButton::clicked,  this,   &page_map_editor::slot_removeUnconnectedVertices);
    connect(pbRemoveSingleVerts,    &QToolButton::clicked,  this,   &page_map_editor::slot_removeSingleConnectVertices);
    connect(pbRemoveZombieEdges,    &QToolButton::clicked,  this,   &page_map_editor::slot_removeZombieEdges);
    connect(pbDumpMap,              &QToolButton::clicked,  this,   &page_map_editor::slot_dumpMap);
    connect(pbCropMap,              &QToolButton::clicked,  this,   &page_map_editor::slot_applyCrop);
    connect(pbMaskMap,              &QToolButton::clicked,  this,   &page_map_editor::slot_applyMask);
    connect(pbCreateMap,            &QToolButton::clicked,  this,   &page_map_editor::slot_createMap);
    connect(pbLoadMap,              &QToolButton::clicked,  this,   &page_map_editor::slot_loadMap);
    connect(pbSaveMap,              &QToolButton::clicked,  this,   &page_map_editor::slot_saveMap);
    connect(pbPushToTiling,         &QToolButton::clicked,  this,   &page_map_editor::slot_pushToTiling);

    connect(pbRestoreConstructs,    &QToolButton::clicked,  this,   &page_map_editor::slot_popstash);
    connect(pbUndoConstructs,       &QToolButton::clicked,  this,   &page_map_editor::slot_undoConstructionLines);
    connect(pbRedoConstructs,       &QToolButton::clicked,  this,   &page_map_editor::slot_redoConstructionLines);
    connect(pbClearConstructs,      &QToolButton::clicked,  this,   &page_map_editor::slot_clearConstructionLines);
    connect(pbSaveTemplate,         &QToolButton::clicked,  this,   &page_map_editor::slot_saveTemplate);
    connect(pbLoadTemplate,         &QToolButton::clicked,  this,   &page_map_editor::slot_loadTemplate);

    connect(hideConsChk,    &QCheckBox::clicked,  this,   &page_map_editor::slot_hideCons);
    connect(hideMapChk,     &QCheckBox::clicked,  this,   &page_map_editor::slot_hideMap);
    connect(hidePtsChk,     &QCheckBox::clicked,  this,   &page_map_editor::slot_hidePoints);
    connect(hideMidPtsChk,  &QCheckBox::clicked,  this,   &page_map_editor::slot_hideMidPoints);

    connect(&mapEdModeGroup,    &QButtonGroup::idToggled, this,   &page_map_editor::slot_mapEdMode_pressed);
    connect(&modeGroup,         &QButtonGroup::idToggled, this,   &page_map_editor::slot_setModes);
    connect(pbRender,           &QToolButton::clicked,    this,   &panel_page::sig_render);

    connect(radiusSpin,     &DoubleSpinSet::sig_valueChanged, this, &page_map_editor::slot_radiusChanged);
    connect(lineWidthSpin,  &DoubleSpinSet::sig_valueChanged, this, &page_map_editor::slot_lineWidthChanged);
    connect(consWidthSpin,  &DoubleSpinSet::sig_valueChanged, this, &page_map_editor::slot_consWidthChanged);

    slot_debugChk(config->mapedStatusBox);
    connect(editorStatusBox, &QGroupBox::toggled, this, &page_map_editor::slot_debugChk);

    hideMidPtsChk->setChecked(true);    // start hidden
}

void  page_map_editor::onEnter()
{
    mapEdModeGroup.blockSignals(true);
    mapEdModeGroup.button(config->mapEditorMode)->setChecked(true);
    mapEdModeGroup.blockSignals(false);

    maped->reload();

    modeGroup.button(maped->getMouseMode())->setChecked(true);

    maped->buildEditorDB();
    emit sig_refreshView();
}

void  page_map_editor::onExit()
{
    maped->mouse_interaction.reset();
}

void page_map_editor::refreshPage()
{
    QStringList txt;
    QString     str;

    eMapEditorMode mode = config->mapEditorMode;
    txt << QString("Editor mode: %1").arg(sMapEditorMode[mode]);
    switch (mode)
    {
    case MAPED_MODE_FIGURE:
    {
        DesignElementPtr delp = maped->getDesignElement();
        if (delp)
        {
            FigurePtr        figp = delp->getFigure();
            FeaturePtr       feap = delp->getFeature();

            txt << QString("DesignElement = %1").arg(Utils::addr(delp.get()));
            txt << QString("Feature = %1").arg(Utils::addr(feap.get()));
            txt << QString("Figure = %1 %2").arg(Utils::addr(figp.get())).arg(figp->getFigTypeString());
        }
        else
        {
            txt << "No DesignElement from WS";
        }
    }
        break;

    case MAPED_MODE_PROTO:
    {
        PrototypePtr pp = maped->getPrototype();
        InnerBorder * ibp;
        CropPtr cr;
        if (pp)
        {
            BorderPtr bp = pp->getBorder();
            ibp = dynamic_cast<InnerBorder*>(bp.get());
            if (ibp) cr = ibp->getInnerBoundary();
        }
        txt << QString("Protoype = %1 Crop = %2").arg(Utils::addr(pp.get())).arg(Utils::addr(cr.get()));
    }
        break;

    case MAPED_MODE_MOSAIC:
    {
        StylePtr sp = maped->getStyle();
        txt << QString("Style = %1").arg(Utils::addr(sp.get()));
    }
        break;

    case MAPED_MODE_TILING:
    case MAPED_MODE_FILLED_TILING:
    {
        TilingPtr tp =  maped->getTiling();
        txt << tp->dump();
    }
        break;

    case MAPED_MODE_DCEL:
    case MAPED_MODE_LOCAL:
    case MAPED_MODE_NONE:
        break;
    }

    MapPtr map = maped->getMap();
    if (map)
        txt << QString("Map: %1  [%2] %3").arg(Utils::addr(map.get())).arg(map->name()).arg(map->summary());
    else
        txt <<  "NO MAP";

    if (mode == MAPED_MODE_DCEL)
    {
        DCELPtr dp = maped->getDCEL();
        if (dp)
            txt << QString("DCEL: vertices = %1 half-edges = %2 faces = %3").arg(dp->getVertices().size()).arg(dp->getEdges().size()).arg(dp->faces.size());
        else
            txt <<  "DCEL: NONE";
    }

    QString mi;
    if (maped->mouse_interaction)
        mi = maped->mouse_interaction->desc;
    else
        mi = "no interaction";

    QPointF a = maped->mousePos;
    QPointF b = maped->screenToWorld(a);
    QString astring;
    QTextStream ts(&astring);
    ts << "   pos: (" << a.x() << ", " << a.y() << ") ("
                      << b.x() << ", " << b.y() << ")";
    mi += astring;

    txt << QString("%1 : %2").arg(sMapMouseMode[maped->getMouseMode()]).arg(mi);

    CropPtr crop = maped->getCrop();
    if (crop)
    {
        QRectF rect = crop->getRect();
        txt <<  QString("Crop  Rect: %1 %2 %3 %4 vert=%5 aspect=%6 state=%7").arg(rect.x()).arg(rect.y()).arg(rect.width()).arg(rect.height())
                   .arg(crop->getAspectVertical()).arg(crop->getAspect()).arg(crop->getStateStr());
    }

    SelectionSet set = maped->getCurrentSelections();
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
                QPointF pt   = sel->getPoint();
                CirclePtr c  = sel->getCircle();
                str += QString(" (%1,%2) center=(%4,%5) radius=%3\n").arg(pt.x()).arg(pt.y()).arg(c->radius).arg(c->centre.x()).arg(c->centre.y());
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

    statusBox.setText(txt.join("\n"));

    modeGroup.button(maped->getMouseMode())->setChecked(true);

    maped->updateStatus();
}

void page_map_editor::slot_mosaicLoaded(QString name)
{
    qDebug() << "page_map_editor: loaded -" << name;
    maped->initStashFrom(name);
    if (config->getViewerType() == VIEW_MAP_EDITOR)
    {
        maped->reload();
    }
}

void page_map_editor::slot_selected_dele_changed()
{
    if (config->mapEditorMode == MAPED_MODE_FIGURE)
    {
        maped->reload();
    }
}

void page_map_editor::slot_tilingLoaded (QString name)
{
    Q_UNUSED(name)
    if (config->getViewerType() == VIEW_MAP_EDITOR)
    {
        maped->reload();
    }
}

void page_map_editor::slot_reload()
{
    maped->reload();
}

void page_map_editor::slot_convertToExplicit()
{
    if (config->mapEditorMode == MAPED_MODE_FIGURE)
    {
        DesignElementPtr delp = maped->getDesignElement();
        FigurePtr        figp = delp->getFigure();

        ExplicitPtr        ep = std::dynamic_pointer_cast<ExplicitFigure>(figp);
        if (ep)
        {
            QMessageBox box(this);
            box.setIcon(QMessageBox::Warning);
            box.setText("Ignoring - Figure is already explicit");
            box.exec();
            return;
        }

        int sides = 10;
        RadialPtr rp = std::dynamic_pointer_cast<RadialFigure>(figp);
        if (rp)
        {
            sides = rp->getN();
        }
        MapPtr map = maped->getMap();
        ep = make_shared<ExplicitFigure>(map,FIG_TYPE_EXPLICIT,sides);
        delp->setFigure(ep);
    }
}

void page_map_editor::slot_verify()
{
    MapPtr map = maped->getMap();
    if (!map) return;

    bool oldConf = config->verifyMaps;
    config->verifyMaps = true;

    bool verified = map->verify(true);

    if (verified)
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
    MapPtr map = maped->getMap();
    if (!map) return;
    map->cleanse(divideupIntersectingEdges);
    updateView();
}

void page_map_editor::slot_joinColinearEdges()
{
    MapPtr map = maped->getMap();
    if (!map) return;
    map->cleanse(joinupColinearEdges);
    updateView();
}

void page_map_editor::slot_cleanNeighbours()
{
    MapPtr map = maped->getMap();
    if (!map) return;
    map->cleanse(cleanupNeighbours);
    updateView();
}

void page_map_editor::slot_removeUnconnectedVertices()
{
    MapPtr map = maped->getMap();
    if (!map) return;
    map->cleanse(badVertices_0);
    updateView();
}

void page_map_editor::slot_removeSingleConnectVertices()
{
    MapPtr map = maped->getMap();
    if (!map) return;
    map->cleanse(badVertices_1);
    updateView();
}


void page_map_editor::slot_removeZombieEdges()
{
    MapPtr map = maped->getMap();
    if (!map) return;
    map->cleanse(badEdges);
    updateView();
}

void page_map_editor::slot_rebuildNeighbours()
{
    MapPtr map = maped->getMap();
    if (!map) return;
    map->buildNeighbours();
    updateView();
}

void page_map_editor::slot_sortVertices()
{
    MapPtr map = maped->getMap();
    if (!map) return;
    map->sortVertices();
    updateView();
}

void page_map_editor::slot_sortEdges()
{
    MapPtr map = maped->getMap();
    if (!map) return;
    map->sortEdges();
    updateView();
}

void page_map_editor::slot_setModes(int mode, bool checked)
{
    if (checked)
    {
        eMapMouseMode mm = static_cast<eMapMouseMode>(mode);

        switch (mm)
        {
        case MAPED_MOUSE_CREATE_CROP:
            pbCropMap->setPalette(pal);
            pbMaskMap->setPalette(palYellow);
            break;

        case MAPED_MOUSE_CREATE_BORDER:
        case MAPED_MOUSE_EDIT_BORDER:
            pbCropMap->setPalette(palYellow);
            pbMaskMap->setPalette(palYellow);
            break;

        default:
            pbCropMap->setPalette(pal);
            pbMaskMap->setPalette(pal);
            break;
        }

        maped->setMapedMouseMode(mm);
    }
}

void page_map_editor::slot_mapEdMode_pressed(int id, bool checked)
{
    if (checked)
    {
        config->mapEditorMode = eMapEditorMode(id);
        maped->reload();
    }
}

void page_map_editor::slot_hideCons(bool hide)
{
    maped->hideConstructionLines = hide;
    updateView();
}

void page_map_editor::slot_hideMap(bool hide)
{
    maped->hideMap = hide;
    updateView();
}

void page_map_editor::slot_hidePoints(bool hide)
{
    maped->hidePoints = hide;
    updateView();
}

void page_map_editor::slot_hideMidPoints(bool hide)
{
    maped->hideMidPoints = hide;
    updateView();
}

void page_map_editor::slot_debugChk(bool on)
{
    config->mapedStatusBox = on;
    if (!on)
    {
        statusBox.hide();
    }
    else
    {
        statusBox.show();
    }
}

void page_map_editor::slot_popstash()
{
    maped->loadCurrentStash();
    updateView();
}

void page_map_editor::slot_undoConstructionLines()
{
    maped->stash.getPrev();
    maped->loadCurrentStash();
    updateView();
}

void page_map_editor::slot_redoConstructionLines()
{
    maped->stash.getNext();
    maped->loadCurrentStash();
    updateView();
}

void page_map_editor::slot_clearConstructionLines()
{
    // this does not affect the stash
    maped->constructionLines.clear();
    maped->constructionCircles.clear();
    updateView();
}

void page_map_editor::slot_clearMap()
{
    maped->clearAllMaps();
}

void page_map_editor::slot_createMap()
{
    maped->setLocalMap(make_shared<Map>(QString("New local map")));
    if (config->mapEditorMode == MAPED_MODE_LOCAL)
    {
        maped->reload();
    }
    else
    {
        mapEdModeGroup.button(MAPED_MODE_LOCAL)->setChecked(true);
    }
}

void page_map_editor::slot_loadMap()
{
    QString dir = config->rootMediaDir + "maps/";
    QString filename = QFileDialog::getOpenFileName(nullptr,"Select Map File",dir, "Map Files (*.xml)");
    if (filename.isEmpty()) return;

    MosaicLoader loader;
    MapPtr map  = loader.loadMosaicMap(filename);
    if (!map)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Map NOT loaded");
        box.exec();
        return;
    }

    mapEdModeGroup.button(MAPED_MODE_LOCAL)->setChecked(true);
    Q_ASSERT(config->mapEditorMode == MAPED_MODE_LOCAL);

    maped->setLocalMap(map);
    maped->reload();

    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    box.setText("Local map loaded OK");
    box.exec();
}

void page_map_editor::slot_saveMap()
{
    MapPtr map = maped->getMap();
    if (!map) return;

    QString dir = config->rootMediaDir + "maps/";
    QString filename = QFileDialog::getSaveFileName(this, "Save Map File",dir,"Map (*.xml)");
    if (filename.isEmpty()) return;

    MosaicWriter writer;
    bool rv = writer.writeXML(filename,map);
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

void page_map_editor::slot_pushToTiling()
{
    MapPtr map           = maped->getMap();
    if (!map) return;
    EdgePoly ep          = map->getEdgePoly();
    FeaturePtr fp        = make_shared<Feature>(ep);
    PlacedFeaturePtr pfp = make_shared<PlacedFeature>(fp,QTransform());
    tilingMaker->addNewPlacedFeature(pfp);

    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    box.setText("Pushed OK");
    box.exec();
}

void page_map_editor::slot_dumpMap()
{
    MapPtr m = maped->getMap();
    if (m)
    {
        m->dumpMap(true);
    }
    else
    {
        qDebug() << " Map not found!";
    }
}

void page_map_editor::slot_applyCrop()
{
#if 1
    maped->applyCrop();
#else
    DlgCrop dlg(maped);
    int rv = dlg.exec();
    if (rv == QDialog::Accepted)
    {
        maped->applyCrop();
    }
#endif
}

void page_map_editor::slot_applyMask()
{
    maped->applyMask();
}

void page_map_editor::slot_cleanseMap()
{
    MapPtr map = maped->getMap();
    if (!map) return;
    map->cleanse(default_cleanse);
    maped->buildEditorDB();
    updateView();
}

void page_map_editor::slot_createDCEL()
{
    MapPtr map = maped->getMap();
    if (!map)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Select a map before creating DCEL");
        box.exec();
        return;
    }

    DCELPtr dcel = map->getDCEL();
    maped->buildEditorDB();

    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    box.setText("DCEL created");
    box.exec();
}

void page_map_editor::slot_radiusChanged(qreal r)
{
    maped->newCircleRadius = r;
}

void page_map_editor::slot_lineWidthChanged(qreal r)
{
    maped->mapLineWidth = r;
    updateView();
}

void page_map_editor::slot_consWidthChanged(qreal r)
{
    maped->constructionLineWidth = r;
    updateView();
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
        maped->stash.saveTemplate(name);
    }
    else
        qInfo() << "No template selected";
}

void page_map_editor::slot_loadTemplate()
{
    QStringList ts = FileServices::getTemplates();

    DlgListSelect dlg(ts);
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
