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

#include "panels/page_control.h"
#include "base/configuration.h"
#include "base/canvas.h"
#include "base/fileservices.h"
#include "base/view.h"
#include "base/misc.h"
#include "base/workspace.h"
#include "base/xmlwriter.h"
#include "base/tilingmanager.h"
#include "designs/patterns.h"
#include "designs/design.h"
#include "base/tiledpatternmaker.h"
#include "viewers/PrototypeView.h"
#include "viewers/TilingView.h"
#include "viewers/workspaceviewer.h"

page_control::page_control(ControlPanel * cpanel) : panel_page(cpanel,"View")
{
    // gui setup has to be here for this page since it receives signals
    createWorkspaceMakers();
    createWorkspaceViewers();
    createWorkspaceStatus();

    QVBoxLayout * vbox2 = new QVBoxLayout();        // shouldbe 597x668
    vbox2->addWidget(workspaceMakersBox);
    vbox2->addSpacing(7);
    vbox2->addWidget(workspaceViewersBox);
    vbox2->addSpacing(7);
    vbox2->addWidget(workspaceStatusBox);

    vbox->addLayout(vbox2);
    vbox->addStretch();

    makeConnections();

    refreshPage();
}

void  page_control::createWorkspaceMakers()
{
    QPushButton * pbViewWorkspace   = new QPushButton("View Workspace");
    QPushButton * btnSplit          = new QPushButton("Split Screen");
    QPushButton * btnPrimary        = new QPushButton("Primary Screen");


    QGridLayout * grid1 = new QGridLayout();
    grid1->addWidget(pbViewWorkspace,   0,0);
    grid1->addWidget(btnSplit,          0,1);
    grid1->addWidget(btnPrimary,        0,2);

    workspaceMakersBox = new QGroupBox("View Control");
    workspaceMakersBox->setLayout(grid1);

    // workspace buttons

    connect(pbViewWorkspace,&QPushButton::clicked,      viewer,     &WorkspaceViewer::slot_viewWorkspace);
    connect(btnPrimary,     &QPushButton::clicked,      maker,      &TiledPatternMaker::slot_bringToPrimaryScreen);
    connect(btnSplit,       &QPushButton::clicked,      maker,      &TiledPatternMaker::slot_splitScreen);

}

