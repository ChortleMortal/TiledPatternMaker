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

#include "panels/page_views.h"
#include "panels/panel.h"
#include "base/configuration.h"
#include "base/fileservices.h"
#include "base/view.h"
#include "base/workspace.h"
#include "base/mosaic_writer.h"
#include "base/tiledpatternmaker.h"
#include "designs/patterns.h"
#include "designs/design.h"
#include "viewers/tiling_view.h"
#include "viewers/workspace_viewer.h"

page_views::page_views(ControlPanel * cpanel) : panel_page(cpanel,"Views")
{
    // gui setup has to be here for this page since it receives signals
    QGroupBox * wsControl = createViewControl();
    QGroupBox * wsViewers = createWorkspaceViewers();

    QVBoxLayout * vbox2 = new QVBoxLayout();        // shouldbe 597x668
    vbox2->addWidget(wsControl);
    vbox2->addSpacing(7);
    vbox2->addWidget(wsViewers);
    vbox2->addSpacing(7);

    vbox->addLayout(vbox2);
    vbox->addStretch();

    refreshPage();
}

QGroupBox * page_views::createViewControl()
{
    QGroupBox * workspaceMakersBox = new QGroupBox("View Control");

    QCheckBox   * chkSplit      = new QCheckBox("Split Screen");
    QPushButton * btnPrimary    = new QPushButton("Primary Screen");
    QCheckBox   * multiSelect   = new QCheckBox("Multi-select");
    cbLockView                  = new QCheckBox("Lock View");

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(chkSplit);
    hbox->addWidget(btnPrimary);
    hbox->addStretch();
    hbox->addWidget(cbLockView);
    hbox->addWidget(multiSelect);
    hbox->addStretch();
    hbox->addStretch();

    workspaceMakersBox->setLayout(hbox);

    chkSplit->setChecked(config->splitScreen);

    // workspace buttons
    connect(btnPrimary,     &QPushButton::clicked, tpm,   &TiledPatternMaker::slot_bringToPrimaryScreen);
    connect(chkSplit,       &QPushButton::clicked, tpm,   &TiledPatternMaker::slot_splitScreen);
    connect(cbLockView,     &QCheckBox::clicked,   this,  &page_views::slot_lockViewClicked);
    connect(multiSelect,    &QCheckBox::clicked,   this,  &page_views::slot_multiSelect);
    connect(multiSelect,    &QCheckBox::clicked,   panel, &ControlPanel::slot_multiSelect);

    return workspaceMakersBox;
}

QGroupBox *  page_views::createWorkspaceViewers()
{
    QGroupBox * workspaceViewersBox  = new QGroupBox("View Select");

    cbRawDesignView      = new QCheckBox("Raw Design");
    cbMosaicView         = new QCheckBox("Mosaic");
    cbPrototypeView      = new QCheckBox("Prototype");
    cbDELView            = new QCheckBox("Design Elements");
    cbFigMapView         = new QCheckBox("Map Editor");
    cbProtoMaker         = new QCheckBox("Prototype Maker");
    cbTilingView         = new QCheckBox("Tiling");
    cbTilingMakerView    = new QCheckBox("Tiling Maker");
    cbFaceSetView        = new QCheckBox("FaceSet");

    lab_activeDesigns       = new QLabel("Raw Designs");
    lab_activeDesigns->setMinimumWidth(501);
    lab_LoadedStyle         = new QLabel("lab_LoadedStyle");
    lab_LoadedStylesTiling  = new QLabel("lab_LoadedStylesTiling");
    lab_LoadedStyleStyles   = new QLabel("lab_LoadedStyleStyles");
    lab_LoadedStylesProto   = new QLabel("lab_LoadedStylesProto");
    lab_LoadedDEL           = new QLabel("lab_LoadedDEL");

    QGridLayout * grid2 = new QGridLayout();
    grid2->setColumnStretch(1,3);
    int row = 0;

    grid2->addWidget(cbRawDesignView,row,0);
    grid2->addWidget(lab_activeDesigns,row,1);
    row++;

    grid2->addWidget(cbMosaicView,row,0);
    grid2->addWidget(lab_LoadedStyle,row,1);
    row++;

    grid2->addWidget(cbPrototypeView,row,0);
    grid2->addWidget(lab_LoadedStylesProto,row,1);
    row++;

    grid2->addWidget(cbDELView,row,0);
    grid2->addWidget(lab_LoadedDEL,row,1);
    row++;

    grid2->addWidget(cbFigMapView,row,0);
    row++;

    grid2->addWidget(cbProtoMaker,row,0);
    row++;

    grid2->addWidget(cbTilingView,row,0);
    grid2->addWidget(lab_LoadedStylesTiling,row,1);
    row++;

    grid2->addWidget(cbTilingMakerView,row,0);
    row++;

    grid2->addWidget(cbFaceSetView,row,0);

    workspaceViewersBox->setLayout(grid2);

    // viewer group
    viewerGroup.addButton(cbRawDesignView,VIEW_DESIGN);
    viewerGroup.addButton(cbMosaicView,VIEW_MOSAIC);
    viewerGroup.addButton(cbPrototypeView,VIEW_PROTOTYPE);
    viewerGroup.addButton(cbTilingView,VIEW_TILING);
    viewerGroup.addButton(cbProtoMaker,VIEW_FROTOTYPE_MAKER);
    viewerGroup.addButton(cbDELView,VIEW_DESIGN_ELEMENT);
    viewerGroup.addButton(cbTilingMakerView,VIEW_TILING_MAKER);
    viewerGroup.addButton(cbFigMapView,VIEW_MAP_EDITOR);
    viewerGroup.addButton(cbFaceSetView,VIEW_FACE_SET);
    viewerGroup.button(config->viewerType)->setChecked(true);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(&viewerGroup,   SIGNAL(buttonToggled(int, bool)),  panel, SLOT(slot_Viewer_pressed(int,bool)));
#else
    connect(&viewerGroup, &QButtonGroup::idToggled, panel, &ControlPanel::slot_Viewer_pressed);
#endif

    connect(panel, &ControlPanel::sig_view_synch,  this, &page_views::slot_view_synch);

    return workspaceViewersBox;
}

