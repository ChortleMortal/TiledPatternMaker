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
#include "panels/layout_sliderset.h"
#include "panels/dlg_listselect.h"
#include "panels/dlg_crop.h"
#include "panels/dlg_name.h"
#include "panels/panel.h"
#include "base/fileservices.h"
#include "base/mosaic_loader.h"
#include "base/mosaic_writer.h"
#include "base/tiledpatternmaker.h"
#include "base/utilities.h"
#include "geometry/dcel.h"
#include "geometry/map_cleanser.h"
#include "makers/decoration_maker/decoration_maker.h"
#include "makers/motif_maker/motif_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "style/style.h"
#include "tapp/explicit_figure.h"
#include "tapp/radial_figure.h"
#include "viewers/viewcontrol.h"

#define E2STR(x) #x

static QString sMapEdMode[] =
{
    E2STR(ME_MODE_UNDEFINED),
    E2STR(MAP_TYPE_FIGURE),
    E2STR(MAP_TYPE_PROTO),
    E2STR(MAP_TYPE_STYLE)
};

page_map_editor:: page_map_editor(ControlPanel *cpanel)  : panel_page(cpanel,"Map Editor")
{
    me = MapEditor::getInstance();

    QRadioButton * mapEdMosaic = new QRadioButton("Mosaic");
    QRadioButton * mapEdProto  = new QRadioButton("Prototype");
    QRadioButton * mapEdFigure = new QRadioButton("Figure");
    QRadioButton * mapEdLocal  = new QRadioButton("Local");
    QRadioButton * mapEdTiling = new QRadioButton("Tiling");
    QRadioButton * mapEdDCEL   = new QRadioButton("DCEL");

    // map editor group
    mapEdModeGroup.addButton(mapEdMosaic,MAP_MODE_MOSAIC);
    mapEdModeGroup.addButton(mapEdProto,MAP_MODE_PROTO);
    mapEdModeGroup.addButton(mapEdFigure,MAP_MODE_FIGURE);
    mapEdModeGroup.addButton(mapEdLocal,MAP_MODE_LOCAL);
    mapEdModeGroup.addButton(mapEdTiling,MAP_MODE_TILING);
    mapEdModeGroup.addButton(mapEdDCEL,MAP_MODE_DCEL);
    mapEdModeGroup.button(config->mapEditorMode)->setChecked(true);

    line0 = new QLabel;
    line1 = new QLabel;
    line2 = new QLabel;
    line3 = new QLabel;
    line3->setMinimumWidth(591);
    line4 = new QLabel;
    line5 = new QLabel;
    line6 = new QTextEdit();
    line6->setReadOnly(true);

    editorStatusBox = new QGroupBox("Map Editor Status");
    editorStatusBox->setMinimumWidth(531);
    editorStatusBox->setCheckable(true);
    editorStatusBox->setChecked(config->mapedStatusBox);

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
    QToolButton * pbCropMap = new QToolButton();
    QToolButton * pbLoadMap = new QToolButton();
    QToolButton * pbSaveMap = new QToolButton();


    QToolButton * pbRestoreConstructs= new QToolButton();
    QToolButton * pbUndoConstructs= new QToolButton();
    QToolButton * pbRedoConstructs= new QToolButton();
    QToolButton * pbClearConstructs = new QToolButton();
    QToolButton * pbCleanseMap = new QToolButton();
    QToolButton * pbSaveTemplate= new QToolButton();
    QToolButton * pbLoadTemplate= new QToolButton();

    QPalette pal = pbVerifyMap->palette();
    pal.setColor(QPalette::Button, QColor(Qt::yellow));
    pbClearMap->setPalette(pal);
    pbCreateMap->setPalette(pal);
    pbCropMap->setPalette(pal);
    pbLoadMap->setPalette(pal);
    pbSaveMap->setPalette(pal);

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
    pbCleanseMap->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbSaveTemplate->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbLoadTemplate->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbClearMap->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbDumpMap->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbCropMap->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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
    pbDumpMap->setText("Dump Map");
    pbCropMap->setText("Apply Crop");
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
    modeGroup.addButton(modeNone,           MAP_MODE_NONE);
    modeGroup.addButton(modeDrawLine,       MAP_MODE_DRAW_LINE);
    modeGroup.addButton(modeDelLine,        MAP_MODE_DELETE);
    modeGroup.addButton(modeSplitLine,      MAP_MODE_SPLIT_LINE);
    modeGroup.addButton(modeConstruction,   MAP_MODE_CONSTRUCTION_LINES);
    modeGroup.addButton(modeExtendLine,     MAP_MODE_EXTEND_LINE);
    modeGroup.addButton(modeCrop,           MAP_MODE_CREATE_CROP);
    modeGroup.addButton(modeConstrCicle,    MAP_MODE_CONSTRUCTION_CIRCLES);

    modeGroup.button(me->getMouseMode())->setChecked(true);

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
    vbox2->addWidget(mapEdMosaic);
    vbox2->addWidget(mapEdProto);
    vbox2->addWidget(mapEdFigure);
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
    mapGrid->addWidget(pbClearMap,row,2);

    row++;
    mapGrid->addWidget(pbRemoveZombieEdges,row,0);
    mapGrid->addWidget(pbRemoveFloatingVerts,row,1);
    mapGrid->addWidget(pbRemoveSingleVerts,row,2);

    row++;
    mapGrid->addWidget(pbDivideEdges,row,0);
    mapGrid->addWidget(pbJoinEdges,row,1);
    mapGrid->addWidget(pbCleanNeighbours,row,2);

    row++;
    mapGrid->addWidget(pbRebuildNeigbours,row,0);
    mapGrid->addWidget(pbSortVertices,row,1);
    mapGrid->addWidget(pbSortEdges,row,2);

    row++;
    mapGrid->addWidget(pbCleanseMap,row,0);
    mapGrid->addWidget(pbDumpMap,row,1);
    mapGrid->addWidget(pbCropMap,row,2);

    row++;
    mapGrid->addWidget(pbCreateMap,row,0);
    mapGrid->addWidget(pbLoadMap,row,1);
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
    editBox->addWidget(modeConstruction);
    editBox->addWidget(modeDelLine);

    editBox->addWidget(modeSplitLine);
    editBox->addWidget(modeExtendLine);
    editBox->addWidget(modeCrop);
    editBox->addWidget(modeConstrCicle);
    editBox->addLayout(radiusSpin);

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

    boxLayout->addWidget(editGBox,0,0,2,1);
    boxLayout->addWidget(mapBox,0,1);
    boxLayout->addWidget(templateBox,1,1);

    boxLayout->addWidget(viewBox,0,2,2,1);

    boxLayout->addWidget(pushBox,2,0,1,3);

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
    connect(pbRemoveFloatingVerts,  &QToolButton::clicked,  this,   &page_map_editor::slot_removeUnconnectedVertices);
    connect(pbRemoveSingleVerts,    &QToolButton::clicked,  this,   &page_map_editor::slot_removeSingleConnectVertices);
    connect(pbRemoveZombieEdges,    &QToolButton::clicked,  this,   &page_map_editor::slot_removeZombieEdges);
    connect(pbDumpMap,              &QToolButton::clicked,  this,   &page_map_editor::slot_dumpMap);
    connect(pbCropMap,              &QToolButton::clicked,  this,   &page_map_editor::slot_applyCrop);
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

    connect(&mapEdModeGroup,    &QButtonGroup::idClicked, this,   &page_map_editor::slot_mapEdMode_pressed);
    connect(&modeGroup,         &QButtonGroup::idClicked, this,   &page_map_editor::slot_setModes);
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

    me->reload();

    modeGroup.button(me->getMouseMode())->setChecked(true);

    me->buildEditorDB();
    emit sig_refreshView();
}

