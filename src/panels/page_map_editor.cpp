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
#include "base/canvas.h"
#include "base/fileservices.h"
#include "base/tiledpatternmaker.h"
#include "base/utilities.h"
#include "style/Style.h"
#include "tapp/ExplicitFigure.h"
#include "tapp/RadialFigure.h"
#include "base/xmlwriter.h"
#include "base/xmlloader.h"


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

    localMap  =  make_shared<Map>(QString("New local map"));
    debugInfo = false;

    QRadioButton * mapEdStyle  = new QRadioButton("Style");
    QRadioButton * mapEdProto  = new QRadioButton("Proto");
    QRadioButton * mapEdFigure = new QRadioButton("Figure");
    QRadioButton * mapEdLocal  = new QRadioButton("Local");

    // map editor group
    mapEdModeGroup.addButton(mapEdStyle,MAP_MODE_STYLE);
    mapEdModeGroup.addButton(mapEdProto,MAP_MODE_PROTO);
    mapEdModeGroup.addButton(mapEdFigure,MAP_MODE_FIGURE);
    mapEdModeGroup.addButton(mapEdLocal,MAP_MODE_LOCAL);
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
    QToolButton * pbRemoveZombieEdges = new QToolButton();
    QToolButton * pbSortNeigbours = new QToolButton();
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
    pbSortNeigbours->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbSortVertices->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbSortEdges->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbRemoveFloatingVerts->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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
    pbSortNeigbours->setText("Sort Neighbours");
    pbSortVertices->setText("Sort Vertices");
    pbSortEdges->setText("Sort Edges");
    pbRemoveFloatingVerts->setText("Remove\nFloating Vertices");
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

    QToolButton * pbReplaceInStyle  = new QToolButton();
    QToolButton * pbAddToStyle      = new QToolButton();
    QToolButton * pbRender          = new QToolButton();

    pbReplaceInStyle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbAddToStyle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pbRender->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    pbReplaceInStyle->setText("Replace in Proto");
    pbAddToStyle->setText("Add to Proto");
    pbRender->setText("Render");

    QRadioButton * targetStyle      = new QRadioButton("To Loaded Styles");
    QRadioButton * targetWS         = new QRadioButton("To WS Styles");
    targetGroup.addButton(targetStyle,TARGET_LOADED_STYLES);
    targetGroup.addButton(targetWS,   TARGET_WS_STYLES);

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
    DoubleSpinSet * radiusSpin    =new DoubleSpinSet("Radius",0.25,0.0,2.0);
    radiusSpin->setPrecision(8);
    radiusSpin->setSingleStep(0.05);

    // View Box
    QVBoxLayout * vbox2 = new QVBoxLayout;
    vbox2->addWidget(mapEdStyle);
    vbox2->addWidget(mapEdProto);
    vbox2->addWidget(mapEdFigure);
    vbox2->addWidget(mapEdLocal);
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
    mapGrid->addWidget(pbDivideEdges,row,0);
    mapGrid->addWidget(pbJoinEdges,row,1);
    mapGrid->addWidget(pbCleanNeighbours,row,2);

    row++;
    mapGrid->addWidget(pbSortNeigbours,row,0);
    mapGrid->addWidget(pbSortVertices,row,1);
    mapGrid->addWidget(pbSortEdges,row,2);


    row++;
    mapGrid->addWidget(pbCleanseMap,row,0);
    mapGrid->addWidget(pbRemoveFloatingVerts,row,1);
    mapGrid->addWidget(pbRemoveZombieEdges,row,2);

    row++;
    mapGrid->addWidget(pbDumpMap,row,0);
    mapGrid->addWidget(pbCreateMap,row,1);
    mapGrid->addWidget(pbCropMap,row,2);

    row++;
    mapGrid->addWidget(pbLoadMap,row,0);
    mapGrid->addWidget(pbSaveMap,row,1);

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

    pushLayout->addWidget(targetStyle);
    pushLayout->addWidget(targetWS);
    pushLayout->addWidget(pbReplaceInStyle);
    pushLayout->addWidget(pbAddToStyle);
    pushLayout->addWidget(pbRender);

    QGroupBox * pushBox = new QGroupBox("Push");
    pushBox->setLayout(pushLayout);

    // arranging the boxes
    QGridLayout * boxLayout = new QGridLayout();

    boxLayout->addWidget(editGBox,0,0,2,1);
    boxLayout->addWidget(mapBox,0,1);
    boxLayout->addWidget(templateBox,1,1);

    boxLayout->addWidget(viewBox,0,2,2,1);

    boxLayout->addWidget(pushBox,2,0,1,2);

    // putting it together
    vbox->addWidget(editorStatusBox);
    vbox->addLayout(boxLayout);

    connect(workspace, &Workspace::sig_ws_dele_changed, this, &page_map_editor::slot_ws_dele_changed);

    connect(maker,  &TiledPatternMaker::sig_loadedTiling,   this,   &page_map_editor::slot_loadedTiling);
    connect(maker,  &TiledPatternMaker::sig_loadedXML,      this,   &page_map_editor::slot_loadedXML);
    connect(canvas, &Canvas::sig_unload,                    this,   &page_map_editor::slot_unload);

    connect(pbVerifyMap,            &QToolButton::clicked,  this,   &page_map_editor::slot_verify);
    connect(pbMakeExplicit,         &QToolButton::clicked,  this,   &page_map_editor::slot_convertToExplicit);
    connect(pbClearMap,             &QToolButton::clicked,  this,   &page_map_editor::slot_clearMap);
    connect(pbDivideEdges,          &QToolButton::clicked,  this,   &page_map_editor::slot_divideIntersectingEdges);
    connect(pbJoinEdges,            &QToolButton::clicked,  this,   &page_map_editor::slot_joinColinearEdges);
    connect(pbCleanNeighbours,      &QToolButton::clicked,  this,   &page_map_editor::slot_cleanNeighbours);
    connect(pbSortNeigbours,        &QToolButton::clicked,  this,   &page_map_editor::slot_sortAllNeighboursByAngle);
    connect(pbSortVertices,         &QToolButton::clicked,  this,   &page_map_editor::slot_sortVertices);
    connect(pbSortEdges,            &QToolButton::clicked,  this,   &page_map_editor::slot_sortEdges);
    connect(pbCleanseMap,           &QToolButton::clicked,  this,   &page_map_editor::slot_cleanseMap);
    connect(pbRemoveFloatingVerts,  &QToolButton::clicked,  this,   &page_map_editor::slot_removeUnconnectedVertices);
    connect(pbRemoveZombieEdges,    &QToolButton::clicked,  this,   &page_map_editor::slot_removeZombieEdges);
    connect(pbDumpMap,              &QToolButton::clicked,  this,   &page_map_editor::slot_dumpMap);
    connect(pbCropMap,              &QToolButton::clicked,  this,   &page_map_editor::slot_applyCrop);
    connect(pbCreateMap,            &QToolButton::clicked,  this,   &page_map_editor::slot_createMap);
    connect(pbLoadMap,              &QToolButton::clicked,  this,   &page_map_editor::slot_loadMap);
    connect(pbSaveMap,              &QToolButton::clicked,  this,   &page_map_editor::slot_saveMap);

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

    connect(&mapEdModeGroup,    SIGNAL(buttonClicked(int)), this,   SLOT(slot_mapEdMode_pressed(int)));
    connect(&targetGroup,       SIGNAL(buttonClicked(int)), this,   SLOT(slot_target_selected(int)));
    connect(&modeGroup,         SIGNAL(buttonClicked(int)), this,   SLOT(slot_setModes(int)));
    connect(pbReplaceInStyle,   &QToolButton::clicked,      this,   &page_map_editor::sig_stylesReplaceProto);
    connect(pbAddToStyle,       &QToolButton::clicked,      this,   &page_map_editor::sig_stylesAddProto);
    connect(pbRender,           &QToolButton::clicked,      this,   &page_map_editor::slot_render);

    connect(radiusSpin,     &DoubleSpinSet::valueChanged, this, &page_map_editor::slot_radiusChanged);
    connect(lineWidthSpin,  &DoubleSpinSet::valueChanged, this, &page_map_editor::slot_lineWidthChanged);
    connect(consWidthSpin,  &DoubleSpinSet::valueChanged, this, &page_map_editor::slot_consWidthChanged);

    slot_debugChk(config->mapedStatusBox);
    connect(editorStatusBox, &QGroupBox::toggled, this, &page_map_editor::slot_debugChk);

    hideMidPtsChk->setChecked(true);    // start hidden
}