void page_views::refreshPage()
{
    updateWsStatus();
}

void page_views::onEnter()
{
    blockSignals(true);
    cbLockView->setChecked(config->lockView);
    viewerGroup.button(config->viewerType)->setChecked(true);
    blockSignals(false);
}

void page_views::slot_loadedXML(QString name)
{
    Q_UNUSED(name)
    onEnter();
}

void page_views::updateWsStatus()
{
    // Designs
    QString astring;
    QVector<DesignPtr> & designs= workspace->getDesigns();
    for (auto design : designs)
    {
        astring += QString("%1 - %2 (%3)").arg(design->getDesignName()).arg(design->getTitle()).arg(addr(design.get()));
    }
    lab_activeDesigns->setText(astring);

    updateLoadedStatus();
}

void page_views::updateLoadedStatus()
{
    QString astring;

    // Mosaic
    MosaicPtr mosaic = workspace->getMosaic();
    astring = mosaic->getName();
    if (mosaic->hasContent())
    {
        astring += QString(" numStyles: %1").arg(mosaic->numStyles());
    }
    else
    {
        astring += " - NO CONTENT";
    }
    lab_LoadedStyle->setText(astring);

    // Tiling
    astring.clear();
    QVector<TilingPtr> tilings = workspace->getTilings();
    for (auto tp: tilings)
    {
        astring += QString("%1 (%2)  ").arg(tp->getName()).arg(addr(tp.get()));
    }

    if (mosaic && mosaic->hasContent())
    {
        QVector<TilingPtr> tilings2 = mosaic->getTilings();
        if (tilings2 != tilings)
        {
            astring += "MOSAIC-TILING MISMATCH";
        }
    }
    lab_LoadedStylesTiling->setText(astring);

    // Styles
    astring.clear();
    if (mosaic)
    {
        const StyleSet & sset = mosaic->getStyleSet();
        for (auto style : sset)
        {
            astring += QString("[%1 (%2)] ").arg(style->getStyleDesc()).arg(addr(style.get()));
        }
    }
    lab_LoadedStyleStyles->setText(astring);

    // Prototype
    astring.clear();
    PrototypePtr pp = workspace->getSelectedPrototype();
    PrototypePtr pp2;
    StylePtr sp;
    if (mosaic)
    {
        sp = mosaic->getFirstStyle();
        if (sp)
        {
            pp2  = sp->getPrototype();
        }
        if (pp != pp2 && mosaic->hasContent())
        {
            astring += "PROTOTYPE-MISMATCH ";
        }
    }
    MapPtr map;
    if (pp)
    {
        map = pp->getExistingProtoMap();
        astring += QString("%2 %1 map=%3").arg(pp->getInfo()).arg(addr(pp.get())).arg(addr(map.get()));
    }
    lab_LoadedStylesProto->setText(astring);

    // Design Element and Figure
    DesignElementPtr del = workspace->getSelectedDesignElement();
    if (del)
    {
        QString astring = QString("(%1)").arg(addr(del.get()));

        FeaturePtr fp = del->getFeature();
        if (fp)
        {
            astring += " Feature: " ;
            astring += fp->info();
        }

        FigurePtr fig = del->getFigure();
        if (fig)
        {
            astring += " Figure: (";
            astring += addr(fig.get());
            astring += ") ";
            astring += fig->getFigureDesc();
        }

        lab_LoadedDEL->setText(astring);
    }
    else
    {
        lab_LoadedDEL->clear();
    }
}

void  page_views::slot_lockViewClicked(bool enb)
{
    config->lockView = enb;
}

void page_views::slot_multiSelect(bool enb)
{
    viewerGroup.setExclusive(!enb);
    if (!enb)
    {
        viewerGroup.blockSignals(true);
        for (int i=0; i <= VIEW_MAX; i++ )
        {
            if (i == config->viewerType)
            {
                continue;
            }
            viewerGroup.button(i)->setChecked(false);
        }
        viewerGroup.blockSignals(false);
        wsViewer->disableAll();
        wsViewer->viewEnable(config->viewerType,true);
        emit sig_viewWS();
    }
}

void  page_views::slot_selectViewer(int id)
{
    if (config->lockView)
    {
        return;
    }

    viewerGroup.button(id)->setChecked(true);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    emit viewerGroup.buttonClicked(id);
#else
    emit viewerGroup.idClicked(id);
#endif
}

void page_views::slot_view_synch(int id, int enb)
{
    viewerGroup.blockSignals(true);
    viewerGroup.button(id)->setChecked(enb);
    viewerGroup.blockSignals(false);
}