void page_map_editor::refreshPage()
{
    QString str;

    eMapType mapType = me->getMapType();
    line0->setText(sMapEdInput[mapType]);

    if (mapType == MAP_TYPE_FIGURE)
    {
        DesignElementPtr delp = me->getDesignElement();
        if (!delp)
        {
            line1->setText("No DesignElement from WS");
            line2->setText("");
            line3->setText("");
        }

        if (delp)
        {
            FigurePtr        figp = delp->getFigure();
            FeaturePtr       feap = delp->getFeature();

            str = QString("DesignElement = %1").arg(Utils::addr(delp.get()));
            line1->setText(str);
            str = QString("Feature = %1").arg(Utils::addr(feap.get()));
            line2->setText(str);
            str = QString("Figure = %1 %2").arg(Utils::addr(figp.get())).arg(figp->getFigTypeString());
            line3->setText(str);
        }
        else
        {
            line1->setText("No DesignElement from WS");
            line2->setText("");
            line3->setText("");
        }
    }
    else if (mapType == MAP_TYPE_PROTO)
    {
        PrototypePtr pp = me->getPrototype();
        str = QString("Protoype = %1").arg(Utils::addr(pp.get()));
        line1->setText(str);
        line2->setText("");
        line3->setText("");
    }
    else if (mapType == MAP_TYPE_STYLE)
    {
        StylePtr sp = me->getStyle();
        str = QString("Style = %1").arg(Utils::addr(sp.get()));
        line1->setText(str);
        line2->setText("");
        line3->setText("");
    }
    else if (mapType == MAP_TYPE_UNDEFINED)
    {
        line1->setText("");
        line2->setText("");
        line3->setText("");
    }

    // line 4
    if (mapType == MAP_TYPE_DCEL)
    {
        DCELPtr dp = me->getDCEL().lock();
        if (dp)
            str = QString("DCEL: vertices = %1 half-edges = %2 faces = %3").arg(dp->vertices.size()).arg(dp->edges.size()).arg(dp->faces.size());
        else
            str = "DECEL: NONE";
    }
    else
    {
        MapPtr map = me->getMap();
        if (map)
            str = QString("Map: %1  [%2] %3").arg(Utils::addr(map.get())).arg(map->name()).arg(map->summary());
        else
            str = "Map: NO MAP";
    }
    line4->setText(str);

    // line 5
    QString mi;
    if (me->mouse_interaction)
        mi = me->mouse_interaction->desc;
    else
        mi = "no interaction";

    QPointF a = me->mousePos;
    QPointF b = me->screenToWorld(a);
    QString astring;
    QTextStream ts(&astring);
    ts << "   pos: (" << a.x() << ", " << a.y() << ") ("
                      << b.x() << ", " << b.y() << ")";
    mi += astring;

    str = QString("%1 : %2").arg(sMapMouseMode[me->getMouseMode()]).arg(mi);
    line5->setText(str);

    // line 6
    SelectionSet set = me->getCurrentSelections();
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
                str += QString(" (%1,%2) centre=(%4,%5) radius=%3\n").arg(pt.x()).arg(pt.y()).arg(c->radius).arg(c->centre.x()).arg(c->centre.y());
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
    line6->setText(str);


    modeGroup.button(me->getMouseMode())->setChecked(true);

    me->updateStatus();
}