void  page_map_editor::onEnter()
{
    mapEdModeGroup.blockSignals(true);
    mapEdModeGroup.button(config->mapEditorMode)->setChecked(true);
    mapEdModeGroup.blockSignals(false);

    reload();

    modeGroup.button(me->getMouseMode())->setChecked(true);
    targetGroup.button(config->pushTarget)->setChecked(true);

    me->buildEditorDB();
    emit sig_viewWS();
}

void  page_map_editor::reload()
{
    Q_ASSERT(me);

    targetGroup.button(config->pushTarget)->setChecked(true);

    switch (config->mapEditorMode)
    {
    case MAP_MODE_FIGURE:
    {
        // this is set by the figure editor
        eWsData wsdata = (config->figureViewer == FV_STYLE) ? WS_LOADED : WS_TILING;
        DesignElementPtr dep  = workspace->getSelectedDesignElement(wsdata);
        if (dep)
        {
            me->setDesignElement(dep);
        }
        else
        {
            me->unload();
        }
        break;
    }
    case MAP_MODE_PROTO:
    {
        eWsData wsdata = (config->designViewer == DV_LOADED_STYLE) ? WS_LOADED : WS_TILING;
        PrototypePtr pp = workspace->getPrototype(wsdata);
        if (pp)
        {
            me->setPrototype(pp);
        }
        else
        {
            me->unload();
        }
        break;
    }
    case MAP_MODE_STYLE:
    {
        eWsData wsdata        = (config->designViewer == DV_LOADED_STYLE) ? WS_LOADED : WS_TILING;
        StyledDesign & sd     = workspace->getStyledDesign(wsdata);
        const StyleSet & sset = sd.getStyleSet();
        if (sset.size())
        {
            StylePtr sp  = sset.first();
            me->setStyle(sp);
        }
        else
        {
            me->unload();
        }
        break;
    }

    case MAP_MODE_LOCAL:
        me->setLocal(localMap);
        break;
    }

    if (config->viewerType == VIEW_MAP_EDITOR)
    {
        emit sig_viewWS();
    }
}