void  page_control::createWorkspaceViewers()
{
    workspaceViewersBox  = new QGroupBox("Workspace Viewers");

    setStyle             = new QPushButton("Set Style");
    setWS                = new QPushButton("Set WS");
    setStyle->setMaximumWidth(91);
    setWS->setMaximumWidth(91);

    cbDesignView         = new QCheckBox("Design");
    cbProtoView          = new QCheckBox("Prototype - Figs");
    cbProtoFeatureView   = new QCheckBox("Prototype - Figs + Feats");
    cbDELView            = new QCheckBox("Design Element");
    cbFigMapView         = new QCheckBox("Map Editor");
    cbFigureView         = new QCheckBox("Figure Maker");
    cbTilingView         = new QCheckBox("Tiling");
    cbTilingMakerView    = new QCheckBox("Tiling Maker");
    cbFaceSetView        = new QCheckBox("FaceSet");

    radioLoadedStyleView = new QRadioButton("Style");
    radioWsStyleView     = new QRadioButton("Workspace");
    radioShapeView       = new QRadioButton("Shapes");

    radioStyleProtoViewer = new QRadioButton("Style");
    radioProtoMakerView   = new QRadioButton("Workspace");

    radioStyleProtoFeatureViewer = new QRadioButton("Style");
    radioProtoFeatureMakerView   = new QRadioButton("Workspace");

    radioDelStyleView = new QRadioButton("Style");
    radioDelWSView    = new QRadioButton("Workspace");

    radioFigureWS     = new QRadioButton("Workspace");
    radioFigureStyle  = new QRadioButton("Style");

    radioLoadedStyleTileView = new QRadioButton("Style");
    radioWSTileView          = new QRadioButton("Workspace");

    radioTileDesSourceStyle = new QRadioButton("Style");
    radioTileDesSourceWS    = new QRadioButton("Workspace");

    mapEdStyle  = new QRadioButton("Style");
    mapEdWS     = new QRadioButton("Workspace");

    QGridLayout * grid2 = new QGridLayout();
    int row = 0;

    grid2->addWidget(setStyle,row,1);
    grid2->addWidget(setWS,row,2);
    row++;

    grid2->addWidget(cbDesignView,row,0);
    grid2->addWidget(radioLoadedStyleView,row,1);
    grid2->addWidget(radioWsStyleView,row,2);
    grid2->addWidget(radioShapeView,row,3);
    row++;

    grid2->addWidget(cbProtoView,row,0);
    grid2->addWidget(radioStyleProtoViewer,row,1);
    grid2->addWidget(radioProtoMakerView,row,2);
    row++;

    grid2->addWidget(cbProtoFeatureView,row,0);
    grid2->addWidget(radioStyleProtoFeatureViewer,row,1);
    grid2->addWidget(radioProtoFeatureMakerView,row,2);
    row++;

    grid2->addWidget(cbDELView,row,0);
    grid2->addWidget(radioDelStyleView,row,1);
    grid2->addWidget(radioDelWSView,row,2);
    row++;

    grid2->addWidget(cbTilingView,row,0);
    grid2->addWidget(radioLoadedStyleTileView,row,1);
    grid2->addWidget(radioWSTileView,row,2);
    row++;

    grid2->addWidget(cbFigMapView,row,0);
    grid2->addWidget(mapEdStyle,row,1);
    grid2->addWidget(mapEdWS,row,2);
    row++;

    grid2->addWidget(cbFigureView,row,0);
    grid2->addWidget(radioFigureStyle,row,1);
    grid2->addWidget(radioFigureWS,row,2);
    row++;

    grid2->addWidget(cbTilingMakerView,row,0);
    grid2->addWidget(radioTileDesSourceStyle,row,1);
    grid2->addWidget(radioTileDesSourceWS,row,2);
    row++;

    grid2->addWidget(cbFaceSetView,row,0);

    workspaceViewersBox->setLayout(grid2);

    // viewer group
    viewerGroup.addButton(cbDesignView,VIEW_DESIGN);
    viewerGroup.addButton(cbProtoView,VIEW_PROTO);
    viewerGroup.addButton(cbProtoFeatureView,VIEW_PROTO_FEATURE);
    viewerGroup.addButton(cbTilingView,VIEW_TILING);
    viewerGroup.addButton(cbFigureView,VIEW_FIGURE_MAKER);
    viewerGroup.addButton(cbDELView,VIEW_DEL);
    viewerGroup.addButton(cbTilingMakerView,VIEW_TILIING_MAKER);
    viewerGroup.addButton(cbFigMapView,VIEW_MAP_EDITOR);
    viewerGroup.addButton(cbFaceSetView,VIEW_FACE_SET);
    viewerGroup.button(config->viewerType)->setChecked(true);

    // designGroup
    designGroup.addButton(radioLoadedStyleView,DV_LOADED_STYLE);
    designGroup.addButton(radioWsStyleView,DV_WS_STYLE);
    designGroup.addButton(radioShapeView,DV_SHAPES);
    designGroup.button(config->designViewer)->setChecked(true);

    // proto group
    protoGroup.addButton(radioStyleProtoViewer,PV_STYLE);
    protoGroup.addButton(radioProtoMakerView,PV_WS);
    protoGroup.button(config->protoViewer)->setChecked(true);

    // protofeature group
    protoFeatureGroup.addButton(radioStyleProtoFeatureViewer,PVF_STYLE);
    protoFeatureGroup.addButton(radioProtoFeatureMakerView,PVF_WS);
    protoFeatureGroup.button(config->protoFeatureViewer)->setChecked(true);

    // tiling group
    tilingGroup.addButton(radioLoadedStyleTileView,TV_STYLE);
    tilingGroup.addButton(radioWSTileView,TV_WORKSPACE);
    tilingGroup.button(config->tilingViewer)->setChecked(true);

    // figure group
    figureGroup.addButton(radioFigureWS,FV_WS);
    figureGroup.addButton(radioFigureStyle,FV_STYLE);
    figureGroup.button(config->figureViewer)->setChecked(true);

    // tiling designer group
    tilingMakerGroup.addButton(radioTileDesSourceStyle,TD_STYLE);
    tilingMakerGroup.addButton(radioTileDesSourceWS,TD_WORKSPACE);
    tilingMakerGroup.button(config->tilingMakerViewer)->setChecked(true);

    // design element group
    delGroup.addButton(radioDelStyleView,DEL_STYLES);
    delGroup.addButton(radioDelWSView,DEL_WS);
    delGroup.button(config->delViewer)->setChecked(true);

    // map editor group
    mapEdGroup.addButton(mapEdStyle,MED_STYLE);
    mapEdGroup.addButton(mapEdWS,MED_WS);
    mapEdGroup.button(config->mapEditorView)->setChecked(true);

    connect(setStyle, &QPushButton::clicked, this, &page_control::slot_setSyle);
    connect(setWS,    &QPushButton::clicked, this, &page_control::slot_setWS);
}