void page_map_editor::slot_mosaicLoaded(QString name)
{
    qDebug() << "page_map_editor: loaded -" << name;
    me->initStashFrom(name);
    if (config->getViewerType() == VIEW_MAP_EDITOR)
    {
        me->reload();
    }
}

void page_map_editor::slot_selected_dele_changed()
{
    if (config->mapEditorMode == MAP_MODE_FIGURE)
    {
        DesignElementPtr dep = motifMaker->getSelectedDesignElement();
        me->setDesignElement(dep);
    }
}

void page_map_editor::slot_tilingLoaded (QString name)
{
    Q_UNUSED(name)
    if (config->getViewerType() == VIEW_MAP_EDITOR)
    {
        me->reload();
    }
}

void page_map_editor::slot_reload()
{
    me->reload();
}

void page_map_editor::slot_convertToExplicit()
{
    if (me->getMapType() == MAP_TYPE_FIGURE)
    {
        DesignElementPtr delp = me->getDesignElement();
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
        MapPtr map = me->getMap();
        ep = make_shared<ExplicitFigure>(map,FIG_TYPE_EXPLICIT,sides);
        delp->setFigure(ep);
    }
}

void page_map_editor::slot_verify()
{
    bool oldConf = config->verifyMaps;
    config->verifyMaps = true;

    MapPtr map = me->getMap();

    bool verified = map->verifyMap("page_figure_editor::verify");

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
    MapPtr map = me->getMap();
    MapCleanser cleanser(map);
    cleanser.divideIntersectingEdges();
    me->buildEditorDB();
    updateView();
}

void page_map_editor::slot_joinColinearEdges()
{
    MapPtr map = me->getMap();
    MapCleanser cleanser(map);
    cleanser.joinColinearEdges();
    me->buildEditorDB();
    updateView();
}

void page_map_editor::slot_cleanNeighbours()
{
    MapPtr map = me->getMap();
    MapCleanser cleanser(map);
    cleanser.cleanNeighbours();
    me->buildEditorDB();
    updateView();
}

void page_map_editor::slot_removeUnconnectedVertices()
{
    MapPtr map = me->getMap();
    MapCleanser cleanser(map);
    cleanser.removeVerticesWithEdgeCount(0);
    me->buildEditorDB();
    updateView();
}

void page_map_editor::slot_removeSingleConnectVertices()
{
    MapPtr map = me->getMap();
    MapCleanser cleanser(map);
    cleanser.removeVerticesWithEdgeCount(1);
    me->buildEditorDB();
    updateView();
}


void page_map_editor::slot_removeZombieEdges()
{
    MapPtr map = me->getMap();
    MapCleanser cleanser(map);
    cleanser.removeBadEdges();
    me->buildEditorDB();
    updateView();
}

void page_map_editor::slot_rebuildNeighbours()
{
    MapPtr map = me->getMap();
    map->buildNeighbours();
    me->buildEditorDB();
    updateView();
}

void page_map_editor::slot_sortVertices()
{
    MapPtr map = me->getMap();
    map->sortVertices();
    me->buildEditorDB();
    updateView();
}