void page_map_editor::slot_ws_dele_changed()
{
    if (config->mapEditorMode == MAP_MODE_FIGURE)
    {
        eWsData wsdata = (config->figureViewer == FV_STYLE) ? WS_LOADED : WS_TILING;
        DesignElementPtr dep = workspace->getSelectedDesignElement(wsdata);
        me->setDesignElement(dep);
    }
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
    MapPtr map = me->getMap();
    if (map)
        str = QString("Map: %1  %2").arg(Utils::addr(map.get())).arg(map->summary());
    else
        str = "Map: NO MAP";
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
    targetGroup.button(config->pushTarget)->setChecked(true);

    me->updateStatus();
}

void page_map_editor::slot_loadedXML(QString name)
{
    qDebug() << "page_map_editor: loaded -" << name;
    reload();
    me->initStashFrom(name);
}

void page_map_editor::slot_loadedTiling (QString name)
{
    Q_UNUSED(name)
    reload();
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

void page_map_editor::slot_target_selected(int id)
{
    config->pushTarget = ePushTarget(id);
}

void page_map_editor::slot_verify()
{
    bool oldConf = config->verifyMaps;
    config->verifyMaps = true;

    MapPtr map = me->getMap();
    map->verifyMap("page_figure_editor::verify");

    config->verifyMaps = oldConf;
}

void page_map_editor::slot_divideIntersectingEdges()
{
    MapPtr map = me->getMap();
    map->divideIntersectingEdges();
    emit sig_viewWS();
}

void page_map_editor::slot_joinColinearEdges()
{
    MapPtr map = me->getMap();
    map->joinColinearEdges();
    emit sig_viewWS();
}

void page_map_editor::slot_cleanNeighbours()
{
    MapPtr map = me->getMap();
    map->cleanNeighbours();
    emit sig_viewWS();
}

void page_map_editor::slot_removeUnconnectedVertices()
{
    MapPtr map = me->getMap();
    map->removeDanglingVertices();
    emit sig_viewWS();
}

void page_map_editor::slot_removeZombieEdges()
{
    MapPtr map = me->getMap();
    map->removeBadEdges();
    emit sig_viewWS();
}

void page_map_editor::slot_sortAllNeighboursByAngle()
{
    MapPtr map = me->getMap();
    map->sortAllNeighboursByAngle();
    emit sig_viewWS();
}

void page_map_editor::slot_sortVertices()
{
    MapPtr map = me->getMap();
    map->sortVertices();
    emit sig_viewWS();
}

void page_map_editor::slot_sortEdges()
{
    MapPtr map = me->getMap();
    map->sortEdges();
    emit sig_viewWS();
}

void page_map_editor::slot_reload()
{
    reload();
}

void page_map_editor::slot_unload()
{
    if (me)
    {
        me->unload();
    }
}

void page_map_editor::slot_setModes(int mode)
{
    eMapMouseMode mm = static_cast<eMapMouseMode>(mode);
    me->setMouseMode(mm);
}

void page_map_editor::slot_mapEdMode_pressed(int id)
{
    config->mapEditorMode = eMapEditorMode(id);
    reload();
}

void page_map_editor::slot_hideCons(bool hide)
{
    me->hideConstructionLines = hide;
    me->buildEditorDB();
    me->forceRedraw();
}

void page_map_editor::slot_hideMap(bool hide)
{
    me->hideMap = hide;
    me->buildEditorDB();
    me->forceRedraw();
}

void page_map_editor::slot_hidePoints(bool hide)
{
    me->hidePoints = hide;
    me->buildEditorDB();
    me->forceRedraw();
}

void page_map_editor::slot_hideMidPoints(bool hide)
{
    me->hideMidPoints = hide;
    me->buildEditorDB();
    me->forceRedraw();
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
    me->forceRedraw();
}

void page_map_editor::slot_undoConstructionLines()
{
    me->stash.getPrev();
    me->loadCurrentStash();
    me->forceRedraw();
}

void page_map_editor::slot_redoConstructionLines()
{
    me->stash.getNext();
    me->loadCurrentStash();
    me->forceRedraw();
}

void page_map_editor::slot_clearConstructionLines()
{
    // this does not affect the stash
    me->constructionLines.clear();
    me->constructionCircles.clear();
    me->buildEditorDB();
    me->forceRedraw();
}

void page_map_editor::slot_clearMap()
{
    MapPtr m = me->getMap();
    m->wipeout();
    me->buildEditorDB();
    me->forceRedraw();
}

void page_map_editor::slot_createMap()
{
    localMap = make_shared<Map>(QString("New local map"));
    if (config->mapEditorMode == MAP_MODE_LOCAL)
    {
        me->setLocal(localMap);
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

    XmlLoader loader;
    MapPtr map  = loader.loadXML(filename);
    if (!map)
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Warning);
        box.setText("Map NOT loaded");
        box.exec();
        return;
    }

    if (config->mapEditorMode == MAP_MODE_LOCAL)
    {
        localMap = map;
        me->setLocal(localMap);
        QMessageBox box;

        box.setIcon(QMessageBox::Information);
        box.setText("Local map loaded OK");
        box.exec();
    }
    else
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Warning);
        box.setText("Not implemented");
        box.exec();
    }
}

void page_map_editor::slot_saveMap()
{
    QString dir = config->rootMediaDir + "maps/";
    QString filename = QFileDialog::getSaveFileName(this, "Save Map File",dir,"Map (*.xml)");
    if (filename.isEmpty()) return;

    XmlWriter writer;
    bool rv = writer.writeXML(filename,me->getMap());
    if (rv)
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Information);
        box.setText("Map saved OK");
        box.exec();
    }
    else
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Warning);
        box.setText("Map NOT saved");
        box.exec();
    }
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
    m->cleanse();
    me->buildEditorDB();
    me->forceRedraw();
}

void page_map_editor::slot_radiusChanged(qreal r)
{
    me->newCircleRadius = r;
}

void page_map_editor::slot_lineWidthChanged(qreal r)
{
    me->mapLineWidth = r;
    me->forceRedraw();
}

void page_map_editor::slot_consWidthChanged(qreal r)
{
    me->constructionLineWidth = r;
    me->forceRedraw();
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

void page_map_editor::slot_render()
{
    if (config->pushTarget == TARGET_LOADED_STYLES)
        emit sig_render(RENDER_LOADED);
    else
        emit sig_render(RENDER_WS);
}