void  page_control::createWorkspaceStatus()
{
    lab_activeDesigns   = new QLabel("activeDesigns");
    lab_LoadedStylename = new QLabel("loadedStyleName");
    lab_LoadedStyles    = new QLabel("loadedStyles");
    lab_LoadedStyles->setFixedWidth(401);
    lab_LoadedStylesTiling = new QLabel("loadedStylesTiling");

    lab_WsStylename     = new QLabel("wsStyleName");
    lab_WsStyles        = new QLabel("loadedStyles");
    lab_WsStyles->setFixedWidth(401);

    lab_wsPrototype     = new QLabel("wsPrototype");
    lab_wsDEL           = new QLabel("wsDEL");
    lab_wsFigure        = new QLabel("wsFigure");
    lab_tiling          = new QLabel("wsTiling");

    workspaceStatusBox = new QGroupBox("Workspace Status");
    workspaceStatusBox->setMinimumWidth(531);
    workspaceStatusBox->setCheckable(true);
    workspaceStatusBox->setChecked(config->wsStatusBox);

    slot_wsStatusBox(config->wsStatusBox);

    connect(workspaceStatusBox, &QGroupBox::toggled, this, &page_control::slot_wsStatusBox);
}

void page_control::slot_wsStatusBox(bool on)
{
    config->wsStatusBox = on;
    QLayout * l = workspaceStatusBox->layout();
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
        workspaceStatusBox->setLayout(dummyStatusBox);
    }
    else
    {
        int row = 0;
        statusBox = new QGridLayout;
        statusBox->addWidget(new QLabel("activeDesigns"),row,0);
        statusBox->addWidget(lab_activeDesigns,row++,1);
        statusBox->addWidget(new QLabel("loadedStyleName"),row,0);
        statusBox->addWidget(lab_LoadedStylename,row++,1);
        statusBox->addWidget(new QLabel("loadedStyles"),row,0);
        statusBox->addWidget(lab_LoadedStyles,row++,1);
        statusBox->addWidget(new QLabel("loadedStylesTiling"),row,0);
        statusBox->addWidget(lab_LoadedStylesTiling,row++,1);

        statusBox->addWidget(new QLabel("wsStyleName"),row,0);
        statusBox->addWidget(lab_WsStylename,row++,1);
        statusBox->addWidget(new QLabel("wsStyles"),row,0);
        statusBox->addWidget(lab_WsStyles,row++,1);

        statusBox->addWidget(new QLabel("wsPrototype"),row,0);
        statusBox->addWidget(lab_wsPrototype,row++,1);
        statusBox->addWidget(new QLabel("wsPlacedDesignElements"),row,0);
        statusBox->addWidget(lab_wsDEL,row++,1);
        statusBox->addWidget(new QLabel("wsFigure"),row,0);
        statusBox->addWidget(lab_wsFigure,row++,1);
        statusBox->addWidget(new QLabel("wsTiling"),row,0);
        statusBox->addWidget(lab_tiling,row++,1);

        workspaceStatusBox->setLayout(statusBox);
    }
}