void page_map_editor::slot_sortEdges()
{
    MapPtr map = me->getMap();
    map->sortEdges();
    me->buildEditorDB();
    updateView();
}

void page_map_editor::slot_setModes(int mode)
{
    eMapMouseMode mm = static_cast<eMapMouseMode>(mode);
    me->setMouseMode(mm);
}

void page_map_editor::slot_mapEdMode_pressed(int id)
{
    config->mapEditorMode = eMapEditorMode(id);
    me->reload();
}

void page_map_editor::slot_hideCons(bool hide)
{
    me->hideConstructionLines = hide;
    me->buildEditorDB();
    updateView();
}

void page_map_editor::slot_hideMap(bool hide)
{
    me->hideMap = hide;
    me->buildEditorDB();
    updateView();
}

void page_map_editor::slot_hidePoints(bool hide)
{
    me->hidePoints = hide;
    me->buildEditorDB();
    updateView();
}

void page_map_editor::slot_hideMidPoints(bool hide)
{
    me->hideMidPoints = hide;
    me->buildEditorDB();
    updateView();
}

void page_map_editor::slot_debugChk(bool on)
{
    config->mapedStatusBox = on;
    QLayout * l = editorStatusBox->layout();
    if (l)
    {
        QLayoutItem * item;
        while ( (item = l->itemAt(0)) != nullptr)
        {
            QWidget * w = item->widget();
            if (w)
            {
                w->setParent(nullptr);
            }
        }
        delete l;
    }
    if (!on)
    {
        dummyStatusBox = new QVBoxLayout;
        editorStatusBox->setLayout(dummyStatusBox);
    }
    else
    {
        statusBox = new QVBoxLayout();
        statusBox->addWidget(line0);
        statusBox->addWidget(line1);
        statusBox->addWidget(line2);
        statusBox->addWidget(line3);
        statusBox->addWidget(line4);
        statusBox->addWidget(line5);
        statusBox->addWidget(line6);
        editorStatusBox->setLayout(statusBox);
    }
}
void page_map_editor::slot_popstash()
{
    me->loadCurrentStash();
    updateView();
}

void page_map_editor::slot_undoConstructionLines()
{
    me->stash.getPrev();
    me->loadCurrentStash();
    updateView();
}

void page_map_editor::slot_redoConstructionLines()
{
    me->stash.getNext();
    me->loadCurrentStash();
    updateView();
}

void page_map_editor::slot_clearConstructionLines()
{
    // this does not affect the stash
    me->constructionLines.clear();
    me->constructionCircles.clear();
    me->buildEditorDB();
    updateView();
}

void page_map_editor::slot_clearMap()
{
    MapPtr map = me->getMap();
    if (map)
    {
        map->wipeout();
    }
    me->buildEditorDB();
    updateView();
}

void page_map_editor::slot_createMap()
{
    me->setLocalMap(make_shared<Map>(QString("New local map")));
    if (config->mapEditorMode == MAP_MODE_LOCAL)
    {
        me->setLocal();
    }
    else
    {
        mapEdModeGroup.button(MAP_MODE_LOCAL)->setChecked(true);
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

    if (config->mapEditorMode == MAP_MODE_LOCAL)
    {
        me->setLocalMap(map);
        me->setLocal();
        QMessageBox box(this);

        box.setIcon(QMessageBox::Information);
        box.setText("Local map loaded OK");
        box.exec();
    }
    else
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("View not implemented - please select Local View");
        box.exec();
    }
}

void page_map_editor::slot_saveMap()
{
    QString dir = config->rootMediaDir + "maps/";
    QString filename = QFileDialog::getSaveFileName(this, "Save Map File",dir,"Map (*.xml)");
    if (filename.isEmpty()) return;

    MosaicWriter writer;
    bool rv = writer.writeXML(filename,me->getMap());
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
    MapPtr map           = me->getMap();
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
    MapPtr m = me->getMap();
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
    DlgCrop dlg(me);
    int rv = dlg.exec();
    if (rv == QDialog::Accepted)
    {
        me->applyCrop();
    }
}

void page_map_editor::slot_cleanseMap()
{
    MapPtr m = me->getMap();
    MapCleanser cleanser(m);
    cleanser.cleanse(default_cleanse);
    me->buildEditorDB();
    updateView();
}

void page_map_editor::slot_radiusChanged(qreal r)
{
    me->newCircleRadius = r;
}

void page_map_editor::slot_lineWidthChanged(qreal r)
{
    me->mapLineWidth = r;
    updateView();
}

void page_map_editor::slot_consWidthChanged(qreal r)
{
    me->constructionLineWidth = r;
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
        me->stash.saveTemplate(name);
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
    me->loadNamedStash(name, animateChk->isChecked());

    QFileInfo fi(name);
    lastNamedTemplate = fi.baseName();
}