void page_control::makeConnections()
{
    setObjectName("page_control");

    connect(this,           &page_control::sig_loadTiling,   maker,   &TiledPatternMaker::slot_loadTiling);

    // workspace checkboxes
    connect(&viewerGroup,   SIGNAL(buttonClicked(int)),      this,     SLOT(slot_Viewer_pressed(int)));
    connect(&designGroup,   SIGNAL(buttonClicked(int)),      this,     SLOT(slot_designViewer_pressed(int)));
    connect(&protoGroup,    SIGNAL(buttonClicked(int)),      this,     SLOT(slot_protoViewer_pressed(int)));
    connect(&protoFeatureGroup,  SIGNAL(buttonClicked(int)), this,     SLOT(slot_protoFeatureViewer_pressed(int)));
    connect(&tilingGroup,   SIGNAL(buttonClicked(int)),      this,     SLOT(slot_tilingViewer_pressed(int)));
    connect(&figureGroup,   SIGNAL(buttonClicked(int)),      this,     SLOT(slot_figureViewer_pressed(int)));
    connect(&tilingMakerGroup,SIGNAL(buttonClicked(int)), this,     SLOT(slot_tilingMakerViewer_pressed(int)));
    connect(&delGroup,      SIGNAL(buttonClicked(int)),      this,     SLOT(slot_delViewer_pressed(int)));
    connect(&mapEdGroup,    SIGNAL(buttonClicked(int)),      this,     SLOT(slot_mapEdView_pressed(int)));
}

void page_control::refreshPage()
{
    updateWsStatus();
}

void page_control::onEnter()
{
    blockSignals(true);
    viewerGroup.button(config->viewerType)->setChecked(true);
    designGroup.button(config->designViewer)->setChecked(true);
    protoGroup.button(config->protoViewer)->setChecked(true);
    protoFeatureGroup.button(config->protoFeatureViewer)->setChecked(true);
    tilingGroup.button(config->tilingViewer)->setChecked(true);
    tilingMakerGroup.button(config->tilingMakerViewer)->setChecked(true);
    figureGroup.button(config->figureViewer)->setChecked(true);
    delGroup.button(config->delViewer)->setChecked(true);
    mapEdGroup.button(config->mapEditorView)->setChecked(true);
    blockSignals(false);
}

void page_control::slot_loadedXML(QString name)
{
    Q_UNUSED(name)
    onEnter();
}

void page_control::updateWsStatus()
{
    if (!config->wsStatusBox)
    {
        return;
    }

    // Designs
    QString astring;
    QVector<DesignPtr> & des = workspace->getDesigns();
    for (auto it= des.begin(); it != des.end(); it++)
    {
        DesignPtr dp = *it;
        astring += QString("%1 - %2 (%3)    ").arg(dp->getDesignName()).arg(dp->getTitle()).arg(addr(dp.get()));
    }
    lab_activeDesigns->setText(astring);

    //  Styled Designs
    StyledDesign & sd = workspace->getLoadedStyles();
    const StyleSet & sset = sd.getStyleSet();
    lab_LoadedStylename->setText(sd.getName());

    astring.clear();
    for (auto it= sset.begin(); it != sset.end(); it++)
    {
        StylePtr sp     = *it;
        PrototypePtr pp = sp->getPrototype();
        MapPtr map      = pp->getExistingProtoMap();
        astring+= QString("[%1 (%2)]  proto=%3  protoMap=%4").arg(sp->getStyleDesc()).arg(addr(sp.get())).arg(addr(pp.get())).arg(addr(map.get()));
    }
    lab_LoadedStyles->setText(astring);

    TilingPtr tp = sd.getTiling();
    if (tp)
        lab_LoadedStylesTiling->setText(QString("%1 (%2)").arg(tp->getName()).arg(addr(tp.get())));
    else
        lab_LoadedStylesTiling->clear();

    // Workspace Styled Designs
    StyledDesign & wsd = workspace->getWsStyles();
    const StyleSet & wsset = wsd.getStyleSet();
    lab_WsStylename->setText(wsd.getName());

    astring.clear();
    for (auto it= wsset.begin(); it != wsset.end(); it++)
    {
        StylePtr sp = *it;
        astring+= QString("[%1 (%2)]  ").arg(sp->getStyleDesc()).arg(addr(sp.get()));
    }
    lab_WsStyles->setText( astring);

    // Workspace proto
    PrototypePtr pp = (workspace->getWSPrototype());
    if (pp)
    {
        QString str = addr(pp.get()) + " DELS:";
        QVector<DesignElementPtr> & dels = pp->getDesignElements();
        for (auto it = dels.begin(); it != dels.end(); it++)
        {
            auto del = *it;
            str += " ";
            str += addr(del.get());
        }
        lab_wsPrototype->setText(str);
    }
    else
        lab_wsPrototype->clear();

    DesignElementPtr del = workspace->getWSDesignElement();
    if (del)
    {
        lab_wsDEL->setText(addr(del.get()));

        FigurePtr fig = del->getFigure();
        if (fig)
        {
            MapPtr map; // = fig->getFigureMap();
            lab_wsFigure->setText(QString("%1 : %2 map = %3").arg(fig->getFigureDesc()).arg(addr(fig.get())).arg(addr(map.get())));
        }
        else
            lab_wsFigure->clear();
    }
    else
    {
        lab_wsDEL->clear();
        lab_wsFigure->clear();
    }

    tp = workspace->getTiling();
    if (tp)
        lab_tiling->setText(QString("%1 (%2)").arg(tp->getName()).arg(addr(tp.get())));
    else
        lab_tiling->clear();

    // viewer section
    if (sset.size())
        astring = QString("Style %1").arg(addr(sset.first()->getPrototype().get()));
    else
        astring = "Style";
    radioStyleProtoViewer->setText(astring);
    radioStyleProtoFeatureViewer->setText(astring);
    astring = QString("Workspace %1").arg(addr(workspace->getWSPrototype().get()));
    radioProtoMakerView->setText(astring);
    radioProtoFeatureMakerView->setText(astring);

    if (sset.size())
    {
        PrototypePtr pp = sset.first()->getPrototype();
        //DesignElementPtr delp =

    }
}



void  page_control::slot_Viewer_pressed(int id)
{
    config->viewerType = static_cast<eViewType>(id);
    emit sig_viewWS();
}

void page_control::slot_designViewer_pressed(int id)
{
    config->designViewer = static_cast<eDesignViewer>(id);
    if (config->viewerType == VIEW_DESIGN)
    {
        emit sig_viewWS();
    }
}

void page_control::slot_tilingViewer_pressed(int id)
{
    config->tilingViewer = static_cast<eTilingViewer>(id);
    if (config->viewerType == VIEW_TILING)
    {
        emit sig_viewWS();
    }
}

void page_control::slot_tilingMakerViewer_pressed(int id)
{
    config->tilingMakerViewer = static_cast<eTilingMakerView>(id);
    if (config->viewerType == VIEW_TILIING_MAKER)
    {
        emit sig_viewWS();
    }
}

void page_control::slot_protoViewer_pressed(int id)
{
    config->protoViewer = static_cast<eProtoViewer>(id);
    if (config->viewerType == VIEW_PROTO)
    {
        emit sig_viewWS();
    }
}

void page_control::slot_protoFeatureViewer_pressed(int id)
{
    config->protoFeatureViewer = static_cast<eProtoFeatureViewer>(id);
    if (config->viewerType == VIEW_PROTO_FEATURE)
    {
        emit sig_viewWS();
    }
}

void  page_control::slot_figureViewer_pressed(int id)
{
    config->figureViewer = static_cast<eFigureViewer>(id);
    if (config->viewerType == VIEW_FIGURE_MAKER)
    {
        emit sig_viewWS();
    }
}

void  page_control::slot_delViewer_pressed(int id)
{
    config->delViewer = static_cast<eDELViewer>(id);
    if (config->viewerType == VIEW_DEL)
    {
        emit sig_viewWS();
    }
}

void  page_control::slot_mapEdView_pressed(int id)
{
    config->mapEditorView = static_cast<eMapEditorView>(id);
    emit sig_mapEdSelection();
    if (config->viewerType == VIEW_MAP_EDITOR)
    {
        emit sig_viewWS();
    }
}



void  page_control::slot_selectViewer(int id, int id2)
{
    selectView(id,id2);

    viewerGroup.button(id)->setChecked(true);
    emit viewerGroup.buttonClicked(id);
}

void  page_control::selectView(int id, int id2)
{
    eViewType vt = static_cast<eViewType>(id);
    switch (vt)
    {
    case VIEW_DESIGN:
        designGroup.button(id2)->setChecked(true);
        emit designGroup.buttonClicked(id2);
        break;
    case VIEW_PROTO:
        protoGroup.button(id2)->setChecked(true);
        emit protoGroup.buttonClicked(id2);
        break;
    case VIEW_PROTO_FEATURE:
        protoFeatureGroup.button(id2)->setChecked(true);
        emit protoFeatureGroup.buttonClicked(id2);
        break;
    case VIEW_DEL:
        delGroup.button(id2)->setChecked(true);
        emit delGroup.buttonClicked(id2);
        break;
    case VIEW_FIGURE_MAKER:
        figureGroup.button(id2)->setChecked(true);
        emit figureGroup.buttonClicked(id2);
        break;
    case VIEW_TILING:
        tilingGroup.button(id2)->setChecked(true);
        emit tilingGroup.buttonClicked(id2);
        break;
    case VIEW_TILIING_MAKER:
        tilingMakerGroup.button(id2)->setChecked(true);
        emit tilingMakerGroup.buttonClicked(id2);
        break;
    case VIEW_MAP_EDITOR:
        mapEdGroup.button(id2)->setChecked(true);
        emit mapEdGroup.buttonClicked(id2);
        break;
    case VIEW_FACE_SET:
        break;
    }
}

void page_control::slot_setSyle()
{
    selectView(VIEW_DESIGN,DV_LOADED_STYLE);
    selectView(VIEW_PROTO,PV_STYLE);
    selectView(VIEW_PROTO_FEATURE,PVF_STYLE);
    selectView(VIEW_DEL,DEL_STYLES);
    selectView(VIEW_FIGURE_MAKER,FV_STYLE);
    selectView(VIEW_TILING,TV_STYLE);
    selectView(VIEW_TILIING_MAKER,TD_STYLE);
    selectView(VIEW_MAP_EDITOR,MED_STYLE);
}

void page_control::slot_setWS()
{
    selectView(VIEW_DESIGN,DV_WS_STYLE);
    selectView(VIEW_PROTO,PV_WS);
    selectView(VIEW_PROTO_FEATURE,PVF_WS);
    selectView(VIEW_DEL,DEL_WS);
    selectView(VIEW_FIGURE_MAKER,FV_WS);
    selectView(VIEW_TILING,TV_WORKSPACE);
    selectView(VIEW_TILIING_MAKER,TD_WORKSPACE);
    selectView(VIEW_MAP_EDITOR,MED_WS);
}
